
/*====================================================================

  C++ CLASS "INTERPOLATED FUNCTION". Loads, keeps and interpolates
  the array of 1D real function emploiyng the GSL subroutines.

  Alexey D. Kondorskiy,
  P.N.Lebedev Physical Institute of the Russian Academy of Science.
  E-mail: kondorskiy@lebedev.ru, kondorskiy@gmail.com.

====================================================================*/

//***** For testing. *************************************************
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <complex>
#include <string>
#include <math.h>
#include <sys/stat.h>
#include <vector>
#include <sstream>
//******************************************************************//

#include <gsl/gsl_errno.h>
#include <gsl/gsl_spline.h>

class InterpFunct
{
  /*******************************************************************
    PRIVATE MEMBERS.
  *******************************************************************/

  // GSL acceleration iterator.
  // Caches the previous value of an index lookup.
  private: gsl_interp_accel *acc;

  // GSL object to store dataset for 1D cubic spline interpolation.
  private: gsl_spline *spline;

  // Min & max values of the argument.
  private: double x_min, x_max;

  // Function values ath the argument interval borders.
  private: double y_x_min, y_x_max;

  // Flag to set f(x) = 1.
  private: bool is_unity;

  // Flag to notify object initialization.
  private: bool is_init;


  /*------------------------------------------------------------------
    Delete object.
  ------------------------------------------------------------------*/
  private: void clear()
  {
    if(is_init) {
      gsl_spline_free (spline);
      gsl_interp_accel_free (acc);
      x_min = 0.0;
      x_max = 0.0;
      y_x_min = 0.0;
      y_x_max = 0.0;
      is_unity = false;
      is_init = false; 
    }
  }

  /*------------------------------------------------------------------
    Read two column data from file.
  ------------------------------------------------------------------*/
  private: bool readTwoColumnData(
    const std::string &name,    // Name of the file to load the data.
    std::vector<double> &x,     // Result vector of arguments.
    std::vector<double> &y)     // Result vector of fcuntion values.
  {
    struct stat st;
    if(stat(name.c_str(), &st) != 0)
      return false;
    else {
      std::ifstream fin(name.c_str(), std::ios::in);
      while(!fin.eof()) {
        double tmp;
        fin >> tmp; if(!fin.eof()) x.push_back(tmp);
        fin >> tmp; if(!fin.eof()) y.push_back(tmp);
      }
      fin.close();
      return true;
    }
  }



  /*******************************************************************
    PUBLIC MEMBERS.
  *******************************************************************/

  /*------------------------------------------------------------------
    Constructor & Destructor.
  ------------------------------------------------------------------*/
  public: InterpFunct()
    { is_init = false; }

  public: ~InterpFunct()
    { clear(); }


  /*------------------------------------------------------------------
    Initialization by loading the data from file. Extended version.
  ------------------------------------------------------------------*/
  public: void init(
    const std::string &name,    // Name of file to load data.
    bool &is_ok)                // Flag to report success.
  {
    clear();

    // Read two column data from file (math_lib.cpp).
    std::vector<double> v_x, v_y;
    is_ok = readTwoColumnData(name, v_x, v_y);
    if( !is_ok )
      return;

    //////////////////////////////////////////////////////////////////
    // Perhaps ordering of the raw data should be added here.
    //////////////////////////////////////////////////////////////////

    // Prepare for the interpolation.
    int n = v_x.size();
    double *x = new double[n];
    double *y = new double[n];
    for (int i = 0; i < n; ++i) {
      x[i] = v_x[i];
      y[i] = v_y[i];
    }
    acc = gsl_interp_accel_alloc();
    spline = gsl_spline_alloc(gsl_interp_cspline, n);
    gsl_spline_init(spline, x, y, n);
    delete [] x, y;

    // Finalize initialization.
    x_min = v_x[0];
    x_max = v_x[n-1];
    y_x_min = v_y[0];
    y_x_max = v_y[n-1];
    is_unity = false;
    is_ok = true;
    is_init = true;
  }


  /*------------------------------------------------------------------
    Initialization by loading the data from file.
  ------------------------------------------------------------------*/
  public: void init(
    const std::string &name)    // Name of file to load data.
  {
    bool is_ok = false;
    init(name, is_ok);
    if(!is_ok) {
      std::cout << "Can not initialize interp_funct using file "
        << name << " !\n";
      exit(0);
    }
  }


  /*------------------------------------------------------------------
    Initialization by setting f(x) = 1.
  ------------------------------------------------------------------*/
  public: void init(
    const double &x_min_i,      // Min value of the argument.
    const double &x_max_i)      // Max value of the argument.
  {
    x_min = x_min_i;
    x_max = x_max_i;
    is_unity = true;
  }


  /*------------------------------------------------------------------
    Borders of the interval. Factors ensure that actual
    border values would be inside the interpolation range.
  ------------------------------------------------------------------*/
  public: double getXmin()
  {
    if(x_min > 0.0)
      return x_min*0.99999;
    else
      return x_min*1.00001;
   }

  public: double getXmax()
  {
    if(x_max > 0.0)
      return x_max*1.00001;
    else
      return x_max*0.99999;
  }


  /*------------------------------------------------------------------
    Interpolate function.
  ------------------------------------------------------------------*/
  public: double getY(const double &x)
  {
    if(is_unity)
      return 1.0;

    if(x < x_min)
      return y_x_min;
    else if(x > x_max)
      return y_x_max;
    else
      return gsl_spline_eval(spline, x, acc);
  }


}; //=================================================================



/*********************************************************************
  Test program.
*********************************************************************/
int main(int argc, char **argv)
{
  std::string file_name = "Xpol_src_ls_flux-wl.dat";

  InterpFunct intfun;
  intfun.init(file_name);
  double xi = intfun.getXmin();
  double xf = intfun.getXmax();

  std::string out_name = "reint-" + file_name;
  std::ofstream fout(out_name.c_str(), std::ios::out);
  for(int i = 0; i < 300; ++i) {
    double x = xi + i*(xf - xi)/300.0;
    fout << x << " " << intfun.getY(x) << "\n";
  }
  fout.close();

  return 0;
};


//==================================================================*/
