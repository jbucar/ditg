/// \ingroup newran
///@{

/// \file tryrand5.cpp
/// Test of newran - "Vari-" generators.

#define WANT_STREAM
#define WANT_MATH

#include "include.h"
#include "newran.h"
#include "format.h"
#include "tryrand.h"
#include "utility.h"



// ****** simple tests of Poisson, binomial and log normal random number generators ******

void TestVariPoisson(Real mu, int N)
{
   VariPoisson VP;                  // Poisson RNG
   Real sum = 0.0, sumsq = 0.0;
   for (int i = 1; i <= N; ++i)
      { int x = VP.iNext(mu); Real d = x - mu; sum += d; sumsq += d * d; }
   Real p_mean = mu;
   Real p_var = mu;
   Real s_mean = mu + sum / N;
   Real s_var = (sumsq - sum * sum / N) / (N - 1);
   Format F1;
   F1.FormatType(Format::SIG_FIGS); F1.Precision(5); F1.Width(10); F1.Suffix(" ");
   Format F2;
   F2.FormatType(Format::DEC_FIGS); F2.Precision(5); F2.Width(10); F2.Suffix(" ");
   cout << F1 << p_mean << s_mean;
   cout << F2 << (s_mean - p_mean) / sqrt(p_var / N);
   cout << F1 << p_var << s_var;
   cout << F2 << s_var / p_var;
   cout << endl;
}

void TestVariBinomial(int n, Real p, int N)
{
   VariBinomial VB;                // Binomial RNG
   Real sum = 0.0, sumsq = 0.0;
   Real mu = n * p;
   for (int i = 1; i <= N; ++i)
      { int x = VB.iNext(n, p); Real d = x - mu; sum += d; sumsq += d * d; }
   Real p_mean = mu;
   Real p_var = mu * (1.0 - p);
   Real s_mean = mu + sum / N;
   Real s_var = (sumsq - sum * sum / N) / (N - 1);
   Format F1;
   F1.FormatType(Format::SIG_FIGS); F1.Precision(5); F1.Width(10); F1.Suffix(" ");
   Format F2;
   F2.FormatType(Format::DEC_FIGS); F2.Precision(5); F2.Width(10); F2.Suffix(" ");
   cout << F1 << p_mean << s_mean;
   cout << F2 << (s_mean - p_mean) / sqrt(p_var / N);
   cout << F1 << p_var << s_var;
   cout << F2 << s_var / p_var;
   cout << endl;
}

void TestVariLogNormal(double mean, double sd, int N)
{
   VariLogNormal VLN;
   Real sum = 0.0, sumsq = 0.0;
   for (int i = 1; i <= N; ++i)
   {
      Real x = VLN.Next(mean, sd);
      Real d = x - mean; sum += d; sumsq += d * d;
   }
   Real p_mean = mean;
   Real p_var = sd * sd;
   Real s_mean = mean + sum / N;
   Real s_var = (sumsq - sum * sum / N) / (N - 1);
   Format F1;
   F1.FormatType(Format::SIG_FIGS); F1.Precision(5); F1.Width(10); F1.Suffix(" ");
   Format F2;
   F2.FormatType(Format::DEC_FIGS); F2.Precision(5); F2.Width(10); F2.Suffix(" ");
   cout << F1 << p_mean << s_mean;
   cout << F2 << (s_mean - p_mean) / sqrt(p_var / N);
   cout << F1 << p_var << s_var;
   cout << F2 << s_var / p_var;
   cout << endl;
}

void test5(int N)
{
   cout << endl << endl;

   cout << "Testing VariPoisson" << endl;
   cout << endl;
   cout << "No formal tests yet." << endl;
   cout << "Norm. diff. should be between -2 and 2 in most cases." << endl;
   cout << "Var ratio should be close to 1." << endl;
   cout << endl;
   cout <<
     " pop. mean smpl. mean  norm diff   pop. var  smpl. var  var ratio" <<
     endl;

   TestVariPoisson(0.25, N);
   TestVariPoisson(1, N);
   TestVariPoisson(4, N);
   TestVariPoisson(10, N);
   TestVariPoisson(20, N);
   TestVariPoisson(30, N);
   TestVariPoisson(39.5, N);
   TestVariPoisson(40, N);
   TestVariPoisson(50, N);
   TestVariPoisson(59.5, N);
   TestVariPoisson(60, N);
   TestVariPoisson(60.5, N);
   TestVariPoisson(99.5, N);
   TestVariPoisson(100, N);
   TestVariPoisson(100.5, N);
   TestVariPoisson(199.5, N);
   TestVariPoisson(200, N);
   TestVariPoisson(200.5, N);
   TestVariPoisson(299.5, N);
   TestVariPoisson(300, N);
   TestVariPoisson(300.5, N);
   TestVariPoisson(399.5, N);
   TestVariPoisson(400, N);
   TestVariPoisson(400.5, N);
   TestVariPoisson(500, N);
   TestVariPoisson(10000, N);
   TestVariPoisson(10000.5, N);
   TestVariPoisson(100000, N);
   TestVariPoisson(100000.5, N);
   TestVariPoisson(10000000, N);
   TestVariPoisson(10000000.5, N);

   cout << endl;
   cout << "Testing VariBinomial" << endl;
   cout << endl;
   cout << "No formal tests yet." << endl;
   cout << "Norm. diff. should be between -2 and 2 in most cases." << endl;
   cout << "Var ratio should be close to 1." << endl;
   cout << endl;
   cout <<
     " pop. mean smpl. mean  norm diff   pop. var  smpl. var  var ratio" <<
     endl;

   TestVariBinomial(1, 0.2, N);
   TestVariBinomial(2, 0.1, N);
   TestVariBinomial(5, 0.35, N);
   TestVariBinomial(20, 0.2, N);
   TestVariBinomial(50, 0.5, N);
   TestVariBinomial(100, 0.4, N);
   TestVariBinomial(200, 0.2, N);
   TestVariBinomial(500, 0.3, N);
   TestVariBinomial(1000, 0.19, N);
   TestVariBinomial(1000, 0.21, N);
   TestVariBinomial(10000, 0.4, N);
   TestVariBinomial(1000000, 0.1, N);
   TestVariBinomial(1000000, 0.5, N);
   TestVariBinomial(1, 0.7, N);
   TestVariBinomial(2, 0.6, N);
   TestVariBinomial(5, 0.9, N);
   TestVariBinomial(20, 0.55, N);
   TestVariBinomial(50, 0.9, N);
   TestVariBinomial(100, 0.99, N);
   TestVariBinomial(200, 0.51, N);
   TestVariBinomial(500, 0.7, N);
   TestVariBinomial(1000, 0.81, N);
   TestVariBinomial(1000, 0.79, N);
   TestVariBinomial(10000, 0.9, N);
   TestVariBinomial(1000000, 0.55, N);
   TestVariBinomial(1000000, 0.7, N);

   cout << endl;
   cout << "Testing VariLogNormal" << endl;
   cout << endl;
   cout << "No formal tests yet." << endl;
   cout << "Norm. diff. should be between -2 and 2 in most cases." << endl;
   cout << "Var ratio should be close to 1." << endl;
   cout << endl;
   cout <<
     " pop. mean smpl. mean  norm diff   pop. var  smpl. var  var ratio" <<
     endl;

   TestVariLogNormal(0.25, 0.5, N);
   TestVariLogNormal( 0.5, 1.5, N);
   TestVariLogNormal( 1.5, 2.5, N);
   TestVariLogNormal( 2.0, 1.0, N);
   TestVariLogNormal( 5.0, 0.1, N);

   cout << endl;
}

///@}

