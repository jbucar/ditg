/// \ingroup newran
///@{

/// \file tryrand4.cpp
/// Test of newran - combinations and permutations.

#define WANT_STREAM
#define WANT_MATH

#include "include.h"
#include "newran.h"
#include "str.h"
#include "test_out.h"
#include "format.h"
#include "tryrand.h"
#include "utility.h"
#include "array.h"


void test_perm(int N, int NN, unsigned int M);


void test4(int n)
{
   cout << endl << endl;
   cout << "Combinations and permutations" << endl;
   cout << endl;
   
   cout << "Just checking that we get a valid permutation or combination"
      << endl;

   {
      cout << "Doing permutations" << endl;
      RandomPermutation RP;
      int i, j, k;
      int p[10];

      cout << "... select 10 items from 100...119 without replacement" << endl;
      for (i = 1; i <= 10; i++)
      {
         RP.Next(20,10,p,100);
         for (j = 0; j < 10; j++)
         {
            int pj = p[j];
            if (pj < 100 || pj > 119)
               Throw(BaseException("Invalid permutation generated"));
            for (k = 0; k < j; ++k) if (p[k] == pj)
               Throw(BaseException("Invalid permutation generated"));
         }
      }

      cout << "... select 10 items from 100...109 without replacement" << endl;
      for (i = 1; i <= 10; i++)
      {
         RP.Next(10,10,p,100);
         for (j = 0; j < 10; j++)
         {
            int pj = p[j]; //cout << pj << " ";
            if (pj < 100 || pj > 109)
               Throw(BaseException("Invalid permutation generated"));
            for (k = 0; k < j; ++k) if (p[k] == pj)
               Throw(BaseException("Invalid permutation generated"));
         }
      }
   }

   {
      cout << "Doing combinations" << endl;
      RandomCombination RC;
      int i, j;
      int p[10];

      cout << "... select 10 items from 100...119 without replacement" << endl;
      for (i = 1; i <= 10; i++)
      {
         RC.Next(20,10,p,100);
         for (j = 0; j < 10; j++)
         {
            int pj = p[j];
            if (pj < 100 || pj > 119)
               Throw(BaseException("Invalid permutation generated"));
            if (j > 0 && pj <= p[j-1])
               Throw(BaseException("Invalid permutation generated"));
         }
      }

      cout << "... select 10 items from 100...109 without replacement" << endl;
      for (i = 1; i <= 10; i++)
      {
         RC.Next(10,10,p,100);
         for (j = 0; j < 10; j++)
         {
            int pj = p[j]; //cout << pj << " ";
            if (pj < 100 || pj > 109)
               Throw(BaseException("Invalid permutation generated"));
            if (j > 0 && pj <= p[j-1])
               Throw(BaseException("Invalid permutation generated"));
         }
         //cout << "\n";
      }
      //cout << "\n";
   }
   
   cout << endl;
   cout << "Doing formal tests of permutation generator" << endl;
   cout << endl;
   cout << "Carries out a number of tests which generate approximately" << endl;
   cout << "normally distributed test statistics;" << endl;
   cout << "param. 1 and param. 2 show the mean and s.d. of the test statistic;"
      << endl;
   cout << "statistic shows the test statistic value;" << endl;
   cout << "n. stat shows the normalised value;" << endl;
   cout << "-lg sig shows -log10 of the significance probability." << endl;
   cout << "Should be less than 1.3 (5% sig) in most cases and" << endl;
   cout << "2 (1% sig) in almost all cases." << endl;
   cout << endl;
   cout << "* shows 5% significance; ** shows 1% significance;" << endl;
   cout << "*** shows 0.1% significance." << endl;

   test_perm(1,  10, n * 10);
   test_perm(4,  10, n * 10);
   test_perm(7,  10, n * 10);
   test_perm(10, 10, n * 10);
   test_perm(1,   100, n);
   test_perm(30,  100, n);
   test_perm(65,  100, n);
   test_perm(100, 100, n);
   test_perm(100, 1000, n / 10);
   
}

void test_perm(int N, int NN, unsigned int M)
{
   int j; unsigned long i; double sum2 = 0;
   Array<long> count(0,NN-1);   // count number of times each ball is drawn
   Array<int> last(0,NN-1);     // details of last draw
   long consec = 0;             // count number of consecutive balls
   last = 0; count = 0;         // set the arrays to zero


   cout << endl;
   cout << "Size of urn = " << NN << ";  ";
   cout << "no. of balls = " << N << ";  ";
   cout << "no. of trials = " << M << endl;
   cout << BaseTest::Header << endl;   
   if (N > NN) { Throw(Runtime_error("N > NN")); }
   long double S0 = 0.0;             // sum of ball numbers
   long double S1 = 0.0;             // sum of ball * draw
   double centre = (NN+1) / 2.0;     // average ball number
   double avj = (N+1)/2.0;           // average draw number
   Array2<int> table1(0,NN-1,0,N-1);     // times ball i occurs at draw j
   Array2<int> table2(0,NN-1,0,NN-1);    // times ball i and ball j both occur
   table1 = 0; table2 = 0;           // clear the tables
   long turnings = 0;                // number of turning points
   RandomPermutation RP;             // reset the Urn
   Array<int> Draw(0,N-1);           // for the draw


   for (i=1; i<=M; i++)              // iterate over total number of trials
   {
      int last_ball = -1;
      RP.Next(NN,N,Draw.base(),1);   // do the draw
      Array<int> check(0,NN-1); check = 0;  // for checking no repeats
      for (j = 1; j <= N; j++)       // N balls in draw
      {
	 int x = Draw(j-1);
	 if (x<1 || x>NN) Throw(Runtime_error("Invalid number"));
	 check(x-1)++; count(x-1)++; sum2 += last(x-1);
	 if (x == last_ball+1 || x == last_ball-1) consec++;
	 last_ball = x;
	 double xc = x - centre;
	 S0 += xc; S1 += xc * (j-avj);
	 table1(x-1,j-1)++;
      }
      for (j=0; j<NN; j++)
      {
	 if (check(j) > 1) Throw(Runtime_error("Repeated number"));
	 last(j) = check(j);
	 if (check(j))              // increment table of pairs
	 {
	    for (int k = j+1; k<NN; k++) if (check(k)) table2(j,k)++;
	 }
      }

      // count number of turnings
      for (j = 2; j < N; j++)
      {
         long last = Draw(j-2);
         long current = Draw(j-1);
         long next = Draw(j);
         if ( (last < current && next < current) ||
            (last > current && next > current) ) turnings++;
      }

   }

   // do the tests
   double sum1 = 0.0; double d = M * (N / (double)NN);
   for (j=0; j<NN; j++) { sum1 += square(count(j) - d); }

   if (N < NN)
   {
      sum1 /= d;
      double sd = (NN - N) * sqrt( 2 * (M - 1)  / (M * ((double)(NN - 1))) );
      NormalTestTwoSided freq_test("Ball freq.", sum1, NN - N, square(sd));
      freq_test.DoTest();

      NormalTestTwoSided repeats_test("Repeats", sum2, (M - 1) * (N * N / (double)NN),
         square((double)(N * (NN - N)) / (double)NN) * (M - 1) / (NN - 1) );
      repeats_test.DoTest();

      double V0 = ((double)N * (NN+1.0) * (NN-N))/12.0;
      NormalTestTwoSided average_test("Average", S0, 0, M * V0);
      average_test.DoTest();
   }

   if (N > 1)
   {
      double V1 = (double)N * (N*N - 1) * NN * (NN + 1) / 144.0;
      NormalTestTwoSided ball_times_draw("Ball * draw", S1, 0, M * V1);
      ball_times_draw.DoTest();

      double e = 2 * M * (N - 1) / (double)NN;
				   // expected # of consecutive balls
      NormalTestTwoSided consec_test("Consec balls", consec, e, e);
      consec_test.DoTest();
   }

   if (N > 1)
   {
      // balls vs draws
      double CS = 0; double ev = (double)M / NN;
      for (int i = 0; i < NN; i++) for (int j = 0; j < N; j++)
	 CS += square(table1(i,j)-ev);
      CS /= ev; ev = N * (NN - 1);
      double v =  2.0 * N * (M - 1) * (N - (2 - NN) * NN) / M / (NN - 1);
      NormalTestTwoSided ball_draw("Ball vs draw", CS, ev, v);
      ball_draw.DoTest();
   }

   if (N>1 && N < NN)
   {
      // pairs of balls
      double CS = 0; double ev = (double)M*N*(N-1)/ NN / (NN-1);
      for (int i = 0; i < NN; i++) for (int j = i+1; j < NN; j++)
	 CS += square(table2(i,j)-ev);
      CS /= ev; ev = (NN-N)*(NN+N-1)/2.0;
      double v =  (double)(M-1)*square(NN-N)*(-3+6*N-3.0*N*N+2*NN-6.0*N*NN+
	 2.0*N*N*NN+NN*NN)/(M*(NN-2)*(NN-3));
      NormalTestTwoSided pairs_test("Pairs table", CS, ev, v);
      pairs_test.DoTest();
   }

   if (N > 2)
   {
      double V2 = (16.0 * N - 29.0) / 90.0;
      NormalTestTwoSided runs_test("Runs test", turnings,
         (double)M * (N-2) / 1.5, M * V2);
      runs_test.DoTest();
   }

}

///@}


