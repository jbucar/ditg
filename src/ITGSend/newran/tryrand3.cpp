/// \ingroup newran
///@{

/// \file tryrand3.cpp
/// Test of newran - chi-squared and KS tests.

#define WANT_STREAM
#define WANT_MATH

#include "include.h"
#include "newran.h"
#include "format.h"
#include "tryrand.h"
#include "utility.h"
#include "test_out.h"


void SortAscending(Real* data, int max);
Real KS(Real* data, int n);
Real NormalDF(Real x);
void ChiSquaredGOFTest(const char* name, int* Observed, Real* Prob, int N, int n);
void TestBinomial(int N, Real p, int n);
void TestPoisson(Real mu, int n);
void TestNegativeBinomial(Real NX, Real p, int n);
void TestDiscreteGen(int N, Real* prob, int n);



void test3(int n)
{
   cout << endl << endl;

   // Do chi-squared tests to discrete data
   cout << "ChiSquared tests for discrete data" << endl;
   cout << endl;
   cout << "param.1 shows the number of degrees of freedom;" << endl;
   cout << "statistic shows the chi-squared value;" << endl;
   cout << "n. stat shows the normalised chi-squared value" << endl;
   cout << "-lg sig shows -log10 of the significance probability." << endl;
   cout << "Should be less than 1.3 (5% sig) in most cases and" << endl;
   cout << "2 (1% sig) in almost all cases." << endl;
   cout << endl;
   cout << "* shows 5% significance; ** shows 1% significance;" << endl;
   cout << "*** shows 0.1% significance." << endl;

   cout << endl;
   cout << BaseTest::Header << endl;   
   {
      Real p[] = { 0.05, 0.10, 0.05, 0.5, 0.01, 0.01, 0.03, 0.20, 0.05 };
      TestDiscreteGen(9, p, n);
   }

   {
      Real p[] = { 0.4, 0.2, 0.1, 0.05, 0.025, 0.0125, 0.00625, 0.00625, 0.2 };
      TestDiscreteGen(9, p, n);
   }


   TestNegativeBinomial(200.3, 0.05, n);
   TestNegativeBinomial(150.3, 0.15, n);
   TestNegativeBinomial(100.8, 0.18, n);
   TestNegativeBinomial(100.8, 1.22, n);
   TestNegativeBinomial(100.8, 9.0, n);
   TestNegativeBinomial(10.5, 0.18, n);
   TestNegativeBinomial(10.5, 1.22, n);
   TestNegativeBinomial(10.5, 9.0, n);
   TestNegativeBinomial(0.35, 0.18, n);
   TestNegativeBinomial(0.35, 1.22, n);
   TestNegativeBinomial(0.35, 9.0, n);

   TestBinomial(100, 0.45, n);
   TestBinomial(100, 0.25, n);
   TestBinomial(100, 0.02, n);
   TestBinomial(100, 0.01, n);
   TestBinomial(49, 0.60, n);
   TestBinomial(21, 0.70, n);
   TestBinomial(10, 0.90, n);
   TestBinomial(10, 0.25, n);
   TestBinomial(10, 0.10, n);

   TestPoisson(0.75, n);
   TestPoisson(4.3, n);
   TestPoisson(10, n);
   TestPoisson(100, n);

   Real* data = new Real[n];
   if (!data) Throw(Bad_alloc());

// Apply KS test to a variety of continuous distributions
//    - use cdf transform to convert to uniform

   cout << endl;
   cout << "Kolmogorov-Smirnoff tests for continuous distributions" << endl;
   cout << endl;
   cout << "Statistic shows the K-S value;" << endl;
   cout << "25%, 5%, 1%, 0.1% upper points are 1.019, 1.358, 1.628, 1.950"
      << endl;
   cout << "-lg sig shows -log10 of the significance probability." << endl;
   cout << "Should be less than 1.3 (5% sig) in most cases and" << endl;
   cout << "2 (1% sig) in almost all cases." << endl;
   cout << endl;
   cout << "* shows 5% significance; ** shows 1% significance;" << endl;
   cout << "*** shows 0.1% significance." << endl;
   cout << endl;

   Format F; F.FormatType(Format::DEC_FIGS); F.Precision(5); F.Width(10);
   Format S; S.Width(12);

   cout << BaseTest::Header << endl;   

   {
      ChiSq X(1, 1.44);
      for (int i = 0; i < n; i++)
      {
         Real x = sqrt(X.Next());
         data[i] = NormalDF(x - 1.2) - NormalDF(-x - 1.2);
      }
      Real ks = KS(data, n);
      KS_Upper ks_upper(X.Name(), ks); ks_upper.DoTest();
   }

   {
      ChiSq X(4);
      for (int i = 0; i < n; i++)
         { Real x = 0.5 * X.Next(); data[i] = (1+x)*exp(-x); }
      Real ks = KS(data, n);
      KS_Upper ks_upper(X.Name(), ks); ks_upper.DoTest();
   }

   {
      ChiSq X(2);
      for (int i = 0; i < n; i++) data[i] = exp(-0.5 * X.Next());
      Real ks = KS(data, n);
      KS_Upper ks_upper(X.Name(), ks); ks_upper.DoTest();
   }

   {
      Pareto X(0.5);
      for (int i = 0; i < n; i++)
         { Real x = X.Next(); data[i] = 1.0 / sqrt(x); }
      Real ks = KS(data, n);
      KS_Upper ks_upper(X.Name(), ks); ks_upper.DoTest();
   }

   {
      Pareto X(1.5);
      for (int i = 0; i < n; i++)
         { Real x = X.Next(); data[i] = 1.0 / (x * sqrt(x)); }
      Real ks = KS(data, n);
      KS_Upper ks_upper(X.Name(), ks); ks_upper.DoTest();
   }

   {
      Normal X;
      for (int i = 0; i < n; i++)
         { Real x = X.Next(); data[i] = NormalDF(x); }
      Real ks = KS(data, n);
      KS_Upper ks_upper(X.Name(), ks); ks_upper.DoTest();
   }

   {
      Normal N; SumRandom X = 10 + 5 * N;
      for (int i = 0; i < n; i++)
         { Real x = X.Next(); data[i] = NormalDF((x-10)/5); }
      Real ks = KS(data, n);
      KS_Upper ks_upper(X.Name(), ks); ks_upper.DoTest();
   }

   {
      Normal N; Cauchy C; MixedRandom X = N(0.9) + C(0.1);
      for (int i = 0; i < n; i++)
      {
         Real x = X.Next();
         data[i] = 0.9*NormalDF(x)+0.1*(atan(x)/3.141592654 + 0.5);
      }
      Real ks = KS(data, n);
      KS_Upper ks_upper(X.Name(), ks); ks_upper.DoTest();
   }

   {
      Normal N; MixedRandom X = N(0.9) + (10*N)(0.1);
      for (int i = 0; i < n; i++)
      {
         Real x = X.Next();
         data[i] = 0.9*NormalDF(x)+0.1*NormalDF(x/10);
      }
      Real ks = KS(data, n);
      KS_Upper ks_upper(X.Name(), ks); ks_upper.DoTest();
   }

   {
      Normal  X0; SumRandom X = X0 * 0.6 + X0 * 0.8;
      for (int i = 0; i < n; i++)
         { Real x = X.Next(); data[i] = NormalDF(x); }
      Real ks = KS(data, n);
      KS_Upper ks_upper(X.Name(), ks); ks_upper.DoTest();
   }

   {
      Normal X1;
      MixedRandom X = X1(0.2) + (X1 * 2.5 + 1.1)(0.35) + (X1 + 2.3)(0.45);
      for (int i = 0; i < n; i++)
      {
         Real x = X.Next();
         data[i] = 0.20 * NormalDF(x)
                 + 0.35 * NormalDF((x - 1.1) / 2.5)
                 + 0.45 * NormalDF(x - 2.3);
      }
      Real ks = KS(data, n);
      KS_Upper ks_upper(X.Name(), ks); ks_upper.DoTest();
   }

   {
      Gamma X(0.5);
      for (int i = 0; i < n; i++)
         { Real x = X.Next(); data[i] = 2.0 * NormalDF(-sqrt(2 * x)); }
      Real ks = KS(data, n);
      KS_Upper ks_upper(X.Name(), ks); ks_upper.DoTest();
   }

   {
      Gamma X(3);
      for (int i = 0; i < n; i++)
         { Real x = X.Next(); data[i] = (1+x+0.5*x*x)*exp(-x); }
      Real ks = KS(data, n);
      KS_Upper ks_upper(X.Name(), ks); ks_upper.DoTest();
   }

   {
      Gamma X1(0.85); Gamma X2(2.15); SumRandom X = X1 + X2;
      for (int i = 0; i < n; i++)
         { Real x = X.Next(); data[i] = (1+x+0.5*x*x)*exp(-x); }
      Real ks = KS(data, n);
      KS_Upper ks_upper(X.Name(), ks); ks_upper.DoTest();
   }

   {
      Gamma X1(0.75); Gamma X2(0.25); SumRandom X = X1 + X2;
      for (int i = 0; i < n; i++) data[i] = exp(-X.Next());
      Real ks = KS(data, n);
      KS_Upper ks_upper(X.Name(), ks); ks_upper.DoTest();
   }

   {
      Gamma X(2);
      for (int i = 0; i < n; i++)
         { Real x = X.Next(); data[i] = (1+x)*exp(-x); }
      Real ks = KS(data, n);
      KS_Upper ks_upper(X.Name(), ks); ks_upper.DoTest();
   }

   {
      Exponential X;
      for (int i = 0; i < n; i++) data[i] = exp(-X.Next());
      Real ks = KS(data, n);
      KS_Upper ks_upper(X.Name(), ks); ks_upper.DoTest();
   }

   {
      Cauchy X;
      for (int i = 0; i < n; i++) data[i] = atan(X.Next())/3.141592654 + 0.5;
      Real ks = KS(data, n);
      KS_Upper ks_upper(X.Name(), ks); ks_upper.DoTest();
   }

   {
      Cauchy X0; SumRandom X = X0 * 0.3 + X0 * 0.7;
      for (int i = 0; i < n; i++) data[i] = atan(X.Next())/3.141592654 + 0.5;
      Real ks = KS(data, n);
      KS_Upper ks_upper(X.Name(), ks); ks_upper.DoTest();
   }

   {
      Uniform X;
      for (int i = 0; i < n; i++) data[i] = X.Next();
      Real ks = KS(data, n);
      KS_Upper ks_upper(X.Name(), ks); ks_upper.DoTest();
   }

   delete [] data;


}

/*************************** Kolmogorov Smirnov Test ************************/

// test the data in the array (length n) for being uniform (0,1)

Real KS(Real* data, int n)
{
   SortAscending(data, n);
   Real D = 0.0;
   for (int i = 0; i < n; i++)
   {
      Real d1 = (Real)(i+1) / (Real)n - data[i];
      Real d2 = data[i] - (Real)i / (Real)n;
      if (D < d1) D = d1; if (D < d2) D = d2;
   }
   return D * (sqrt((Real)n) + 0.12 + 0.11 / sqrt((Real)n));
}




/******************************** Quick sort ********************************/

// Quicksort.
// Essentially the method described in Sedgewick's algorithms in C++
// My version is still partially recursive, unlike Segewick's, but the
// smallest segment of each split is used in the recursion, so it should
// not overlead the stack.

// If the process does not seems to be converging an exception is thrown.


#define DoSimpleSort 17            // when to switch to insert sort
#define MaxDepth 50                // maximum recursion depth


static Real SortThreeDescending(Real* a, Real* b, Real* c);
static void MyQuickSortAscending(Real* first, Real* last, int depth);
static void InsertionSortAscending(Real* first, const int length, int guard);



static Real SortThreeDescending(Real* a, Real* b, Real* c)
{
   // sort *a, *b, *c; return *b; optimise for already sorted
   if (*a >= *b)
   {
      if (*b >= *c) return *b;
      else if (*a >= *c) { Real x = *c; *c = *b; *b = x; return x; }
      else { Real x = *a; *a = *c; *c = *b; *b = x; return x; }
   }
   else if (*c >= *b) { Real x = *c; *c = *a; *a = x; return *b; }
   else if (*a >= *c) { Real x = *a; *a = *b; *b = x; return x; }
   else { Real x = *c; *c = *a; *a = *b; *b = x; return x; }
}



void SortAscending(Real* data, int max)
{
   if (max > DoSimpleSort) MyQuickSortAscending(data, data + max - 1, 0);
   InsertionSortAscending(data, max, DoSimpleSort);
}

static void InsertionSortAscending(Real* first, const int length,
   int guard)
// guard gives the length of the sequence to scan to find first
// element (eg guard = length)
{
   if (length <= 1) return;

   // scan for first element
   Real* f = first; Real v = *f; Real* h = f;
   if (guard > length) guard = length; int i = guard - 1;
   while (i--) if (v > *(++f)) { v = *f; h = f; }
   *h = *first; *first = v;

   // do the sort
   i = length - 1; f = first;
   while (i--)
   {
      Real* g = f++; h = f; v = *h;
      while (*g > v) *h-- = *g--;
      *h = v;
   }
}

static void MyQuickSortAscending(Real* first, Real* last, int depth)
{
   for (;;)
   {
      const int length = last - first + 1;
      if (length < DoSimpleSort) return;
      if (depth++ > MaxDepth)
         Throw(BaseException("QuickSortAscending fails"));
      Real* centre = first + length/2;
      const Real test = SortThreeDescending(last, centre, first);
      Real* f = first; Real* l = last;
      for (;;)
      {
         while (*(++f) < test) {}
         while (*(--l) > test) {}
         if (l <= f) break;
         const Real temp = *f; *f = *l; *l = temp;
      }
      if (f > centre) { MyQuickSortAscending(l+1, last, depth); last = f-1; }
      else { MyQuickSortAscending(first, f-1, depth); first = l+1; }
   }
}

Real NormalDF(Real x)
{
   // from Abramowitz and Stegun - accuracy 7.5E-8
   // accuracy is absolute; not relative
   // eventually will need a better method
   // but good enough here
   Real t = 1.0 / (1.0 + 0.2316419 * fabs(x));
   t = ( 0.319381530
     + (-0.356563782
     + ( 1.781477937
     + (-1.821255978
     +   1.330274429 * t) * t) * t) * t) * t;
   t = 0.3989422804014326779399461 * exp(-0.5 * x * x) * t;
   return (x < 0) ? t : 1.0 - t;
}

void ChiSquaredGOFTest(const char* name, int* Observed, Real* Prob, int N, int n)
{
   // go for at least two expected observations per cell
   // work in from ends

   if (N <= 0) { cout << "no categories" << endl; return; }
   if (n <= 0) { cout << "no data" << endl; return; }

   int O1 = 0; Real E1 = 0.0; int O2 = 0; Real E2 = 0.0;
   Real CS = 0.0; int df = 0; int i = 0; int Ni = N; Real ToGo = n;
   for (;;)
   {
      O1 += Observed[i]; Real e1 = n * Prob[i]; E1 += e1; ToGo -= e1;
      if (E1 >= 2.0 && ToGo + E2 >= 2.0)
         { CS += square(O1 - E1) / E1; df += 1; O1 = 0; E1 = 0.0; }

      if (i == Ni) break;
      ++i;

      O2 += Observed[Ni]; Real e2 = n * Prob[Ni]; E2 += e2; ToGo -= e2;
      if (E2 >= 2.0 && ToGo + E1 >= 2.0)
         { CS += square(O2 - E2) / E2; df += 1; O2 = 0; E2 = 0.0; }

      if (i == Ni) break;
      --Ni;
   }

   E1 += E2; O1 += O2;
   if (E1 > 0.0) { CS += square(O1 - E1) / E1; df += 1; }
   if (fabs(ToGo) >= 0.01) cout << "chi-squared program fails " << endl;
   else
      { ChiSquaredTestUpper CSTU(name, CS, df-1); CSTU.DoTest(); }

}


void TestBinomial(int N, Real p, int n)
{
   Binomial X(N, p);
   Real q = 1.0 - p; Real ln_p = log(p); Real ln_q = log(q);
   int* obs = new int [N+1]; if (!obs) Throw(Bad_alloc());
   Real* prob = new Real [N+1]; if (!prob) Throw(Bad_alloc());
   int i;
   for (i = 0; i <= N; i++)
   {
      obs[i] = 0;
      prob[i] = exp(ln_gamma(N+1) - ln_gamma(i+1) - ln_gamma(N-i+1)
         + i * ln_p + (N-i) * ln_q);
   }
   for (i = 0; i < n; i++)
   {
      int b = (int)X.Next();
      if (b < 0 || b > N) Throw(Logic_error("Binomial error"));
      obs[b]++;
   }
   ChiSquaredGOFTest("Binomial", obs, prob, N, n);

   delete [] obs; delete [] prob;
}

void TestPoisson(Real mu, int n)
{
   Poisson X(mu);
   Real ln_mu = log(mu);
   int N = (int)(20 + mu + 10 * sqrt(mu));         // set upper bound
   if (N > n)
   {
      cout << "Poisson: range too large" << endl;
      return;
   }
   int* obs = new int [N+1]; if (!obs) Throw(Bad_alloc());
   Real* prob = new Real [N+1]; if (!prob) Throw(Bad_alloc());
   int i;
   for (i = 0; i <= N; i++)
      { obs[i] = 0; prob[i] = exp(i * ln_mu - mu - ln_gamma(i+1)); }
   for (i = 0; i < n; i++)
   {
      int b = (int)(X.Next());
      if (b < 0 || b > N) Throw(Logic_error("Poisson error"));
      obs[b]++;
   }
   ChiSquaredGOFTest("Poisson", obs, prob, N, n);

   delete [] obs; delete [] prob;
}

void TestNegativeBinomial(Real NX, Real P, int n)
{
   NegativeBinomial X(NX, P);
   Real Q = 1.0 + P; Real p = 1.0 / Q; Real q = 1.0 - p;
   Real ln_p = log(p); Real ln_q = log(q);
   Real mean = NX * P; Real var = mean * Q;
   int N = (int)(20 + mean + 100 * sqrt(var));         // set upper bound
      // won't be good enough for large P
   if (N > n)
   {
      cout << "NegativeBinomial: range too large" << endl;
      return;
   }
   int* obs = new int [N+1]; if (!obs) Throw(Bad_alloc());
   Real* prob = new Real [N+1]; if (!prob) Throw(Bad_alloc());
   int i;
   for (i = 0; i <= N; i++)
   {
      obs[i] = 0;
      prob[i] = exp(ln_gamma(NX+i) - ln_gamma(i+1) - ln_gamma(NX)
         + NX * ln_p + i * ln_q);
   }
   for (i = 0; i < n; i++)
   {
      int b = (int)X.Next();
      if (b < 0 || b > N) Throw(Logic_error("NegativeBinomial error"));
      obs[b]++;
   }
   ChiSquaredGOFTest("NegBinomial",obs, prob, N, n);

   delete [] obs; delete [] prob;
}

void TestDiscreteGen(int N, Real* prob, int n)
{
   DiscreteGen X(N, prob);
   int* obs = new int [N]; if (!obs) Throw(Bad_alloc());
   int i;
   for (i = 0; i < N; i++) obs[i] = 0;
   for (i = 0; i < n; i++)
   {
      int b = (int)X.Next();
      if (b < 0 || b >= N) Throw(Logic_error("DiscreteGen error"));
      obs[b]++;
   }
   ChiSquaredGOFTest("DiscreteGen",obs, prob, N-1, n);

   delete [] obs;
}

///@}

