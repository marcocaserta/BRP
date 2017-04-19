/***************************************************************************
 *   Copyright (C) 2007 by Marco Caserta                                   *
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

/*! \file heuristic.cpp
  \brief Heuristic Rule for the Block Relocation Problem

  This code receives as input a given bay and, applying the heuristic of 
  XXX, it solves to the end the relocation problem. This function is called
  by the Corridor Method Algorithm as a look-ahead mechanism.

*/
#include <iostream>
#include <vector>
#include <cassert>
#include <iomanip>
#include <limits>
#include <cstdlib>

using namespace std;
const long _MAXRANDOM   = numeric_limits<int>::max();       //!< Max Integer (2147483647)

int counter;

bool find_element(int l, std::vector < std::vector <int> > node, int & row, 
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

/* bool find_element(int l, std::vector < std::vector <int> > node, int & row, int & col)
 * {
 *     bool found = false;
 *     row = 0;
 *     col = 0;
 *
 *     std::vector < std::vector <int> >::iterator sIt;
 *     std::vector <int>::iterator pcol;
 *     sIt = node.begin();
 *     cout << "Node here is " << *sIt << endl;
 *     while (!found &&  sIt != node.end())
 *     {
 *         pcol = (*sIt).begin();
 *         col  = 0;
 *         while (*pcol != l && pcol != (*sIt).end())
 *         {
 *             pcol++;
 *             col++;
 *         }
 *
 *         if (pcol != (*sIt).end())
 *             found = true;
 *         else
 *         {
 *             do
 *             {
 *                 row++;
 *                 sIt++;
 *             }while (node[row].size() == 0);
 *         }
 *     }
 *     return found;
 * } */

/// Print bay on screen
void print_node(std::vector< std::vector <int> > bay, int m)
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



int chkemptystack(std::vector < std::vector <int> > bay, int m)
{
    int out = -1;
    for (int i = 0; i < m; i++)
    {
        if (bay[i].size() == 0)
            out = i;
    }
    return out;      
}

int min_el_i(std::vector < std::vector <int> > bay, int i)
{
    int min = _MAXRANDOM;
    for (unsigned j = 0; j < bay[i].size(); j++)
        if (bay[i][j] < min)
            min = bay[i][j];

    //assert(min != _MAXRANDOM);
    return min;
}

/// Compute a greedy score to find the new stack
int max_in_choosestack(int * choosestack, int el, int m)
{
    int max = -1;
    int pos = -1;
    int min_greater = 10000;
    int pos_min     = -1;
    for (int i = 0; i < m; i++)
    {
        if (choosestack[i] > el && choosestack[i] < min_greater)
        {
            min_greater = choosestack[i];
            pos_min     = i;
        }	 
        if (choosestack[i] > max)
        {
            max = choosestack[i];
            pos = i;
        }
    }
    assert(pos != -1);

    if (pos_min != -1)
        return pos_min;
    else
        return pos;
}

int block_heuristic(std::vector < std::vector <int> > bay, int m, int h, int nels, int k, std::vector < std::vector< std::vector<int> > > & heurPath)
{
    int ki, kj;

    counter = 0;
    // print_node(bay, m);
    heurPath.push_back(bay);

    while (k < nels)
    {
        
        if (!find_element(k, bay, ki, kj))
        {    
            print_node(bay, m);
            exit(-1);
        }
        // cout << "EL = " << k << " ki and ky are " << ki << " " << kj << endl;

        // last position 
        if (kj == (int)bay[ki].size() - 1)
        {
            bay[ki].pop_back();
            k++;	 
        }
        else
        {
            while (kj < (int)bay[ki].size() - 1)
            {
                int mptystack = chkemptystack(bay, m);
                if (mptystack > -1)
                {
                    bay[mptystack].push_back(bay[ki][bay[ki].size()-1]);
                    
                    bay[ki].pop_back();
                    counter++;
                }
                else
                {
                    int * choosestack = new int[m];
                    for (int i = 0; i < m; i++)
                        choosestack[i] = 0;

                    for (int i = 0; i < m; i++)
                    {
                        if (i == ki) continue;
                        if ((int)bay[i].size() < h)
                            choosestack[i] = min_el_i(bay, i);
                    }

                    int newi = max_in_choosestack(choosestack, bay[ki][bay[ki].size()-1], m);
                    bay[newi].push_back(bay[ki][bay[ki].size()-1]);
                    bay[ki].pop_back();
                    counter++;

                    delete [] choosestack;
                }
            }
            // cout << "counter now is " << counter << endl;

            assert(bay[ki][bay[ki].size()-1] == k);
            bay[ki].pop_back();
            k++;
        }
        // print_node(bay, m);
        heurPath.push_back(bay);
    }
    return counter;
}

