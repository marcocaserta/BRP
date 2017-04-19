/***************************************************************************
 *   copyright (C) 2005 by Marco Caserta                                   *
 *   marco.caserta@itesm.mx                                                *
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

/*! \file options.cpp 
  \brief Read options from command line.

  Options are:
  - -f : problem instance file               [default = NONE]
  - -t : wall-clock time limit for execution [default =  180]
  - -d : horizontal corridor width           [default =  -1 ]
  - -v : vertical corridor width
  - -n : help (list of all options)
  - -c : constant vertical corridor          [default = 1   ]
*/

#include <iostream>
#include <vector>
#include <cstdlib>

/**********************************************************/
#define   TIME_LIMIT_def  60   //!< default wall-clock time limit
#define   DELTA_def       -1   //!< default horizontal width
#define   VCORR_def        1   //!< default vertical corridor
/**********************************************************/

using namespace std;


extern char* _FILENAME; 	//!< name of the instance file
extern int time_limit;
extern int max_ite;
extern int n;
extern int delta;
extern int constantV;

/// Parse command line options
int parseOptions(int argc, char* argv[])
{
   time_limit   = TIME_LIMIT_def;
   delta        = DELTA_def;
   constantV    = VCORR_def;
   bool setFile = false;
   bool setVert = false;

   if (argc == 1)
   {
      cout << "No options specified. Try ./scp -h " << endl;
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
	    case 'f':
	       _FILENAME = argv[i+1];
	       setFile = true;
	       i++;
	       break;
	    case 't':
	       time_limit = atol(argv[i+1]);
	       i++;
	       break;
	    case 'd':
	       delta = atol(argv[i+1]);
	       i++;
	       break;
	    case 'n':
	       n = atol(argv[i+1]);
	       setVert = true;
	       i++;
	       break;
	    case 'c':
	       constantV = atol(argv[i+1]);
	       i++;
	       break;
	    case 'h':
	       cout << "OPTIONS :: " << endl;
	       cout << "-f : problem instance file" << endl;
	       cout << "-d : horizontal corridor width" << endl;
	       cout << "-n : vertical corridor width" << endl;
	       cout << "-t : time limit (real)" << endl;
	       cout << "-c : constant vertical corridor (1 : true; 0 : false)" << endl;
	       cout << endl;
	       return -1;
	 }
      }
   }
 
   if (setFile && setVert)
      return 0;
   else
   {
      cout <<"Options -f and -n are mandatory. Try -h" << endl;
      return -1;
   }

}
