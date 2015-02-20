#ifndef FormulaH
#define FormulaH
//---------------------------------------------------------------------------
#include "StrUtils.h"
#include "FileIO.h"
//---------------------------------------------------------------------------
#include <vector>
using namespace std;
//---------------------------------------------------------------------------
class Formula;
extern vector<Formula> dataSet;
extern int numD;
extern int numC;
//---------------------------------------------------------------------------
class Formula
{
public:
    char *DLPfile = "./data/DLP.lp";
    // char *QBFfile = "./data/QBF.qdimacs";
    TSmallStr formula;
    vector<Formula*> subFormulas;
    //whether *this is an atom
    bool atom;
    bool neg;
    //relation among subFormulas: (|, 1), (&, 2), (<->, 3), (->, 4)
    int oprat;
    //the truth value of *this: (0, unknow), (1, true), (-1, false)
    int truth;

    Formula()
    {
        atom = false;
        neg = false;
        oprat = 0;
        truth = 0;
    }

    Formula(const Formula& o)
    {
        formula = o.formula;
        subFormulas = o.subFormulas;
        atom = o.atom;
        neg = o.neg;
        oprat = o.oprat;
        truth = o.truth;
    }

    void Alloc()
    {
        formula.Alloc(1024*1024);
        subFormulas.reserve(64);
    }

    void Init()
    {
        Alloc();
        atom = false;
        neg = false;
        oprat = 0;
        truth = 0;
    }

    bool Build(char *QBF);
    bool ConvertToCNF();
    bool EquFree();
    bool ImplyFree();
    bool NotFree();
    bool CNF();
    bool NegCNF();
    bool BracketFree(char *&formula);
    int GetSubFormula(char *start, char *&end);
    bool Simplify();
    bool UpdateFormula();
    bool Print(char *filename = NULL);
    bool DNFtoDLP(int nVar);
    // bool CNFtoQdimacs();
};
//---------------------------------------------------------------------------
#endif
