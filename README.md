*ASO: Answer Set Optimization*

This program implements the answer set optimization problem. It accepts ASP
constraints and preferences, computes optimal answer sets with Pareto principle
as the preference aggregation rule. It provides two different methods:

1. Iterative improvement method
2. Disjunctive logic programming method

and solves three problems:

1. Finding one optimal answer set
2. Given one answer set, finding a different optimal answer set
3. Given one answer set, finding an optimal answer set which is far from or
   close to the given answer set (the distance is measured by Hamming distance).

The ASP solvers used in this program include: clasp, claspD, and dlv.

Three data sets are provides to test the program with randomly generated 3-CNF
formulas with n atoms and 4n clauses as the constraints:

1. Randomly generated 3n preference rules without rank (n being as before the
   number of atoms)
2. Randomly generated 3n rules with two ranks, half of the rules having rank 1
   and half of the rules having rank 2
3. Preference rules extracted from two lexicographic preference trees (LP-trees,
   for short)

This code is published for research purpose only.

Copyright (c) Ying Zhu (isaswing AT gmail DOT com)

**REQUIREMENTS**

Only tested in Windows but code should be easily ported to Linux systems.

To build ASO, you need to first install [re2c](http://re2c.org) in your
system. Modify the re2c path in MAKEFILE. Then just `make`.

To run ASO, you need to place following binary files under the `bin/` folder:

- [clingo](http://potassco.sourceforge.net/) 4.4.0
- [Gringo](http://potassco.sourceforge.net/) 3.0.5
- [claspD](http://www.cs.uni-potsdam.de/claspD/)  1.1.2
- [claspD2](http://www.cs.uni-potsdam.de/claspD/) Rev. 6814
- [dlv](http://www.dlvsystem.com/dlv/) dlv-2012-12-17

**USAGE**

`./ASO solver method problem dataset [sim|dis distance]`

solver: [1|2|3|4]
    
if `method=1` or `method=2`:
    
- 1: clingo 4.4.0
- 2: gringo 3.0.5 + clasp 3.1.1
- 3: gringo 3.0.5 + claspD 1.1.2
- 4: gringo 3.0.5 + claspD2 Rev. 6814

if `method=3`:

- 1: gringo 3.0.5 + clasp 3.1.1
- 2: gringo 3.0.5 + claspD 1.1.2
- 3: gringo 3.0.5 + claspD2 Rev. 6814
- 4: dlv-2012-12-17

method: [1|2|3]

- 1: Iterative method
- 2: An alternative iterative method (only different for computing
  similar/dissimilar optimal answer set)
- 3: Disjunctive logic programming method

problem: [1|2|3]

- 1: Finding one optimal answer set
- 2: Given one answer set, finding a different optimal answer set
- 3: Given one answer set, finding a similar/dissimilar optimal answer set

dataset: [1|2|3]

- 1: dataset 1
- 2: dataset 2
- 3: dataset 3

dis: [sim|dis] This argument should only be used when `problem=3`.

- sim: Finding a dissimilar optimal answer set
- dis: Finding a similar optimal answer set
	 
distance: [0..10] This argument should only be used when `problem=3`.
     
- If `dis=sim`, it means finding an optimal answer set whose distance from the
  given answer set is smaller than or equal to distance*nAtoms/10.
- If `dis=dis`, it means finding an optimal answer set whose distance from the
  given answer set is greater than or equal to distance*nAtoms/10.

**EXAMPLES**

`./ASO 1 1 1 1`

Using iterative method to find one optimal answer set for dataset 1 with clingo
as the solver.

`./ASO 3 3 3 2 sim 5`

Given an answer set, using disjunctive logic programming method to find a
similar optimal answer set (distance is smaller than or equal to 50% of the
number of atoms) for dataset 2 with claspD2 as the solver.

**CITATION**

You need to cite the following paper if using this code:

<pre>
@incollection{Zhu2013
  title={On Optimal Solutions of Answer Set Optimization Problems},
  author={Zhu, Ying and Truszczynski, Miroslaw},
  booktitle={Logic Programming and Nonmonotonic Reasoning},
  year={2013},
  url={http://dx.doi.org/10.1007/978-3-642-40564-8_55},
}
</pre>
