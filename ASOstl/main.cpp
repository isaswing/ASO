#include "ASPSolver.h"
#include "QBFSolver.h"

char *dir = "../DataGen/data/";
int looptimes = 3;
int timeout = 500;
/*
In ASPSolver:
solveMode = 1: Using clingo
solveMode = 2: Using gringo+clasp
solveMode = 3: Using gringo+claspD
solveMode = 4: Using gringo+claspD2
In QBFSolver:
solveMode = 1: Using gringo+clasp3
solveMode = 2: Using gringo+claspD
solveMode = 3: Using gringo+claspD2
solveMode = 4: Using dlv
*/
int solveMode = 1;

void GetInputFileName(TSmallStr &fileName, int dataset)
{
    if (dataset < 1 || dataset > 3)
    {
        printf("Please select a correct data set.\n");
        return;
    }
    fileName = dir;
    if (dataset == 1)
    {
        fileName += "oneRankData.in";
    }
    else if (dataset == 2)
    {
        fileName += "twoRanksData.in";    }
    else if (dataset = 3)
    {
        fileName += "twoTreesData.in";
    }
}

void GetOutputFileName(TSmallStr &fileName, int method, int problem, int dataset, bool dis = false)
{
    if (method < 1 || method > 3)
    {
        printf("Please select an available method.\n");
        return;
    }
    if (problem < 1 || problem > 3)
    {
        printf("Please select an available problem.\n");
        return;       
    }
    if (dataset < 1 || dataset > 3)
    {
        printf("Please select an available data set.\n");
        return;     
    }
    fileName = dir;
    if (problem == 1)
    {
        fileName += "one";
    }
    else if (problem == 2)
    {
        fileName += "ano";
    }
    else if (problem == 3)
    {
        if (dis)
        {
            fileName += "dis";
        }
        else
        {
            fileName += "sim";
        }
    }
    if (method == 1 || method == 2)
    {
        fileName += "Iterative";
    }
    else if (method == 3)
    {
        fileName += "DLP";
    }
    if (dataset == 1)
    {
        fileName += "1Rank.txt";
    }
    else if (dataset == 2)
    {
        fileName += "2Ranks.txt";
    }
    else if (dataset = 3)
    {
        fileName += "2Trees.txt";
    }
}
/*
The input is a txt file storing the paths of data instances (generator and 
preferences). For example "../DataGen/data/oneRankData.in".
method = 1 if using Iterative method (original for finding sim/dis).
method = 2 if using Iterative method (new method for finding sim/dis).
method = 3 if using disjunctive logic programming.
method = 4 if using disjunctive logic programming by DLV.
problem = 1 if finding one optimal answer set.
problem = 2 if finding another optimal answer set.
problem = 3 if finding a similar/dissimilar optimal answer set.
dataset = 1 if using the data set with one rank.
dataset = 2 if using the data set with two ranks.
dataset = 3 if using the data set generated from two LP-trees.
dis = true if finding a dissimilar optimal answer set.
dis = false if finding a similar optimal answer set.
The output is a txt file storing the time used to solve the problem for each 
data instance. For example: "../DataGen/data/oneIterative1Rank.txt".
*/
bool Solve(int method, int problem, int dataset, bool dis = false, int dist = 0)
{
    if (solveMode < 1 || solveMode > 4)
    {
        printf("Please select an available solver.\n");
        return true;  
    }
    if (method < 1 || method > 3)
    {
        printf("Please select an available method.\n");
        return true;
    }
    if (problem < 1 || problem > 3)
    {
        printf("Please select an available problem to solve.\n");
        return true;
    }    
    if (dataset < 1 || dataset > 3)
    {
        printf("Please select an available data set.\n");
        return true;
    }
    if (dist < 0 || dist > 10)
    {
        printf("Please input a valid distance.\n");
        return true;        
    }
    //input file
    TSmallStr fileName;
    GetInputFileName(fileName, dataset);
    FileIO file;
    if (file.Open(fileName(), "rb") == false)
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
    char *end = ptr;
    //output file
    GetOutputFileName(fileName, method, problem, dataset, dis);
    if (file.Open(fileName()) == false)
    {
        return false;
    }
    TSmallStr result;
    //for every file
    while (ptr)                                 
    {
        end = strstr(ptr, " ");
        TSmallStr gen;
        gen.SetFast(ptr, end-ptr);
        ptr = end + 1;
        end = strstr(ptr, " ");
        TSmallStr pref;
        pref.SetFast(ptr, end-ptr);
        TSmallStr imp;
        //If the data set has ranks, get the files storing ranks.
        if (dataset != 1)
        {
            ptr = end + 1;
            end = strstr(ptr, " ");
            imp.SetFast(ptr, end-ptr);
        }

        printf("generator: %s\n", gen());
        float minTime = timeout;
        int found = 0;
        //run three times and pick the fastest one
        for (int i=0; i<looptimes; i++)                         
        {
            float time = 0;
            //Iterative method
            if (method == 1 || method == 2)
            {
                ASPSolver ASO;
                if (ASO.Init(gen(), pref(), dataset != 1, imp()) == false)
                {
                    return false;
                }
                if (ASO.GetFirstOptAS(&time) == -1)
                {
                    return false;
                }
                //finding another optimal answer set
                if (problem == 2)
                {
                    if (ASO.GetAnoOptAS(&time) == -1)
                    {
                        return false;
                    }
                }
                //finding a similar/dissimilar optimal answer set
                else if (problem == 3)
                {
                    vector<bool> given = ASO.answerSet;
                    if (method == 1)
                    {
                        found = ASO.GetDisOpt(dis, given, (ASO.numAtom/10)*dist, &time);
                    }
                    else
                    {
                        found = ASO.GetDisOptNew(dis, given, (ASO.numAtom/10)*dist, &time);
                    }
                    if (found == -1)
                    {
                        return false;
                    }
                }
            }
            //The disjunctive logic programming method
            else
            {
                QBFSolver ASO;
                if (ASO.Parse(gen(), pref(), dataset != 1, imp()) == false)
                {
                    return false;
                }
                if (ASO.GetOneOptAS(&time) == -1)
                {
                    return false;
                }
                //finding another optimal answer set
                if (problem == 2)
                {
                    if (ASO.GetAnoOptAS(&time) == -1)
                    {
                        return false;
                    }
                }
                //finding a similar/dissimilar optimal answer set
                else if (problem == 3)
                {
                    vector<bool> given = ASO.answerSet;
                    if (ASO.GetDisOptAS(dis, given, (ASO.numAtom/10)*dist, &time) == -1)
                    {
                        return false;
                    }
                }
            }
            if (time > timeout)
            {
                printf("Time out!\n");
                return true;
            }
            if (minTime > time)
            {
                minTime = time;
            }
        }
        printf("minTime: %f\n", minTime);
        result.Format("%lf\n", minTime);
        if (file.Write(result(), 1, result.Len) == false)
        {
            return false;
        }
        file.Flush();

        ptr = end+1;
        while (*ptr == ' ')
        {
            ptr++;
        }
        if (*ptr == '\0')
        {
            break;
        }
        while (*ptr == '\n' || *ptr == '\r')
        {
            ptr++;
        }
    }
    file.Close();
    return true;
}

int main(int argc, char *argv[])
{
    if (argc < 5)
    {
        printf("Incorrect arguments!\n");
        return 1;
    }
    solveMode = atoi(argv[1]);
    int method = atoi(argv[2]);
    int problem = atoi(argv[3]);
    int dataset = atoi(argv[4]);
    bool dis = false;
    if (argc > 5)
    {
        dis = strcmp(argv[5], "dis") == 0;
    }
    int dist = 0;
    if (argc > 6)
    {
        dist = atoi(argv[6]);
    }
    if (Solve(method, problem, dataset, dis, dist) == false)
    {
        return 1;
    }

//Test code
    // solveMode = 3;
    //Using iterative method to find one optimal answer set from the dataset 
    //with one rank.
    // if (Solve(4, 1, 1) == false)
    // {
    //     return 1;
    // }
    //Using iterative method to find another optimal answer set from the dataset 
    //with one rank.
    // if (Solve(1, 2, 1) == false)
    // {
    //     return 1;
    // }
    //Using the original iterative method to find a dissimilar optimal answer 
    //set from the dataset with one rank.
    // if (Solve(1, 3, 1, true, 1) == false)
    // {
    //     return 1;
    // }
    //Using the original iterative method to find a similar optimal answer set 
    //from the dataset with one rank.
    // if (Solve(1, 3, 1, false, 3) == false)
    // {
    //     return 1;
    // }
    //Using the new iterative method to find a dissimilar optimal answer set 
    //from the dataset with one rank.
    // if (Solve(2, 3, 1, true) == false)
    // {
    //     return 1;
    // }
    //Using the new iterative method to find a similar optimal answer set from 
    //the dataset with one rank.
    // if (Solve(2, 3, 1, false) == false)
    // {
    //     return 1;
    // }
    //Using iterative method to find one optimal answer set from the dataset 
    //generated from two LP trees.
    // if (Solve(1, 1, 3) == false)
    // {
    //     return 1;
    // }    
    //Using disjunctive logic programming by claspD to find one optimal answer 
    //set from the dataset with one rank.
    // if (Solve(3, 1, 1) == false)
    // {
    //     return 1;
    // }
    //Using disjunctive logic programming by DLV to find one optimal answer 
    //set from the dataset with one rank.    
    // if (Solve(4, 1, 1) == false)
    // {
    //     return 1;
    // }
    // if (Solve(2, 3, 1, false, 6) == false)
    // {
    //     return 1;
    // }
    return 0;
}