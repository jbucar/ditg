/// \defgroup newran Newran random number generator library
///@{

/// \file newran.h
/// Definition file for random number generator library.

#ifndef NEWRAN_LIB
#define NEWRAN_LIB 0

//******************* utilities and definitions *************************

#include "include.h"
#include "myexcept.h"
#include "extreal.h"
#include "simpstr.h"

#ifdef use_namespace
namespace NEWRAN { using namespace RBD_COMMON; }
namespace RBD_LIBRARIES { using namespace NEWRAN; }
namespace NEWRAN {
using RBD_COMMON::Real;
using RBD_COMMON::long_Real;
#endif

/// Probability function (Real function of Real variable).
typedef Real (*PDF)(Real);

// for log gamma function
Real gamma_series(long_Real z);
inline Real gamma_g_value() { return 10.0; }
Real ln_gamma(Real z, int& sign);
Real ln_gamma(Real z);
Real ln_gamma_lanczos(Real z);
Real rbd_gamma(Real z);


class RepeatedRandom;
class SelectedRandom;

/// Base for random number classes.
class Random                              // uniform random number generator
{

private:
   void operator=(const Random&) {}       // private so can't access

   virtual void CSFD(bool update) {}
   virtual void CSTD() {}

protected:
   static Random* RNG;                    // pointer to uniform rng
   static Random* Dummy;                  // pointer to dummy rng
   static SimpleString Dir;               // directory where seed is stored

public:
   static void Set(Random& r);            // set type
   static void CopySeedFromDisk(bool update=false);
   static void CopySeedToDisk();
   static void SetDirectory(const char* dir); // set the directory for storing seed

   virtual Real Next() { return RNG->Next(); } // get new value
   virtual unsigned long ulNext();        // unsigned long value
   virtual const char* Name();            // identification
   virtual Real Density(Real) const;      // used by PosGen & Asymgen
   Random() {}                            // do nothing
   virtual ~Random() {}                   // make destructors virtual
   SelectedRandom& operator()(double);    // used by MixedRandom
   RepeatedRandom& operator()(int);       // for making combinations
   virtual ExtReal Mean() const { return 0.5; }
                                          // mean of distribution
   virtual ExtReal Variance() const { return 1.0/12.0; }
					  // variance of distribution
   virtual void tDelete() {}              // delete components of sum
   virtual int nelems() const { return 1; }
                                          // used by MixedRandom
   virtual void load(int*,Real*,Random**);
   friend class RandomPermutation;
};

/// Base for Lewis Goodman Miller generators.
/// \internal
class LGM_base : public Random
{
   void operator=(const LGM_base&) {}      // private so can't access
protected:
   long seed;
   void NextValue();
   bool Update;                            // true if update seed on disk

public:
   LGM_base(double s);                     // seed between 0 and 1
   Real Next();                            // get new value
   unsigned long ulNext();                 // new unsigned long value
   const char* Name();                     // identification
};

/// Lewis Goodman Miller generator with no mixing.
class LGM_simple : public LGM_base
{
   void operator=(const LGM_simple&) {}    // private so can't access
   void CSFD(bool update);
   void CSTD();

public:
   LGM_simple(double s=0.46875);           // seed between 0 and 1
   const char* Name();                     // identification
   ~LGM_simple();
};

/// Lewis Goodman Miller generator with mixing.
class LGM_mixed : public LGM_base
{
   long Buffer[128];                       // for mixing random numbers
   void NextValue();
   long seed_mixed;
   void operator=(const LGM_mixed&) {}     // private so can't access
   void CSFD(bool update);
   void CSTD();

public:
   LGM_mixed(double s=0.46875);            // s between 0 and 1
   Real Next();                            // get new value
   unsigned long ulNext();                 // new unsigned long value
   const char* Name();                     // identification
   ~LGM_mixed();
};

/// Wichmann Hill generator.
class WH : public Random
{
   bool Update;                            // true if update seed on disk
   void operator=(const WH&) {}            // private so can't access
   unsigned long seed1, seed2, seed3;
   void CSFD(bool update);
   void CSTD();

public:
   WH(double s=0.46875);                   // seed between 0 and 1
   Real Next();                            // get new value
   unsigned long ulNext();                 // new unsigned long value
   const char* Name();                     // identification
   ~WH();
};

/// Fishman Moore generator.
class FM : public Random
{
   bool Update;                            // true if update seed on disk
   void operator=(const FM&) {}            // private so can't access
   unsigned long seed;
   void NextValue();
   void CSFD(bool update);
   void CSTD();

public:
   FM(double s=0.46875);                   // s between 0 and 1
   Real Next();
   unsigned long ulNext();                 // new unsigned long value
   const char* Name();                     // identification
   ~FM();
};

/// Marsaglia's mother of all generators.
class MotherOfAll : public Random
{
   bool Update;                           // true if update seed on disk
   short mother1[10];
   short mother2[10];
   short mStart;
   unsigned long seed;
   void Mother();
   void operator=(const MotherOfAll&) {}  // private so can't access
   void CSFD(bool update);
   void CSTD();

public:
   MotherOfAll(double s=0.9375);
   Real Next();
   unsigned long ulNext();                // new unsigned long value
   const char* Name();
   ~MotherOfAll();
};   

/// Marsaglia's multiply with carry generator.
class MultWithCarry : public Random
{
   bool Update;                           // true if update seed on disk
   unsigned long x;
   unsigned long crry;

   void NextValue();

   void operator=(MultWithCarry&) {}      // private so can't access
   void CSFD(bool update);
   void CSTD();

public:
   MultWithCarry(double s=0.46875);
   Real Next();
   unsigned long ulNext();                // new unsigned long value
   const char* Name();
   ~MultWithCarry();
};   

/// Mersenne Twister generator.
class MT : public Random
{
   bool Update;                           // true if update seed on disk
   void init_genrand(unsigned long s);
   unsigned long genrand_int32();
   void operator=(const MT&) {}           // private so can't access
   void CSFD(bool update);
   void CSTD();

public:
   MT(double s=0.46875);
   Real Next();
   unsigned long ulNext() { return genrand_int32(); }
   const char* Name();
   ~MT();
};   

/// Dummy RNG for returning an error message when RNG not setup.
/// \internal 
class DummyRNG : public Random
{
   void operator=(const DummyRNG&) {}    // private so can't access
   void CSFD(bool update);
   void CSTD();

public:
   Real Next();
   const char* Name();
};   


/// Uniform random number generator.
class Uniform : public Random
{
   void operator=(const Uniform&) {}      // private so can't access

public:
   const char* Name();                    // identification
   Uniform() {}                           // set value
   Real Next() { return RNG->Next(); }
   ExtReal Mean() const { return 0.5; }
   ExtReal Variance() const { return 1.0/12.0; }
   Real Density(Real x) const { return (x < 0.0 || x > 1.0) ? 0 : 1.0; }
};

/// Return a constant.
class Constant : public Random
{
   void operator=(const Constant&) {}     // private so can't access
   Real value;                            // value to be returned

public:
   const char* Name();                    // identification
   Constant(Real v) { value=v; }          // set value
   Real Next() { return value; }
   ExtReal Mean() const { return value; }
   ExtReal Variance() const { return 0.0; }
};

/// Automatic generator for positive random variables.
class PosGen : public Random              // generate positive rv
{
   void operator=(const PosGen&) {}       // private so can't access

protected:
   Real xi, *sx, *sfx;
   bool NotReady;
   void Build(bool);                      // called on first call to Next

public:
   const char* Name();                    // identification
   PosGen();                              // constructor
   ~PosGen();                             // destructor
   Real Next();                           // to get a single new value
   ExtReal Mean() const { return (ExtReal)Missing; }
   ExtReal Variance() const { return (ExtReal)Missing; }
};

/// Automatic generator for symmetric random variables.
class SymGen : public PosGen              // generate symmetric rv
{
   void operator=(const SymGen&) {}       // private so can't access

public:
   const char* Name();                    // identification
   Real Next();                           // to get a single new value
};

/// Normal random number generator.
class Normal : public SymGen              // generate standard normal rv
{
   void operator=(const Normal&) {}       // private so can't access
   static Real Nxi, *Nsx, *Nsfx;          // so we need initialise only once
   static long count;                     // assume initialised to 0

public:
   const char* Name();                    // identification
   Normal();
   ~Normal();
   Real Density(Real) const;              // normal density function
   ExtReal Mean() const { return 0.0; }
   ExtReal Variance() const { return 1.0; }
};

/// Chi-squared (central) random number generator.
class ChiSq : public Random               // generate non-central chi-sq rv
{
   void operator=(const ChiSq&) {}        // private so can't access
   Random* c1;                            // pointers to generators
   Random* c2;                            // pointers to generators
   int version;                           // indicates method of generation
   Real mean, var;

public:
   const char* Name();                    // identification
   ChiSq(int, Real=0.0);                  // df and non-centrality parameter
   ~ChiSq();
   ExtReal Mean() const { return mean; }
   ExtReal Variance() const { return var; }
   Real Next();
};

/// Cauchy random number generator.
class Cauchy : public SymGen              // generate standard cauchy rv
{
   void operator=(const Cauchy&) {}       // private so can't access

public:
   const char* Name();                    // identification
   Real Density(Real) const;              // Cauchy density function
   ExtReal Mean() const { return Indefinite; }
   ExtReal Variance() const { return PlusInfinity; }
};

///Exponential random number generator

class Exponential : public Uniform
{
  Uniform U;
  double mean;
  double variance;

public:

  Exponential(double md=1):mean(md){}
  Real Next();
  const char* Name();			 		// identification
  Real Density(Real) const; 			// Exponential density function
  ExtReal Mean() const { return mean; }
  ExtReal Variance() const { return mean*mean; }
};
/// Weibull random generator
class Weibull : public Uniform
{
  Uniform U;
  double scale;
  double shape;
public:
  Weibull(double scale_, double shape_)
    {
      scale = scale_;
      shape = shape_;
    }
  Real Next();
  const char* Name();
};

/// Extreme_Largest random generator
class Extreme_Largest : public Uniform
{
  Uniform U;
  double mu;
  double sigma;
public:
  Extreme_Largest(double mu_, double sigma_)
    {
      mu = mu_;
      sigma = sigma_;
    }
  Real Next();
  const char* Name();
};

/// T-Student
class Student : public Uniform
{
private:
  Real mu;
  Real sigma;
  Real nu;

public:
  Student(Real mu_, Real sigma_, Real nu_)
    {
      mu = mu_;
      sigma = sigma_;
      nu=nu_;
    }
  Real Next();
  const char* Name();
};

/// Automatic asymmetric random number generator.
class AsymGen : public Random             // generate asymmetric rv
{
   void operator=(const AsymGen&) {}      // private so can't access
   Real xi, *sx, *sfx; int ic;
   bool NotReady;
   void Build();                          // called on first call to Next

protected:
   Real mode;

public:
   const char* Name();                    // identification
   AsymGen(Real);                         // constructor (Real=mode)
   ~AsymGen();                            // destructor
   Real Next();                           // to get a single new value
   ExtReal Mean() const { return (ExtReal)Missing; }
   ExtReal Variance() const { return (ExtReal)Missing; }
};

/// Gamma random number generator.
class Gamma : public Random               // generate gamma rv
{
   void operator=(const Gamma&) {}        // private so can't access
   Random* method;

public:
   const char* Name();                    // identification
   Gamma(Real);                           // constructor (Real=shape)
   ~Gamma() { delete method; }
   Real Next() { return method->Next(); }
   ExtReal Mean() const { return method->Mean(); }
   ExtReal Variance() const { return method->Variance(); }
};

/// Generator for positive rngs where we have a pointer to the density.
class PosGenX : public PosGen
{
   void operator=(const PosGenX&) {}      // private so can't access
   PDF f;

public:
   const char* Name();                    // identification
   PosGenX(PDF fx);
   Real Density(Real x) const { return (*f)(x); }
};

/// Generator for symmetric rngs where we have a pointer to the density.
class SymGenX : public SymGen
{
   void operator=(const SymGenX&) {}      // private so can't access
   PDF f;

public:
   const char* Name();                    // identification
   SymGenX(PDF fx);
   Real Density(Real x) const { return (*f)(x); }
};

/// Generator for asymmetric rngs where we have a pointer to the density.
class AsymGenX : public AsymGen
{
   void operator=(const AsymGenX&) {}     // private so can't access
   PDF f;

public:
   const char* Name();                          // identification
   AsymGenX(PDF fx, Real mx);
   Real Density(Real x) const { return (*f)(x); }
};

/// Pareto random number generator.
/// Use definition of Kotz and Johnson: "Continuous univariate distributions 1",
/// chapter 19 with k = 1.
class Pareto : public Random
{
   void operator=(const Pareto&) {}       // private so can't access
   Real Shape, RS;

public:
   const char* Name();                    // identification
   Pareto(Real shape);                    // constructor (Real=shape)
   ~Pareto() {}
   Real Next();
   ExtReal Mean() const;
   ExtReal Variance() const;
};

/// Discrete random number generator.
class DiscreteGen : public Random
{
   void operator=(const DiscreteGen&) {}  // private so can't access
   Real *p; int *ialt; int n; Real *val;
   void Gen(int, Real*);                  // called by constructors
   Real mean, var;                        // calculated by the constructor

public:
   const char* Name();                    // identification
   DiscreteGen(int,Real*);                // constructor
   DiscreteGen(int,Real*,Real*);          // constructor
   ~DiscreteGen();                        // destructor
   Real Next();                           // new single value
   ExtReal Mean() const { return mean; }
   ExtReal Variance() const { return var; }
};

/// Poisson random number generator.
class Poisson : public Random             // generate poisson rv
{
   void operator=(const Poisson&) {}      // private so can't access
   Random* method;

public:
   const char* Name();                    // identification
   Poisson(Real);                         // constructor (Real=mean)
   ~Poisson() { delete method; }
   Real Next() { return method->Next(); }
   ExtReal Mean() const { return method->Mean(); }
   ExtReal Variance() const { return method->Variance(); }
};

/// Binomial random number generator.
class Binomial : public Random            // generate binomial rv
{
   void operator=(const Binomial&) {}     // private so can't access
   Random* method;

public:
   const char* Name();                    // identification
   Binomial(int p, Real n);               // constructor (int=n, Real=p)
   ~Binomial() { delete method; }
   Real Next() { return method->Next(); }
   ExtReal Mean() const { return method->Mean(); }
   ExtReal Variance() const { return method->Variance(); }
};

/// Negative binomial random number generator.
class NegativeBinomial : public AsymGen   // generate negative binomial rv
{
   Real N, P, Q, p, ln_q, c;

public:
   const char* Name();
   NegativeBinomial(Real NX, Real PX);    // constructor
   Real Density(Real) const;              // Negative binomial density function
   Real Next();
   ExtReal Mean() const { return N * P; }
   ExtReal Variance() const { return N * P * Q; }
};

/// Stable random number generator.
class Stable : public Random
{
public:
   enum Notation { Chambers, Standard, Kalpha, Default };
private:
   Real alpha, bprime;
   Real beta;
   Notation notation;
   Real d2(Real z);
   Real tan2(Real x);
   Real k(Real a);
   const Real piby2;
   const Real piby4;

public:
   const char* Name();
   Stable(Real a, Real bx, Notation n = Default);
   Real Next();
   ExtReal Mean() const;
   ExtReal Variance() const;
   Real beta_prime() const { return bprime; }
};   


// ***************** Simple functions of random variables *********************

/// Negative of a random variable.
/// \internal
class NegatedRandom : public Random
{
protected:
   Random* rv;
   NegatedRandom(Random& rvx) : rv(&rvx) {}
   void tDelete() { rv->tDelete(); delete this; }

public:
   Real Next();
   ExtReal Mean() const;
   ExtReal Variance() const;
   friend NegatedRandom& operator-(Random&);
};

/// Scaled version of a random variable.
/// \internal
class ScaledRandom : public Random
{
protected:
   Random* rv; Real s;
   ScaledRandom(Random& rvx, Real sx) : rv(&rvx), s(sx) {}
   void tDelete() { rv->tDelete(); delete this; }

public:
   Real Next();
   ExtReal Mean() const;
   ExtReal Variance() const;
   friend ScaledRandom& operator*(Real, Random&);
   friend ScaledRandom& operator*(Random&, Real);
   friend ScaledRandom& operator/(Random&, Real);
};

/// Reciprocal of a random variable.
/// \internal
class ReciprocalRandom : public Random
{
protected:
   Random* rv; Real s;
   ReciprocalRandom(Random& rvx, Real sx) : rv(&rvx), s(sx) {}
   void tDelete() { rv->tDelete(); delete this; }

public:
   Real Next();
   ExtReal Mean() const { return Missing; }
   ExtReal Variance() const { return Missing; }
   friend ReciprocalRandom& operator/(Real, Random&);
};

/// Random variable plus a constant.
/// \internal
class ShiftedRandom : public ScaledRandom
{
   ShiftedRandom(Random& rvx, Real sx) : ScaledRandom(rvx, sx) {}

public:
   Real Next();
   ExtReal Mean() const;
   ExtReal Variance() const;
   friend ShiftedRandom& operator+(Real, Random&);
   friend ShiftedRandom& operator+(Random&, Real);
   friend ShiftedRandom& operator-(Random&, Real);
};

/// Constant minus a random variable.
/// \internal
class ReverseShiftedRandom : public ScaledRandom
{
   ReverseShiftedRandom(Random& rvx, Real sx) : ScaledRandom(rvx, sx) {}

public:
   Real Next();
   ExtReal Mean() const;
   ExtReal Variance() const;
   friend ReverseShiftedRandom& operator-(Real, Random&);
};

/// Sum of iid random variables.
/// \internal
class RepeatedRandom : public Random
{
   Random* rv; int n;
   void tDelete() { rv->tDelete(); delete this; }
   RepeatedRandom(Random& rvx, int nx)  : rv(&rvx), n(nx) {}

public:
   Real Next();
   ExtReal Mean() const;
   ExtReal Variance() const;
   friend class Random;
};

/// Product of two random variables.
/// \internal
class MultipliedRandom : public Random
{
protected:
   Random* rv1; Random* rv2;
   void tDelete() { rv1->tDelete(); rv2->tDelete(); delete this; }
   MultipliedRandom(Random& rv1x, Random& rv2x) : rv1(&rv1x), rv2(&rv2x) {}

public:
   Real Next();
   ExtReal Mean() const;
   ExtReal Variance() const;
   friend MultipliedRandom& operator*(Random&, Random&);
};

/// Sum of two random variables.
/// \internal
class AddedRandom : public MultipliedRandom
{
   AddedRandom(Random& rv1x, Random& rv2x) : MultipliedRandom(rv1x, rv2x) {}

public:
   Real Next();
   ExtReal Mean() const;
   ExtReal Variance() const;
   friend AddedRandom& operator+(Random&, Random&);
};

/// Difference of random variables.
/// \internal
class SubtractedRandom : public MultipliedRandom
{
   SubtractedRandom(Random& rv1x, Random& rv2x)
      : MultipliedRandom(rv1x, rv2x) {}

public:
   Real Next();
   ExtReal Mean() const;
   ExtReal Variance() const;
   friend SubtractedRandom& operator-(Random&, Random&);
};

/// Ratio of random variables.
/// \internal
class DividedRandom : public MultipliedRandom
{
   DividedRandom(Random& rv1x, Random& rv2x)
      : MultipliedRandom(rv1x, rv2x) {}

public:
   Real Next();
   ExtReal Mean() const { return Missing; }
   ExtReal Variance() const { return Missing; }
   friend DividedRandom& operator/(Random&, Random&);
};

/// Sum of random variables.
class SumRandom : public Random
{
   void operator=(const SumRandom&) {}    // private so can't access
   Random* rv;

public:
   const char* Name();                    // identification
   SumRandom(NegatedRandom& rvx) : rv(&rvx) {}
   SumRandom(AddedRandom& rvx) : rv(&rvx) {}
   SumRandom(MultipliedRandom& rvx) : rv(&rvx) {}
   SumRandom(SubtractedRandom& rvx) : rv(&rvx) {}
   SumRandom(DividedRandom& rvx) : rv(&rvx) {}
   SumRandom(ShiftedRandom& rvx) : rv(&rvx) {}
   SumRandom(ReverseShiftedRandom& rvx) : rv(&rvx) {}
   SumRandom(ScaledRandom& rvx) : rv(&rvx) {}
   SumRandom(ReciprocalRandom& rvx) : rv(&rvx) {}
   SumRandom(RepeatedRandom& rvx) : rv(&rvx) {}
   Real Next() { return rv->Next(); }
   ExtReal Mean() const { return rv->Mean(); }
   ExtReal Variance() const { return rv->Variance(); }
   ~SumRandom() { rv->tDelete(); }
};

// ******************** Mixtures of random numbers ************************

/// Random variable and mixture probability.
/// \internal
class SelectedRandom : public Random
{
   friend class Random;
   Random* rv; Real p;
   SelectedRandom(Random& rvx, Real px) : rv(&rvx), p(px) {}
   void tDelete() { rv->tDelete(); delete this; }

public:
   void load(int*, Real*, Random**);
   Real Next();
};

/// Sum of two random variables with mixture probabilities.
/// \internal
class AddedSelectedRandom : public Random
{
   friend class Random;

protected:
   Random* rv1; Random* rv2;
   AddedSelectedRandom(Random& rv1x, Random& rv2x)
      : rv1(&rv1x), rv2(&rv2x) {}
   void tDelete() { rv1->tDelete(); rv2->tDelete(); delete this; }

public:
   int nelems() const;
   void load(int*, Real*, Random**);
   Real Next();
   friend AddedSelectedRandom& operator+(SelectedRandom&,
      SelectedRandom&);
   friend AddedSelectedRandom& operator+(AddedSelectedRandom&,
      SelectedRandom&);
   friend AddedSelectedRandom& operator+(SelectedRandom&,
      AddedSelectedRandom&);
   friend AddedSelectedRandom& operator+(AddedSelectedRandom&,
      AddedSelectedRandom&);
};

/// Mixtures of random numbers.
class MixedRandom : public Random 
{
   void operator=(const MixedRandom&) {}  // private so can't access
   int n;                                 // number of components
   DiscreteGen* dg;                       // discrete mixing distribution
   Random** rv;                           // array of pointers to rvs
   ExtReal mean, var;
   void Build(Real*);                     // used by constructors

public:
   const char* Name();                    // identification
   MixedRandom(int, Real*, Random**);
   MixedRandom(AddedSelectedRandom&);
   ~MixedRandom();
   Real Next();
   ExtReal Mean() const { return mean; }
   ExtReal Variance() const { return var; }
};

/// Generate random permutation.
class RandomPermutation
{
   Random U;

public:
   void Next(int N, int M, int p[], int start = 0);
                                          // select permutation of M numbers
                                          // from start, ..., start+N-1
                                          // results to p
   void Next(int N, int p[], int start = 0) { Next(N, N, p, start); }
};

/// Generate random combination.
/// Result is sorted in ascending order
class RandomCombination : public RandomPermutation
{
   void SortAscending(int n, int gm[]);

public:
   void Next(int N, int M, int p[], int start = 0)
      { RandomPermutation::Next(N, M, p, start); SortAscending(M, p); }
   void Next(int N, int p[], int start = 0) { Next(N, N, p, start); }
};

/// Poisson random variable - select parameter when called.
class VariPoisson
{
   Uniform U;
   Normal N;
   Poisson P100;
   Poisson P200;
   Real Next_very_small(Real mu);
   Real Next_small(Real mu);
   Real Next_large(Real mu);
public:
   VariPoisson();
   int iNext(Real mu);
   Real Next(Real mu);
};

/// Binomial random variable - select parameters when called.
class VariBinomial
{
   Uniform U;
   Normal N;
   Real Next_very_small(int n, Real p);
   Real Next_small(int n, Real p);
   Real Next_large(int n, Real p);
public:
   VariBinomial();
   int iNext(int n, Real p);
   Real Next(int n, Real p);
};

/// Log normal random variable - select parameters when called.
class VariLogNormal
{
   Normal N;
public:
   VariLogNormal() {}
   Real Next(Real mean, Real sd);
};

// friend functions declared again outside class definitions
NegatedRandom& operator-(Random&);
ScaledRandom& operator*(Real, Random&);
ScaledRandom& operator*(Random&, Real);
ScaledRandom& operator/(Random&, Real);
ReciprocalRandom& operator/(Real, Random&);
ShiftedRandom& operator+(Real, Random&);
ShiftedRandom& operator+(Random&, Real);
ShiftedRandom& operator-(Random&, Real);
ReverseShiftedRandom& operator-(Real, Random&);
MultipliedRandom& operator*(Random&, Random&);
AddedRandom& operator+(Random&, Random&);
SubtractedRandom& operator-(Random&, Random&);
DividedRandom& operator/(Random&, Random&);
AddedSelectedRandom& operator+(SelectedRandom&, SelectedRandom&);
AddedSelectedRandom& operator+(AddedSelectedRandom&, SelectedRandom&);
AddedSelectedRandom& operator+(SelectedRandom&, AddedSelectedRandom&);
AddedSelectedRandom& operator+(AddedSelectedRandom&, AddedSelectedRandom&);




#ifdef use_namespace
}
#endif

#endif

// body file: newran1.cpp
// body file: newran2.cpp

///@}

