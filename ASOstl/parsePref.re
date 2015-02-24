//---------------------------------------------------------------------------
#include "ASPSolver.h"
//---------------------------------------------------------------------------
//bool ParseRule(const char *&cursor, int rule);
//---------------------------------------------------------------------------
bool ASPSolver::ParsePref()
{
	FileIO input;
    if (input.Open(inputPref, "rb") == 0)
    {
		return false;
	}
	TSmallStr buff;
	buff.SetLen(input.size());
	if (input.Read(buff(), 1,input.size()) == false)
	{
		return false;
	}
	input.Close();
	TSmallStr write;

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

eol 	= [\n|\r\n];
space 	= ([ ]|eol);
comment = "%";

number 	= "0"|([1-9][0-9]*);
var		= [A-Z]+;
arg		= (number|var) space* ("," space* (number|var) space*)*;

atom1 	= [a-zA-Z][a-zA-Z0-9\_]*;
atom2 	= atom1 "(" arg ")";
atom 	= (atom1|atom2);
negAtom = "not " atom;
literal = (atom|negAtom);
others  = "others";

and 	= "&";
or 		= "|";
imply 	= ":-";
better 	= ">";

setSize = "#" space* atom number "," number ".";

DNF1 	= space* literal space* (or space* literal space*)*;
DNF2 	= space* "(" DNF1 ")" space*;
DNF 	= (DNF1|DNF2);

CNF1 	= DNF (and DNF)*;
CNF2	= space* "(" CNF1 ")" space*;
CNF 	= (CNF1|CNF2);

body 	= space* literal space* ("," space* literal space*)*;

rule1 	= CNF (better CNF)+ imply body ".";
rule2 	= CNF (better CNF)+ ".";
rule 	= (rule1|rule2);
*/

	for (const char *p=cursor;*p;p=cursor)
	{
/*!re2c
	space		{ continue; }
	setSize		{ continue; }
	rule1		{
					numRule++;
					if (ParsePrefRule(p, write) == false)					{
						return false;
					}
					continue;
				}
	rule2		{
					numRule++;
					write.Append("body(%d).\n", numRule);
					if (ParsePrefRule(p, write) == false)
					{
						return false;
					}
					continue;					
				}				
	comment		{
					while (*cursor != '\n')
					{
						cursor++;
					}
					continue;
				}
	[^]			{ break; }
*/
	}
	
	write.Append("\nrule(1..%d).\n", numRule);
    FileIO file;
    if (file.Open(testerTXT, "ab") == 0)
    {
		return false;
    }
	if (file.Write(write(), 1, write.Len) == false)
	{
		return false;
	}
	file.Close();
    if (file.Open(genAnoTXT, "ab") == 0)
    {
		return false;
    }
	if (file.Write(write(), 1, write.Len) == false)
	{
		return false;
	}
	file.Close();
    if (file.Open(calDegreeTXT, "ab") == 0)
    {
		return false;
    }
   	if (file.Write(write(), 1, write.Len) == false)
	{
		return false;
	}
	file.Close();
	return true;
}
//-----------------------------------------------------------------------------
bool ASPSolver::ParsePrefRule(const char *&cursor, TSmallStr &write)
{
	const char *yyMarker = NULL;
    const char *yyCTXMarker = NULL;
	int count = 0;	
	for (const char *p=cursor;*p;p=cursor)
	{
/*!re2c
	CNF			{
					numComb++;
					count++;
                 	write.Append("heads(%d) :- comb(%d).\n"
                 				 "v0(%d, %d) :- comb(%d), body(%d)", 
                 				 numRule, numComb, 
                 				 numRule, count, numComb, numRule);
                    for (int i=1; i<count; i++)
                    {
						write.Append(", not v0(%d, %d)", numRule, count-i);
                    }
                    write.AddFast(".\n");
					if (ParseCNF(p, write) == false)
					{
						return false;
					}
					continue;
				}		
	imply/body	{ 
					if (ParseBody(p, write) == false)
					{
						return false;
					}
					continue; 
				}
	"."			{ break; }
	[^]			{ continue; }
*/
	}
	return true;
}
//-----------------------------------------------------------------------------
bool ASPSolver::ParseCNF(const char *&cursor, TSmallStr &write)
{
	const char *yyMarker = NULL;
    const char *yyCTXMarker = NULL;	
	TSmallStr line;
	line.Format("comb(%d) :- ", numComb);
	for (const char *p=cursor;*p;p=cursor)
	{
/*!re2c
	DNF/and	{
				numDNF++;
				line.Append("dnf(%d), ", numDNF);
				if (ParseDNF(p, write) == false)
				{
					return false;
				}
				continue;
			}	
	DNF		{
				numDNF++;
				line.Append("dnf(%d).\n", numDNF);
				if (ParseDNF(p, write) == false)
				{
					return false;
				}
				break;
			}	
	and		{	continue;	}
	better	{	break;	}
	imply	{	break;	}
	[^]		{	continue;	}
*/
	}
	write += line;	
	return true;
}
//-----------------------------------------------------------------------------
bool ASPSolver::ParseDNF(const char *&cursor, TSmallStr &write)
{	
	const char *yyMarker = NULL;
    const char *yyCTXMarker = NULL;
	write.Append("dnf(%d) :- ", numDNF);
	for (const char *p=cursor;*p;p=cursor)
	{
/*!re2c
	"("		{	continue;	}
	or		{	
				write.Append("dnf(%d) :- ", numDNF);
				continue;	
			}			
	space	{	continue;	}
	others	{
				for (int i=0; i<numAtom; i++)
				{
					write.Append("dnf(%d) :- atom(%d).\n", numDNF, i+1);
				}
				continue;
			}			
	atom2  	{
				write.AddFast(p, cursor-p);
				write.AddFast(".\n");
				continue;	
			}
	atom1	{
				int index = 0;
				TSmallStr string;
				string.SetFast(p, cursor-p);
				for (int i=0; i<numAtom; i++)
				{
					TSmallStr comp;
					comp.Format("atom%d", i+1);
					if (strcmp(string(), comp()) == 0)
					{
						index = i+1;	
						break;
					}
				}
				write.Append("atom(%d).\n", index);
				continue;
			}			
	"not "	{
				write.AddFast("not ");
				continue;
			}
	")"		{	break;	}
	and		{	break;	}
	better	{	break;	}
	[^]		{	break;	}
*/
	}	
	return true;
}
//-----------------------------------------------------------------------------
bool ASPSolver::ParseBody(const char *&cursor, TSmallStr &write)
{
	const char *yyMarker = NULL;
    const char *yyCTXMarker = NULL;
	write.Append("body(%d) :- ", numRule);
	for (const char *p=cursor;*p;p=cursor)
	{
/*!re2c
	imply	{	continue;	}
	"not"	{
				write.AddFast("not ");
				continue;					
			}	
	atom	{	
				int index = 0;
				TSmallStr string;
				string.SetFast(p, cursor-p);
				for (int i=0; i<numAtom; i++)
				{
					TSmallStr comp;
					comp.Format("atom%d", i+1);
					if (strcmp(string(), comp()) == 0)
					{
						index = i+1;
						break;
					}
				}
				write.Append("atom(%d)", index);
				continue;
			}
	","		{
				write.AddFast(", ");
				continue;
			}
	"."		{	break;	}
	[^]		{	continue;	}
*/
	}
	write.AddFast(".\n");	
	return true;
}
//-----------------------------------------------------------------------------
