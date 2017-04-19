/// \ingroup newran
///@{

/// \file tryrand6.cpp
/// Test of newran - stable rngs.

#define WANT_STREAM
#define WANT_MATH

#include "include.h"
#include "newran.h"
#include "format.h"
#include "tryrand.h"
#include "utility.h"
#include "test_out.h"



// ****** simple tests of Stable random number generators ******


void CharFnTest(Real u, Real alpha, Real beta, int N,
   Stable::Notation notation = Stable::Kalpha);


void test6(int N)
{
   cout << endl << endl;

   cout << "Testing Stable distributions" << endl;
   cout << endl;
   cout << "Compares estimated values of characteristic function with" << endl;
   cout << "theoretical values producing approximately normally" << endl;
   cout << "distributed test statistics;"<< endl;
   cout << "param.1 and param.2 show the mean and s.d. of the test statistic;"
      << endl;
   cout << "statistic shows the test statistic value;" << endl;
   cout << "n. stat shows the normalised value;" << endl;
   cout << "-lg sig shows - log10 of the significance probability." << endl;
   cout << "Should be less than 1.3 (5% sig) in most cases and" << endl;
   cout << "2 (1% sig) in almost all cases." << endl;
   cout << endl;
   cout << "* shows 5% significance; ** shows 1% significance;" << endl;
   cout << "*** shows 0.1% significance." << endl;
   cout << endl;

   cout << BaseTest::Header << endl;

   CharFnTest(0.3, 0.2, 0, N, Stable::Kalpha);
   CharFnTest(0.3, 0.5, 0, N, Stable::Kalpha);
   CharFnTest(0.3, 1.0, 0, N, Stable::Kalpha);
   CharFnTest(0.3, 1.5, 0, N, Stable::Kalpha);
   CharFnTest(0.3, 2.0, 0, N, Stable::Kalpha);

   CharFnTest(0.5, 0.2, 0, N, Stable::Default);
   CharFnTest(0.5, 0.5, 0, N, Stable::Default);
   CharFnTest(0.5, 1.0, 0, N, Stable::Default);
   CharFnTest(0.5, 1.5, 0, N, Stable::Default);
   CharFnTest(0.5, 2.0, 0, N, Stable::Default);

   CharFnTest(0.7, 0.2, 0, N, Stable::Chambers);
   CharFnTest(0.7, 0.5, 0, N, Stable::Chambers);
   CharFnTest(0.7, 1.0, 0, N, Stable::Chambers);
   CharFnTest(0.7, 1.5, 0, N, Stable::Chambers);
   CharFnTest(0.7, 2.0, 0, N, Stable::Chambers);

   CharFnTest(1.0, 0.2, 0, N, Stable::Standard);
   CharFnTest(1.0, 0.5, 0, N, Stable::Standard);
   CharFnTest(1.0, 1.0, 0, N, Stable::Standard);
   CharFnTest(1.0, 1.5, 0, N, Stable::Standard);
   CharFnTest(1.0, 2.0, 0, N, Stable::Standard);

   CharFnTest(2.5, 0.2, 0, N, Stable::Kalpha);
   CharFnTest(2.5, 0.5, 0, N, Stable::Kalpha);
   CharFnTest(2.5, 1.0, 0, N, Stable::Kalpha);
   CharFnTest(2.5, 1.5, 0, N, Stable::Kalpha);
   CharFnTest(2.5, 2.0, 0, N, Stable::Kalpha);


   CharFnTest(0.3, 0.2,  0.4, N, Stable::Kalpha);
   CharFnTest(0.3, 0.5,  0.7, N, Stable::Standard);
   CharFnTest(0.3, 1.0, -0.4, N, Stable::Chambers);
   CharFnTest(0.3, 1.5,  0.3, N, Stable::Kalpha);

   CharFnTest(0.5, 0.2, -0.7, N, Stable::Chambers);
   CharFnTest(0.5, 0.5, -0.2, N, Stable::Kalpha);
   CharFnTest(0.5, 1.0,  0.9, N, Stable::Default);
   CharFnTest(0.5, 1.5, -0.6, N, Stable::Chambers);

   CharFnTest(0.7, 0.2, -0.3, N, Stable::Standard);
   CharFnTest(0.7, 0.5,  0.3, N, Stable::Chambers);
   CharFnTest(0.7, 1.0, -0.4, N, Stable::Kalpha);
   CharFnTest(0.7, 1.5,  0.8, N, Stable::Standard);

   CharFnTest(1.0, 0.2,  0.8, N, Stable::Kalpha);
   CharFnTest(1.0, 0.5, -0.8, N, Stable::Standard);
   CharFnTest(1.0, 1.0,  0.9, N, Stable::Chambers);
   CharFnTest(1.0, 1.5, -0.2, N, Stable::Kalpha);

   CharFnTest(2.5, 0.2, -0.3, N, Stable::Chambers);
   CharFnTest(2.5, 0.5,  0.3, N, Stable::Kalpha);
   CharFnTest(2.5, 1.0, -0.6, N, Stable::Standard);
   CharFnTest(2.5, 1.5,  0.4, N, Stable::Chambers);

   CharFnTest(0.5, 0.2,  1.00, N, Stable::Standard);
   CharFnTest(0.5, 0.5,  1.00, N, Stable::Chambers);
   CharFnTest(0.5, 1.0,  1.00, N, Stable::Kalpha);
   CharFnTest(0.5, 1.5,  1.00, N, Stable::Standard);
   CharFnTest(0.5, 2.0,  1.00, N, Stable::Chambers);

   CharFnTest(1.0, 0.2, -1.00, N, Stable::Kalpha);
   CharFnTest(1.0, 0.5, -1.00, N, Stable::Standard);
   CharFnTest(1.0, 1.0, -1.00, N, Stable::Chambers);
   CharFnTest(1.0, 1.5, -1.00, N, Stable::Kalpha);
   CharFnTest(1.0, 2.0, -1.00, N, Stable::Standard);

   CharFnTest(2.5, 0.2,  1.00, N, Stable::Kalpha);
   CharFnTest(2.5, 0.5,  1.00, N, Stable::Kalpha);
   CharFnTest(2.5, 1.0,  1.00, N, Stable::Standard);
   CharFnTest(2.5, 1.5,  1.00, N, Stable::Chambers);
   CharFnTest(2.5, 2.0,  1.00, N, Stable::Kalpha);


   cout << endl;
}



void CharFnTest(Real u, Real alpha, Real beta, int N, Stable::Notation notation)
{
   Real au = fabs(u);
   int su = (u > 0) ? 1 : (u < 0) ? -1 : 0;
   Real re_f = 0.0, im_f = 0.0;
   if (alpha != 1.0)
   {
      if (notation == Stable::Kalpha)
      {
         Real ka = 1.0 - fabs(1.0 - alpha);
         Real arg = -1.570796327 * beta * ka * su;
         Real px = - pow(au, alpha);
         Real re_e = px * cos(arg); Real im_e = px * sin(arg);
         Real av = exp(re_e);
         re_f = av * cos(im_e); im_f = av * sin(im_e);
      }
      else if (notation == Stable::Standard)
      {
         Real re = - pow(au, alpha);
         Real im = - re * beta * tan(1.570796327 * alpha) * su;
         Real av = exp(re);
         re_f = av * cos(im); im_f = av * sin(im);
      }
      else if (notation == Stable::Chambers)
      {
         Real re = - pow(au, alpha);
         Real im = - (u + re * su) * beta * tan(1.570796327 * alpha);
         Real av = exp(re);
         re_f = av * cos(im); im_f = av * sin(im);
      }
      else if (notation == Stable::Default)
      {
         Real re = - pow(au, alpha);
         re_f = exp(re); im_f = 0;
      }
   }
   else
   {
      Real av = exp(-au);
      Real im = - beta * log(au) * u / 1.570796327;
      re_f = av * cos(im); im_f = av * sin(im);
   }

   Real re_sum = 0.0, im_sum = 0.0;
   Real re_sum_sq = 0.0, im_sum_sq = 0.0;

   Real count0 = 0; Real count1 = 0; Real count2 = 0;

   Stable stable(alpha, beta, notation);

   for (int i = 1; i <= N; ++i)
   {
      Real x = stable.Next();
      Real re, im;
      if (fabs(x) > 1.0E12) re = im = 0.0;
      else { re = cos(u * x); im = sin(u * x); }
      re_sum += re; re_sum_sq += re * re;
      im_sum += im; im_sum_sq += im * im;
      if (x > 0) ++count0;
      if (x > 1) ++count1;
      if (x > 2) ++count2;
   }
   Real re_av = re_sum / N;
   Real re_var = (re_sum_sq - re_av * re_sum) / (N-1) / N;
   Real im_av = im_sum / N;
   Real im_var = (im_sum_sq - im_av * im_sum) / (N-1) / N;

   //cout << "*** " << u << "  " << alpha << "  " << beta << "  " << stable.beta_prime() << " ***" << endl;
   NormalTestTwoSided ntts_re("Stable re_cf", re_av, re_f, re_var);
   ntts_re.DoTest();
   NormalTestTwoSided ntts_im("Stable im_cf", im_av, im_f, im_var);
   ntts_im.DoTest();
   //cout << setw(15) << setprecision(10) << count0 / N << "  ";
   //cout << setw(15) << setprecision(10) << count1 / N << "  ";
   //cout << setw(15) << setprecision(10) << count2 / N << endl;

}

///@}

