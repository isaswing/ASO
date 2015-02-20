//---------------------------------------------------------------------------
#include "ASPSolver.h"
//---------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
//---------------------------------------------------------------------------
//parse the generator files for test cases
bool ASPSolver::ParseTestGen(char *fileName)
{
    if (fileName == NULL)
    {
        return false;
    }
    FileIO file;
    if (file.Open(fileName, "rb") == false)
    {
        return false;
    }
    TSmallStr buffer;
    buffer.SetLen(file.size());
    if (file.Read(buffer(), 1, file.size()) == false)
    {
        return false;
    }
    file.Close();

    char *ptr = buffer();
    ptr = strstr(buffer(), "p cnf ");
    if (ptr == NULL)
    {
        return false;
    }
    ptr += 6;
    numAtom = 0;
    while (*ptr != ' ')
    {
        numAtom = numAtom*10 + *ptr - 48;
        ptr++;
    }
    TSmallStr result;
    for (int i=0; i<numAtom; i++)
    {
        result.Append("{atom(%d)}.\n", i+1);
    }
    while (*ptr != '\n')
    {
        ptr++;
    }
    while (*ptr == '\n' || *ptr == '\r')
    {
        ptr++;
    }
    char *start = ptr;
    //for each clause
    while (start && *start != '\0')
    {
        result.AddFast(":- ");
        while (start)
        {
            if (*start != '-')
            {
                result.AddFast("not ");
            }
            else
            {
                start++;
            }
            int index = 0;
            while (*start != ' ')
            {
                index = index*10 + *start - 48;
                start++;
            }
            result.Append("atom(%d)", index);
            start++;
            if (*start == '0')
            {
                start = strstr(start, "\n");
                while (*start == '\n' || *start == '\r')
                {
                    start++;
                }
                break;
            }
            result.AddFast(", ");
        }
        result.AddFast(".\n");
    }

    if (file.Open(generator) == 0)
    {
		return false;
    }
    if (file.Write(result(), 1, result.Len) == false)
    {
        return false;
    }
    file.Close();
    if (file.Open(testerTXT, "ab") == 0)
    {
		return false;
    }
    if (file.Write(result(), 1, result.Len) == false)
    {
        return false;
    }
    file.Close();
    if (file.Open(genAnoTXT, "ab") == 0)
    {
		return false;
    }
    if (file.Write(result(), 1, result.Len) == false)
    {
        return false;
    }
    file.Close();
    return true;
}
//---------------------------------------------------------------------------
bool ASPSolver::GetOneAS()
{
    if (genCommand(generator, genResult) == false)
    {
        printf("generate command fail!\n");
        return false;
    }
    if (system(command()) < 0)
    {
        printf("solver fail!\n");
        return false;
    }
    return true;
}
//---------------------------------------------------------------------------
/*
using a start point to generate a tester (pref.lp)
prevAS: the file name of previous result
rule: the preference constraints (rank or unrank) 
*/
//---------------------------------------------------------------------------
bool ASPSolver::GenTester(char *prevAS, TSmallStr *rule)
{
    FileIO prevASFile;
    if (prevASFile.Open(prevAS, "rb") == false)
    {
        printf("open answer set fail!\n");
        return false;
    }
    TSmallStr buffer;
    buffer.SetLen(prevASFile.size());
    if (prevASFile.Read(buffer(), 1, prevASFile.size()) == false)
    {
        return false;
    }
    prevASFile.Close();
    if (GetDegree(buffer()) == false)
    {
        printf("Get degree fail!\n");
        return false;
    }
    if (GenTester(degree, rule) == false)
    {
        return false;
    }
    return true;
}
//---------------------------------------------------------------------------
bool ASPSolver::GenTester(vector<int> &degree, TSmallStr *rule)
{
    if (degree.size() != numRule)
    {
        return false;
    }
    TSmallStr insert;
    for (int i=0; i<numRule; i++)
    {
        insert.Append("v1(%d, %d).\n", i+1, degree[i]);
    }
    FileIO prefFile;
    if (prefFile.Open(testerTXT, "rb") == 0)
    {
        printf("open preferences fail!");
        return false;
    }
    TSmallStr buffer;
    buffer.SetLen(prefFile.size());
    if (prefFile.Read(buffer(), 1, prefFile.size()) == false)
    {
        return false;
    }
    prefFile.Close();
    insert += *rule;
    insert += buffer;
    if (prefFile.Open(tester) == 0)
    {
        printf("open preferences fail!");
        return false;
    }
    if (prefFile.Write(insert(), 1, insert.Len) == false)
    {
        return false;
    }
    prefFile.Close();
    return true;
}
//---------------------------------------------------------------------------
int ASPSolver::GetAS(char *buffer)
{
    // char * ptr = strstr(buffer, "UNSAT");
    // if (ptr != NULL)
    // {
    //     printf("no answer set!\n");
    //     return 0;
    // }
    if (strstr(buffer, "UNSAT") != NULL || strstr(buffer, "Models     : 0") != NULL)
    {
        printf("no answer set!\n");
        return 0;
    }
    char *ptr = buffer;
    std::fill(answerSet.begin(), answerSet.end(), false);
    //get the answer set
    while (ptr)
    {
        ptr = strstr(ptr, "atom(");
        if (ptr == NULL)
        {
            break;
        }
        ptr = ptr+5;
        int temp = 0;
        while (*ptr != ')')
        {
            if (ptr == NULL)
            {
                return -1;
            }
            temp = temp*10 + *ptr - 48;
            ptr++;
        }
        if (temp < numAtom+1)
        {
            answerSet[temp-1] = true;
        }
        else
        {
            break;
        }
    }
    return 1;
}
//---------------------------------------------------------------------------
bool ASPSolver::GetDegree(char *buffer)
{
    // char * ptr = strstr(buffer, "UNSAT");
    // if (ptr != NULL)
    // {
    //     printf("unsatisfiable problem!\n");
    //     return false;
    // }
    if (strstr(buffer, "UNSAT") != NULL || strstr(buffer, "Models     : 0") != NULL)
    {
        printf("unsatisfiable problem!\n");
        return false;
    }
    //if the values are not get from the file
    if (strstr(buffer, "v0") == NULL)
    {
        if (CalcuDegree(buffer) == false)
        {
            return false;
        }
    }
    else
    {
        std::fill(degree.begin(), degree.end(), 0);
        TSmallStr string;
        for (int i=0; i<numRule; i++)
        {
            string.Format("v0(%d,", i+1);
            char *ptr = strstr(buffer, string());
            if (ptr == NULL)
            {
                return false;
            }
            while (*ptr != ',')
            {
                ptr++;
            }
            ptr++;
            while (*ptr != ')')
            {
                if (ptr == NULL)
                {
                    return false;
                }
                degree[i] = degree[i]*10 + *ptr -48;
                ptr++;
            }
        }
    }
    return true;
}
//---------------------------------------------------------------------------
//calculate the degree values of the current answer set (using calDegree.lp)
//---------------------------------------------------------------------------
bool ASPSolver::CalcuDegree(char *buffer)
{
    std::fill(degree.begin(), degree.end(), 0);
    char *ptr = buffer;
    char *end = ptr;
    ptr = strstr(ptr, "Answer");
    if (ptr == NULL)
    {
        return false;
    }
    TSmallStr insert;
    while (ptr)
    {
        ptr = strstr(ptr, "atom");
        if (ptr == NULL)
        {
            break;
        }
        end = strstr(ptr, ")");
        if (end == NULL)
        {
            return false;
        }
        insert.AddFast(ptr, end+1-ptr);
        insert.Append(".\n");
        ptr = end;
    }
    FileIO file;
    if (file.Open(calDegreeTXT, "rb") == false)
    {
        return false;
    }
    TSmallStr buff;
    buff.SetLen(file.size());
    if (file.Read(buff(), 1, file.size()) == false)
    {
        return false;
    }
    file.Close();
    insert += buff;
    if (file.Open(calDegreeFile) == false)
    {
        return false;
    }
    if (file.Write(insert(), 1, insert.Len) == false)
    {
        return false;
    }
    file.Close();
    if (genCommand(calDegreeFile, calDegreeResult) == false)
    {
        printf("generate command fail!\n");
        return false;
    }
    if (system(command()) < 0)
    {
        printf("solver fail!\n");
        return false;
    }
    if (file.Open(calDegreeResult, "rb") == false)
    {
        return false;
    }
    buff.SetLen(file.size());
    if (file.Read(buff(), 1, file.size()) == false)
    {
        return false;
    }
    file.Close();
    return GetDegree(buff());
}
//---------------------------------------------------------------------------
int ASPSolver::GetBetterAS()
{
    if (!ranked)
    {
        if (GenTester(degree, &prefString) == false)
        {
            return -1;
        }
    }
    else
    {
        if (GenTester(degree, &rankPrefString) == false)
        {
            return -1;
        }
    }
    if (genCommand(tester, testResult) == false)
    {
        printf("generate command fail!\n");
        return -1;
    }
    if (system(command()) < 0)
    {
        printf("solver fail!\n");
        return -1;
    }
    FileIO resultFile;
    if (resultFile.Open(testResult, "rb") == 0)
    {
        printf("open test result fail!");
        return -1;
    }
    TSmallStr buffer;
    buffer.SetLen(resultFile.size());
    if (resultFile.Read(buffer(), 1, resultFile.size()) == false)
    {
        return -1;
    }
    resultFile.Close();
    if (strstr(buffer(), "UNSAT") != NULL || strstr(buffer(), "Models     : 0") != NULL)
    {
        return 0;
    }
    else
    {
        int result = GetAS(buffer());
        if (result != 1)
        {
            return result;
        }
        if (GetDegree(buffer()) == false)
        {
            return -1;
        }
        return 1;
    }
}
//---------------------------------------------------------------------------
/*
loop tester until cannot find any satisfiable answer set
use the previous result as a better start point to generate a new tester
startAS: the name for the file contains the initial answer set
*/
//---------------------------------------------------------------------------
int ASPSolver::GetOneOptAS(char *startAS)
{
    if (numOpt > maxNum-1)
    {
        printf("Maximum optimal answer sets!\n");
        return 0;
    }
    FileIO file;
    if (file.Open(startAS, "rb") == 0)
    {
        printf("open answer set fail!\n");
        return -1;
    }
    TSmallStr buffer;
    buffer.SetLen(file.size());
    if (file.Read(buffer(), 1, file.size()) == false)
    {
        return -1;
    }
    file.Close();
    int result = GetAS(buffer());
    if (result != 1)
    {
        return result;
    }
    if (GetDegree(buffer()) == false)
    {
        return -1;
    }
    while (true)
    {
        result = GetBetterAS();
        if (result == -1)
        {
            return -1;
        }
        if (result == 0)
        {
            numOpt++;
            if (printResult() == false)
            {
                return -1;
            }
            return 1;
        }
    }
}
//---------------------------------------------------------------------------
//generate the generator(gen_ano.lp) to get another not-worse start point
//---------------------------------------------------------------------------
bool ASPSolver::GetGenAno()
{
    TSmallStr insert;
    insert.Set(":-");
    for (int i=0; i<numAtom; i++)
    {
        if (answerSet[i] == false)
        {
            insert.AddFast(" not");
        }
        insert.Append(" atom(%d),", i+1);
    }
    insert.Buff[insert.Len-1] = '.';
    insert += "\n";
    for (int i=0; i<numRule; i++)
    {
        insert.Append("v%d(%d, %d).\n", numOpt, i+1, degree[i]);
    }
    if (!ranked)
    {
        insert.Append(":- not not_worse%d.\n"
                      "not_worse%d :- v0(R, V0), v%d(R, V1), V1 > V0.\n"
                      "equ%d(R) :- v0(R, V0), v%d(R, V1), V0 = V1.\n"
                      "not_worse%d :- equ%d(R) : rule(R).\n",
                      numOpt, numOpt, numOpt, numOpt, numOpt, numOpt, numOpt);
    }
    else
    {
        insert.Append(":- not not_worse%d.\n"
                      "equ%d(R) :- v0(R, V0), v%d(R, V1), V0 = V1.\n"
                      "not_worse%d :- equ%d(R) : rule(R).\n"
                      "better%dAt(L) :- v0(R, V0), v%d(R, V1), V0 < V1, level(R, L).\n"
                      "fail%dAt(L) :- better%dAt(L), v0(R, V0), v%d(R, V1), V0 > V1, level(R, L0), L0 < L.\n"
                      "not_worse%d :- not fail%dAt(L), better%dAt(L).\n",
                      numOpt, numOpt, numOpt, numOpt, numOpt, numOpt, numOpt, 
                      numOpt, numOpt, numOpt, numOpt, numOpt, numOpt);
    }
    FileIO prefFile;
    if (prefFile.Open(genAnoFile, "ab") == 0)
    {
        printf("open file fail!");
        return false;
    }
    if (prefFile.Write(insert(), 1, insert.Len) == false)
    {
        return false;
    }
    prefFile.Close();
    return true;
}
//---------------------------------------------------------------------------
//generate an optimal answer set different from the current answerSet
//---------------------------------------------------------------------------
int ASPSolver::GetAnoOptAS(float *time)
{
    if (numOpt == 0)
    {
        printf("No given answer set!\n");
        return 0;
    }
    if (numOpt > maxNum-1)
    {
        printf("Maximum optimal answer sets!\n");
        return 0;
    }
    TTimeInterval timer;
    timer.Start();
    if (GetGenAno() == false)
    {
        return -1;
    }
    if (genCommand(genAnoFile, genAnoResult) == false)
    {
        printf("generate command fail!\n");
        return false;
    }
    if (system(command()) < 0)
    {
        printf("solver fail!\n");
        return -1;
    }
    int result = GetOneOptAS(genAnoResult);
    timer.Pause();
    if (time != NULL)
    {
        *time = timer.GetTime();
    }
    else
    {
        printf("Time: %f\n", timer.GetTime());
    }
    return result;
}
//---------------------------------------------------------------------------
//generate an optimal answer set different from the givenAS
//---------------------------------------------------------------------------
int ASPSolver::GetAnoOptAS(vector<bool> givenAS, vector<int> givenDG, float *time)
{
    if (givenAS.size() != numAtom || givenDG.size() != numRule)
    {
        return 0;
    }
    answerSet = givenAS;
    degree = givenDG;
    numOpt = 1;
    FileIO file;
    if (file.Open(genAnoTXT, "rb") == false)
    {
        return false;
    }
    TSmallStr buffer;
    buffer.SetLen(file.size());
    if (file.Read(buffer(), 1, file.size()) == false)
    {
        return false;
    }
    file.Close();
    if (file.Open(genAnoFile) == false)
    {
        return false;
    }
    if (file.Write(buffer(), 1, buffer.Len) == false)
    {
        return false;
    }
    file.Close();
    return GetAnoOptAS(time);
}
//---------------------------------------------------------------------------
bool ASPSolver::Dissimilar(bool dis, vector<bool> &AS1, vector<bool> &AS2, int dist)
{
    if (AS1.size() != AS2.size())
    {
        printf("Answer sets have difference sizes!\n");
        return false;
    }
    if (AS1 == AS2)
    {
        return false;
    }
    int distance = 0;
    for (int i=0; i<AS1.size(); i++)
    {
        if (AS1[i] != AS2[i])
        {
            distance++;
            if (dis == true)
            {
                if (distance >= dist)
                {
                    return true;
                }
            }
            else
            {
                if (distance > dist)
                {
                    return false;
                }
            }
        }
    }
    return dis == false;
}
//---------------------------------------------------------------------------
//Iteratively find optimal answer set, and check the distance one by one.
//---------------------------------------------------------------------------
int ASPSolver::GetDisOpt(bool dis, vector<bool> &givenAS, int dist, float *time)
{
    if (givenAS.size() != numAtom)
    {
        return 0;
    }
    TTimeInterval timer;
    timer.Start();
    int result = GetFirstOptAS(); 
    if (result == -1)
    {
        return -1;
    }
    while (result == 1)
    {
        if (Dissimilar(dis, answerSet, givenAS, dist))
        {
            if (printResult() == false)
            {
                return -1;
            }
            timer.Pause();
            if (time != NULL)
            {
                *time = timer.GetTime();
            }
            else
            {
                printf("Time: %f\n", timer.GetTime());
            }
            return 1;
        }
        result = GetAnoOptAS();
        if (result == -1)
        {
            return -1;
        }
    }
    timer.Pause();
    printf("There is no satisfied answer set!\n");
    if (time != NULL)
    {
        *time = timer.GetTime();
    }
    else
    {
        printf("Time: %f\n", timer.GetTime());
    }
    return 0;
}
//---------------------------------------------------------------------------
/*
Build new generator and tester generate similar/dissimilar optimal answer sets, 
check whether it is "real" optimal.
*/
//---------------------------------------------------------------------------
int ASPSolver::GetDisOptNew(bool dis, vector<bool> &givenAS, int dist, float *time)
{
    if (givenAS.size() != numAtom)
    {
        return 0;
    }
    TTimeInterval timer;
    timer.Start();
    TSmallStr insert;
    insert.Set(":- diff(X), option(X), given(X), atom(X).\n"
               ":- diff(X), option(X), not given(X), not atom(X).\n"
               ":- not diff(X), option(X), given(X), not atom(X).\n"
               ":- not diff(X), option(X), not given(X), atom(X).\n"
               "#show atom/1.\n");
    if (dis == true)
    {
        insert.Append("%d{diff(X):option(X)}.\n", dist);
    }
    else
    {
        insert.Append("{diff(X):option(X)}%d.\n", dist);
    }
    for (int i=0; i<numAtom; i++)
    {
        insert.Append("option(%d).\n", i+1);
    }
    for (int i=0; i<numAtom; i++)
    {
        insert.Append("{given(%d)}.\n", i+1);
    }
    insert.AddFast(":-");

    for (int i=0; i<numAtom; i++)
    {
        if (answerSet[i] == false)
        {
            insert.AddFast(" not");
        }
        insert.Append(" atom(%d),", i+1);
    }
    insert.Buff[insert.Len-1] = '.';
    insert += "\n";
    for (int i=0; i<givenAS.size(); i++)
    {
        if (givenAS[i] == true)
        {
            insert.Append("given(%d).\n", i+1);
        }
        else
        {
            insert.Append(":- given(%d).\n", i+1);
        }
    }

    ASPSolver disGen;
    if (disGen.Init(inputGen, inputPref, false, NULL, "./data2/") == false)
    {
        return -1;
    }
    FileIO file;
    if (file.Open(disGen.generator, "rb") == false)
    {
        return -1;
    }
    TSmallStr buffer;
    buffer.SetLen(file.size());
    if (file.Read(buffer(), 1, buffer.Len) == false)
    {
        return -1;
    }
    file.Close();
    TSmallStr temp = insert;
    temp += buffer;
    if (file.Open(disGen.generator) == false)
    {
        return -1;
    }
    if (file.Write(temp(), 1, temp.Len) == false)
    {
        return -1;
    }
    file.Close();
    if (file.Open(disGen.testerTXT, "rb") == false)
    {
        return -1;
    }
    buffer.SetLen(file.size());
    if (file.Read(buffer(), 1, buffer.Len) == false)
    {
        return -1;
    }
    file.Close();
    temp = insert;
    temp += buffer;
    if (file.Open(disGen.testerTXT) == false)
    {
        return -1;
    }
    if (file.Write(temp(), 1, temp.Len) == false)
    {
        return -1;
    }
    file.Close();
    if (file.Open(disGen.genAnoFile, "rb") == false)
    {
        return -1;
    }
    buffer.SetLen(file.size());
    if (file.Read(buffer(), 1, buffer.Len) == false)
    {
        return -1;
    }
    file.Close();
    temp = insert;
    temp += buffer;
    if (file.Open(disGen.genAnoFile) == false)
    {
        return -1;
    }
    if (file.Write(temp(), 1, temp.Len) == false)
    {
        return -1;
    }
    file.Close();
    timer.Pause();
    int result = disGen.GetFirstOptAS();
    if (result == -1)
    {
        return -1;
    }
    while (result != 0)
    {
        //verify whether the answer set is real optimal
        answerSet = disGen.answerSet;
        degree = disGen.degree;
        result = GetBetterAS();
        if (result == -1)
        {
            return -1;
        }
        if (result == 0)
        {
            numOpt++;
            if (printResult() == false)
            {
                return -1;
            }
            timer.Pause();
            if (time != NULL)
            {
                *time = timer.GetTime();
            }
            else
            {
                printf("Time: %f\n", timer.GetTime());
            }
            return 1;
        }
        else
        {
            result = disGen.GetAnoOptAS();
            if (result == -1)
            {
                return -1;
            }
        }
    }
    timer.Pause();
    printf("There is no satisfied answer set!\n");
    if (time != NULL)
    {
        *time = timer.GetTime();
    }
    else
    {
        printf("Time: %f\n", timer.GetTime());
    }
    return 0;
}
//---------------------------------------------------------------------------
// int ASPSolver::GetOneOptMC(float *time)
// {
//     FileIO file;
//     if (file.Open(claspResult) == false)
//     {
//         return -1;
//     }
//     file.Close();
//     //command.Set(partCmd);
//     command.Format("gringo %s | claspOpt 0", generator);

//     TTimeInterval timer;
//     timer.Start();

//     if (system(command()) < 0)
//     {
//         printf("solver fail!\n");
//         return -1;
//     }

//     timer.Pause();
//     if (time != NULL)
//     {
//         *time = timer.GetTime();
//     }
//     else
//     {
//         printf("Time: %f\n", timer.GetTime());
//     }

//     if (file.Open(claspResult, "rb") == false)
//     {
//         return -1;
//     }
//     TSmallStr buffer;
//     buffer.SetLen(file.size());
//     if (file.Read(buffer(), 1, buffer.Len) == false)
//     {
//         return -1;
//     }
//     file.Close();

//     char *ptr = strstr(buffer(), "OPT");
//     if (ptr == NULL)
//     {
//         printf("No optimal answer set satisfies the distance requirement.\n");
//         return 0;
//     }
//     else
//     {
//         numOpt++;
        
//         // answerSet.InitZero();
//         std::fill(answerSet.begin(), answerSet.end(), 0);

//         TSmallStr string;

//         ptr = buffer();

//         while (ptr)
//         {
//             ptr = strstr(ptr, "atom(");
//             if (ptr == NULL)
//             {
//                         //return false;
//                 break;
//             }

//             ptr = ptr+5;

//             int temp = 0;
//             while (*ptr != ')')
//             {
//                 if (ptr == NULL)
//                 {
//                     return -1;
//                 }
//                 temp = temp*10 + *ptr - 48;
//                 ptr++;
//             }

//             if (temp < numAtom+1)
//             {
//                 string += atom[temp-1];
//                 string += " ";

//                 answerSet[temp-1] = 1;
//             }
//             else
//             {
//                 break;
//             }
//         }
//         string += "\n";

//         printf("%s", string());

//         return 1;
//     }
// }
//---------------------------------------------------------------------------
// int ASPSolver::GetAnoOptMC(float *time)
// {
//     if (numOpt == 0)
//     {
//         return -1;
//     }

//     TTimeInterval timer;
//     timer.Start();

// //    if (CalcuDegree(claspResult) == false)
// //    {
// //        return -1;
// //    }

//     if (GetGenAno() == false)
//     {
//         return -1;
//     }

//     timer.Pause();

//     FileIO file;
//     if (file.Open(claspResult) == false)
//     {
//         return -1;
//     }
//     file.Close();

//     //command.Set(partCmd);
//     command.Format("gringo %s | claspOpt 0", genAnoFile);

//     timer.Start();

//     if (system(command()) < 0)
//     {
//         printf("solver fail!\n");
//         return -1;
//     }

//     timer.Pause();
//     if (time != NULL)
//     {
//         *time = timer.GetTime();
//     }
//     else
//     {
//         printf("Time: %f\n", timer.GetTime());
//     }

//     if (file.Open(claspResult, "rb") == false)
//     {
//         return -1;
//     }
//     TSmallStr buffer;
//     buffer.SetLen(file.size());
//     if (file.Read(buffer(), 1, buffer.Len) == false)
//     {
//         return -1;
//     }
//     file.Close();

//     char *ptr = strstr(buffer(), "OPT");
//     if (ptr == NULL)
//     {
//         printf("No more optimal answer set.\n");
//         return 0;
//     }
//     else
//     {
//         numOpt++;
        
//         // answerSet.InitZero();
//         std::fill(answerSet.begin(), answerSet.end(), 0);

//         TSmallStr string;

//         ptr = buffer();

//         while (ptr)
//         {
//             ptr = strstr(ptr, "atom(");
//             if (ptr == NULL)
//             {
//                         //return false;
//                 break;
//             }

//             ptr = ptr+5;

//             int temp = 0;
//             while (*ptr != ')')
//             {
//                 if (ptr == NULL)
//                 {
//                     return -1;
//                 }
//                 temp = temp*10 + *ptr - 48;
//                 ptr++;
//             }

//             if (temp < numAtom+1)
//             {
//                 string += atom[temp-1];
//                 string += " ";

//                 answerSet[temp-1] = 1;
//             }
//             else
//             {
//                 break;
//             }
//         }
//         string += "\n";

//         printf("%s", string());

//         return 1;
//     }
// }
//---------------------------------------------------------------------------
// int ASPSolver::GetDisOptMC(bool dis, TArrayBase< int > &givenAS, int dist, float *time)
// {
//     TTimeInterval timer;
//     timer.Start();

//     TSmallStr insert;
//     if (dis == true)
//     {
//         //insert.Format("diff(X) :- given(X), not atom(X), X<=%d.\ndiff(X) :- atom(X), not given(X), X<=%d.\ndist(K) :- K = #count {diff(X)}.\n:- dist(K), K<%d.\n#hide given/1.\n#hide diff/1.\n#hide dist/1.\n", numAtom, numAtom, dist);
//         insert.Format("diff(X) :- given(X), not atom(X), X<=%d.\ndiff(X) :- atom(X), not given(X), X<=%d.\n:- #count{diff(X)}%d.\n#hide given/1.\n#hide diff/1.\n#hide dist/1.\n", numAtom, numAtom, dist-1);
//     }
//     else
//     {
//         insert.Format("diff(X) :- given(X), not atom(X), X<=%d.\ndiff(X) :- atom(X), not given(X), X<=%d.\n:- %d#count{diff(X)}.\n#hide given/1.\n#hide diff/1.\n#hide dist/1.\n", numAtom, numAtom, dist+1);
//     }

//     insert.AddFast(":-");

// //    int p = 0;
// //    for (int i=1; i<numAtom+1; i++)
// //    {
// //        if (i == givenAS[p])
// //        {
// //            if (i == numAtom)
// //            {
// //                insert.Append(" atom(%d).\n", i);
// //                break;
// //            }
// //            insert.Append(" atom(%d),", i);
// //            p++;
// //        }
// //        else
// //        {
// //            if (i == numAtom)
// //            {
// //                insert.Append(" not atom(%d).\n", i);
// //                break;
// //            }
// //            insert.Append(" not atom(%d),", i);
// //        }
// //    }

//     for (int i=0; i<numAtom; i++)
//     {
//         if (i == numAtom-1)
//         {
//             if (answerSet[i] == 0)
//             {
//                 insert.Append(" not atom(%d).\n", i+1);
//             }
//             else
//             {
//                 insert.Append(" atom(%d).\n", i+1);
//             }
//             break;
//         }

//         if (answerSet[i] == 0)
//         {
//             insert.Append(" not atom(%d),", i+1);
//         }
//         else
//         {
//             insert.Append(" atom(%d),", i+1);
//         }
//     }

//     for (int i=0; i<givenAS.len; i++)
//     {
//         if (givenAS[i] == 1)
//         {
//             insert.Append("given(%d).\n", i+1);
//         }
//     }

// //    if (UpdateGen() == false)
// //    {
// //        return -1;
// //    }

//     FileIO file;
//     if (file.Open(generator, "rb") == false)
//     //if (file.Open(genAnoFile, "rb") == false)
//     {
//         return -1;
//     }

//     TSmallStr buffer;
//     buffer.SetLen(file.size());
//     if (file.Read(buffer(), 1, buffer.Len) == false)
//     {
//         return -1;
//     }
//     file.Close();

//     insert += buffer;

//     if (file.Open(genDist) == false)
//     {
//         return -1;
//     }
//     if (file.Write(insert(), 1, insert.Len) == false)
//     {
//         return -1;
//     }
//     file.Close();

//     timer.Pause();

//     if (file.Open(claspResult) == false)
//     {
//         return -1;
//     }
//     file.Close();
//     //command.Set(partCmd);
//     command.Format("gringo %s | claspOpt 0", genDist);

//     timer.Start();

//     if (system(command()) < 0)
//     {
//         printf("solver fail!\n");
//         return -1;
//     }

//     timer.Pause();
//     if (time != NULL)
//     {
//         *time = timer.GetTime();
//     }

//     if (file.Open(claspResult, "rb") == false)
//     {
//         return -1;
//     }
//     buffer.SetLen(file.size());
//     if (file.Read(buffer(), 1, buffer.Len) == false)
//     {
//         return -1;
//     }
//     file.Close();

//     char *ptr = strstr(buffer(), "OPT");
//     if (ptr == NULL)
//     {
//         printf("No optimal answer set satisfies the distance requirement.\n");
//         return 0;
//     }
//     else
//     {
//         TSmallStr string;

//         ptr = buffer();
//         //answerSet.InitZero();
//         //for (int i=0; i<size; i++)
//         //int i = 0;
//         while (ptr)
//         {
//             ptr = strstr(ptr, "atom(");
//             if (ptr == NULL)
//             {
//                         //return false;
//                 break;
//             }

//             ptr = ptr+5;

//             int temp = 0;
//             while (*ptr != ')')
//             {
//                 if (ptr == NULL)
//                 {
//                     return -1;
//                 }
//                 temp = temp*10 + *ptr - 48;
//                 ptr++;
//             }

//             if (temp < numAtom+1)
//             {
//                 string += atom[temp-1];
//                 string += " ";

//                 //answerSet[i] = temp;
//                 //i++;
//                 answerSet[temp-1] = 1;
//             }
//             else
//             {
//                 break;
//             }
//         }
//         string += "\n";

//         printf("%s", string());

//         return 1;
//     }

// }
//---------------------------------------------------------------------------
#pragma package(smart_init)
