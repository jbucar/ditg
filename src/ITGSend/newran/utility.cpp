/// \ingroup newran
///@{

/// \file utility.cpp
/// Some miscellanous functions I need - bodies.

#define WANT_MATH
#define WANT_STREAM
#define WANT_TIME

#include "include.h"
#include "myexcept.h"
#include "utility.h"
#include "format.h"

#ifdef use_namespace
using namespace RBD_STRING;
#endif



double invchi95(int N)
// upper 95% point of chi-squared distribution
{
   if (N < 0)
      Throw(Invalid_argument("Error in invchi95 arg"));
   if (N < 30)
   {
      double Q[] = { 0, 3.841459, 5.991465, 7.814728, 9.487729, 11.0705,
         12.59159, 14.06714, 15.50731, 16.91898, 18.30704, 19.67506,
         21.02601, 22.36199, 23.68475, 24.99576, 26.2962, 27.58709,
         28.86928, 30.14351, 31.4104, 32.6705, 33.9244, 35.1725,
         36.4151, 37.6525, 38.8852, 40.1133, 41.3372, 42.5569 };
      return Q[N];
   }
   else
   {
      double A = 1.0/(4.5 * N); double H = (-0.0002 * 60)/N;
      double Q = N * cube(1 - A + (1.645 - H) * sqrt(A));
      return Q;
   }
}

double invchi99(int N)
// upper 99% point of chi-squared distribution
{
   if (N < 0)
      Throw(Invalid_argument("Error in invchi99 arg"));
   if (N < 30)
   {
      double Q[] = { 0, 6.63490, 9.21034, 11.3449, 13.2767, 15.0863,
         16.8119, 18.4753, 20.0902, 21.6660, 23.2093, 24.7250,
         26.2170, 27.6883, 29.1413, 30.5779, 31.9999, 33.4087,
         34.8053, 36.1908, 37.5662, 38.9321, 40.2894, 41.6384,
         42.9798, 44.3141, 45.6417, 46.9630, 48.2782, 49.5879 };
      return Q[N];
   }
   else
   {
      double A = 1.0/(4.5 * N); double H = (0.0008 * 60)/N;
      double Q = N * cube(1 - A + (2.326 - H) * sqrt(A));
      return Q;
   }
}

double invchi999(int N)
// upper 99.9% point of chi-squared distribution
// not perfect but good enough here
{
   if (N < 0)
      Throw(Invalid_argument("Error in invchi999 arg"));
   if (N < 30)
   {
      double Q[] = { 0, 10.828, 13.816, 16.266, 18.467, 20.515,
         22.458, 24.322, 26.125, 27.877, 29.588, 31.264,
         32.909, 34.528, 36.123, 37.697, 39.252, 40.790,
         42.312, 43.820, 45.315, 46.797, 48.268, 49.728,
         51.179, 52.620, 54.052, 55.476, 56.892, 58.302 };

      return Q[N];
   }
   else
   {
      double A = 1.0/(4.5 * N); double H = (0.0045 * 60)/N;
      double Q = N * cube(1 - A + (3.090 - H) * sqrt(A));
      return Q;
   }
}

// cdf of normal distribution function
// From A.R. Curtis: In Methods of Numerical Approximation (1966)
// edited by D.C. Handscomb
// maximum relative error is bounded, but not good near x = 0
double cdfnml(double x)
{
   double c[]={ .592364,1.44485,-.193396,2.66832,3.73438,-1.73883 };
   double ax=fabs(x);
   double cdf=exp(-.5*square(ax))*(c[0]/(ax+c[1])+c[2]*(ax+c[5])
      /((ax+c[3])*ax+c[4]));
   if(x>=0.0) return  1.0-cdf; else return cdf;
}

// inverse of cdf of normal distribution function
// Abramowitz & Stegun,  26.2.23
double invcdfnml(double p)
{
   double c[] = { 2.515517, 0.802853, 0.010328 };
   double d[] = { 1, 1.432788, 0.189269, 0.001308 };
   bool small = true;
   if (p > 0.5) { p = 1.0 - p; small = false; }
   if (p < 0.0) Throw(Invalid_argument("Error in invcdfnml arg"));
   double t = sqrt(-2.0*log(p));
   double   x = t - (c[0] + t * (c[1] + t * c[2]))
      / (d[0] + t * (d[1] + t * (d[2] + t * d[3])));
   if (small) return - x;  else return x;
}



//*********************** chi-squared cdf ************************************

static double ln_gamma(double xx)
{
   // log gamma function adapted from numerical recipes in C

   if (xx<1.0)                           // Use reflection formula
   {
      double piz = 3.14159265359 * (1.0-xx);
      return log(piz/sin(piz))-ln_gamma(2.0-xx);
   }
   else
   {
      static double cof[6]={76.18009173,-86.50532033,24.01409822,
         -1.231739516,0.120858003e-2,-0.536382e-5};

      double x=xx-1.0; double tmp=x+5.5;
      tmp -= (x+0.5)*log(tmp); double ser=1.0;
      for (int j=0;j<=5;j++) { x += 1.0; ser += cof[j]/x; }
      return -tmp+log(2.50662827465*ser);
   }
}



// method good for x >> df
// also use for small df, but should sum in other direction
// need to do more work on accuracy
static Real cdf_chisq_large(Real x, int df, bool upper)
{
   Real df2 = df / 2.0; Real x2 = x / 2.0;
   Real term = exp((df2-1) * log(x2) - x2 - ln_gamma(df2));
   Real sum = term;
   if (sum == 0) return upper ? 0.0 : 1.0;
   for (;;)
   {
      df -= 2;
      if (df == 1)
         { sum += 2.0 * cdfnml(-sqrt(x)); return upper ? sum : 1.0 - sum; }
      Real ratio = df / x; term *= ratio; sum += term;
      if (ratio < 1.0)
      {
         if (upper) { if (term < 0.00001 * sum * (1 - ratio)) return sum; }
         else if (term < 0.000001 * (1 - ratio)) return 1.0 - sum;
      }
   }
}

// method good for x << df - need to do more work on accuracy
static Real cdf_chisq_small(Real x, int df, bool upper)
{
   df += 2;
   Real df2 = df / 2.0; Real x2 = x / 2.0;
   Real term = exp((df2-1) * log(x2) - x2 - ln_gamma(df2));
   Real sum = term;
   if (sum == 0) return upper ? 1.0 : 0.0;
   for (;;)
   {
      Real ratio = x / df; df += 2; term *= ratio; sum += term;
      if (ratio < 1.0)
      {
         if (upper) { if (term < 0.000001 * (1 - ratio)) return 1.0 - sum; }
         else if (term < 0.00001 * sum * (1 - ratio)) return sum;
      }
   }
}

// method good for x << 1, reduces round-off error for very low x
static Real cdf_chisq_very_small(Real x, int df)
{
   Real df2 = df / 2.0; Real x2 = x / 2.0;
   Real term = 2.0 * exp(df2 * log(x2) - ln_gamma(df2)); Real sum = term / df;
   if (sum == 0) return 0.0;
   for (int i = 1;; i++)
   {
      df += 2; term *= - x2 / i; Real addon = term / df;  sum += addon;
      if (fabs(addon) < 0.00001 * sum) return sum;
   }
}


// evaluate cdf of central chi-squared rv;
// do upper tail if upper is true
// need to set accuracy better but this is good enough here
// need to upgrade cdfnml
// inefficient for large df, x near df.

Real cdf_chisq(Real x, int df, bool upper)
{
   if (df <= 0) Throw(Out_of_range("Non-positive df in cdf_chisq"));
   if (x <= 0) return upper ? 1.0 : 0.0;
   if (x <= 0.5)
   {
      // better for !upper but cdfnml is giving suffient error for
      // upper to also be a problem with odd df
      Real sum = cdf_chisq_very_small(x, df);
      return upper ? 1.0 - sum : sum;
   }
   if (df == 1)
      { Real sum = 2.0 * cdfnml(-sqrt(x)); return upper ? sum : 1.0 - sum; }
   if (df == 2)
      { Real sum = exp(- x / 2); return upper ? sum : 1.0 - sum; }
   // would like to do this with larger df but cdfnml is a problem
   if (df < 3 || x > df) return cdf_chisq_large(x, df, upper);
   return cdf_chisq_small(x, df, upper);
}


// upper tail probabilities for 2 sided Kolmogorov-Smirnov test
// Stuart & Ord: Advanced Theory of Statistics, vol2 p1188

double KS_probabilities(double z)
{
   if (z <= 0)
      Throw(Out_of_range("Non-positive argument in KS_probabilities"));
   double sum = 0.0; int sign = 1;
   for (int r = 1; r <= 10000; r++)
   {
      double term = exp(-2.0 * square(r * z));
      sum += sign * term; sign = - sign;
      if (term <= 0.000001) return 2.0 * sum;
   }
   Throw(Runtime_error("Convergence error in KS_probabilities"));
#ifndef UseExceptions
   return -1;
#endif
}


//************** elapsed time class ****************

time_lapse::time_lapse()
{
   start_time = ((double)clock())/(double)CLOCKS_PER_SEC;
}

time_lapse::~time_lapse()
{
   Format F; F.FormatType(Format::DEC_FIGS); F.Precision(1);
   double time = ((double)clock())/(double)CLOCKS_PER_SEC - start_time;
   cout << "Elapsed (processor) time = " << F << time << " seconds" << endl;
   cout << endl;
}



///@}

