/***************************************************************************
 *   Copyright (C) 2007 by Marco Caserta                                   *
 *   caserta at econ dot uni-hamburg dot de                                *
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
/*! \file randomGen.cpp
  \brief Generation of random instances
  
*/

#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstring>
#include <ctime>

//#define M_DEBUG	/*!< Comment this to remove debug */

using namespace std;

/************************ Global Constants *******************/
const char* RESULT_FILE = "data/data_random.dat";
/************************ Global Constants *******************/

int parseOptionsRandom(int argc, char* argv[]);

//==============================================================
// Global Variables
//==============================================================
int m;				//!< Number of stacks
int h;				//!< Number of teils
int n;				//!< Total number of blocks
//===========================================================
//23456789012345678901234567890123456789012345678901234567890
//===========================================================
/// Main Program for Containers Terminal Relocation
/** Overall description of the algorithm
 */
int main(int argc, char *argv[])
{

   int err = parseOptionsRandom(argc, argv);
   if ( err != 0)
   {
      if (err != -1)
	 cout << "Error argument " << err+1 << endl;
      exit(1);
   }
   time_t start_time, cur_time;
   
   time(&start_time);
   do
   {
      time(&cur_time);
   }
   while((cur_time - start_time) < 2);

   int random_seed = time(0);
   srand(random_seed);

   n = m*h;
   int ** y = new (int*[m]);
   for (int i = 0; i < m; i++)
   {
      y[i] = new int[h];
      for (int j = 0; j < h; j++)
	 y[i][j] = -1;
   }
   // random generate coordinates
   for (int k = 1; k <= n; k++)
   {
      bool found = false;
      while (!found)
      {
	 int r = rand() % m;
	 int c = rand() % h;
	 if (y[r][c] == -1)
	 {
	    y[r][c] = k;
	    found = true;
	 }      
      }
   }

   ofstream fOutput(RESULT_FILE, ios::out);
   if (!fOutput)
   {
      cerr << "Cannot open file " << RESULT_FILE << endl;
      exit(1);
   }

   fOutput << m << " " << n << endl;
   for (int i = 0; i < m; i++)
   {
      fOutput << h;
      for (int j = 0; j < h; j++)
	 fOutput << " " << y[i][j]; 
      fOutput << endl;
   }

   fOutput.close();

   cout << "Random bay of size " << m << " x " << h << " has been generated." << endl;

   delete [] y;
   return 0;
}

/// Parse command line options
int parseOptionsRandom(int argc, char* argv[])
{
   bool setm = false;
   bool seth = false;

   if (argc == 1)
   {
      cout << "No options specified. Try ./ -l " << endl;
      return -1;
   }  
   
   int i = 0;
   while (++i < argc)
   {
      const char *option = argv[i];
      if (*option != '-')
	 return i;
      else if (*option == '\0')
	 return i;
      else if (*option == '-')
      {
	 switch (*++option)
	 {
	    case '\0':
	       return i + 1;
	    case 'm':
	       m = atol(argv[i+1]);
	       setm = true;
	       i++;
	       break;
	    case 'h':
	       h = atol(argv[i+1]);
	       seth = true;
	       i++;
	       break;
	    case 'l':
	       cout << "OPTIONS :: " << endl;
	       cout << "-m : number of stacks" << endl;
	       cout << "-h : number of tiers" << endl;
	       cout << endl;
	       return -1;
	 }
      }
   }
 
   if (setm && seth)
      return 0;
   else
   {
      cout <<"Options -m and -h are mandatory. Try -l" << endl;
      return -1;
   }

}
