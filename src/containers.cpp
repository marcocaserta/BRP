/***************************************************************************
 *   Copyright (C) 2008 by Marco Caserta                                   *
 *   marco dot caserta at uni-hamburg dot de                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
/*! \mainpage Containers Terminal Relocation

  \author Marco Caserta 2008 (c) 
  \version v. 4.0.0
  \date Begins : 04.09.08
  \date Ends   : 05.09.08

  This project is concerned with the implementation of a dynamic programming
  algorithm for relocation of container terminals in a bay. The prolblem is 
  described in the paper <i>"A Heuristic Rule for Relocating Blocks"</i>, 
  <b>Computers & Operations Research</b> 33, 2006, 940--954. In this version,
  a hybrid scheme is introduced, in which the following new features are
used:
- stochastic corridor selection: the corridor is defined in a fashion similar
to what the CE method would do (random roulette) and stacks included into
the corridor need not be adjacent
- once the corridor/neighborhood is defined, we look for the best solution
in the neighborhood. The fitness value of each solution is computed by
using a "greedy" score. Once the best solution is found, we move from the
incumbent to such a solution (the corresponding move is executed). The
process is repeated until the final configuration is reached. The final
result is a trajectory from the initial configuration to the target
configuration (rather than a tree, we only keep one intermediate state
at a time)
- the process of trajectory generation is repeated until a predefined 
stopping criterion in matched

\section models Mathematical Models

A mathematical formulation for the problem is :

[TODO]

\section input Input Parameters

Parameters are read from command line. The following parameters can be defined:

- -f : filename of data file
- -d : horizontal corridor width (symmetric w.r.t. the stack in which the element
to be removed is placed)
- -c \f$ \in \left\{0, 1\right\} \f$, with the following meaning:
<ul>
<li> c 1 : constant vertical corridor (in this case, the value of n, set via 
-n numb, will be taken as the maximum height of each stack)
<li> c 2 : variable vertical corridor (what is kept constant now is the number
of empty spots in each stack; consequently, the max height of each
stack depends on the number of elements in the stack)
</ul>
- -n : this parameter adquire a different meaning, according to the value of
parameter c. If c = 1, then n is used as maximum height of a stack;
if c = 0, then n is used as the number of empty spots allowed on each
stack
- -t : CPU time limit for the algorithm (seconds). It stops after max time
limit is reached (no solution returned if the algorithm does not terminate)

\section modification Project Modifications History
\date 03.01.08 first version completed
\date 04.01.08 add corridor version (horizontal)
\date 27.01.08 add vertical corridor (\f$ \lambda_i \f$) 
\date 28.05.08 add embedded heuristic
\date 04.09.08 redesign heuristic scheme
\date 19.04.17 upon request from SV, recomputation of Tables from OR Spectrum 
paper. In doing that, I realized that the maximum height was 
sometimes not respected. I added a single line of code (within 
function "define_stochastic_corridor()".

In addition, I created a new data structure to keep track of the
path leading to best solution. If the bay configurations, from the
initial one to the empty bay, should be printed, activate the
directive #W_OUT.

More precisely, two arrays of bays are created. Each solution is composed of 
two parts: The relocations from 1 to k, and the heuristic (look ahead) relocations
from k to nEl. In other words, every time we try to relocate block "k", we evaluate
the goodness of such a move with a look-ahead mechanism (a simple heuristic) that 
tries to complete the solution from k+1 till the end. Thus, the objective 
function value of a solution is composed of a fixed number of relocations (from
1 to k) and a heuristic-based (say, expected, or upper bound) number of relocations
from k+1 onward. To account for these two portions, we use two data structures:
- path : this is the path from the original bay to the current configuration. It 
gets progressively enlarged as we move from 1 to nEl.
-heurPath : this is the second part of the solution, from k+1 to the end, as 
suggested by the heuristic scheme.
==> bestPath provides the final path from the initial configuration to the empty
bay, with total number of relations equal to best_z.

*/

/*! \file containers.cpp
  \brief Algorithm for Containers Terminal Relocation

  This program solves the problem of blocks relocation by finding
  the relocation pattern that minimizes the overall number of 
  relocation in a bay. Blocks are numbered according to their
  priority, i.e., blocks with lower numbers must be picked up
  before than blocks with higher numbers.

  \todo 
  -# 
  */

#include <iostream>
#include <limits>
#include <iomanip>
#include <fstream>
#include <cstring>
#include <ctime>
#include <cassert>
#include <cstdlib>
#include <cmath>
#include <vector>
#include "timer.h"
#include "options.h"
#include "heuristic.h"

//#define M_DEBUG	/*!< Comment this to remove debug */
#define W_OUT
// #define W_PATH
using namespace std;

/************************ Global Constants *******************/
const double _DINF      = numeric_limits<double>::max();    //!< Double Infinity
const long _MAXRANDOM   = numeric_limits<int>::max();       //!< Max Integer (2147483647)
const double _ZERO      = 0.0e0;			    //!< Double Zero
const double _ONE       = 1.0e0;			    //!< Double One
const double _EPSILON   = numeric_limits<float>::epsilon(); //!< Double \f$ \epsilon\f$ value
const char* RESULT_FILE = "result.dat";
/************************ Global Constants *******************/

//==============================================================
// Global Variables
//==============================================================
char * _FILENAME;               //!< Data file (read from command line)
std::vector< std::vector<int> > bay;
std::vector < std::vector< std::vector<int> > > path;
std::vector < std::vector< std::vector<int> > > bestPath;
int * lambda;
int m;				//!< Number of Stacks
int n;				//!< Max height of each Stack
int delta;			//!< Max horizontal width corridor
int nels;			//!< Total number of blocks in the bay
int constantV;			//!< Vertical corridor type (1 : constant; 0 : variable)
int best_z;			//!< Objective function value of best solution
double best_time;		//!< Time to best solution
int time_limit;			//!< Max time allowed
timer tTime;			//!< Ojbect clock to measure REAL and VIRTUAL (cpu) time
//==============================================================
void read_problem_data();	
void printing_parameters();	
int stopping_criterion();	
void print_bay(std::vector< std::vector<int> > bay);
bool found_element(int l, std::vector < std::vector <int> > node, int & row, int & col);
void update_best(int z, std::vector< std::vector<int> > bay, std::vector < std::vector< std::vector<int> > > path, std::vector < std::vector< std::vector<int> > > heurPath);
void define_stochastic_corridor(std::vector< std::vector <int> > state, int row, int * lambda, int delta, int constantV, bool * is_in_corridor, int h);
void normalize_scores(bool * is_in_corridor, int target, double * score_stack);
int  neighborhood_search(std::vector< std::vector <int> > state, int row, int h, int l, int z_cum);
void weight_assignment(double & w1, double & w2, double & w3, double tot_mins1, double tot_mins2, int n_empty_stacks);
void search_trajectory();
//===========================================================
//23456789012345678901234567890123456789012345678901234567890
//===========================================================
/// Main Program for Containers Terminal Relocation
/** Overall description of the algorithm goes here.
*/
int main(int argc, char *argv[])
{

    int err = parseOptions(argc, argv);
    if ( err != 0)
    {
        if (err != -1)
            cout << "Error argument " << err+1 << endl;
        exit(1);
    }

    ofstream fResult(RESULT_FILE, ios::out);
    if (!fResult)
    {
        cerr << "Cannot open file " << RESULT_FILE << endl;
        exit(1);
    }

    int random_seed = time(0);
    srand(random_seed);
    best_z = _MAXRANDOM;

    read_problem_data();
#ifdef W_OUT
    printing_parameters();
#endif
    tTime.resetTime();		// start clock

    bestPath.push_back(bay); // copy initial configuration

    while(!stopping_criterion())
    {
        search_trajectory(); 
        // print_bay(bay);
    }

#ifdef W_PATH
    cout << "Initial configuration and BEST PATH is :: " << endl;
    cout << "==================================================" << endl;  
    print_bay(bestPath[0]);
    cout << "==================================================" << endl;  
    for (int k = 1; k < bestPath.size(); k++) 
        print_bay(bestPath[k]);
#endif
#ifdef W_OUT
    cout <<"Algorithm terminates because time limit was reached. Best solution found requires " << best_z << " relocations." << endl;
#endif
    cout << "CM : Solution found with " << best_z << " moves." << endl;	
    fResult << setw(12) << _FILENAME << setw(4) << m << setw(4) << n << setw(4) 
        << nels << setw(12) << best_z << setw(8) 
        << delta << setw(6) << setprecision(3) 
        << best_time << endl;
    fResult.close();

    return 0;
}
//===========================================================
//234567890123456789 FUNCTIONS 123456789012345678901234567890
//===========================================================

/// Read instance file
/** The structure of the instance file is:
  - row 1 : number_of_stack (\c m) total_number_of_blocks (\c nels)
  - rows 2 to \c m + 1 : number_of_blocks in stack ... list of blocks in stack
  - EOF
  */
void read_problem_data()
{
    int temp, n_el;

    ifstream fdata(_FILENAME, ios::in);
    if (!fdata)
    {
        cerr << "Cannot open file " << _FILENAME << endl;
        exit(1);
    }

    fdata >> m;		// number of stacks
    if (delta == m) // if corridor width is equal to bay width, deactivate CM
        delta = -1;
    fdata >> nels;	// total number of elements

    std::vector<int> temp_vector;
    for (int i = 0; i < m; i++)
    {
        fdata >> n_el;
        for (int j = 0; j < n_el; j++)
        {
            fdata >> temp;
            temp_vector.push_back(temp);
        }      
        bay.push_back(temp_vector);
        temp_vector.clear();
    }
    fdata.close();
}


/// Print algorithmic parameters.
void printing_parameters()
{
    cout << "=========================================" << endl;
    cout << "* DYNA CONTAINERS v. 4.0.0              *" << endl;
    cout << "*                                       *" << endl;
    cout << "* Instance       : " << setw(20) << _FILENAME << setw(2) << "*" << endl;
    cout << "* No. Containers : " << setw(20) << nels << setw(2) << "*" << endl;
    cout << "* No. Stacks     : " << setw(20) << m << setw(2) << "*" << endl;
    cout << "* Type Vertical  : " << setw(20);
    if (constantV == 1) 
        cout << "C";
    else
        cout <<"NC";
    cout << setw(2) << "*" << endl;
    cout << "* Max Height     : " << setw(20) << n << setw(2) << "*" << endl;
    cout << "* Max Width      : " << setw(20) << delta << setw(2) << "*" << endl;
    cout << "* Max Time       : " << setw(20) << time_limit << setw(2) << "*" << endl;
    cout << "*                                       *" << endl;
    cout << "=========================================" << endl;
    cout << "* MC 2008 (c) -  UNI-HAMBURG            *" << endl;
    cout << "=========================================" << endl;
}

/// Define stopping criterion for the algorithm
/** The algorithm stops whenever one of the following 
  conditions is reached:
  1. time limit is reached (wall-clock time)
  */
int stopping_criterion()
{
    return (tTime.elapsedTime(timer::VIRTUAL) >= time_limit);
}


/// Print bay on screen
void print_bay(std::vector< std::vector <int> > bay)
{
    std::vector < int>::iterator sIt;

    for (int i = 0; i < m; i++)
    {
        cout << "stack " << setw(3) << i << setw(2) << "|";
        for (sIt = bay[i].begin(); sIt != bay[i].end(); sIt++)
            cout << setw(4) << *sIt;
        cout << endl;
    }
    cout << endl;

}

bool found_element(int l, std::vector < std::vector <int> > node, int & row, 
        int & col)
{
    bool found = false;
    row = -1;
    col = -1;
    for (int i = 0; i < node.size(); i++) 
    {
        for (int j = 0; j < node[i].size(); j++) 
        {
            if (node[i][j] == l)
            {
                found = true;
                row = i;
                col = j;
                break;
            }
        }
        if (found) break;

    }
    return found; 
}

/// Update best objective function value
void update_best(int z, std::vector< std::vector<int> > bay, 
        std::vector < std::vector< std::vector<int> > > path, 
        std::vector < std::vector< std::vector<int> > > heurPath)
{
    best_z = z;
    best_time = tTime.elapsedTime(timer::VIRTUAL);
#ifdef W_OUT
    cout << "***  After " << setw(8) << setprecision(3) << best_time << " seconds z :: " << best_z << endl;
#endif
    // save path of best solution
    bestPath.clear();
    bestPath.push_back(bay);

    for (int k = 0; k < path.size(); k++) 
        bestPath.push_back(path[k]);
    for (int k = 1; k < heurPath.size(); k++) 
        bestPath.push_back(heurPath[k]);
}

/// Define the size of the corridor using a greedy scheme
/** The element that must be relocated is found in
  \c state[row].back() and a "taylor-made" corridor for
  this element can be defined.

  In this version, the meaning of \f$ \delta \f$ changes, since
  it indicates the total number of stacks to be used.

  The idea is to compute a score for each stack in the bay (excluding
  the stack in which the item to be relocated is currently placed) and
  then to "stochastically" select \f$ \delta \f$ stacks with a
  success probability proportional to the score of the stack itself.

  Given the current block to be relocated, say \c el, we identify three
  types of stacks (according to whether they create deadlocks):
  -# [type I  ] empty stacks (no deadlock is ever created). We indicate with
  \f$ B_{I} \f$ the set of such stacks;
  -# [type II ] stacks for which the minimum block value is greated than \c el, 
  which is, \f$ \min (i) > el\f$ (no deadlock is created here). We indicate
  with \f$ B_{II} \f$ the set of such stacks;
  -# [type III] stacks for which \f$ \min (i) < el\f$, which is, if \c el 
  is placed on such a stack, a new deadlock is created.) We indicate
  with \f$ B_{III} \f$ the set of such stacks;

  Let us indicate with \c tot_mins1 the total sum of \f$ \min (i)\f$ of 
  stacks of type II, while we indicate with \c tot_mins2 the total sum of 
  \f$ \min (i) \f$ of stacks of type III. Furthermore, we count the
  number of empty stacks in \c n_empty_stacks. 

  We now compute the following score for each stack, according to the
  class (I, II, or III) the stack belongs to:
  \f[ s_{I} = \frac{1}{|B_{I}|} \f]
  \f[ s_{II} =  \frac{\displaystyle\sum_{i \in B_{II}} \min (i)}{\min (i)} \f]
  \f[ s_{III} = \frac{\min (i)}{\displaystyle \sum_{i \in B_{III}} \min (i)} \f]

  [NOTE: this part could be improved.] Finally, we need to normalize everything 
  to \f$ 1\f$ by attributing weights to each stack cathegory. We selected the 
  following weights:
  \f[ w_{I} = 0.25; \ w_{II} = 0.5; \ w_{III} = 0.25. \f]
  Now the total sum of the scores of the available stacks adds up to \f$ 1\f$.
  We implemented a <i>roulette-type</i> mechanism that, iteratively, selects which
  stacks should be added to the corridor, until the number of \f$ \delta\f$ 
  stacks is reached.

  \param state : current state of the bay
  \param row : stack in which the current target element is found
  \param delta : number of stacks in the corridor
  \param constantV : type of vertical corridor defined

  \return lambda : height limit for each stack
  \return is_in_corridor : true for each stack if stack is in current corridor
  */
void define_stochastic_corridor(std::vector< std::vector <int> > state, int row, int * lambda, int delta, int constantV, bool * is_in_corridor, int h)
{
    // initialization
    for (int i = 0; i < m; i++)
        is_in_corridor[i] = false;

    // full width corridor
    if (delta == -1)
        for (int i = 0; i < m; i++)
            is_in_corridor[i] = true;
    is_in_corridor[row] = false;

    if (constantV == 1)
        for (int i = 0;i < m; i++)
            lambda[i] = n;

    if (constantV == 1 && delta == -1)
        return;

    // (i) define horizontal corridor
    // a. compute stack scores
    int el = state[row].back();	// element to be relocated
    double * score_stack = new double[m];
    int * min_in_stack   = new int[m];
    score_stack[row] = _ZERO;
    int tot_mins1 = 0;
    int tot_mins2 = 0;
    int n_empty_stacks = 0;
    for (int i = 0; i < m; i++)
    {
        if (i == row) continue;
        // note: This was added on Apr. 19, 2017 (mail Silvia)
        // needed to respect max height (it seems it was a bug introduced
        // at a certain point in time). Thus, the maximum height was respected
        // by the heuristic, but not in this phase. See Excel file with
        // corrected results.
        if (state[i].size() >= h) continue;
        // find minumum block in the stack
        min_in_stack[i] = min_el_i(state, i);
        if (min_in_stack[i] == _MAXRANDOM)
            n_empty_stacks++;	// case I : empty stack
        else
            if (min_in_stack[i] > el)
                tot_mins1 += min_in_stack[i]; // case II : no deadlocks
            else
                tot_mins2 += min_in_stack[i]; // case III : deadlock
    }

    // compute stack score
    double den = _ZERO;
    for (int i = 0; i < m; i++)
    {
        if (state[i].size() >= h || i == row)
        {
            score_stack[i] = _ZERO;
            continue;
        }
        if (min_in_stack[i] == _MAXRANDOM)
            // case I : empty stack
            score_stack[i] = 1.0/(double)n_empty_stacks;
        else
            if (min_in_stack[i] > el)
            {
                // case II : no deadlocks
                score_stack[i] = (double)tot_mins1 / (double)min_in_stack[i];
                den += (double)tot_mins1 / (double)min_in_stack[i];
            }
            else
                // case III : deadlock
                score_stack[i] = (double)min_in_stack[i]/ (double)tot_mins2;
    }

    // [This part should be improved] Assign a weight to each stack type
    double w1 = _ZERO;
    double w2 = _ZERO;
    double w3 = _ZERO;
    weight_assignment(w1, w2, w3, tot_mins1, tot_mins2, n_empty_stacks);

    for (int i = 0; i < m; i++)
        if (min_in_stack[i] == _MAXRANDOM)
            score_stack[i] *= w1;	// case I : empty stack
        else
            if (min_in_stack[i] > el)
            {
                if (den != _ZERO)	// case II : no deadlocks
                    score_stack[i] /= den;
                score_stack[i] *= w2;
            }
            else
                score_stack[i] *= w3; // case III : deadlock

#ifdef M_DEBUG
    cout << "Printing stack scores :: " << endl;
    for (int i = 0; i < m; i++)
        cout << "s("<<i<<") = " << score_stack[i] << " and " << is_in_corridor[i] << endl;
#endif

    // b. randomly select stacks
    int n_selected = 0;
    while (n_selected < delta)
    {
        // scores must always be normalized to 1
        int rI = rand();
        double r = (double)rI/(double)_MAXRANDOM;
        int k = 0;
        while (is_in_corridor[k] == true || k == row && k < m)
            k++;
        double cum = score_stack[k];
        while (cum < r && k < m || k == row)
        {
            k++;
            if (is_in_corridor[k] == false)
                cum += score_stack[k];
        }
        // stack selection
        is_in_corridor[k] = true;
        n_selected++;
        // normalize scores
        if (n_selected < delta)
        {
            normalize_scores(is_in_corridor, k, score_stack);
            score_stack[k] = _ZERO;
#ifdef M_DEBUG
            cout << "Printing normalized stack scores :: " << endl;
            for (int i = 0; i < m; i++)
                cout << "s("<<i<<") = " << score_stack[i] << " and " << is_in_corridor[i] << endl;
#endif
        }

    }
#ifdef M_DEBUG   
    cout << "Selected Stacks :: (d = " << delta << ") " << endl;
    for (int i = 0; i < m; i++)
        if (is_in_corridor[i])
            cout << " " << i;
    cout << endl;
#endif
}

/// Normalize scores in such a way that total sum of stack score is \f$ 1\f$
/** This function is called after the selection of a previously unused stack.
  The stack \c target is added to the corridor and, consequently, the scores
  of the remaning stacks must be normalized to add up to \f$ 1\f$.
  */
void normalize_scores(bool * is_in_corridor, int target, double * score_stack)
{
    for (int i = 0; i < m; i++)
    {
        if (is_in_corridor[i] == true)
            continue;
        score_stack[i] /= (1.0 - score_stack[target]);
    }
}

/// Exaustive enumeration of all solutions in the current corridor/neighborhood
/** Given the current element to be relocated and the corresponding corridor,
  we evaluate all possible moves and identify the "best" move in the neighborhood.

  The quality of a move is determined via a greedy score that computes the
  total number of moves required to complete the retrieval process given 
  a specific configuration (see block_heuristic() for more details.)
  */
int neighborhood_search(std::vector< std::vector <int> > state, int row, int h, int l, int z_cum)
{
    std::vector< std::vector <int> > aux;	   //!< Node
    std::vector< std::vector <int> >::iterator it;
    std::vector< int > stack_aux;
    std::vector< int >::iterator stack_it;

#ifdef W_GRASP
    std::vector< int > scores;
    int tot_score = 0;
#endif

    lambda = new int[m];
    bool * is_in_corridor = new bool[m];
    define_stochastic_corridor(state, row, lambda, delta, constantV, is_in_corridor, h);

    // evaluate each possible move in the neighborhood
    int z_heur = _MAXRANDOM;
    int target = -1;
    for (int i = 0; i < m; i++)
    {
        if (!is_in_corridor[i]) continue;

        // copy state into auxiliary structure      
        aux.clear();
        for (it = state.begin(); it != state.end(); it++)
        {
            for (stack_it = (*it).begin(); stack_it != (*it).end(); stack_it++)
                stack_aux.push_back(*stack_it);
            aux.push_back(stack_aux);
            stack_aux.clear();

        }

        aux[i].push_back(aux[row].back());
        aux[row].pop_back();

        // now complete the solution using the heuristic
        std::vector < std::vector< std::vector<int> > > heurPath;
        int heur_value = block_heuristic(aux, m, h, nels, l, heurPath);
        // cout << "heur value is " << heur_value << endl;

#ifdef W_GRASP
        scores.push_back(heur_value);
        tot_score += heur_value;
#endif

        if (heur_value < z_heur)
        {
            z_heur = heur_value;
            target = i;
        }
        // count also the current relocation (+1)
        if ((heur_value + z_cum + 1) < best_z)
            update_best(heur_value + z_cum + 1, bay, path, heurPath);
    }

    return target;
}

/// Define a search trajectory from the incumbent bay to the final configuration
/** Given the current bay and the retrieval order, define a path that leads to
  the final configuration (all blocks are retrieved) minimizing the total
  number of relocations.
  */
void search_trajectory()
{
    std::vector< std::vector <int> > state;
    std::vector< std::vector <int> >::iterator it;
    std::vector< int > stack_aux;
    std::vector< int >::iterator stack_it;
    int row, col, n_rel;
    int h;
    bool no_relocations = true;

    if (constantV == 1)
        h = n;
    else
        h = bay[0].size() + n;


    int z_cum = 0;
    // copy bay into auxiliary structure      
    for (it = bay.begin(); it != bay.end(); it++)
    {
        for (stack_it = (*it).begin(); stack_it != (*it).end(); stack_it++)
            stack_aux.push_back(*stack_it);
        state.push_back(stack_aux);
        stack_aux.clear();
    }
    // retrieve one block at a time
    for (int l = 1; l < nels-1; l++)
    {
        // cout << "RETRIEVING block " << l << endl;
        // print_bay(state);
        if (stopping_criterion()) break;

        // find position of block to be retrieved
        if (!found_element(l, state, row, col))
        {
            cout << "Element " << l << " was not found in node: " << endl;
            print_bay(state);
            exit(-1);
        }
        assert(state[row][col] == l);
#ifdef M_DEBUG
        cout << "El " << l << " found in " << row << ", " << col << endl;
#endif
        // compute number of blocks to be relocated
        n_rel       = state[row].size() - col - 1;
        if (n_rel == 0)
        {  // no alternatives (the target block is already on top of the stack)
            state[row].pop_back();
            path.push_back(state);
            continue;
        }

        // relocate all elements on top of target block
        for (int nn = 0; nn < n_rel; nn++)
        {
            no_relocations = false;
            // explore neighborhood
            //int target_stack = neighborhood_search_grasp(state, row, h, l, z_cum);
            int target_stack = neighborhood_search(state, row, h, l, z_cum);
            //  relocate block (move)
            z_cum++;		// count current move
            // check if trajectory can be fathomed
            if (z_cum >= best_z)
            {
                // empty structure holding current path
                path.clear();
                return;
            }

            state[target_stack].push_back(state[row].back());
            state[row].pop_back();
        }
        // now remove element
        state[row].pop_back();
        path.push_back(state);
    }
    if (no_relocations && best_z > 0)
    {
        best_z = 0;
        best_time = -999;
    }
}

/// Assign a weight to each stack type (I, II, or III)
void weight_assignment(double & w1, double & w2, double & w3, double tot_mins1, double tot_mins2, int n_empty_stacks)
{
    if (tot_mins1 > _ZERO && tot_mins2 > _ZERO && n_empty_stacks > 0)
    {
        w1 = 0.25; w2 = 0.5; w3 = 0.25;
    }
    else
        if (tot_mins1 > _ZERO && tot_mins2 > _ZERO && n_empty_stacks == 0)
        {
            w1 = _ZERO; w2 = 2.0/3.0; w3 = 1.0 - w2;	 
        }
        else
            if (tot_mins1 > _ZERO && tot_mins2 ==  _ZERO && n_empty_stacks > 0)
            {
                w3 = _ZERO; w2 = 2.0/3.0; w1 = 1.0 - w2;	 
            }
            else
                if (tot_mins1 > _ZERO && tot_mins2 ==  _ZERO && n_empty_stacks == 0)
                {
                    w3 = _ZERO; w2 = 1.0; w1 = _ZERO;
                }
                else
                    if (tot_mins1 == _ZERO && tot_mins2 >  _ZERO && n_empty_stacks > 0)
                    {
                        w3 = 0.5; w2 = _ZERO; w1 = 0.5;
                    }
                    else
                        if (tot_mins1 == _ZERO && tot_mins2 >  _ZERO && n_empty_stacks == 0)
                        {
                            w2 = _ZERO; w3 = 1.0; w1 = _ZERO;
                        }
                        else
                            if (tot_mins1 == _ZERO && tot_mins2 ==  _ZERO && n_empty_stacks > 0)
                            {
                                w2 = _ZERO; w1 = 1.0; w3 = _ZERO;
                            }

}

