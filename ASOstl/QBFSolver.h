//---------------------------------------------------------------------------
#ifndef QBFSolverH
#define QBFSolverH
//---------------------------------------------------------------------------
// #include <windows.h>
//---------------------------------------------------------------------------
#include <vector>
using namespace std;
//---------------------------------------------------------------------------
#include "StrUtils.h"
#include "FileIO.h"
#include "Formula.h"
#include "TimerInterval.h"
//---------------------------------------------------------------------------
extern int solveMode; //1: clasp3 2: claspD 3: claspD2 4: dlv
class QBFSolver
{
public:
    char *inputGen;
    char *inputPref;
    char *inputImp;
    char *DLP = "./tmp/DLP.lp";
    char *claspResult = "./tmp/claspResult.txt";
    char *dlvResult = "./tmp/dlvResult.txt";

    vector<bool> answerSet;
    vector<int> degree;
    int numAtom;
    //number of preference rules
    int numRule;
    bool ranked;
    int numLevel;
    vector<int> impLevels;

    //QBF of the generator
    TSmallStr genX;
    TSmallStr genY;

    vector<TSmallStr> XHead;
    vector<TSmallStr> YHead;
    TSmallStr XBody;
    TSmallStr YBody;
    //QBF of Y > X on some preference rule
    TSmallStr pref1;
    //QBF of not X > Y on any preference rule
    TSmallStr pref2;
    vector<TSmallStr> prefs1;
    vector<TSmallStr> prefs2;
    vector<TSmallStr> prefs3;
    TSmallStr QBF;

    //Solvers:
    char *gringo = "./bin/gringo.exe";
    char *clasp = "./bin/clasp-3.1.1-win32.exe";
    char *claspD = "./bin/claspD-1.1.2.exe";
    char *claspD2 = "./bin/claspD2.exe";
    char *dlv = "./bin/dlv.mingw.exe";

    QBFSolver()
    {
        inputGen = NULL;
        inputPref = NULL;
        inputImp = NULL;
        ranked = false;
        numAtom = 0;
        numRule = 0;
        numLevel = 0;
        XHead.clear();
        XHead.reserve(10);
        YHead.clear();
        YHead.reserve(10);
        QBF.Alloc(10000);
    }

    bool ParseTestGen(char *fileName);
    bool ParseTestPref(char *fileName);
    bool ParsePref();
    bool ParsePrefRule(const char *&cursor);
    bool ParseCNF(const char *&cursor);
    bool ParseDNF(const char *&cursor);
    bool ParseBody(const char *&cursor);

    bool ParseImp()
    {
        numLevel = 1;
        if (ranked == false)
        {
            return true;
        }
        if (inputImp != NULL)
        {
            impLevels.clear();
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
            while (start && *start != '\0')
            {
                int temp = 0;
                while (*start != '\0' && *start != '\r' && *start != '\n')
                {
                    temp = temp*10 + *start - 48;
                    start++;
                }
                if (temp > numLevel)
                {
                    numLevel = temp;
                }
                impLevels.push_back(temp);
                while (*start == '\n' || *start == '\r')
                {
                    start++;
                }
            }
        }
        return true;
    }

    bool Parse(char *gen, char *pref, bool rank = false, char *imp = NULL)
    {
        if (gen == NULL || pref == NULL)
        {
            printf("Null generator or null preference.\n");
            return false;
        }
        inputGen = gen;
        inputPref = pref;
        inputImp = imp;
        ranked = rank;
        if (ParseImp() == false)
        {
            return false;
        }
        if (ranked && numLevel > 1)
        {
            prefs1.clear();
            prefs1.reserve(numLevel);
            prefs2.clear();
            prefs2.reserve(numLevel);
            prefs3.clear();
            prefs3.reserve(numLevel);
        }
        if (ParseTestGen(inputGen) == false || ParsePref() == false)
        //if (ParseTestGen(inputGen) == false || ParseTestPref(inputPref) == false)
        {
            return false;
        }
        answerSet.clear();
        answerSet.insert(answerSet.begin(), numAtom, false);
        degree.clear();
        degree.insert(degree.begin(), numRule, 0);
        TSmallStr better;
        if (!ranked || numLevel <= 1)
        {
            better = pref1;
            better.AddFast("&");
            better += pref2;
        }
        else
        {
            better.Set("(");
            for (int i=0; i<numLevel; i++)
            {
                better.AddFast("(");
                better += prefs1[i];
                better.AddFast("&");
                better += prefs2[i];
                if (prefs3[i].Len != 0)
                {
                    better.AddFast("&");
                    better += prefs3[i];
                }
                better.AddFast(")");
                if (i < numLevel-1)
                {
                    better.AddFast("|");
                }
            }
            better.AddFast(")");
        }
        QBF = genY;
        QBF.AddFast(" & ");
        QBF += better;
        return true;
    }

    bool StrReplace(TSmallStr &src, char *target, char *replace);

    //write the problem as a QBF and solve it using a QBF solver (sKizzo)
    // bool SolvebyQBF(TSmallStr &formula, float *time = NULL);
    // bool GetOneOptASbyQBF(float *time = NULL);
    // bool GetAnoOptASbyQBF(float *time = NULL);
    // bool GetKOptASbyQBF(int k);

    bool WriteDLP(TSmallStr &formula, char *insert = NULL, char *aggre = NULL);
    int GetASClasp(char *aggre = NULL);
    int GetASDLV(char *aggre = NULL);
    int SolvebyDLP(TSmallStr &formula, char *insert = NULL, char *aggre = NULL, float *time = NULL);
    int GetOneOptAS(float *time = NULL);
    int GetAnoOptAS(float *time = NULL);
    int GetDisOptAS(bool dis, vector<bool> &givenAS, int dist, float *time = NULL);
    bool printResult(char *filename = NULL)
    {
        TSmallStr string;
        for (int i=0; i<answerSet.size(); i++)
        {
            if (answerSet[i] == true)
            {
                string.Append("atom%d ", i+1);
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
