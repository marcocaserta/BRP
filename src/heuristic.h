#ifndef heuristic_H
#define heuristic_H

int block_heuristic(std::vector < std::vector <int> > bay, int m, int h, int nels, int k, std::vector < std::vector< std::vector<int> > > & heurPath);
bool find_element(int l, std::vector < std::vector <int> > node, int & row, int & col);
int chkemptystack(std::vector < std::vector <int> > bay, int m);
int min_el_i(std::vector < std::vector <int> > bay, int i);
int max_in_choosestack(int * choosestack, int el, int m);
void print_node(std::vector< std::vector<int> > bay, int m);
#endif
