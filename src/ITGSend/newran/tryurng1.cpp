/// \ingroup newran
///@{

/// \file tryurng1.cpp
/// Test uniform random number generators - the tests.

#define WANT_STREAM
#define WANT_MATH

#include "include.h"
#include "newran.h"
#include "format.h"
#include "tryurng.h"
#include "utility.h"
#include "test_out.h"
#include "array.h"





double chisq_test(const Array<double>& x, int n);

short int CountBits(char c);

short int CountBits(short int i);



void test1(double N)
{
   Uniform U;
   double sum = 0.0, sumsq = 0.0, ar1 = 0.0, last = 0.0;
   double j;
   Array<double> chi0(0,15);
   Array<double> chi1(0,255);
   Array<double> chi1x(0,255);
   Array<double> chi2(0,65535);
   Array<double> chi2x(0,65535);
   chi0 = 0; chi1 = 0; chi1x = 0; chi2 = 0; chi2x = 0;

   Array<double> crawl7(0,127);
   Array<double> crawl8(0,255);
   Array<double> crawl15(0,32767);
   Array<double> crawl16(0,65535);
   crawl7 = 0;
   crawl8 = 0;
   crawl15 = 0;
   crawl16 = 0;
   unsigned long crawler = 0;


   int m_bits = (int)(log(N) / 0.693 - 0.471);  // number of bits in sparse monkey test
   unsigned long M = 1; M <<= (m_bits - 3);     // 2**m_bits / 8
   String Seen(M, (char)0);                     // to accumulate results
   unsigned long mask1 = (M - 1);

   for (j = 0; j < N; ++j)
   {
      double u = U.Next();
      if (u == 1.0) { cout << "Reject value == 1" << endl; continue; }
      double v = u - 0.5;
      sum += v;
      sumsq += v * v;
      ar1 += v * (last - 0.5);
      int k = (int)floor(u * 256); ++chi1(k);
      int m = (int)floor(u * 65536); ++chi2(m);
      int a = (int)floor(u * 16); ++chi0(a);
      if (j > 0)
      {
         int b = (int)floor(last * 16);
         ++chi1x(a + 16 * b);
         int l = (int)floor(last * 256); ++chi2x(k + 256 * l);
      }
      last = u;

      crawler <<= 1; if (v >= 0) ++crawler;
      if (j >= 6)  ++crawl7(crawler & 0x7F);
      if (j >= 7)  ++crawl8(crawler & 0xFF);
      if (j >= 14) ++crawl15(crawler & 0x7FFF);
      if (j >= 15) ++crawl16(crawler & 0xFFFF);

      
      if ( j >= (unsigned int)(m_bits-1) )
      {
         unsigned char mask2 = 1; mask2 <<= crawler & 7;
         Seen[(crawler >> 3) & mask1] |= mask2;
      }

   }

   // check - have I got these right

   NormalTestTwoSided test_mean("Mean", sum, 0.0, N / 12.0); test_mean.DoTest();
   NormalTestTwoSided test_var("Variance", sumsq, N / 12.0, N / 180.0); test_var.DoTest();
   NormalTestTwoSided test_cov("AutoCov 1", ar1, 0.0, N / 144.0); test_cov.DoTest();

   ChiSquaredTestTwoSided Chi_4("Chi 4", chisq_test(chi0, 16), 15); Chi_4.DoTest();
   ChiSquaredTestTwoSided Chi_8("Chi 8", chisq_test(chi1, 256), 255); Chi_8.DoTest();
   ChiSquaredTestTwoSided Chi_16("Chi 16", chisq_test(chi2, 65536), 65535); Chi_16.DoTest();

   ChiSquaredTestTwoSided MM_0_4_2("MM 0-4-2",
      chisq_test(chi1x, 256) - chisq_test(chi0, 16), 240);
   MM_0_4_2.DoTest();

   ChiSquaredTestTwoSided MM_0_8_2("MM 0-8-2",
      chisq_test(chi2x, 65536) - chisq_test(chi1, 256), 65280);
   MM_0_8_2.DoTest();

   ChiSquaredTestTwoSided MM_0_1_8("MM 0-1-8",
      chisq_test(crawl8, 256) - chisq_test(crawl7, 128), 128);
   MM_0_1_8.DoTest();

   ChiSquaredTestTwoSided MM_0_1_16("MM 0-1-16",
      chisq_test(crawl16, 65536) - chisq_test(crawl15, 32768), 32768);
   MM_0_1_16.DoTest();
   

   double seen = 0.0;
   double M8 = M * 8.0;                 // number of possible m bit strings
   for (unsigned int i = 0; i < M; i++) seen += CountBits(Seen[i]);
   double unseen = M8 - seen;
   //cout << "Number of bits = " << m_bits << endl;
   //cout << "Number of target strings = " << M8 << endl;
   //cout << "Number seen              = " << seen << endl;
   //cout << "Number unseen            = " << unseen << endl;
   double expected = exp(m_bits * log(2.0) - N / M8);
   //cout << "Approx expected unseen   = " << expected << endl;
   double p = exp(- (N / M8));
   double var = expected * (1.0 - p * (1.0 + N / M8));
   //cout << "Uncorrected s.d.         = " << sqrt(var) << endl;
   var *= (1.0 + 2.0 * pow(p, 0.30));   // approximate correction
   //cout << "Approx s.d.              = " << sqrt(var) << endl;

   NormalTestTwoSided mm_sparse("MM sparse", unseen, expected, var); mm_sparse.DoTest();




}


double chisq_test(const Array<double>& x, int n)
{
   double sum = 0; double total = 0; int i;
   for (i = 0; i < n; ++i) total += x(i);
   double e = (double)total / (double)n;
   for (i = 0; i < n; ++i) sum += square(x(i) - e);
   return sum / e;
}



// count bits in char

static short int SumOfBits[] = {                   // sum of bits in a byte
      0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,4,1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,
      1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
      1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
      2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
      1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
      2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
      2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
      3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,4,5,5,6,5,6,6,7,5,6,6,7,6,7,7,8 };

short int CountBits(char c) { return SumOfBits[(unsigned char)c]; }

short int CountBits(short int i)
{
   return (short int)(SumOfBits[ (unsigned char)(i & 255) ]
      + SumOfBits[ (unsigned char)(i >> 8) ]);
}

///@}








