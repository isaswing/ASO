//---------------------------------------------------------------------------
#include "QBFSolver.h"
//---------------------------------------------------------------------------
bool QBFSolver::ParsePref()
{
    FileIO input;
    if (input.Open(inputPref, "rb") == 0)
    {
        return false;
    }
    TSmallStr buff;
    buff.SetLen(input.size());
    if (input.Read(buff(), 1, input.size()) == false)
    {
        return false;
    }
    input.Close();
    if (buff.Len == 0)
    {
        return true;
    }
    numRule = 0;
    char *ptr = buff();
    while (ptr = strstr(ptr, "."))
    {
        numRule++;
        ptr++;
    }
    if (numRule == 0)
    {
        return true;
    }
    numRule = 0;

    const char *cursor      = buff();
    const char *yyMarker    = NULL;
    const char *yyCTXMarker = NULL;

/*!re2c
re2c:define:YYCTYPE     = char;
re2c:define:YYCURSOR    = cursor;
re2c:define:YYMARKER    = yyMarker;
re2c:define:YYCTXMARKER = yyCTXMarker;
re2c:yyfill:enable      = 0;
re2c:indent:top         = 2;

eol     = [\n|\r\n];
space   = ([ ]|eol);
comment = "%";

atom    = [a-zA-Z][a-zA-Z0-9\_]*;
negAtom = "not " atom;
literal = (atom|negAtom);

and     = "&";
or      = "|";
imply   = ":-";
better  = ">";

DNF1    = space* literal space* (or space* literal space*)*;
DNF2    = space* "(" DNF1 ")" space*;
DNF     = (DNF1|DNF2);

CNF1    = DNF (and DNF)*;
CNF2    = space* "(" CNF1 ")" space*;
CNF     = (CNF1|CNF2);

body    = space* literal space* ("," space* literal space*)*;

rule1   = CNF (better CNF)+ imply body ".";
rule2   = CNF (better CNF)+ ".";
rule    = (rule1|rule2);
*/

    if (!ranked)
    {
        pref1.Set("(");
    }
    else
    {
        for (int i=0; i<numLevel; i++)
        {
            TSmallStr p;
            prefs1.push_back(p);
            prefs1.back().Set("(");
            prefs2.push_back(p);
            prefs3.push_back(p);
        }
    }
    int count = 0;
    for (const char *p=cursor;*p;p=cursor)
    {
/*!re2c
    space   {   continue;   }
    rule    {
                numRule++;
                count++;
                if (ParsePrefRule(p) == false)
                {
                    return false;
                }
                continue;
            }
    comment {
                while (*cursor != '\n' && *cursor != '\0')
                {
                    cursor++;
                }
                continue;
            }
    [^]     {   break;  }
*/
    }
    if (!ranked)
    {
        if (pref1.Len > 1)
        {
            pref1.Buff[pref1.Len-1] = ')';
            //pref1.ReCountLen();
        }
        else
        {
            pref1.Close();
        }
        if (pref2.Len > 0)
        {
            pref2.Buff[pref2.Len-1] = 0;
            pref2.ReCountLen();
        }
    }
    else
    {
        for (int i=0; i<numLevel; i++)
        {
            if (prefs1[i].Len > 1)
            {
                prefs1[i].Buff[prefs1[i].Len-1] = ')';
            }
            else
            {
                prefs1[i].Close();
            }
            if (prefs2[i].Len > 0)
            {
                prefs2[i].Buff[prefs2[i].Len-1] = 0;
                prefs2[i].ReCountLen();
            }
            if (prefs3[i].Len > 0)
            {
                prefs3[i].Buff[prefs3[i].Len-1] = 0;
                prefs3[i].ReCountLen();
            }
        }
    }
/*  FileIO out;
    out.Open("./data/pref1.out");
    if (out.Write(pref1(), 1, pref1.Len) == false)
    {
        return false;
    }
    out.Close();
    out.Open("./data/pref2.out");
    if (out.Write(pref2(), 1, pref2.Len) == false)
    {
        return false;
    }
    out.Close();
*/
    return true;
}
//-----------------------------------------------------------------------------
bool QBFSolver::ParsePrefRule(const char *&cursor)
{
    const char *yyMarker = NULL;
    const char *yyCTXMarker = NULL;

    XHead.clear();
    YHead.clear();
    XHead.reserve(10);
    YHead.reserve(10);
    XBody.Close();
    YBody.Close();

    for (const char *p=cursor;*p;p=cursor)
    {
/*!re2c
    CNF     {
                if (ParseCNF(p) == false)
                {
                    return false;
                }
                continue;
            }
    imply/body
            {
                if (ParseBody(p) == false)
                {
                    return false;
                }
                break;
            }
    "."     {   break;  }
    [^]     {   continue;   }
*/
    }

    if (XHead.size() <= 1 || YHead.size() <= 1)
    {
        printf("Preference has no head.\n");
        return false;
    }
    TSmallStr *temp1 = NULL;
    TSmallStr *temp2 = NULL;
    if (!ranked)
    {
        temp1 = &pref1;
        temp2 = &pref2;
    }
    else
    {
        temp1 = &prefs1[impLevels[numRule-1]-1];
        temp2 = &prefs2[impLevels[numRule-1]-1];
    }

    temp2->AddFast("~");
    if (XBody.Len != 0)
    {
        temp1->Append("(%s&%s&", XBody(), YBody());
        temp2->Append("(%s&%s&", XBody(), YBody());
    }
    temp1->AddFast("(");
    temp2->AddFast("(");
    for (int i=0; i<XHead.size()-1; i++)
    {
        temp1->Append("(%s&", YHead[i]());
        temp2->Append("(%s&", XHead[i]());
        for (int j=0; j<=i; j++)
        {
            temp1->Append("~%s&", XHead[j]());
            temp2->Append("~%s&", YHead[j]());
        }
        temp1->AddFast("(");
        temp2->AddFast("(");
        for (int j=i+1; j<XHead.size(); j++)
        {
            temp1->Append("%s|", XHead[j]());
            temp2->Append("%s|", YHead[j]());
        }
        temp1->Buff[temp1->Len-1] = ')';
        temp2->Buff[temp2->Len-1] = ')';
        temp1->AddFast(")|");
        temp2->AddFast(")|");
    }
    temp1->Buff[temp1->Len-1] = ')';
    temp2->Buff[temp2->Len-1] = ')';
    if (XBody.Len != 0)
    {
        temp1->AddFast(")");
        temp2->AddFast(")");
    }
    temp1->AddFast("|(");
    temp2->AddFast("&~(");
    if (XBody.Len != 0)
    {
        temp1->Append("(~%s|(", YBody());
        temp2->Append("(~%s|(", XBody());
    }
    //pref1.AddFast("(");
    for (int i=0; i<XHead.size(); i++)
    {
        temp1->Append("~%s&", YHead[i]());
        temp2->Append("~%s&", XHead[i]());
    }
    if (XBody.Len != 0)
    {
        temp1->Buff[temp1->Len-1] = ')';
        temp2->Buff[temp2->Len-1] = ')';
        temp1->AddFast(")&");
        temp2->AddFast(")&");
    }
    if (XBody.Len != 0)
    {
        temp1->Append("%s&", XBody());
        temp2->Append("%s&", YBody());
    }
    temp1->Append("~%s&(", XHead[0]());
    temp2->Append("~%s&(", YHead[0]());
    for (int i=1; i<XHead.size(); i++)
    {
        temp1->Append("%s|", XHead[i]());
        temp2->Append("%s|", YHead[i]());
    }
    temp1->Buff[temp1->Len-1] = ')';
    temp2->Buff[temp2->Len-1] = ')';
    temp1->AddFast(")|");
    temp2->AddFast(")&");

    if (ranked)
    {
        for (int i=impLevels[numRule-1]; i<numLevel; i++)
        {
            TSmallStr &pref3 = prefs3[i];
            pref3.AddFast("(");
            if (XBody.Len != 0)
            {
                pref3.Append("((~%s|(", XBody());
                for (int j=0; j<XHead.size(); j++)
                {
                    pref3.Append("~%s&", XHead[j]());
                }
                pref3.Buff[pref3.Len-1] = ')';
                pref3.Append(")&(~%s|(", YBody());
                for (int j=0; j<XHead.size(); j++)
                {
                    pref3.Append("~%s&", YHead[j]());
                }
                pref3.Buff[pref3.Len-1] = ')';
                pref3.AddFast("))|");

                pref3.Append("((~%s|(", XBody());
                for (int j=0; j<XHead.size(); j++)
                {
                    pref3.Append("~%s&", XHead[j]());
                }
                pref3.Buff[pref3.Len-1] = ')';
                pref3.Append(")&%s&%s)|", YBody(), YHead[0]());

                pref3.Append("((~%s|(", YBody());
                for (int j=0; j<XHead.size(); j++)
                {
                    pref3.Append("~%s&", YHead[j]());
                }
                pref3.Buff[pref3.Len-1] = ')';
                pref3.Append(")&%s&%s)|", XBody(), XHead[0]());
                pref3.Append("(%s&%s&(", XBody(), YBody());
            }
            for (int j=0; j<XHead.size(); j++)
            {
                pref3.Append("(%s&%s&", XHead[j](), YHead[j]());
                for (int k=0; k<j; k++)
                {
                    pref3.Append("~%s&~%s&", XHead[k](), YHead[k]());
                }
                pref3.Buff[pref3.Len-1] = ')';
                if (j < XHead.size()-1)
                {
                    pref3.AddFast("|");
                }
            }
            if (XBody.Len != 0)
            {
                pref3.AddFast("))");
            }
            pref3.AddFast(")&");
        }
    }
    return true;
}
//-----------------------------------------------------------------------------
bool QBFSolver::ParseCNF(const char *&cursor)
{
    const char *yyMarker = NULL;
    const char *yyCTXMarker = NULL;

    XHead.push_back("(");
    TSmallStr &Xcomb = XHead.back();
    YHead.push_back("(");
    TSmallStr &Ycomb = YHead.back();

    for (const char *p=cursor;*p;p=cursor)
    {
/*!re2c
    DNF     {
                if (ParseDNF(p) == false)
                {
                    return false;
                }
                continue;
            }
    and|"("|")"
            {
                Xcomb.AddFast(p, cursor-p);
                Ycomb.AddFast(p, cursor-p);
                continue;
            }
    better  {   break;  }
    imply   {   break;  }
    [^]     {   break;  }
*/
    }
    Xcomb.AddFast(")");
    Ycomb.AddFast(")");
    return true;
}
//-----------------------------------------------------------------------------
bool QBFSolver::ParseDNF(const char *&cursor)
{
    const char *yyMarker = NULL;
    const char *yyCTXMarker = NULL;

    for (const char *p=cursor;*p;p=cursor)
    {
/*!re2c
    or|"("  {
                XHead.back().AddFast(p, cursor-p);
                YHead.back().AddFast(p, cursor-p);
                continue;
            }
    space   {   continue;   }
    "not"   {
                XHead.back().AddFast("~");
                YHead.back().AddFast("~");
                continue;
            }
    atom    {
                int index = 0;
                TSmallStr string;
                string.SetFast(p, cursor-p);
                for (int i=0; i<numAtom; i++)
                {
                    TSmallStr temp;
                    temp.Format("atom%d", i+1);
                    if (strcmp(string(), temp()) == 0)
                    {
                        index = i+1;
                        break;
                    }
                }
                XHead.back().Append(" x%d ", index);
                YHead.back().Append(" y%d ", index);
                continue;
            }
    ")"     {
                XHead.back().AddFast(p, cursor-p);
                YHead.back().AddFast(p, cursor-p);
                break;
            }
    and     {   break;  }
    better  {   break;  }
    [^]     {   break;  }
*/
    }
    return true;
}
//-----------------------------------------------------------------------------
bool QBFSolver::ParseBody(const char *&cursor)
{
    const char *yyMarker = NULL;
    const char *yyCTXMarker = NULL;

    XBody.Set("(");
    YBody.Set("(");

    for (const char *p=cursor;*p;p=cursor)
    {
/*!re2c
    imply   {   continue;   }
    "not"   {
                XBody.AddFast("~");
                YBody.AddFast("~");
                continue;
            }
    atom    {
                int index = 0;
                TSmallStr string;
                string.SetFast(p, cursor-p);
                for (int i=0; i<numAtom; i++)
                {
                    TSmallStr temp;
                    temp.Format("atom%d", i+1);
                    if (strcmp(string(), temp()) == 0)
                    {
                        index = i+1;
                        break;
                    }
                }
                XBody.Append(" x%d ", index);
                YBody.Append(" y%d ", index);
                continue;
            }
    ","     {
                XBody.AddFast("&");
                YBody.AddFast("&");
                continue;
            }
    "."     {   break;  }
    [^]     {   continue;   }
*/
    }
    XBody.AddFast(")");
    YBody.AddFast(")");
    return true;
}
//-----------------------------------------------------------------------------