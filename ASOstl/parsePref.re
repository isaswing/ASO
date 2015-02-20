//---------------------------------------------------------------------------
#include "ASPSolver.h"
//---------------------------------------------------------------------------
//bool ParseRule(const char *&cursor, int rule);
//---------------------------------------------------------------------------
bool ASPSolver::ParsePref()
{
	//char buff[1024] = {0};
	FileIO input;
    if (input.Open(inputPref, "rb") == 0)
    {
		return false;
	}

	TSmallStr buff;
	buff.SetLen(input.size());
	input.Read(buff(), 1,input.size());
	input.Close();
	
    FileIO outputTest;
    if (outputTest.Open(testerTXT, "ab") == 0)
    {
		return false;
    }

	FileIO outputAnoGen;
    if (outputAnoGen.Open(genAnoTXT, "ab") == 0)
    {
		return false;
    }

	FileIO outputCalDeg;
    if (outputCalDeg.Open(calDegreeTXT, "ab") == 0)
    {
		return false;
    }
    
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
*/


//CNF1 = space* word space* (and space* word space*)*;
//CNF2 = space* "(" CNF1 ")" space*;
//CNF = (CNF1|CNF2);
//DNF = CNF (or CNF)*;
//rule = DNF (better DNF)* imply DNF;
//atom1 	= [a-zA-Z\_]+;


/*!re2c
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
					if (ParsePrefRule(p, outputTest, outputAnoGen, outputCalDeg) == false)
					{
						return false;
					}
					continue;
				}
				
	rule2		{
					numRule++;
					TSmallStr string;
					string.Format("body(%d).\n", numRule);
                    
					if (outputTest.Write(string(), 1, string.Len) == false || outputAnoGen.Write(string(), 1, string.Len) == false || outputCalDeg.Write(string(), 1, string.Len) == false)
					{
						return false;
					}		
					
					if (ParsePrefRule(p, outputTest, outputAnoGen, outputCalDeg) == false)
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
	
    TSmallStr string;
    string.Append("\nrule(1..%d).\n", numRule);
    if (outputTest.Write(string(), 1, string.Len) == false || outputAnoGen.Write(string(), 1, string.Len) == false || outputCalDeg.Write(string(), 1, string.Len) == false)
	{
		return false;
	}
	
	outputTest.Close();
	outputAnoGen.Close();
	outputCalDeg.Close();
	
	return true;
}
//---------------------------------------------------------------------------
bool ASPSolver::ParsePrefRule(const char *&cursor, FileIO &outputTest, FileIO &outputAnoGen, FileIO &outputCalDeg)
{
	const char *yyMarker = NULL;
    const char *yyCTXMarker = NULL;
	
	int count = 0;
	TSmallStr string;
	
	for (const char *p=cursor;*p;p=cursor)
	{
/*!re2c
	
	CNF			{
					numComb++;
					count++;

	                string.Format("heads(%d) :- comb(%d).\nv0(%d, %d) :- comb(%d), body(%d)", numRule, numComb, numRule, count, numComb, numRule);
                    for (int i=1; i<count; i++)
                    {
						string.Append(", not v0(%d, %d)", numRule, count-i);
                    }
                    string.Append(".\n");
                 
                    if (outputTest.Write(string(), 1, string.Len) == false || outputAnoGen.Write(string(), 1, string.Len) == false || outputCalDeg.Write(string(), 1, string.Len) == false)
					{
						return false;
					}

					if (ParseCNF(p, outputTest, outputAnoGen, outputCalDeg) == false)
					{
						return false;
					}
					continue;
				}	
	
	imply/body	
				{ 
					if (ParseBody(p, outputTest, outputAnoGen, outputCalDeg) == false)
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
//---------------------------------------------------------------------------
bool ASPSolver::ParseCNF(const char *&cursor, FileIO &outputTest, FileIO &outputAnoGen, FileIO &outputCalDeg)
{
	const char *yyMarker = NULL;
    const char *yyCTXMarker = NULL;
	
	TSmallStr line;
	line.Format("comb(%d) :- ", numComb);
	
	for (const char *p=cursor;*p;p=cursor)
	{
/*!re2c
	
	DNF/and	
			{
				numDNF++;
				line.Append("dnf(%d), ", numDNF);
				
				if (ParseDNF(p, outputTest, outputAnoGen, outputCalDeg) == false)
				{
					return false;
				}
				continue;
			}
	
	DNF	
			{
				numDNF++;
				line.Append("dnf(%d).\n", numDNF);
				
				if (ParseDNF(p, outputTest, outputAnoGen, outputCalDeg) == false)
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
	
	if (outputTest.Write(line(), 1, line.Len) == false || outputAnoGen.Write(line(), 1, line.Len) == false || outputCalDeg.Write(line(), 1, line.Len) == false)
	{
		return false;
	}
	
	return true;
}
//---------------------------------------------------------------------------
bool ASPSolver::ParseDNF(const char *&cursor, FileIO &outputTest, FileIO &outputAnoGen, FileIO &outputCalDeg)
{	
	const char *yyMarker = NULL;
    const char *yyCTXMarker = NULL;
	
	TSmallStr line;
	line.Format("dnf(%d) :- ", numDNF);

	for (const char *p=cursor;*p;p=cursor)
	{
/*!re2c

	"("		{	continue;	}
	
	or		{	
				line.Append("dnf(%d) :- ", numDNF);
				continue;	
			}
			
	space	{	continue;	}
	
	others
			{
//				TSmallStr line;
				
//				line.Format("v0(%d, %d) :- not heads(%d), body(%d).\n", numRule, count, numRule, numRule);
						
				// for (int i=0; i<atom.Number; i++)
				//for (int i=0; i<atom.size(); i++)
				for (int i=0; i<numAtom; i++)
				{
					line.Append("dnf(%d) :- atom(%d).\n", numDNF, i+1);
				}
				
//				if (outputTest.Write(line(), 1, line.Len) == false || outputAnoGen.Write(line(), 1, line.Len) == false || outputCalDeg.Write(line(), 1, line.Len) == false)
//				{
//					return false;
//				}
				continue;
			}
			
	atom2  	{
				//line.Format("dnf(%d) :- ", numDNF);
				line.AddFast(p, cursor-p);
				line.Append(".\n");
				continue;	
			}
	atom1
			{
				int index = 0;
				TSmallStr string;
				string.SetFast(p, cursor-p);
				// for (int i=0; i<atom.Number; i++)
				//for (int i=0; i<atom.size(); i++)
				for (int i=0; i<numAtom; i++)
				{
					TSmallStr comp;
					comp.Format("atom%d", i+1);
					//if (strcmp(string(), atom[i]()) == 0)
					if (strcmp(string(), comp()) == 0)
					{
						index = i+1;	
						break;
					}
				}
//				TSmallStr line;
				//line.Format("dnf(%d) :- atom(%d).\n", numDNF, index);
				line.Append("atom(%d).\n", index);
						
//				if (outputTest.Write(line(), 1, line.Len) == false || outputAnoGen.Write(line(), 1, line.Len) == false || outputCalDeg.Write(line(), 1, line.Len) == false)
//				{
//					return false;
//				}
				continue;
			}
			
	"not "
			{
				line.Append("not ");
//				line.Format("dnf(%d) :- not ", numDNF);
				
//				if (ParseNegAtom(p+4, line) == false)
//				{
//					return false;
//				}
//				line.Append(".\n");

//				int index = 0;
//				TSmallStr string;
//				string.SetFast(p+4, cursor-p-4);
//				for (int i=0; i<atom.Number; i++)
//				{
//					if (strcmp(string(), atom[i]()) == 0)
//					{
//						index = i+1;
//						break;
//					}
//				}
//				TSmallStr line;
//				line.Format("dnf(%d) :- not atom(%d).\n", numDNF, index);
						
//				if (outputTest.Write(line(), 1, line.Len) == false || outputAnoGen.Write(line(), 1, line.Len) == false || outputCalDeg.Write(line(), 1, line.Len) == false)
//				{
//					return false;
//				}
				continue;
			}
	
	")"		{	break;	}
	and		{	break;	}
	better	{	break;	}
	[^]		{	break;	}
*/
	}
	
	if (outputTest.Write(line(), 1, line.Len) == false || outputAnoGen.Write(line(), 1, line.Len) == false || outputCalDeg.Write(line(), 1, line.Len) == false)
	{
		return false;
	}
	
	return true;
}
//---------------------------------------------------------------------------
bool ASPSolver::ParseNegAtom(const char *&cursor, TSmallStr &line)
{
	const char *yyMarker = NULL;
    const char *yyCTXMarker = NULL;
	
//	TSmallStr line;
	
	for (const char *p=cursor;*p;p=cursor)
	{
/*!re2c

	atom2	{
				//line.Format("dnf(%d) :- not ", numDNF);
				line.AddFast(p, cursor-p);
				//line.Append(".\n");
				continue;
			}
	
	atom1 	{
				int index = 0;
				TSmallStr string;
				string.SetFast(p, cursor-p);
				// for (int i=0; i<atom.Number; i++)
				//for (int i=0; i<atom.size(); i++)
				for (int i=0; i<numAtom; i++)
				{					
					TSmallStr comp;
					comp.Format("atom%d", i+1);
					//if (strcmp(string(), atom[i]()) == 0)
					if (strcmp(string(), comp()) == 0)
					{
						index = i+1;
						break;
					}
				}
			
				//line.Format("dnf(%d) :- not atom(%d).\n", numDNF, index);
				line.Append("atom(%d)", index);
				continue;
			}

	[^]		{	break;	}
*/
	}
	
//	if (outputTest.Write(line(), 1, line.Len) == false || outputAnoGen.Write(line(), 1, line.Len) == false || outputCalDeg.Write(line(), 1, line.Len) == false)
//	{
//		return false;
//	}
	
	return true;
}
//---------------------------------------------------------------------------
bool ASPSolver::ParseBody(const char *&cursor, FileIO &outputTest, FileIO &outputAnoGen, FileIO &outputCalDeg)
{
	const char *yyMarker = NULL;
    const char *yyCTXMarker = NULL;
	
	TSmallStr line;
	line.Format("body(%d) :- ", numRule);

	for (const char *p=cursor;*p;p=cursor)
	{
/*!re2c

	imply	{	continue;	}
	
	"not"	{
				line.AddFast("not ");
				continue;					
			}
	
	atom		
			{	
				int index = 0;
				TSmallStr string;
				string.SetFast(p, cursor-p);
				// for (int i=0; i<atom.Number; i++)
				//for (int i=0; i<atom.size(); i++)
				for (int i=0; i<numAtom; i++)
				{
					TSmallStr comp;
					comp.Format("atom%d", i+1);
					//if (strcmp(string(), atom[i]()) == 0)
					if (strcmp(string(), comp()) == 0)
					{
						index = i+1;
						break;
					}
				}
				line.Append("atom(%d)", index);
				continue;
			}

/*!ignore:re2c			
	negAtom/space* ","	
			{
				int index = 0;
				TSmallStr string;
				string.SetFast(p+4, cursor-p-4);
				// for (int i=0; i<atom.Number; i++)
				//for (int i=0; i<atom.size(); i++)
				for (int i=0; i<numAtom; i++)
				{
					TSmallStr comp;
					comp.Format("atom%d", i+1);
					//if (strcmp(string(), atom[i]()) == 0)
					if (strcmp(string(), comp()) == 0)
					{
						index = i+1;
						break;
					}
				}
				line.Append("not atom(%d), ", index);
				continue;
			}		
			
	atom
			{
				int index = 0;
				TSmallStr string;
				string.SetFast(p, cursor-p);
				// for (int i=0; i<atom.Number; i++)
				//for (int i=0; i<atom.size(); i++)
				for (int i=0; i<numAtom; i++)
				{
					TSmallStr comp;
					comp.Format("atom%d", i+1);
					//if (strcmp(string(), atom[i]()) == 0)
					if (strcmp(string(), comp()) == 0)
					{
						index = i+1;
						break;
					}
				}
				line.Append("atom(%d).\n", index);
				continue;
			}

	negAtom
			{
				int index = 0;
				TSmallStr string;
				string.SetFast(p+4, cursor-p-4);
				// for (int i=0; i<atom.Number; i++)
				//for (int i=0; i<atom.size(); i++)
				for (int i=0; i<numAtom; i++)
				{
					TSmallStr comp;
					comp.Format("atom%d", i+1);
					//if (strcmp(string(), atom[i]()) == 0)
					if (strcmp(string(), comp()) == 0)
					{
						index = i+1;
						break;
					}
				}
				line.Append("not atom(%d).\n", index);
				continue;
			}
*/
	","		{
				line.AddFast(", ");
				continue;
			}
			
	"."		{	break;	}
	[^]		{	continue;	}
*/
	}
	
	line.AddFast(".\n");
	
	if (outputTest.Write(line(), 1, line.Len) == false || outputAnoGen.Write(line(), 1, line.Len) == false || outputCalDeg.Write(line(), 1, line.Len) == false)
	{
		return false;
	}
	
	return true;
}
