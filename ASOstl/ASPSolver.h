//---------------------------------------------------------------------------
#ifndef ASPSolverH
#define ASPSolverH
//---------------------------------------------------------------------------
#include <string.h>
//---------------------------------------------------------------------------
#include <vector>
using namespace std;
//---------------------------------------------------------------------------
#include "StrUtils.h"
#include "FileIO.h"
#include "TimerInterval_win.h"
//---------------------------------------------------------------------------
extern int solveMode;  //1: clingo;2:gringo+clasp;3:gringo+claspD;4:gringo+claspD2
class ASPSolver
{
public:
    //max number of optimal answer sets
    int maxNum;                   
    //file names for input generator, input preference, and input importances
    char *inputGen;
    char *inputPref;
    char *inputImp;

    //intermedia output/input, ./data/
    char generator[100];
    char genResult[100];
    char testerTXT[100];
    char tester[100];
    char testResult[100];
    char finalResult[100];
    char genAnoTXT[100];
    char genAnoFile[100];
    char genAnoResult[100];
    char calDegreeTXT[100];
    char calDegreeFile[100];
    char calDegreeResult[100];
    //constraints for preferences
    TSmallStr prefString;
    //constraints for ranked preferences
    TSmallStr rankPrefString;
    //commands
    char *clingo = "./bin/clingo.exe";
    char *gringo = "./bin/gringo.exe";
    char *clasp = "./bin/clasp-3.1.1-win32.exe";
    char *claspD = "./bin/claspD-1.1.2.exe";
    char *claspD2 = "./bin/claspD2.exe";
    TSmallStr command;
    //number of atoms
    int numAtom;
    //number of preferences
    int numRule;                
    //number of combinations in preferences
    int numComb;                
    int numDNF;
    //number of optimal answer sets
    int numOpt;
    //whether the preferences are ranked
    bool ranked;                
    //the important levels for all preference        
    vector<int> impLevels;              
    //the answer set, which is a 0/1 vector of size #atoms
    vector<bool> answerSet;
    //satisfaction degrees of all preference for one answer set
    vector<int> degree;

    ASPSolver()
    {
        maxNum = 500;
        inputGen = NULL;
        inputPref = NULL;
        inputImp = NULL;
        memset(generator, 0, 100);
        memset(genResult, 0, 100);
        memset(testerTXT, 0, 100);
        memset(tester, 0, 100);
        memset(testResult, 0, 100);
        memset(finalResult, 0, 100);
        memset(genAnoTXT, 0, 100);
        memset(genAnoFile, 0, 100);
        memset(genAnoResult, 0, 100);
        memset(calDegreeTXT, 0, 100);
        memset(calDegreeFile, 0, 100);
        memset(calDegreeResult, 0, 100);

        prefString = ":- not better.\n"
                     "better :- v0(R, V0), v1(R, V1), V0 < V1.\n"
                     ":- v0(R, V0), v1(R, V1), V0 > V1.\n";
        rankPrefString = ":- not better.\n"
                         "betterAt(L) :- v0(R, V0), v1(R, V1), V0 < V1, level(R, L).\n"
                         "failAt(L) :- betterAt(L), v0(R, V0), v1(R, V1), V0 > V1, level(R, L0), L0 <= L.\n"
                         "better :- not failAt(L), betterAt(L).\n";
        numRule = 0;
        numComb = 0;
        numDNF = 0;
        numOpt = 0;
        numAtom = 0;
        ranked = false;
    }

    bool Init(char *genFile, char *prefFile, bool isRank = false, char *impFile = NULL, char *outDir = NULL)
    {
        if (genFile == NULL || prefFile == NULL)
        {
            return false;
        }
        if (outDir == NULL)
        {
        	// outDir = "./data/";
            outDir = "./tmp/";
        }
        strcpy(generator, outDir);
        strcat(generator, "gen.lp");
        strcpy(genResult, outDir);
        strcat(genResult, "gen_result.txt");
        strcpy(testerTXT, outDir);
        strcat(testerTXT, "pref.txt");
        strcpy(tester, outDir);
        strcat(tester, "pref.lp");
        strcpy(testResult, outDir);
        strcat(testResult, "pref_result.txt");
        strcpy(finalResult, outDir);
        strcat(finalResult, "result.txt");
        strcpy(genAnoTXT, outDir);
        strcat(genAnoTXT, "gen_ano.txt");
        strcpy(genAnoFile, outDir);
        strcat(genAnoFile, "gen_ano.lp");
        strcpy(genAnoResult, outDir);
        strcat(genAnoResult, "gen_ano_result.txt");
        strcpy(calDegreeTXT, outDir);
        strcat(calDegreeTXT, "calDegree.txt");
        strcpy(calDegreeFile, outDir);
        strcat(calDegreeFile, "calDegree.lp");
        strcpy(calDegreeResult, outDir);
        strcat(calDegreeResult, "calDegreeResult.txt");
        numRule = 0;
        numComb = 0;
        numDNF  = 0;
        numOpt  = 0;
        numAtom = 0;
        inputGen = genFile;
        inputPref = prefFile;
        inputImp = impFile;
        ranked = isRank;
        //parse input generator and preferece files, rewrite them as files 
        //which can be read by gringo/clingo
        if (Parse() == false)
        {
            return false;
        }
        answerSet.clear();
        answerSet.insert(answerSet.begin(), numAtom, false);
        degree.clear();
        degree.insert(degree.begin(), numRule, 0);
        FileIO file;
        if (file.Open(finalResult) == false)
        {
            return false;
        }
        file.Close();
        return true;
    }

    bool ParseTestGen(char *fileName);
    bool ParsePref();
    bool ParsePrefRule(const char *&cursor, TSmallStr &write);
    bool ParseCNF(const char *&cursor, TSmallStr &write);
    bool ParseDNF(const char *&cursor, TSmallStr &write);
    bool ParseBody(const char *&cursor, TSmallStr &write);

    bool ParseImp()
    {
        if (!ranked)
        {
            return true;
        }
        impLevels.clear();
        impLevels.insert(impLevels.begin(), numRule, 0);
        if (inputImp == NULL)
        {
            return true;
        }
        FileIO file;
        if (file.Open(inputImp, "rb") == 0)
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
        if (buff.Buff == NULL || buff.Len == 0)
        {
            return true;
        }
        char *start = buff();
        for (int i=0; i<numRule; i++)
        {
            while (*start == '%')
            {
                while (*start != '\n')
                {
                    start++;
                }
                while (*start == '\n' || *start == '\r')
                {
                    start++;
                }
            }
            while (*start != '\0' && *start != '\r' && *start != '\n')
            {
                impLevels[i] = impLevels[i]*10 + *start - 48;
                start++;
            }
            while (*start == '\n' || *start == '\r')
            {
                start++;
            }
        }
        TSmallStr insert;
        insert.Set("");
        for (int i=0; i<numRule; i++)
        {
            insert.Append("level(%d, %d).\n", i+1, impLevels[i]);
        }
        if (file.Open(genAnoTXT, "ab") == false)
        {
            return false;
        }
        if (file.Write(insert(), 1, insert.Len) == false)
        {
            return false;
        }
        file.Close();
        if (file.Open(testerTXT, "ab") == false)
        {
            return false;
        }
        if (file.Write(insert(), 1, insert.Len) == false)
        {
            return false;
        }
        file.Close();
        return true;
    }

    bool Parse()
    {
        TSmallStr rule;
        rule.Set("v0(R, 1) :- rule(R), not body(R).\n"
                 "v0(R, 1) :- rule(R), not heads(R), body(R).\n"
                 "#show atom/1.\n"
                 "#show v0/2.\n");
        FileIO file;
        // if (file.Open(testerTXT) == false)
        // {
        //     return false;
        // }
        // if (file.Write(rule, 1, strlen(rule)) == false)
        // {
        //     return false;
        // }
        // file.Close();
        if (file.Open(genAnoTXT) == false)
        {
            return false;
        }
        // rule = "v0(R, 1) :- rule(R), not body(R).\n"
        //          "v0(R, 1) :- rule(R), not heads(R), body(R).\n"
        //          "#show atom/1.\n"
        //          "#show v0/2.\n";
        if (file.Write(rule(), 1, rule.Len) == false)
        {
            return false;
        }
        file.Close();
        if (file.Open(calDegreeTXT) == false)
        {
            return false;
        }
        // rule = "v0(R, 1) :- rule(R), not body(R).\n"
        //          "v0(R, 1) :- rule(R), not heads(R), body(R).\n"
        //          "#show v0/2.\n";
        if (file.Write(rule(), 1, rule.Len) == false)
        {
            return false;
        }
        file.Close();
        if (ranked)
        {
            rule += rankPrefString;
        }
        else
        {
            rule += prefString;
        }
        if (file.Open(testerTXT) == false)
        {
            return false;
        }
        if (file.Write(rule(), 1, rule.Len) == false)
        {
            return false;
        }
        file.Close();
        //if (ParseGen() == false || ParsePref() == false)
        if (ParseTestGen(inputGen) == false || ParsePref() == false)
        {
            return false;
        }
        if (ParseImp() == false)
        {
            return false;
        }

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
        return true;
    }

    int GetAS(char *buffer);
    bool GetDegree(char *buffer);
    bool CalcuDegree(char *buffer);
    //iterative method
    bool GetOneAS();
    bool GenTester(char *prevAS);
    bool GenTester(vector<int> &degree);
    int GetBetterAS();
    int GetOneOptAS(char *startAS);
    int GetFirstOptAS(float *time = NULL)
    {
        numOpt = 0;
        std::fill(answerSet.begin(), answerSet.end(), false);
        TTimeInterval timer;
        timer.Start();
        if (GetOneAS() == false)
        {
            return -1;
        }
        int result = GetOneOptAS(genResult);
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
    bool GetGenAno();
    int GetAnoOptAS(float *time = NULL);
    int GetAnoOptAS(vector<bool> givenAS, vector<int> givenDG, float *time);
    int GetKOptAS(int k)
    {
        if (numOpt >= k)
        {
            return 1;
        }
        if (numOpt == 0)
        {
            if (GetFirstOptAS() == -1)
            {
                return -1;
            }
        }
        int num = k-numOpt;
        for (int i=0; i<num; i++)
        {
            int result = GetAnoOptAS();
            if (result != 1)
            {
                return result;
            }
        }
        return 1;
    }
    bool GetAllOptAS()
    {
        TTimeInterval timer;
        timer.Start();
        if (numOpt == 0)
        {
            if (GetFirstOptAS() == -1)
            {
                return false;
            }
        }
        for (int i=0; i<maxNum; i++)
        {
            int result = GetAnoOptAS();
            if (result == -1)
            {
                return false;
            }
            else if (result == 0)
            {
                break;
            }
        }
        timer.Pause();
        printf("There are %d optimal answer set(s).\n", numOpt);
        printf("Time: %f\n", timer.GetTime());
        return true;
    }

    bool Dissimilar(bool dis, vector<bool> &AS1, vector<bool> &AS2, int dist);
    int GetDisOpt(bool dis, vector<bool> &givenAS, int dist, float *time = NULL);
    int GetDisOptNew(bool dis, vector<bool> &givenAS, int dist, float *time = NULL);

    //modified clasp
    // int GetOneOptMC(float *time = NULL);
    // int GetAnoOptMC(float *time = NULL);
    // int GetDisOptMC(bool dis, TArrayBase< int > &givenAS, int dist, float *time);          //Hamming distance

    //solveMode = 1: Using clingo
    //solveMode = 2: Using gringo+clasp
    //solveMode = 3: Using gringo+claspD
    //solveMode = 4: Using gringo+claspD2
    bool genCommand(char *input, char *output)
    {
        if (input == NULL || output == NULL)
        {
            return false;
        }
        if (solveMode == 1)
        {
            command.Format("%s %s > %s", clingo, input, output);
        }
        else if (solveMode == 2)
        {
            command.Format("%s %s | %s > %s", gringo, input, clasp, output);
        }
        else if (solveMode == 3)
        {
            command.Format("%s %s | %s > %s", gringo, input, claspD, output);
        }
        else if (solveMode == 4)
        {
            command.Format("%s %s | %s > %s", gringo, input, claspD2, output);
        }
        else
        {
            return false;
        }
        return true;
    }

    bool printResult(char *filename = NULL)
    {
        TSmallStr string;
        for (int i=0; i<answerSet.size(); i++)
        {
            if (answerSet[i] == true)
            {
                string.Append("atom%d", i+1);
                string += " ";
            }
        }
        string += "\n";
        printf("%s", string());
        if (filename != NULL)
        {
            FileIO file;
            if (file.Open(filename, "ab") == 0)
            {
                return false;
            }
            if (file.Write(string(), 1, string.Len) == false)
            {
                return false;
            }
            file.Close();
        }
        return true;
    }
};
//---------------------------------------------------------------------------
#endif
