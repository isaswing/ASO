//---------------------------------------------------------------------------
#include "QBFSolver.h"
#include <stdlib.h>
//---------------------------------------------------------------------------
//get genX and genY
bool QBFSolver::ParseTestGen(char *fileName)
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
    //get the number of atoms
    numAtom = 0;
    while (*ptr != ' ')
    {
        numAtom = numAtom*10 + *ptr - 48;
        ptr++;
    }
    ptr = strstr(ptr, "\n");
    while (*ptr == '\n' || *ptr == '\r')
    {
        ptr++;
    }
    char *start = ptr;
    //for each clause
    while (start && *start != '\0')
    {
        genY.AddFast("(");
        genX.AddFast(":- ");
        while (start)
        {
            if (*start == '-')
            {
                genY.AddFast("~");
                start++;
            }
            else
            {
                genX.AddFast("not ");
            }
            int index = 0;
            while (*start != ' ')
            {
                index = index*10 + *start - 48;
                start++;
            }
            genY.Append(" y%d ", index);
            genX.Append(" x%d ", index);
            start++;
            if (*start == '0')
            {
                start = strstr(start, "\n");
                if (start == NULL)
                {
                    break;
                }
                while (*start == '\n' || *start == '\r')
                {
                    start++;
                }
                break;
            }
            genY.AddFast("|");
            genX.AddFast(", ");
        }
        genY.AddFast(")");
        genX.AddFast(".\n");
        if (start && *start != '\0')
        {
            genY.AddFast("&");
        }
    }
    return true;
}
//---------------------------------------------------------------------------
//get pref1 and pref2
bool QBFSolver::ParseTestPref(char *inFile)
{
    if (inFile == NULL)
    {
        return false;
    }
    FileIO file;
    if (file.Open(inFile, "rb") == false)
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

    pref1.Set("(");
    char *ptr = buffer();
    char *p = ptr;
    //for each preference rule
    while (ptr && *ptr != '\0')
    {
        p = strstr(ptr, ".");
        bool condition = strstr(ptr, ":-") && strstr(ptr, ":-") < p;
        int neg = 2;
        if (strstr(ptr, "not") < strstr(ptr, ">"))
        {
            neg = 1;
        }
        ptr = strstr(ptr, "atom");
        ptr += strlen("atom");
        int head = 0;
        while (*ptr >= '0' && *ptr <= '9')
        {
            head = head*10 + *ptr -48;
            ptr++;
        }
        if (condition == false)
        {
            if (neg == 1)
            {
                pref1.Append("(~ y%d & x%d)", head, head);
                pref2.Append("~(~ x%d & y%d)", head, head);
            }
            else
            {
                pref1.Append("(y%d & ~ x%d)", head, head);
                pref2.Append("~(x%d & ~ y%d)", head, head);
            }
        }
        else
        {
            vector<int> conds;
            ptr = strstr(ptr, ":-");
            //get all conditions
            while (*ptr != '.')
            {
                ptr++;
                p = strstr(ptr, ",") && strstr(ptr, ",") < strstr(ptr, ".") ? strstr(ptr, ",") : strstr(ptr, ".");
                bool conNeg = strstr(ptr, "not") && strstr(ptr, "not") < p ? true : false;
                int cond = 0;
                ptr = strstr(ptr, "atom");
                ptr += strlen("atom");
                while (*ptr >= '0' && *ptr <= '9')
                {
                    cond = cond*10 + *ptr -48;
                    ptr++;
                }
                if (conNeg)
                {
                    conds.push_back(0-cond);
                }
                else
                {
                    conds.push_back(cond);
                }
                ptr = p;
            }
            if (neg == 1)
            {
                pref1.Append("(~ y%d & x%d & ", head, head);
                pref2.Append("~(~ x%d & y%d & ", head, head);
            }
            else
            {
                pref1.Append("(y%d & ~ x%d & ", head, head);
                pref2.Append("~(x%d & ~ y%d & ", head, head);
            }
            if (conds.size() == 1)
            {
                if (conds[0] < 0)
                {
                    pref1.Append("~ x%d & ~ y%d)|(~ x%d & y%d & ", abs(conds[0]), 
                                 abs(conds[0]), abs(conds[0]), abs(conds[0]));
                    pref2.Append("~ x%d & ~ y%d)&~(~ y%d & x%d & ", abs(conds[0]), 
                                 abs(conds[0]), abs(conds[0]), abs(conds[0]));
                }
                else
                {
                    pref1.Append("x%d & y%d)|(x%d & ~ y%d & ", conds[0], conds[0], conds[0], conds[0]);
                    pref2.Append("x%d & y%d)&~(y%d & ~ x%d & ", conds[0], conds[0], conds[0], conds[0]);
                }
            }
            else if (conds.size() == 2)
            {
                if (conds[0] < 0)
                {
                    pref1.Append("~ x%d & ~ y%d & ", abs(conds[0]), abs(conds[0]));
                    pref2.Append("~ x%d & ~ y%d & ", abs(conds[0]), abs(conds[0]));
                }
                else
                {
                    pref1.Append("x%d & y%d & ", conds[0], conds[0]);
                    pref2.Append("x%d & y%d & ", conds[0], conds[0]);
                }
                if (conds[1] < 0)
                {
                    pref1.Append("~ x%d & ~ y%d)|(", abs(conds[1]), abs(conds[1]));
                    pref2.Append("~ x%d & ~ y%d)&~(", abs(conds[1]), abs(conds[1]));
                }
                else
                {
                    pref1.Append("x%d & y%d)|(", conds[1], conds[1]);
                    pref2.Append("x%d & y%d)&~(", conds[1], conds[1]);
                }

                if (conds[0] < 0)
                {
                    pref1.Append("~ x%d & ", abs(conds[0]));
                    pref2.Append("~ y%d & ", abs(conds[0]));
                }
                else
                {
                    pref1.Append("x%d & ", conds[0]);
                    pref2.Append("y%d & ", conds[0]);
                }
                if (conds[1] < 0)
                {
                    pref1.Append("~ x%d & ", abs(conds[1]));
                    pref2.Append("~ y%d & ", abs(conds[1]));
                }
                else
                {
                    pref1.Append("x%d & ", conds[1]);
                    pref2.Append("y%d & ", conds[1]);
                }
                if (conds[0] < 0)
                {
                    pref1.Append("(y%d | ", abs(conds[0]));
                    pref2.Append("(x%d | ", abs(conds[0]));
                }
                else
                {
                    pref1.Append("(~ y%d | ", conds[0]);
                    pref2.Append("(~ x%d | ", conds[0]);
                }
                if (conds[1] < 0)
                {
                    pref1.Append("y%d) & ", abs(conds[1]));
                    pref2.Append("x%d) & ", abs(conds[1]));
                }
                else
                {
                    pref1.Append("~ y%d) & ", conds[1]);
                    pref2.Append("~ x%d) & ", conds[1]);
                }
            }
            else
            {
                return false;
            }
            if (neg == 1)
            {
                pref1.Append("x%d)", head);
                pref2.Append("y%d)", head);
            }
            else
            {
                pref1.Append("~ x%d)", head);
                pref2.Append("~ y%d)", head);
            }
        }
        ptr = strstr(ptr, ".");
        ptr++;
        while (*ptr == '\r' || *ptr == '\n')
        {
            ptr++;
        }
        if (strstr(ptr, "."))
        {
            pref1.AddFast("|");
            pref2.AddFast("&");
        }
    }
    pref1.AddFast(")");
    return true;
}
//---------------------------------------------------------------------------
bool QBFSolver::StrReplace(TSmallStr &src, char * target, char *replace)
{
    char *ptr = strstr(src(), target);
    char *nptr = src();
    TSmallStr newStr;
    while (ptr)
    {
        newStr.AddFast(nptr, ptr-nptr);
        newStr.AddFast(replace);
        nptr = ptr + strlen(target);
        ptr++;
        ptr = strstr(ptr, target);
    }
    newStr.AddFast(nptr);
    src.operator =(newStr);
    return true;
}
//---------------------------------------------------------------------------
bool QBFSolver::WriteDLP(TSmallStr &formula, char *insert, char *aggre)
{
    numD = 0;
    numC = 0;
    dataSet.clear();
    // dataSet.reserve(400000);
    dataSet.reserve(5*formula.Len);
    TTimeInterval timer;
    Formula f;
    if (f.Build(formula()) == false)
    {
        return false;
    }
    if (f.ConvertToCNF() == false)
    {
        return false;
    }
    if (f.NegCNF() == false)
    {
        return false;
    }
    //translate to DLP and print out to DLP.lp
    if (f.DNFtoDLP(numAtom) == false)
    {
        return false;
    }
    dataSet.clear();

    if (insert == NULL && aggre == NULL)
    {
        return true;
    }
    FileIO file;
    if (file.Open(DLP, "rb") == false)
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
    if (insert != NULL)
    {
        buffer += insert;
    }
    if (aggre != NULL)
    {
        for (int i=0; i<numAtom; i++)
        {
            TSmallStr target;
            target.Format(" x%d ", i+1);
            TSmallStr replace;
            replace.Format(" x(%d) ", i+1);
            if (StrReplace(buffer, target(), replace()) == false)
            {
                return false;
            }
        }
        buffer += aggre;
    }
    if (file.Open(DLP) == false)
    {
        return false;
    }
    if (file.Write(buffer(), 1, buffer.Len) == false)
    {
        return false;
    }
    file.Close();
    return true;
}
//---------------------------------------------------------------------------
//Get the answer set from a clasp output
int QBFSolver::GetASClasp(char *aggre = NULL)
{
    FileIO file;
    if (file.Open(claspResult, "rb") == false)
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
    char *ptr = strstr(buffer(), "Answer");
    if (ptr == NULL)
    {
        printf("No answer set found.\n");
        return 0;
    }
    std::fill(answerSet.begin(), answerSet.end(), false);
    while (true)
    {
        ptr = strstr(ptr, "x");
        if (ptr == NULL)
        {
            break;
        }
        ptr++;
        if (aggre != NULL)
        {
            ptr++;
        }
        int index = 0;
        while (*ptr >= '0' && *ptr <= '9')
        {
            index = index*10 + *ptr - 48;
            ptr++;
        }
        if (index > numAtom)
        {
            break;
        }
        answerSet[index-1] = true;
    }
    return 1;
}
//---------------------------------------------------------------------------
//Get the answer set from a dlv output
int QBFSolver::GetASDLV(char *aggre = NULL)
{
    FileIO file;
    if (file.Open(dlvResult, "rb") == false)
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

    char *ptr = strstr(buffer(), "{");
    if (ptr == NULL)
    {
        printf("No answer set found.\n");
        return 0;
    }
    std::fill(answerSet.begin(), answerSet.end(), false);
    while (true)
    {
        ptr = strstr(ptr, "x");
        if (ptr == NULL)
        {
            break;
        }
        ptr++;
        if (aggre != NULL)
        {
            ptr++;
        }
        int index = 0;
        while (*ptr >= '0' && *ptr <= '9')
        {
            index = index*10 + *ptr - 48;
            ptr++;
        }
        if (index > numAtom)
        {
            break;
        }
        answerSet[index-1] = true;
    }
    return 1;
}
//---------------------------------------------------------------------------
int QBFSolver::SolvebyDLP(TSmallStr &formula, char *insert, char *aggre, float *time)
{
    if (WriteDLP(formula, insert, aggre) == false)
    {
        return -1;
    }
    TSmallStr command;
    if (solveMode == 4)
    {
        command.Format("%s -n=1 %s > %s", dlv, DLP, dlvResult);
    }
    else
    {
        if (aggre == NULL)
        {
            command.Format("%s -g %s | ", gringo, DLP);
        }
        else
        {
            command.Format("%s %s | ", gringo, DLP);
        }
        if (solveMode == 1)
        {
            command.Append("%s > %s", clasp, claspResult);
        }
        else if (solveMode == 2)
        {
            command.Append("%s > %s", claspD, claspResult);
        }
        else if (solveMode == 3)
        {
            command.Append("%s > %s", claspD2, claspResult);
        }
        else
        {
            return -1;
        }
    }
    TTimeInterval timer;
    timer.Start();
    if (system(command()) < 0)
    {
        printf("solver fail!\n");
        return -1;
    }
    timer.Pause();

    int result = 0;
    if (solveMode == 4)
    {
        result = GetASDLV(aggre);
    }
    else
    {
        result = GetASClasp(aggre);
    }
    if (result == -1)
    {
        return -1;
    }
    printResult();
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
int QBFSolver::GetOneOptAS(float *time)
{
    return SolvebyDLP(QBF, genX(), NULL, time);
}
//---------------------------------------------------------------------------
int QBFSolver::GetAnoOptAS(float *time)
{
    TSmallStr anoQBF;
    anoQBF += ":- ";
    for (int i=0; i<answerSet.size(); i++)
    {
        if (answerSet[i] == false)
        {
            anoQBF.AddFast(" not");
        }
        anoQBF.Append(" x%d ,", i+1);
    }
    anoQBF.Buff[anoQBF.Len-1] = '.';
    anoQBF += "\n";
    anoQBF += genX;
    return SolvebyDLP(QBF, anoQBF(), NULL, time);
}
//---------------------------------------------------------------------------
int QBFSolver::GetDisOptAS(bool dis, vector<bool> &givenAS, int dist, float *time)
{
    TSmallStr aggre;
    if (solveMode == 4)
    {
        aggre.Set("diff(X) :- given(X), not x(X).\n"
                  "diff(X) :- x(X), not given(X).\n");
        if (dis == true)
        {
            aggre.Append(":-#count{X:diff(X)} < %d.\n", dist);
        }
        else
        {
            aggre.Append(":-#count{X:diff(X)} > %d.\n", dist);
        }
    }
    else
    {
        aggre.Set(":- diff(X), option(X), given(X), x(X).\n"
                  ":- diff(X), option(X), not given(X), not x(X).\n"
                  ":- not diff(X), option(X), given(X), not x(X).\n"
                  ":- not diff(X), option(X), not given(X), x(X).\n");
        if (dis == true)
        {
            aggre.Append("%d{diff(X):option(X)}.\n", dist);
        }
        else
        {
            aggre.Append("{diff(X):option(X)}%d.\n", dist);
        }
        for (int i=0; i<numAtom; i++)
        {
            aggre.Append("option(%d).\n", i+1);
        }
    }
    aggre.AddFast(":-");
    for (int i=0; i<givenAS.size(); i++)
    {
        if (givenAS[i] == false)
        {
            aggre.AddFast(" not");
        }
        aggre.Append(" x(%d),", i+1);
    }
    aggre.Buff[aggre.Len-1] = '.';
    aggre += "\n";
    for (int i=0; i<givenAS.size(); i++)
    {
        if (givenAS[i] == true)
        {
            aggre.Append("given(%d).\n", i+1);
        }
    }
    return SolvebyDLP(QBF, genX(), aggre(), time);
}
//---------------------------------------------------------------------------
//QBF->CNF->qdimacs
// bool QBFSolver::SolvebyQBF(TSmallStr &formula, float *time)
// {
//     //numX = atom.Number;
//     // numY = atom.Number;
//     //numY = atom.size();
//     numY = numAtom;
//     numD = 0;
//     numC = 0;

//     //int a = sizeof(Formula);
//     //dataSet.Alloc(20000);
    
//     // dataSet.Alloc(350000);
//     //dataSet.resize(350000);
//     dataSet.clear();
//     dataSet.reserve(350000);

//     TTimeInterval timer;
//     timer.Start();

//     Formula f;

//     if (f.Build(formula()) == false)
//     {
//         return false;
//     }
//     if (f.ConvertToCNF() == false)
//     {
//         return false;
//     }
//     f.Print("./data/CNF.txt");
//     if (f.CNFtoQdimacs() == false)            //print out to QBF.qdimacs
//     {
//         return false;
//     }

//     timer.Pause();

//     // dataSet.Close();
//     dataSet.clear(); // Why?

// //    if (insert != NULL)
// //    {
// //        FileIO file;
// //        if (file.Open("./data/QBF.qdimacs", "rb") == false)
// //        {
// //            return -1;
// //        }
// //        TSmallStr buffer;
// //        buffer.SetLen(file.size());
// //        if (file.Read(buffer(), 1, file.size()) == false)
// //        {
// //            return -1;
// //        }
// //        file.Close();
// //
// //        buffer += insert;
// //
// //        if (file.Open("./data/QBF.qdimacs") == false)
// //        {
// //            return -1;
// //        }
// //
// //        if (file.Write(buffer(), 1, buffer.Len) == false)
// //        {
// //            return -1;
// //        }
// //        file.Close();
// //    }

//     char *command = "sKizzo -log ./data/QBF.qdimacs > ./data/QBFtruth.txt";

//     timer.Start();

//     if (system(command) < 0)
//     {
//         printf("solver fail!\n");
//         return false;
//     }

//     timer.Pause();

//     FileIO file;
//     if (file.Open("./data/QBFtruth.txt", "rb") == false)
//     {
//         return false;
//     }
//     TSmallStr buffer;
//     buffer.SetLen(file.size());
//     if (file.Read(buffer(), 1, buffer.Len) == false)
//     {
//         return false;
//     }
//     file.Close();

//     char *ptr = strstr(buffer(), "TRUE");
//     if (ptr == NULL)
//     {
//         if (time != NULL)
//         {
//             *time = timer.GetTime();
//         }
//         else
//         {
//             printf("Time: %f\n", timer.GetTime());
//         }

//         printf("The instance evaluates to FALSE.\n");
//         return true;
//     }
//     else
//     {
//         printf("The instance evaluates to TRUE.\n");
//     }

//     command = "ozziKs -dump qdc ./data/QBF.qdimacs.sKizzo.log";

//     timer.Start();

//     if (system(command) < 0)
//     {
//         printf("solver fail!\n");
//         return false;
//     }

//     timer.Pause();

//     if (file.Open("./data/QBF.qdimacs.qdc", "rb") == false)
//     {
//         return false;
//     }
//     buffer.SetLen(file.size());
//     if (file.Read(buffer(), 1, file.size()) == false)
//     {
//         return false;
//     }
//     file.Close();

//     ptr = buffer();

//     for (int i=0; i<3; i++)   //scan 3 lines
//     {
//         ptr = strstr(ptr, "\n");
//         if (ptr == NULL)
//         {
//             return false;
//         }
//         ptr++;
//     }
//     while (*ptr == '\n' || *ptr == '\r')
//     {
//         ptr++;
//     }
//     if (*ptr != 'v')
//     {
//         return false;
//     }

//     ptr++;
//     if (!ptr)
//     {
//         return false;
//     }

//     // answerSet.InitZero();
//     std::fill(answerSet.begin(), answerSet.end(), 0);

//     TSmallStr result;
//     //int j = 0;
//     //for (int i=0; i<numAtom; i++)
//     for (int i=0; i<numX; i++)
//     {
//         // if (i%atom.Number == 0)
//         //if (i%atom.size() == 0)
//         if (i%numAtom == 0)
//         {
//             // answerSet.InitZero();
//             std::fill(answerSet.begin(), answerSet.end(), 0);
//             result += "\n";
//         }

//         while (*ptr == ' ')
//         {
//             ptr++;
//         }

//         if (*ptr != '-')
//         {
//             int index = 0;
//             while (*ptr != ' ')
//             {
//                 if (*ptr < '0' || *ptr > '9')
//                 {
//                     return false;
//                 }
//                 index = index*10 + *ptr - 48;
//                 ptr++;
//             }

//             //if (index > numAtom)
//             if (index > numX)
//             {
//                 //return false;
//                 break;
//             }
//             // index = (index-1)%atom.Number;
//             //index = (index-1)%atom.size();
//             index = (index-1)%numAtom;
//             //answerSet[j] = index;
//             answerSet[index] = 1;
//             //j++;

//             //result += atom[index];
//             result.Append("atom%d", index+1);
//             result += " ";
//         }
//         else
//         {
//             ptr++;

//             while (*ptr != ' ')
//             {
//                 if (*ptr < '0' || *ptr > '9')
//                 {
//                     return false;
//                 }
//                 ptr++;
//             }
//         }
//     }

// //    TSmallStr result;
// //    for (int i=0; i<answerSet.len; i++)
// //    {
// ////        if (answerSet[i] == 0)
// ////        {
// ////            break;
// ////        }
// ////        result += atom[answerSet[i]-1];
// ////        result += " ";
// //        if (answerSet[i] == 1)
// //        {
// //            result += atom[i];
// //            result += " ";
// //        }
// //    }
//     result.AddFast("\n");

//     printf("%s", result());

//     if (time != NULL)
//     {
//         *time = timer.GetTime();
//     }
//     else
//     {
//         printf("Time: %f\n", timer.GetTime());
//     }

//     return true;
// }
//---------------------------------------------------------------------------
// bool QBFSolver::GetOneOptASbyQBF(float *time)
// {
//     //QBF.Format("%s&(~(%s)|~(%s&%s))", genX(), genY(), pref1(), pref2());
//     //QBF.Format("%s&(~(%s)|~%s|~(%s))", genX(), genY(), pref1(), pref2());
// //    QBF = genX;
// //    QBF.AddFast("&(~(");
// //    QBF += genY;
// //    QBF.AddFast(")|~");
// //    QBF += pref1;
// //    QBF.AddFast("|~(");
// //    QBF += pref2;
// //    QBF.AddFast("))");



// //    QBF.Set("~(");
// //    QBF += genY;
// //    QBF.AddFast(")|~");
// //
// //    QBF.AddFast("(");
// //    QBF += better;
// //    QBF.AddFast(")");
// //
// //    FileIO file;
// //    if (file.Open(inputGen, "rb") == false)
// //    {
// //        return false;
// //    }
// //    TSmallStr insert;
// //    insert.SetLen(file.size());
// //    if (file.Read(insert(), 1, file.size()) == false)
// //    {
// //        return false;
// //    }
// //
// //    insert.Set(strstr(insert(), "\n"));

//     // numX = atom.Number;
//     //numX = atom.size();
//     numX = numAtom;
//     if (SolvebyQBF(QBF, time) == false)
//     {
//         return false;
//     }

//     return true;
// }
//---------------------------------------------------------------------------
// bool QBFSolver::GetAnoOptASbyQBF(float *time)
// {
//     //if (answerSet[0] == 0 || QBF.Len == 0)
//     if (genX.Len == 0 || better.Len == 0)
//     {
//         return false;
//     }

//     QBF = genX;
//     QBF.AddFast("&(~(");
//     QBF += genY;
//     QBF.AddFast(")|~");

//     QBF.AddFast("(");
//     QBF += better;
//     QBF.AddFast("))");

//     TSmallStr anoQBF;
//     anoQBF += "~(";
//     // for (int i=0; i<answerSet.len; i++)
//     for (int i=0; i<answerSet.size(); i++)
//     {
// //        if (answerSet[i] == 0)
// //        {
// //            break;
// //        }
// //        anoQBF.Append("x%d&", answerSet[i]);
//         if (answerSet[i] == 0)
//         {
//             anoQBF.AddFast("~");
//         }
//         anoQBF.Append("x%d&", i+1);
//     }

//     anoQBF.Buff[anoQBF.Len-1] = ')';
//     anoQBF += "&";
//     anoQBF += QBF;

//     // numX = atom.Number;
//     //numX = atom.size();
//     numX = numAtom;
//     if (SolvebyQBF(anoQBF, time) == false)
//     {
//         return false;
//     }

//     return true;
// }
//---------------------------------------------------------------------------
// bool QBFSolver::GetKOptASbyQBF(int k)
// {
//     TSmallStr test;
//     test += genX;
//     test += "&";
//     for (int i=1; i<k; i++)
//     {
//         TSmallStr src = genX;

//         // for (int j=0; j<atom.Number; j++)
//         //for (int j=0; j<atom.size(); j++)
//         for (int j=0; j<numAtom; j++)
//         {
//             TSmallStr target;
//             target.Format(" x%d ", j+1);
//             TSmallStr replace;
//             // replace.Format(" x%d ", j+1+i*atom.Number);
//             //replace.Format(" x%d ", j+1+i*atom.size());
//             replace.Format(" x%d ", j+1+i*numAtom);

//             if (StrReplace(src, target(), replace()) == false)
//             {
//                 return false;
//             }
//         }
//         test += src;
//         test.AddFast("&");
//     }

//     for (int i=0; i<k-1; i++)
//     {
//         for (int j=i+1; j<k; j++)
//         {
//             //i!=j
//             test += "(";
//             // for (int l=0; l<atom.Number; l++)
//             //for (int l=0; l<atom.size(); l++)
//             for (int l=0; l<numAtom; l++)
//             {
//                 // test.Append("(x%d&~x%d)|(~x%d&x%d)", i*atom.Number+l+1, j*atom.Number+l+1, i*atom.Number+l+1, j*atom.Number+l+1);
//                 //test.Append("(x%d&~x%d)|(~x%d&x%d)", i*atom.size()+l+1, j*atom.size()+l+1, i*atom.size()+l+1, j*atom.size()+l+1);
//                 test.Append("(x%d&~x%d)|(~x%d&x%d)", i*numAtom+l+1, j*numAtom+l+1, i*numAtom+l+1, j*numAtom+l+1);
//                 // if (l < atom.Number-1)
//                 //if (l < atom.size()-1)
//                 if (l < numAtom-1)
//                 {
//                     test += "|";
//                 }
//             }
//             //test.Buff[test.Len-1] = ")";
//             test += ")&";
//         }
//     }

//     test.AddFast("(~(");
//     test += genY;
//     test.AddFast(")|(");
//     test += "~(";
//     test += better;
//     test += ")&";

//     for (int i=1; i<k; i++)
//     {
//         TSmallStr src = better;
//         // for (int j=0; j<atom.Number; j++)
//         //for (int j=0; j<atom.size(); j++)
//         for (int j=0; j<numAtom; j++)
//         {
//             TSmallStr target;
//             target.Format(" x%d ", j+1);
//             TSmallStr replace;
//             // replace.Format(" x%d ", j+1+i*atom.Number);
//             //replace.Format(" x%d ", j+1+i*atom.size());
//             replace.Format(" x%d ", j+1+i*numAtom);

//             if (StrReplace(src, target(), replace()) == false)
//             {
//                 return false;
//             }
//         }
//         //test.Append("~(%s)", src);
//         test += "~(";
//         test += src;
//         test += ")";
//         if (i < k-1)
//         {
//             test += "&";
//         }
//     }
//     test += "))";

//     FileIO file;
//     if (file.Open("./data/kopt.qbf") == false)
//     {
//         return false;
//     }
//     if (file.Write(test(), 1, test.Len) == false)
//     {
//         return false;
//     }
//     //printf("%s\n", test());

//     // numX = k*atom.Number;
//     //numX = k*atom.size();
//     numX = k*numAtom;
//     if (SolvebyQBF(test) == false)
//     {
//         return false;
//     }

//     return true;
// }
//---------------------------------------------------------------------------
#pragma package(smart_init)
