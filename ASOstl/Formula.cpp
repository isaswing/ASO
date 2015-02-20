//---------------------------------------------------------------------------
#include "Formula.h"
//---------------------------------------------------------------------------
vector< Formula > dataSet;
int numD;
int numC;
//---------------------------------------------------------------------------
bool Formula::Build(char *str)
{
    if (atom == true)
    {
        return true;
    }
    if (BracketFree(str) == false)
    {
        return false;
    }
    formula.Set(str);
    TSmallStr A;
    char *p = str;
    char *cursor = str;
    while (*cursor != '\0')
    {
        Formula subFormulaAdd;
        subFormulaAdd.Alloc();
        dataSet.push_back(subFormulaAdd);
        subFormulas.push_back( &dataSet.back() ); // BUG? change of pointer.
        Formula& subFormula = dataSet.back(); // Using reference!
        p = cursor;
        if (*p == '~')
        {
            p++;
            subFormula.neg = true;
        }
        while (*p == ' ')
        {
            p++;
        }
        int state = GetSubFormula(p, cursor);
        if (state < 0)
        {
            return false;
        }
        A.SetFast(p, cursor-p);
        if (state == 1)
        {
            subFormula.atom = true;
            subFormula.formula.Set(A);
        }
        if (subFormula.Build(A()) == false)
        {
            return false;
        }
        while (*cursor == ' ')
        {
            cursor++;
        }
        if (*cursor == '\0')
        {
            return true;
        }
        else if (*cursor == '|')
        {
            if (oprat == 0)
            {
                oprat = 1;
            }
            else if (oprat != 1)
            {
                return false;
            }
            cursor++;
        }
        else if (*cursor == '&')
        {
            if (oprat == 0)
            {
                oprat = 2;
            }
            else if (oprat != 2)
            {
                return false;
            }
            cursor++;
        }
        else if (*cursor == '<')
        {
            if (*(cursor+1) != '-' || *(cursor+2) != '>')
            {
                return false;
            }
            if (oprat == 0)
            {
                oprat = 3;
            }
            else if (oprat != 3)
            {
                return false;
            }
            cursor += 3;
        }
        else if (*cursor == '-')
        {
            if (*(cursor+1) != '>')
            {
                return false;
            }
            if (oprat == 0)
            {
                oprat = 4;
            }
            else if (oprat != 4)
            {
                return false;
            }
            cursor += 2;
        }
        else
        {
            return false;
        }
        while (*cursor == ' ')
        {
            cursor++;
        }
    }
    return true;
}
//---------------------------------------------------------------------------
bool Formula::ConvertToCNF()
{
    if (UpdateFormula() == false)
    {
    	return false;
    }
    // if (EquFree() == false)
    // {
    //     return false;
    // }
    // if (ImplyFree() == false)
    // {
    //     return false;
    // }
    if (NotFree() == false)
    {
        return false;
    }
    if (UpdateFormula() == false)
    {
    	return false;
    }
    if (CNF() == false)
    {
        return false;
    }
    return true;
}
//---------------------------------------------------------------------------
//atom return 1, formula return 2
int Formula::GetSubFormula(char *start, char *&end)
{
    if (start == NULL)
    {
        return -1;
    }
    char *cursor = start;
    if (*cursor == '(')
    {
        int stack = 1;
        cursor++;
        while (*cursor != '\0')
        {
            if (*cursor == '(')
            {
                stack++;
            }
            else if (*cursor == ')')
            {
                stack--;
                if (stack == 0)
                {
                    break;
                }
            }
            cursor++;
        }
        if (stack != 0)
        {
            return -1;
        }
        end = cursor+1;
        return 2;
    }
    else
    {
        //while (*cursor != '\0' && *cursor != ' ' && *cursor != ')' && *cursor != '|' && *cursor != '&' && *cursor != '<' && *cursor != '-')
        while (*cursor != '\0' && *cursor != ' ' && *cursor != '|' && *cursor != '&' && *cursor != '<' && *cursor != '-')
        {
            cursor++;
        }
        end = cursor;
        return 1;
    }
}
//---------------------------------------------------------------------------
//oprat == 3
bool Formula::EquFree()
{
    if (atom == true)
    {
        return true;
    }
    for (int i=0; i<subFormulas.size(); i++)
    {
        if (subFormulas[i]->EquFree() == false)
        {
            return false;
        }
    }
    if (oprat == 3)
    {
        if (subFormulas.size() != 2)
        {
            return false;
        }
        TSmallStr newFormula;
        //(~A | B) & (A | ~B)
        if (subFormulas[0]->atom == true && subFormulas[1]->atom == true)
        {
            newFormula.Format("(~%s | %s) & (%s | ~%s)", subFormulas[0]->formula(), subFormulas[1]->formula(), subFormulas[0]->formula(), subFormulas[1]->formula());
        }
        else if (subFormulas[0]->atom == true)
        {
            //(~A | B) & (A | ~(B))
            if (subFormulas[1]->oprat == 1)
            {
                newFormula.Format("(~%s | %s) & (%s | ~(%s))", subFormulas[0]->formula(), subFormulas[1]->formula(), subFormulas[0]->formula(), subFormulas[1]->formula());
            }
            //(~A | (B)) & (A | ~(B))
            else
            {
                newFormula.Format("(~%s | (%s)) & (%s | ~(%s))", subFormulas[0]->formula(), subFormulas[1]->formula(), subFormulas[0]->formula(), subFormulas[1]->formula());
            }
        }
        else if (subFormulas[1]->atom == true)
        {
            //(~(A) | B) & (A | ~B)
            if (subFormulas[0]->oprat == 1)
            {
                newFormula.Format("(~(%s) | %s) & (%s | ~%s)", subFormulas[0]->formula(), subFormulas[1]->formula(), subFormulas[0]->formula(), subFormulas[1]->formula());
            }
            //(~(A) | B) & ((A) | ~B)
            else
            {
                newFormula.Format("(~(%s) | %s) & ((%s) | ~%s)", subFormulas[0]->formula(), subFormulas[1]->formula(), subFormulas[0]->formula(), subFormulas[1]->formula());
            }
        }
        Init();
        if (Build(newFormula()) == false)
        {
            return false;
        }
    }
    return true;
}
//---------------------------------------------------------------------------
//oprat == 4
bool Formula::ImplyFree()
{
    if (atom == true)
    {
        return true;
    }
    for (int i=0; i<subFormulas.size(); i++)
    {
        if (subFormulas[i]->ImplyFree() == false)
        {
            return false;
        }
    }
    if (oprat == 4)
    {
        if (subFormulas.size() != 2)
        {
            return false;
        }
        oprat = 1;
        subFormulas[0]->neg = subFormulas[0]->neg == false ? true : false;
    }
    return true;
}
//---------------------------------------------------------------------------
bool Formula::NotFree()
{
    if (atom == true)
    {
        return true;
    }
    if (neg == true)
    {
        if (oprat == 3)
        {
            if (EquFree() == false)
            {
                return false;
            }
        }
        else if (oprat == 4)
        {
            if (ImplyFree() == false)
            {
                return false;
            }
        }
        if (oprat == 1)
        {
            oprat = 2;
        }
        else if (oprat == 2)
        {
            oprat = 1;
        }
        else
        {
            return false;
        }
        neg = false;
        for (int i=0; i<subFormulas.size(); i++)
        {
            subFormulas[i]->neg = subFormulas[i]->neg == false ? true : false;
        }
    }
    for (int i=0; i<subFormulas.size(); i++)
    {
        if (subFormulas[i]->NotFree() == false)
        {
            return false;
        }
    }
    return true;
}
//---------------------------------------------------------------------------
bool Formula::CNF()
{
    if (oprat == 3 || oprat == 4)
    {
        return false;
    }
    if (neg == true && atom == false)
    {
        return false;
    }
    if (atom == true)
    {
        return true;
    }
    for (int i=0; i<subFormulas.size(); i++)
    {
        if (subFormulas[i]->atom == false)
        {
            break;
        }
        if (i == subFormulas.size()-1)
        {
            return true;
        }
    }
    for (int i=0; i<subFormulas.size(); i++)
    {
        if (subFormulas[i]->CNF() == false)
        {
            return false;
        }
    }
    if (oprat == 0 || oprat == 2)
    {
        if (UpdateFormula() == false)
        {
            return false;
        }
        return true;
    }
    TSmallStr insert1;
    TSmallStr insert2;
    TSmallStr insert3;
    TSmallStr insertD;
    TSmallStr insertC;
    int num = subFormulas.size();
    //for each d
    for (int i=0; i<num; i++)
    {
        if (subFormulas[i]->atom == true)
        {
            insert1.AddFast(" | ");
            if (subFormulas[i]->neg == true)
            {
                insert1.AddFast("~");
            }
            insert1 += subFormulas[i]->formula;
        }
        else
        {
            if (subFormulas[i]->neg == true)
            {
                return false;
            }
            numD++;
            insert1.Append(" | d%d", numD);
            insert2.Format("d%d", numD);
            //for each c
            for (int j=0; j<subFormulas[i]->subFormulas.size(); j++)
            {
                insert2.AddFast(" | ");
                if (subFormulas[i]->subFormulas[j]->atom == true)
                {
                    insertD.Format("~d%d | ", numD);
                    if (subFormulas[i]->subFormulas[j]->neg == false)
                    {
                        insert2.AddFast("~");
                    }
                    else
                    {
                        insertD.AddFast("~");
                    }
                    insert2 += subFormulas[i]->subFormulas[j]->formula;
                    insertD += subFormulas[i]->subFormulas[j]->formula;
                }
                else
                {
                    if (subFormulas[i]->subFormulas[j]->neg == true)
                    {
                        return false;
                    }
                    insert2.AddFast("~");
                    numC++;
                    insert2.Append("c%d", numC);
                    insertD.Format("~d%d | c%d", numD, numC);
                    insert3.Format("~c%d", numC);
                    for (int k=0; k<subFormulas[i]->subFormulas[j]->subFormulas.size(); k++)
                    {
                        if (subFormulas[i]->subFormulas[j]->subFormulas[k]->atom == false)
                        {
                            return false;
                        }
                        insert3.AddFast(" | ");
                        insertC.Format("c%d | ", numC);
                        if (subFormulas[i]->subFormulas[j]->subFormulas[k]->neg == true)
                        {
                            insert3.AddFast("~");
                        }
                        else
                        {
                            insertC.AddFast("~");
                        }
                        insert3 += subFormulas[i]->subFormulas[j]->subFormulas[k]->formula;
                        insertC += subFormulas[i]->subFormulas[j]->subFormulas[k]->formula;
                        Formula newFormula;
                        newFormula.Alloc();
                        dataSet.push_back(newFormula);
                        subFormulas.push_back( &dataSet.back() );
                        dataSet.back().Build(insertC());
                    }
                    Formula newFormula;
                    newFormula.Alloc();
                    dataSet.push_back(newFormula);
                    subFormulas.push_back( &dataSet.back() );
                    dataSet.back().Build(insert3());
                }
                Formula subFormula;
                subFormula.Alloc();
                dataSet.push_back(subFormula);
                subFormulas.push_back( &dataSet.back() );
                dataSet.back().Build(insertD());
            }
            Formula newFormula;
            newFormula.Alloc();
            dataSet.push_back(newFormula);
            subFormulas.push_back( &dataSet.back() );
            dataSet.back().Build(insert2());
        }
    }
    Formula subFormula;
    subFormula.Alloc();
    dataSet.push_back(subFormula);
    subFormulas.push_back( &dataSet.back() );
    dataSet.back().Build(insert1()+3);
    subFormulas.erase(subFormulas.begin(), subFormulas.begin() + num);
    oprat = 2;
    if (UpdateFormula() == false)
    {
        return false;
    }
    return true;
}
//---------------------------------------------------------------------------
//accept a simplified CNF
bool Formula::NegCNF()
{
    if (atom == true)                                     
    {
        neg = neg == true ? false : true;
    }
    //clause
    if (oprat == 1)
    {
        for (int i=0; i<subFormulas.size(); i++)
        {
            if (subFormulas[i]->atom == false)
            {
                return false;
            }
            subFormulas[i]->neg = subFormulas[i]->neg == true ? false : true;
        }
        oprat = 2;
    }
    //CNF
    else if (oprat == 2)
    {
        for (int i=0; i<subFormulas.size(); i++)
        {
            if (subFormulas[i]->atom == true)
            {
                subFormulas[i]->neg = subFormulas[i]->neg == true ? false : true;
            }
            else
            {
                if (subFormulas[i]->oprat != 1)
                {
                    return false;
                }             
                for (int j=0; j<subFormulas[i]->subFormulas.size(); j++)
                {
                    if (subFormulas[i]->subFormulas[j]->atom == false)
                    {
                        return false;
                    }                    
                    subFormulas[i]->subFormulas[j]->neg = subFormulas[i]->subFormulas[j]->neg == true ? false : true;
                }
                subFormulas[i]->oprat = 2;
            }
        }
        oprat = 1;
    }
    else
    {
        return false;
    }
    if (UpdateFormula() == false)
    {
        return false;
    }
    return true;
}
//---------------------------------------------------------------------------
bool Formula::BracketFree(char *&formula)
{
    char *cursor = formula;
    while (*cursor == '(')
    {
        int stack = 1;
        cursor++;
        while (*cursor != '\0')
        {
            if (*cursor == '(')
            {
                stack++;
            }
            else if (*cursor == ')')
            {
                stack--;
                if (stack == 0)
                {
                    break;
                }
            }
            cursor++;
        }
        if (stack != 0)
        {
            return false;
        }
        if (*(cursor+1) == '\0')
        {
            formula++;
            *cursor = '\0';
            cursor = formula;
        }
        else
        {
            break;
        }
    }
    return true;
}
//---------------------------------------------------------------------------
bool Formula::Simplify()
{
    if (atom == true || truth != 0)
    {
        return true;
    }
    for (int i=0; i<subFormulas.size(); i++)
    {
        if (subFormulas[i]->truth == 1)
        {
            if (oprat == 1)
            {
                truth = neg == false ? 1 : -1;
                return true;
            }
            else if (oprat == 2)
            {
                subFormulas.erase(subFormulas.begin()+i, subFormulas.begin()+i+1);
                i--;
                continue;
            }
            else
            {
                return false;
            }
        }
        else if (subFormulas[i]->truth == -1)
        {
            if (oprat == 1)
            {
                subFormulas.erase(subFormulas.begin()+i, subFormulas.begin()+i+1);
                i--;
                continue;
            }
            else if (oprat == 2)
            {
                truth = neg == false ? -1 : 1;
                return true;
            }
            else
            {
                return false;
            }
        }
        if (subFormulas[i]->atom == true)
        {
            for (int j=i+1; j<subFormulas.size(); j++)
            {
                if (subFormulas[j]->atom == true && strcmp(subFormulas[i]->formula(), subFormulas[j]->formula()) == 0)
                {
                    if (subFormulas[i]->neg == subFormulas[j]->neg)
                    {                            
                        subFormulas.erase(subFormulas.begin()+j, subFormulas.begin()+j+1);
                        j--;
                    }
                    else
                    {
                        if (oprat == 1)
                        {
                            truth = neg == false ? 1 : -1;
                            return true;
                        }
                        else if (oprat == 2)
                        {
                            truth = neg == false ? -1 : 1;
                            return true;
                        }
                        else
                        {
                            return false;
                        }
                    }
                }
            }
        }
    }
    if (oprat == 0)
    {
        if (subFormulas.size() != 1)
        {
            return false;
        }
        if (subFormulas[0]->atom == true)
        {
            atom = true;
            if (subFormulas[0]->neg == true)
            {
                neg = neg == true ? false : true;
            }
            formula.Set(subFormulas[0]->formula());
            subFormulas.erase(subFormulas.begin(), subFormulas.begin()+1);
            return true;
        }
        for (int i=0; i<subFormulas[0]->subFormulas.size(); i++)
        {
            subFormulas.push_back(subFormulas[0]->subFormulas[i]);
        }
        oprat = subFormulas[0]->oprat;
        if (subFormulas[0]->neg == true)
        {
            neg = neg == true ? false : true;
        }
        subFormulas.erase(subFormulas.begin(), subFormulas.begin()+1);

    }
    //DNF
    else if (oprat == 1)
    {
        int num = subFormulas.size();
        for (int i=0; i<num; i++)
        {
            if (subFormulas[i]->atom == false && subFormulas[i]->neg == false && subFormulas[i]->oprat == 1)
            {
                for (int j=0; j<subFormulas[i]->subFormulas.size(); j++)
                {
                    subFormulas.push_back(subFormulas[i]->subFormulas[j]);
                }
                subFormulas.erase(subFormulas.begin()+i, subFormulas.begin()+i+1);
                i--;
            }
        }
    }
    //CNF
    else if (oprat == 2)
    {
        int num = subFormulas.size();
        for (int i=0; i<num; i++)
        {
            if (subFormulas[i]->atom == false && subFormulas[i]->neg == false && subFormulas[i]->oprat == 2)
            {
                for (int j=0; j<subFormulas[i]->subFormulas.size(); j++)
                {
                    subFormulas.push_back(subFormulas[i]->subFormulas[j]);
                }
                subFormulas.erase(subFormulas.begin()+i, subFormulas.begin()+i+1);
                i--;
            }
        }
    }
    return true;
}
//---------------------------------------------------------------------------
bool Formula::UpdateFormula()
{
    if (atom == true || truth != 0)
    {
        return true;
    }
    for (int i=0; i<subFormulas.size(); i++)
    {
        if (subFormulas[i]->UpdateFormula() == false)
        {
            return false;
        }
    } 
    if (Simplify() == false)
    {
        return false;
    }
    if (atom == true || truth != 0)
    {
        return true;
    }
    formula.Set("");
    for (int i=0; i<subFormulas.size(); i++)
    {
        if (subFormulas[i]->neg == true)
        {
            formula.AddFast("~");
        }
        if (subFormulas[i]->atom == true)
        {
            formula += subFormulas[i]->formula;
        }
        else
        {
            formula.AddFast("(");
            formula += subFormulas[i]->formula;
            formula.AddFast(")");
        }
        if (i < subFormulas.size()-1)
        {
            if (oprat == 1)
            {
                formula.AddFast(" | ");
            }
            else if (oprat == 2)
            {
                formula.AddFast(" & ");
            }
            else if (oprat == 3)
            {
                formula.AddFast(" <-> ");
            }
            else if (oprat == 4)
            {
                formula.AddFast(" -> ");
            }
            else
            {
                return false;
            }
        }
    }
    return true;
}
//---------------------------------------------------------------------------
//suppose the formula is CNF or DNF
bool Formula::Print(char *filename)
{
    TSmallStr str;
    if (atom == true)
    {
        if (neg == true)
        {
            str.Set("~");
        }
        str += formula;
    }
    else
    {
    	str.Alloc(1024*1024);
        for (int i=0; i<subFormulas.size(); i++)
    	{
    		if (subFormulas[i]->atom == true && subFormulas[i]->neg == true)
    		{
    			str.AddFast("~");
    		}
    		str += subFormulas[i]->formula;
    		str.AddFast("\n");
    	}
    }
    if (filename == NULL)
    {
    	printf("%s\n", str());
    	return true;
    }
    FileIO file;
    if (file.Open(filename) == false)
    {
        return false;
    }
    if (file.Write(str(), 1, str.Len) == false)
    {
        return false;
    }
    file.Close();
    return true;
}
//---------------------------------------------------------------------------
bool Formula::DNFtoDLP(int nVar)
{
    int numX = nVar;
    int numY = nVar;
    TSmallStr DLP;
    DLP.Alloc(1024*1024);
    for (int i=0; i<numX; i++)
    {
        DLP.Append(" x%d | v%d .\n", i+1, i+1);
    }
    for (int i=0; i<numY; i++)
    {
        DLP.Append(" y%d | z%d .\n", i+1, i+1);
        DLP.Append(" y%d :- w .\n", i+1);
        DLP.Append(" z%d :- w .\n", i+1);
    }
    for (int i=0; i<numC; i++)
    {
        DLP.Append(" c%d | z%d .\n", i+1, numY+i+1);
        DLP.Append(" c%d :- w .\n", i+1);
        DLP.Append(" z%d :- w .\n", numY+i+1);
    }
    for (int i=0; i<numD; i++)
    {
        DLP.Append(" d%d | z%d .\n", i+1, numY+numC+i+1);
        DLP.Append(" d%d :- w .\n", i+1);
        DLP.Append(" z%d :- w .\n", numY+numC+i+1);
    }
    for (int i=0; i<subFormulas.size(); i++)
    {
        DLP.AddFast(" w :- ");
        if (subFormulas[i]->atom == true)
        {
            if (subFormulas[i]->neg == true)
            {
                if (*subFormulas[i]->formula() == 'x')
                {
                    DLP.Append(" v%s .\n", subFormulas[i]->formula()+1);
                }
                else if (*subFormulas[i]->formula() == 'y')
                {
                    DLP.Append(" z%s .\n", subFormulas[i]->formula()+1);
                }
                else if (*subFormulas[i]->formula() == 'c')
                {
                    int temp;
                    temp = atoi(subFormulas[i]->formula()+1);
                    DLP.Append(" z%d .\n", numY+temp);
                }
                else if (*subFormulas[i]->formula() == 'd')
                {
                    int temp;
                    temp = atoi(subFormulas[i]->formula()+1);
                    DLP.Append(" z%d .\n", numY+numC+temp);
                }
                else
                {
                    return false;
                }
            }
            else
            {
                DLP += " ";
                DLP += subFormulas[i]->formula;
                DLP += " .\n";
            }
            continue;
        }
        for (int j=0; j<subFormulas[i]->subFormulas.size(); j++)
        {
            if (subFormulas[i]->subFormulas[j]->atom == false)
            {
                return false;
            }
            if (subFormulas[i]->subFormulas[j]->neg == true)
            {
                if (*subFormulas[i]->subFormulas[j]->formula() == 'x')
                {
                    DLP.Append(" v%s ", subFormulas[i]->subFormulas[j]->formula()+1);
                }
                else if (*subFormulas[i]->subFormulas[j]->formula() == 'y')
                {
                    DLP.Append(" z%s ", subFormulas[i]->subFormulas[j]->formula()+1);
                }
                else if (*subFormulas[i]->subFormulas[j]->formula() == 'c')
                {
                    int temp;
                    temp = atoi(subFormulas[i]->subFormulas[j]->formula()+1);
                    DLP.Append(" z%d ", numY+temp);
                }
                else if (*subFormulas[i]->subFormulas[j]->formula() == 'd')
                {
                    int temp;
                    temp = atoi(subFormulas[i]->subFormulas[j]->formula()+1);
                    DLP.Append(" z%d ", numY+numC+temp);
                }
                else
                {
                    return false;
                }
            }
            else
            {
                DLP += " ";
                DLP += subFormulas[i]->subFormulas[j]->formula;
                DLP += " ";
            }
            if (j<subFormulas[i]->subFormulas.size()-1)
            {
                DLP.AddFast(", ");
            }
        }
        DLP.AddFast(".\n");
    }
    DLP.AddFast(" w :- not w .\n");
    FileIO file;
    if (file.Open(DLPfile) == false)
    {
        return false;
    }
    if (file.Write(DLP(), 1, DLP.Len) == false)
    {
        return false;
    }
    file.Close();
    return true;
}
//---------------------------------------------------------------------------
// bool Formula::CNFtoQdimacs()
// {
//     TSmallStr qdimacs;
//     qdimacs.Format("p cnf %d %d\ne ", numX+numY+numC+numD, subFormulas.size());
//     for (int i=0; i<numX; i++)
//     {
//         qdimacs.Append("%d ", i+1);
//     }
//     qdimacs.AddFast("0\na ");
//     for (int i=0; i<numY; i++)
//     {
//         qdimacs.Append("%d ", numX+i+1);
//     }
//     qdimacs.AddFast("0\ne ");
//     for (int i=0; i<numC; i++)
//     {
//         qdimacs.Append("%d ", numX+numY+i+1);
//     }
//     for (int i=0; i<numD; i++)
//     {
//         qdimacs.Append("%d ", numX+numY+numC+i+1);
//     }
//     qdimacs.AddFast("0\n");
//     for (int i=0; i<subFormulas.size(); i++)
//     {
//         if (subFormulas[i]->atom == true)
//         {
//             if (subFormulas[i]->neg == true)
//             {
//                 qdimacs.AddFast("-");
//             }
//             int temp = 0;
//             temp = atoi(subFormulas[i]->formula()+1);
//             if (*subFormulas[i]->formula() == 'x')
//             {
//                 qdimacs.Append("%d 0\n", temp);
//             }
//             else if (*subFormulas[i]->formula() == 'y')
//             {
//                 qdimacs.Append("%d 0\n", numX+temp);
//             }
//             else if (*subFormulas[i]->formula() == 'c')
//             {
//                 qdimacs.Append("%d 0\n", numX+numY+temp);
//             }
//             else if (*subFormulas[i]->formula() == 'd')
//             {
//                 qdimacs.Append("%d 0\n", numX+numY+numC+temp);
//             }
//             continue;
//         }
//         for (int j=0; j<subFormulas[i]->subFormulas.size(); j++)
//         {
//             if (subFormulas[i]->subFormulas[j]->atom != true)
//             {
//                 return false;
//             }
//             if (subFormulas[i]->subFormulas[j]->neg == true)
//             {
//                 qdimacs.AddFast("-");
//             }
//             int temp = 0;
//             temp = atoi(subFormulas[i]->subFormulas[j]->formula()+1);
//             if (*subFormulas[i]->subFormulas[j]->formula() == 'x')
//             {
//                 qdimacs.Append("%d ", temp);
//             }
//             else if (*subFormulas[i]->subFormulas[j]->formula() == 'y')
//             {
//                 qdimacs.Append("%d ", numX+temp);
//             }
//             else if (*subFormulas[i]->subFormulas[j]->formula() == 'c')
//             {
//                 qdimacs.Append("%d ", numX+numY+temp);
//             }
//             else if (*subFormulas[i]->subFormulas[j]->formula() == 'd')
//             {
//                 qdimacs.Append("%d ", numX+numY+numC+temp);
//             }
//         }
//         qdimacs.AddFast("0\n");
//     }
//     FileIO file;
//     // file.Open("./data/QBF.qdimacs");
//     if (file.Open(QBFfile) == false)
//     {
//         return false;
//     }
//     if (file.Write(qdimacs(), 1, qdimacs.Len) == false)
//     {
//         return false;
//     }
//     file.Close();
//     return true;
// }
//---------------------------------------------------------------------------
#pragma package(smart_init)
