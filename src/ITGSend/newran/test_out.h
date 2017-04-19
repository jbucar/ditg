/// \ingroup newran
///@{

/// \file test_out.h
/// Statistical tests - header file.



#ifndef TEST_OUT
#define TEST_OUT

#ifdef use_namespace
using namespace RBD_STRING;
using namespace RBD_COMMON;
namespace NEWRAN {
#endif



class BaseTest
{
public:
   double SigProb;                        // significance probability
protected:
   const String LongName;                 // name of test
   double Statistic;                      // the test statistic
   void WrapUp(double sig_prob);          // print out test reuslts
public:
   BaseTest(const String& ln, double stat)
      : LongName(ln), Statistic(stat) {}
   virtual void DoTest() = 0;             // do the test
   static const char* Header;
   virtual ~BaseTest() {}
};


// normal one-sided test

class NormalTestUpper : public BaseTest
{
   double Mean, Var;
public:
   NormalTestUpper(const String& ln, double stat, double mean, double var)
      : BaseTest(ln, stat), Mean(mean), Var(var) {}
   void DoTest();                         // do the test
};

// normal two sided test

class NormalTestTwoSided : public BaseTest
{
   double Mean, Var;
public:
   NormalTestTwoSided(const String& ln, double stat, double mean, double var)
      : BaseTest(ln, stat), Mean(mean), Var(var) {}
   void DoTest();                         // do the test
};

// chi-squared one sided test - upper tail

class ChiSquaredTestUpper : public BaseTest
{
   int DF;
public:
   ChiSquaredTestUpper(const String& ln, double stat, int df)
      : BaseTest(ln, stat), DF(df) {}
   void DoTest();                         // do the test
};

// chi-squared one sided test - lower tail

class ChiSquaredTestLower : public BaseTest
{
   int DF;
public:
   ChiSquaredTestLower(const String& ln, double stat, int df)
      : BaseTest(ln, stat), DF(df) {}
   void DoTest();                         // do the test
};

// chi-squared two sided test

class ChiSquaredTestTwoSided : public BaseTest
{
   int DF;
public:
   ChiSquaredTestTwoSided(const String& ln, double stat, int df)
      : BaseTest(ln, stat), DF(df) {}
   void DoTest();                         // do the test
};

// quasi-uniform one sided test

class QuasiUniformLower : public BaseTest
{
public:
   QuasiUniformLower(const String& ln, double stat)
      : BaseTest(ln, stat) {}
   void DoTest();                         // do the test
};

// uniform one sided test

class UniformLower : public BaseTest
{
public:
   UniformLower(const String& ln, double stat)
      : BaseTest(ln, stat) {}
   void DoTest();                         // do the test
};

// Kolmogorov-Smirnov one sided test

class KS_Upper : public BaseTest
{
public:
   KS_Upper(const String& ln, double stat)
      : BaseTest(ln, stat) {}
   void DoTest();                         // do the test
};

#ifdef use_namespace
}
#endif


#endif



// body file: test_out.cpp

///@}


