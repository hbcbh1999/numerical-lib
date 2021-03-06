#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstdlib>
#include <cmath>
#include "math.h"
#include "lib/ode.h"

using namespace std;

/*
  data structure for the integraion methods:

  args contains the variables which will be integrated, so here omega, theta
    args[0] = omega
    args[1] = theta

  params contains the paramaters of the integration, so here t, q, f, big omega
    params[0] = t
    params[1] = q
    params[2] = f
    params[3] = big omega
*/

//returns dw/dt for the givven set of variables and parameters according to the structure above
double f_d_omega(double* args, double* params)
{
  const double omega = args[0];
  const double theta = args[1];

  const double t = params[0];
  const double q = params[1];
  const double f = params[2];
  const double big_omega = params[3];

  return -sin(theta) - q*omega + f*cos(big_omega * t);
}

//returns dtheta/dt for the givven set of variables and parameters according to the structure above
double f_d_theta(double* args, double* params)
{
  const double omega = args[0];

  return omega;
}


/*
calling argument structure:
  ./pendulum output_file_path q Omega f maximum_simulation_time time_step_quotient theta(0) omega(0) alternative_theta_remap_flag alternative_output_timing_flag

for the creation of the error comparison the program has been called like this:
  ./pendulum data_a.dat 0.3333 0.75 1.40 500 5000 0 0.75 1 1
  and
  ./pendulum data_b.dat 0.3333 0.75 1.40 500 500000 0 0.75 1 1

for the creation of the phase diagrams the program has been called like this:
  ./pendulum data_f_1.35.dat 0.3333 0.75 1.35 500 5000 0 0.75 1 1
  ./pendulum data_f_1.45.dat 0.3333 0.75 1.45 500 5000 0 0.75 1 1

to simulate the pendulum and save the values t_n, Theta(t_n) and omega(t_n) call the program like this:
  ./pendulum data_f_1.45.dat 0.3333 0.75 1.45 500 5000 0 0.75

Further information about the usage of the program can be obtained in the following few lines OR by reading the output of the program once it has been called
*/

int main(int argc, char* argv[])
{
  //print information for the usage
  cout << "Use this to solve the system of ODEs and to save the calculated values of theta, omega for the drivven, damped pendulum." << endl;
  cout << "Expects the follwing set of parameters:" << endl;

  cout << "\toutput file:\tThe path to the file in which the results of the bifurcation will be stored." << endl;
  cout << "\tq:\t\tThe damping parameter." << endl;
  cout << "\tOmega:\t\tThe frequency of the driving force of the pendulum." << endl;
  cout << "\tf:\t\tThe value of the strength of the force f." << endl;
  cout << "\tmaximum time:\tThe maximum time for the integration (should be high to reach period behaviour)" << endl;
  cout << "\tx:\t\tIndicates the time step size via delta_t = 2pi/(Omega * x)]" << endl << endl;
  cout << "\tTheta(0):\tThe boundary condition for the angle Theta]" << endl << endl;
  cout << "\tomega(0):\tThe boundary condition for the angular speed omega]" << endl << endl;
  cout << "\tremap theta alt. flag (default value = 0):\t If this is set to 1, then theta will be remapped to [-pi,pi] instead of [0,2pi] (this is useful for phase diagrams)" << endl;
  cout << "\talt. output timing flag (default value = 0):\t If this is set to 1, then not only the t_n but every 100th value will be saved (this is useful for phase diagrams)" << endl;

  cout << endl;

  //check if at least all necessary parameters have been submitted
  //if not, exit now
  if (argc < 7)
  {
    return -1;
  }

  //read input parameters

  //output file path
  char* file_path = argv[1];
  //parameter q
  double q = atof(argv[2]);
  //parameter Omega
  double big_omega = atof(argv[3]);
  //parameter f
  double f = atof(argv[4]);
  //the time until which the system will be simulated
  double max_t = atof(argv[5]);
  //the time step
  double delta_t = 2.0/big_omega * M_PI / atof(argv[6]);

  //the boundary conditions of the problem
  double theta_null = atof(argv[7]);// 0.0;
  double omega_null = atof(argv[8]);//big_omega;

  //if this is set to true, then theta will not be mapped into [0, 2pi] but [-pi,pi].
  //this is usefull for the creation and comparison of phase diagrams
  bool use_alternative_theta_mapping = false;

  //if this is set to true, then not only the values for t_n will be saved, but every 100th value
  //this is usefull for comparing the difference between to integrations with different time steps over the time
  bool use_alternative_output_timing = false;

  //check if these flags have been set
  if (argc == 10)
  {
      use_alternative_theta_mapping = atoi(argv[9]) == 1;
  }
  else if (argc == 11)
  {
    use_alternative_theta_mapping = atoi(argv[9]) == 1;
    use_alternative_output_timing = atoi(argv[10]) == 1;
  }

  //print entered parameters and settings for the user again before starting the calculation
  cout << "Entered parameters:" << endl;
  cout << "\toutput file:\t" << file_path << endl;
  cout << "\tq:\t\t." << q << endl;
  cout << "\tOmega:\t\t" << big_omega << endl;
  cout << "\tf:\t" << f << endl;
  cout << "\tmaximum time:\t" << max_t << endl;
  cout << "\tx:\t" << atof(argv[6]) << endl;
  cout << "\tTheta(0):\t" << theta_null << endl;
  cout << "\tomega(0):\t" << omega_null << endl;

  if (use_alternative_theta_mapping)
  {
     cout << "\tAlternative mapping of theta has been enabled." << endl;
  }
  if (use_alternative_output_timing)
  {
       cout << "\tAlternative output timing has been enabled." << endl;
  }

  //create output file stream
  ofstream outputFile;
  outputFile.open(file_path);
  outputFile << "#t" << "\t" << "omega" << "\t" << "theta" << endl;

  //create internal data for the RK4 integration implementation
  //set system of ODEs
  numerical::odeFunction* functions = new numerical::odeFunction[2];
  functions[0] = &f_d_omega;
  functions[1] = &f_d_theta;

  //create storage for the variables. will contain omega and theta according to the comment in the top
  double* rkValues = new double[2];
  //set start values
  rkValues[0] = omega_null;
  rkValues[1] = theta_null;

  //set the other parameters of the problem for the evaluation of the ODEs
  double* rkParams = new double[4];
  //rkParams[0] will contain the current time t
  rkParams[1] = q;
  rkParams[2] = f;
  rkParams[3] = big_omega;

  //gives the difference in t between to t_n values: print_interval_t = t_{n+1}-t_n
  double print_interval_t = 2.0/big_omega * M_PI;

  //is beeing used to store the time for which the values haven been saved the last time
  //use this value for the initialization, so that the first iteration will be used
  double last_printed_t = -print_interval_t;

  //this helper index is being used of the use_alternative_output_timing has been enabled. now not only the t_ns but every 100th value will be saved.
  //this is helpful to draw phasediagrams. set this to 100 to print the first value set.
  int last_printed_index = 100;
  for (double t=0; t<=max_t; t+=delta_t)
  {
    //to be sure that t = t_n we have to test if the interval between this time and the last time hast been 2*pi/Omega = print_interval_t
    if ((!use_alternative_output_timing  && t - last_printed_t >= print_interval_t) || (use_alternative_output_timing && last_printed_index == 100))
    {
      //set this index to zero to start counting again
      last_printed_index = 0;

      //memorize the current time
      last_printed_t = t;

      //print the data in the format
      //time  omega theta
      outputFile << t << "\t" << rkValues[0] << "\t" << rkValues[1] << endl;
    }

    //increase this hlper index
    last_printed_index++;

    //RK4 integration
    //set the time parameter
    rkParams[0] = t;

    //do one explicit RK4 integration step
    numerical::step_rk4_explicit(2, functions, delta_t, rkValues, rkParams);

    //remap the values of theta
    if (!use_alternative_theta_mapping)
    {
      //default remapping of theta into the interval [0,2*pi]
      while (rkValues[1] > 2*M_PI)
      {
        rkValues[1] -= 2*M_PI;
      }

      while (rkValues[1] < -0)
      {
        rkValues[1] += 2*M_PI;
      }
    }
    else
    {
      //alternative remapping of theta into the interval [-pi,pi]
      while (rkValues[1] > M_PI)
      {
        rkValues[1] -= 2*M_PI;
      }

      while (rkValues[1] < -M_PI)
      {
        rkValues[1] += 2*M_PI;
      }
    }
  }
}
