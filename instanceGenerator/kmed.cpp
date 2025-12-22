#include<iostream>
#include<fstream>
#include<cstdlib>
#include<cmath>

using namespace std;

int facilities, cities;

void export_problem(char *datei, double *open_cost, double *cost_matrix, int locations)
{ 
  int      cnt, facs;

  ofstream ausgabe;
  ausgabe.setf(ios::fixed);
  ausgabe.precision(0);

  facs = 0;
  ausgabe.open(datei, ios::out);
  
  if (ausgabe.good()) {
    //Simple format
      
    ausgabe << "FILE: " << datei << endl;
    ausgabe << locations << " " << locations << " 0 " << endl;
		
    for(facs = 0; facs < locations; facs++) {
      ausgabe << facs+1 << " " << open_cost[facs] << " ";
      for(cnt = 0; cnt < locations; cnt++)
	ausgabe << cost_matrix[(facs*locations)+cnt] << " ";
      ausgabe << endl;
    }

    ausgabe.close();
    cout << "Done exporting graph !\n";
  } // end if ausgabe.good
}


void build_random_graph(const int locations, const int denom, const bool make_metric, 
			double *open_cost, double *cost_matrix)
{
  int   i,j,ei;
 
  double *posix = new double [locations];
  double *posiy = new double [locations];

  cout << "Locations: " << locations << endl;
  
  if (!cost_matrix) {
    cout << "memory allocation failed!\n" ;
    exit(2);
  }

  for (i = 0; i < locations; i++) {  
    posix[i] = (double) rand() / RAND_MAX;
    posiy[i] = (double) rand() / RAND_MAX;
 
    open_cost[i] = (double) rand() / RAND_MAX;
    open_cost[i] = sqrt(locations) * ((double) 10000 / denom);
    open_cost[i] = (int) (open_cost[i]) + 1;
  }

  for (i = 0; i < locations; i++) {
    for (j = 0; j < locations; j++) {
      
      ei = (i*locations) + j;
      cost_matrix[ei] = 10000 * sqrt(pow(posix[i]-posix[j],2) + pow(posiy[i]-posiy[j],2));
      	
      if (make_metric) cost_matrix[ei] = ceil(cost_matrix[ei]);
      else cost_matrix[ei] = (int) (cost_matrix[ei] + 0.5);     
    }
  }     
  
  delete[] posix;
  delete[] posiy;
  cout << "Done building problem !" << endl;
  return ;
}


int main(int argc, char **argv)
{
  double *open_cost, *cost_matrix;

  if (argc < 5) {
    cout << "kmgen [LOCATIONS] [DENOM] [METRIC] [OUTPUT_FILE]\n All numeric data integer/boolean, please !\n\n";
    exit(0);
  }

  srand(1234567);
  
  int locations = atoi(argv[1]);
  int denom = atoi(argv[2]);
  bool metric = atoi(argv[3]);

  open_cost = new double [locations];
  cost_matrix = new double [locations*locations];
  
  build_random_graph(locations, denom, metric, open_cost, cost_matrix);
  export_problem(argv[4], open_cost, cost_matrix, locations);

  delete[] open_cost;
  delete[] cost_matrix;
}