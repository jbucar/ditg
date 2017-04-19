/// \ingroup newran
///@{

/// \file utility.h
/// Some miscellanous functions I need.
///
/// The small ones (square and cube) are compiled inline so I give their
/// full details here.
///
/// The large one(s) (invchi95) are compiled separately so I give just the
/// definition and the actual code is in utility.cpp.
///
/// Also define the time lapse class.


// C and C++ don't provide the exponentiation operator (** in Fortran)
// so I provide a function for taking squares and cubes of numbers.

#ifndef UTILITY_H
#define UTILITY_H

#ifdef use_namespace
using namespace RBD_COMMON;
#endif


inline double square(double x) { return x*x; }  ///< return x squared.

inline double cube(double x) { return x*x*x; }  ///< return x cubed.

inline long iround(double x) { return (long)floor(x + 0.5); }
                                                ///< rounded version of x.


/// Calculate 95% point of chi-squared distribution.
double invchi95(int N);


/// Calculate 99% point of chi-squared distribution.
double invchi99(int N);


/// Calculate 99.9% point of chi-squared distribution.
double invchi999(int N);


/// Inverse of chisquared distribution.
Real invchi(Real p, int df, bool upper = false);


/// Cdf of normal distribution function.
double cdfnml(double x);

/// Inverse cdf of normal distribution function.
double invcdfnml(double p);


/// Chi-squared distribution function.
double cdf_chisq(double x, int df, bool upper=false);


/// Upper tail probabilities for 2 sided Kolmogorov-Smirnov test.
double KS_probabilities(double z);



/// Print time between construction and destruction.
class time_lapse
{
   double start_time;
public:
   time_lapse();
   ~time_lapse();
};





#endif


// body file: utility.cpp

///@}

