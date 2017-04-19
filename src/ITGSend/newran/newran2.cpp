/// \ingroup newran
///@{

/// \file newran2.cpp
/// Nonuniform random number generators.

#define WANT_STREAM
#define WANT_MATH
#include "include.h"

#include "newran.h"

#ifdef use_namespace
namespace NEWRAN { using namespace RBD_COMMON; }
namespace RBD_LIBRARIES { using namespace NEWRAN; }
namespace NEWRAN {
#endif

//********* classes which are used internally only **********************

/// Chi-square with 1df random number generator.
/// \internal
class ChiSq1 : public Normal              // generate non-central chi-square
					  // rv with 1 degree of freedom
{
   Real deltasq;                          // non-centrality parameter
   Real delta;

public:
   ChiSq1(Real);                          // non-centrality parameter
   ExtReal Mean() const { return 1.0+deltasq; }
   ExtReal Variance() const { return 2.0+4.0*deltasq; }
   Real Next();
};

/// Poisson random number generator, larger mu.
/// \internal
class Poisson1 : public AsymGen           // generate poisson rv, large mu
{
   Real mu, ln_mu;
public:
   Poisson1(Real);                        // constructor (Real=mean)
   Real Density(Real) const;              // Poisson density function
   Real Next() { return floor(AsymGen::Next()); }
   ExtReal Mean() const { return mu; }
   ExtReal Variance() const { return mu; }
};

/// Poisson random number generator, mu <= 10.
/// \internal
class Poisson2 : public Random            // generate poisson rv, large mu
{
   DiscreteGen* dg;
public:
   Poisson2(Real);                        // constructor (Real=mean)
   ~Poisson2();
   Real Next() { return dg->Next(); }
   ExtReal Mean() const { return dg->Mean(); }
   ExtReal Variance() const { return dg->Variance(); }
};

/// Gamma random number generator, alpha <= 1.
/// \internal
class Gamma1 : public PosGen              // generate gamma rv
					  // shape parameter <= 1
{
   Real ln_gam, ralpha, alpha;
public:
   Gamma1(Real);                          // constructor (Real=shape)
   Real Density(Real) const;              // gamma density function
   Real Next();                           // carries out power transform
   ExtReal Mean() const { return alpha; }
   ExtReal Variance() const { return alpha; }
};

/// Weibull::Next()

Real Weibull :: Next()
{
  double rnd;
  rnd = U.Next();
  rnd = scale * pow(log(1/rnd),1/shape);  		//inversa della CDF
  return rnd;
}

/// Extreme_Largest::Next()

Real Extreme_Largest :: Next()
{

  long  double rnd1;
  rnd1 = U.Next();
  long  double op1=log(double(rnd1));
  rnd1=mu-sigma*(log(double(-op1)));
  return rnd1;
}

/// T-Student::Next()
Real Student::Next()
{
  Normal N;
  Gamma G(nu*0.5);
  Real rnd1 = G.Next();
  Real rnd2 = N.Next();
  Real rnd=mu+(sigma*(rnd2*sqrt(nu/(2*rnd1))));
  return rnd;
}

/// Exponential::Next()

Real Exponential :: Next()
{

  double rnd;
  rnd = U.Next();
  rnd = mean*log(1/(1-rnd));
  return rnd;
}


/// Gamma random number generator, alpha > 1.
/// \internal
class Gamma2 : public AsymGen             // generate gamma rv
					  // shape parameter > 1
{
   Real alpha, ln_gam;
public:
   Gamma2(Real);                          // constructor (Real=shape)
   Real Density(Real) const;              // gamma density function
   ExtReal Mean() const { return alpha; }
   ExtReal Variance() const { return alpha; }
};

/// Binomial random number generator, n > 40.
/// \internal
class Binomial1 : public AsymGen           // generate binomial rv, larger n
{
   Real p, q, ln_p, ln_q, ln_n_fac; int n;
public:
   Binomial1(int nx, Real px);
   Real Density(Real) const;
   Real Next() { return floor(AsymGen::Next()); }
   ExtReal Mean() const { return p * n; }
   ExtReal Variance() const { return p * q * n; }
};

/// Binomial random number generator, n < 40 or n*p <= 8.
/// \internal
class Binomial2 : public Random            // generate binomial rv, smaller n
{
   DiscreteGen* dg;
public:
   Binomial2(int nx, Real px);
   ~Binomial2();
   Real Next() { return dg->Next(); }
   ExtReal Mean() const { return dg->Mean(); }
   ExtReal Variance() const { return dg->Variance(); }
};

//************************ static variables ***************************

Real Normal::Nxi;
Real* Normal::Nsx;
Real* Normal::Nsfx;
long Normal::count=0;

//**************************** utilities ******************************

inline Real square(Real x) { return x*x; }
inline ExtReal square(const ExtReal& x) { return x*x; }

static void ErrorNoSpace() { Throw(Bad_alloc("Newran: out of space")); }

//************************* end of definitions ************************

static int Round(Real f)
{
   int i = (int)floor(f + 0.5);
   if (fabs((Real)i - f) > 0.5)
   {
      Tracer et("Round ");
      Throw(Runtime_error("Cannot convert to integer "));
   }
   return i;
} 

PosGen::PosGen()                             // Constructor
{
   #ifdef MONITOR
      cout << "constructing PosGen\n";
   #endif
   NotReady=true;
}

PosGen::~PosGen()
{
   if (!NotReady)
   {
      #ifdef MONITOR
	 cout << "freeing PosGen arrays\n";
      #endif
      delete [] sx; delete [] sfx;
   }
   #ifdef MONITOR
      cout << "destructing PosGen\n";
   #endif
}

void PosGen::Build(bool sym)                 // set up arrays
{
   #ifdef MONITOR
      cout << "building PosGen arrays\n";
   #endif
   int i;
   NotReady=false;
   sx=new Real[60]; sfx=new Real[60];
   if (!sx || !sfx) ErrorNoSpace();
   Real sxi=0.0; Real inc = sym ? 0.01 : 0.02;
   for (i=0; i<60; i++)
   {
      sx[i]=sxi; Real f1=Density(sxi); sfx[i]=f1;
      if (f1<=0.0) goto L20;
      sxi+=inc/f1;
   }
   Throw(Runtime_error("Newran: area too large"));
L20:
   if (i<50) Throw(Runtime_error("Newran: area too small"));
   xi = sym ? 2*i : i;
   return;
}

Real PosGen::Next()
{
   Real ak,y; int ir;
   if (NotReady) Build(false);
   do
   {
      Real r1=RNG->Next();
      ir = (int)(r1*xi); Real sxi=sx[ir];
      ak=sxi+(sx[ir+1]-sxi) * RNG->Next();
      y=sfx[ir] * RNG->Next();
   }
   while ( y>=sfx[ir+1] && y>=Density(ak) );
   return ak;
}

Real SymGen::Next()
{
   Real s,ak,y; int ir;
   if (NotReady) Build(true);
   do
   {
      s=1.0;
      Real r1=RNG->Next();
      if (r1>0.5) { s=-1.0; r1=1.0-r1; }
      ir = (int)(r1*xi); Real sxi=sx[ir];
      ak=sxi+(sx[ir+1]-sxi)*RNG->Next();
      y=sfx[ir]*RNG->Next();
   }
   while ( y>=sfx[ir+1] && y>=Density(ak) );
   return s*ak;
}

AsymGen::AsymGen(Real modex)                 // Constructor
{
   #ifdef MONITOR
      cout << "constructing AsymGen\n";
   #endif
   mode=modex; NotReady=true;
}

void AsymGen::Build()                        // set up arrays
{
   #ifdef MONITOR
      cout << "building AsymGen arrays\n";
   #endif
   int i;
   NotReady=false;
   sx=new Real[121]; sfx=new Real[121];
   if (!sx || !sfx)  ErrorNoSpace();
   Real sxi=mode;
   for (i=0; i<120; i++)
   {
      sx[i]=sxi; Real f1=Density(sxi); sfx[i]=f1;
      if (f1<=0.0) goto L20;
      sxi+=0.01/f1;
   }
   Throw(Runtime_error("Newran: area too large (a)"));
L20:
   ic=i-1; sx[120]=sxi; sfx[120]=0.0;
   sxi=mode;
   for (; i<120; i++)
   {
      sx[i]=sxi; Real f1=Density(sxi); sfx[i]=f1;
      if (f1<=0.0) goto L30;
      sxi-=0.01/f1;
   }
   Throw(Runtime_error("Newran: area too large (b)"));
L30:
   if (i<100)  Throw(Runtime_error("Newran: area too small"));
   xi=i;
   return;
}

Real AsymGen::Next()
{
   Real ak,y; int ir1;
   if (NotReady) Build();
   do
   {
      Real r1=RNG->Next();
      int ir=(int)(r1*xi); Real sxi=sx[ir];
      ir1 = (ir==ic) ? 120 : ir+1;
      ak=sxi+(sx[ir1]-sxi)*RNG->Next();
      y=sfx[ir]*RNG->Next();
   }
   while ( y>=sfx[ir1] && y>=Density(ak) );
   return ak;
}

AsymGen::~AsymGen()
{
   if (!NotReady)
   {
      #ifdef MONITOR
	       cout << "freeing AsymGen arrays\n";
      #endif
      delete [] sx; delete [] sfx;
   }
   #ifdef MONITOR
      cout << "destructing AsymGen\n";
   #endif
}

PosGenX::PosGenX(PDF fx) { f=fx; }

SymGenX::SymGenX(PDF fx) { f=fx; }

AsymGenX::AsymGenX(PDF fx, Real mx) : AsymGen(mx) { f=fx; }


Normal::Normal()
{
   if (count) { NotReady=false; xi=Nxi; sx=Nsx; sfx=Nsfx; }
   else { Build(true); Nxi=xi; Nsx=sx; Nsfx=sfx; }
   count++;
}

Normal::~Normal()
{
   count--;
   if (count) NotReady=true;                     // disable freeing arrays
}

Real Normal::Density(Real x) const               // normal density
{ return (fabs(x)>8.0) ? 0 : 0.398942280 * exp(-x*x / 2); }

ChiSq1::ChiSq1(Real d) : Normal()                // chisquare with 1 df
{ deltasq=d; delta=sqrt(d); }

Real ChiSq1::Next()
{ Real z=Normal::Next()+delta; return z*z; }

ChiSq::ChiSq(int df, Real noncen)
{
  if (df<=0 || noncen<0.0) Throw(Logic_error("Newran: illegal parameters"));
  else if (df==1) { version=0; c1=new ChiSq1(noncen); }
  else if (noncen==0.0)
  {
     if (df==2) { version=1; c1=new Exponential(); }
     else { version=2; c1=new Gamma2(0.5*df); }
  }
  else if (df==2) { version=3; c1=new ChiSq1(noncen/2.0); }
  else if (df==3) { version=4; c1=new Exponential(); c2=new ChiSq1(noncen); }
  else { version=5; c1=new Gamma2(0.5*(df-1)); c2=new ChiSq1(noncen); }
  if (!c1 || (version>3 && !c2)) ErrorNoSpace();
  mean=df+noncen; var=2*df+4.0*noncen;
}

ChiSq::~ChiSq() { delete c1; if (version>3) delete c2; }

Real ChiSq::Next()
{
   switch(version)
   {
   case 0: return c1->Next();
   case 1: case 2: return c1->Next()*2.0;
   case 3: return c1->Next() + c1->Next();
   case 4: case 5: Real s1 = c1->Next()*2.0; Real s2 = c2->Next();
	   return s1 + s2; // this is to make it work with Microsoft VC5
   }
   return 0;
}

Pareto::Pareto(Real shape) : Shape(shape)
{
   if (Shape <= 0) Throw(Logic_error("Newran: illegal parameter"));
   RS = -1.0 / Shape;
}

Real Pareto::Next()
{ return pow(RNG->Next(), RS); }

ExtReal Pareto::Mean() const
{
   if (Shape > 1) return Shape/(Shape-1.0);
   else return PlusInfinity;
}

ExtReal Pareto::Variance() const
{
   if (Shape > 2) return Shape/(square(Shape-1.0))/(Shape-2.0);
   else return PlusInfinity;
}

Real Cauchy::Density(Real x) const               // Cauchy density function
{ return (fabs(x)>1.0e15) ? 0 : 0.31830988618 / (1.0+x*x); }

Poisson1::Poisson1(Real mux) : AsymGen(mux)      // Constructor
{ mu=mux; ln_mu=log(mu); }

Real Poisson1::Density(Real x) const             // Poisson density function
{
   if (x < 0.0) return 0.0;
   double ix = floor(x);                         // use integer part
   double l = ln_mu * ix - mu - ln_gamma(1.0 + ix);
   return  (l < -40.0) ? 0.0 : exp(l);
}

Binomial1::Binomial1(int nx, Real px)
   : AsymGen((nx + 1) * px), p(px), q(1.0 - px), n(nx)
      { ln_p = log(p); ln_q = log(q); ln_n_fac = ln_gamma(n+1); }

Real Binomial1::Density(Real x) const            // Binomial density function
{
   double ix = floor(x);                         // use integer part
   if (ix < 0.0 || ix > n) return 0.0;
   double l = ln_n_fac - ln_gamma(ix+1) - ln_gamma(n-ix+1)
      + ix * ln_p + (n-ix) * ln_q;
   return  (l < -40.0) ? 0.0 : exp(l);
}

Poisson2::Poisson2(Real mux)
{
   Real probs[40];
   probs[0]=exp(-mux);
   for (int i=1; i<40; i++) probs[i]=probs[i-1]*mux/i;
   dg=new DiscreteGen(40,probs);
   if (!dg) ErrorNoSpace();
}

Poisson2::~Poisson2() { delete dg; }

Binomial2::Binomial2(int nx, Real px)
{
   Real qx = 1.0 - px;
   Real probs[40];
   int k = (int)(nx * px);
   probs[k] = exp(ln_gamma(nx+1) - ln_gamma(k+1) - ln_gamma(nx-k+1)
      + k * log(px) + (nx-k) * log(qx));
   int i;
   int m = (nx >= 40) ? 39 : nx;
   for (i=k+1; i<=m; i++) probs[i]=probs[i-1] * px * (nx-i+1) / qx / i;
   for (i=k-1; i>=0; i--) probs[i]=probs[i+1] * qx * (i+1) / px / (nx-i);
   dg = new DiscreteGen(m + 1, probs);
   if (!dg) ErrorNoSpace();
}

Binomial2::~Binomial2() { delete dg; }

Real Exponential::Density(Real x) const          // Negative exponential
{ return  (x > 5*mean || x < 0.0) ? 0.0 : (1/mean)*exp(-(x/mean)); }


Poisson::Poisson(Real mu)
{
   if (mu <= 8.0) method = new Poisson2(mu);
   else method = new Poisson1(mu);
   if (!method) ErrorNoSpace();
}

Binomial::Binomial(int nx, Real px)
{
   if (nx < 40 || nx * px <= 8.0) method = new Binomial2(nx, px);
   else method = new Binomial1(nx, px);
   if (!method) ErrorNoSpace();
}

NegativeBinomial::NegativeBinomial(Real NX, Real PX)
   : AsymGen(0.0), N(NX), P(PX), Q(1.0 + PX)
{
   p = 1.0 / Q;  ln_q = log(1.0 - p);
   c = N * log(p) - ln_gamma(N);  mode = (N - 1) * P;
   if (mode < 1.0) mode = 0.0;
}

Real NegativeBinomial::Next() { return floor(AsymGen::Next()); }

Real NegativeBinomial::Density(Real x) const
{
   if (x < 0.0) return 0.0;
   Real ix = floor(x);
   Real l = c + ln_gamma(ix + N) - ln_gamma(ix + 1) + ix * ln_q;
   return  (l < -40.0) ? 0.0 : exp(l);
}

Gamma1::Gamma1(Real alphax)                      // constructor (Real=shape)
{ ralpha=1.0/alphax; ln_gam=ln_gamma(alphax+1.0); alpha=alphax; }

Real Gamma1::Density(Real x) const               // density function for
{                                                // transformed gamma
   Real l = - pow(x,ralpha) - ln_gam;
   return  (l < -40.0) ? 0.0 : exp(l);
}

Real Gamma1::Next()                               // transform variable
{ return pow(PosGen::Next(),ralpha); }

Gamma2::Gamma2(Real alphax) : AsymGen(alphax-1.0) // constructor (Real=shape)
{ alpha=alphax; ln_gam=ln_gamma(alpha); }

Real Gamma2::Density(Real x) const                // gamma density function
{
   if (x<=0.0) return 0.0;
   double l = (alpha-1.0)*log(x) - x - ln_gam;
   return  (l < -40.0) ? 0.0 : exp(l);
}

Gamma::Gamma(Real alpha)                         // general gamma generator
{
   if (alpha<1.0) method = new Gamma1(alpha);
   else if (alpha==1.0) method = new Exponential();
   else method = new Gamma2(alpha);
   if (!method)  ErrorNoSpace();
}


Stable::Stable(Real a, Real bx, Notation n)
   : alpha(a), notation(n), piby2(1.570796327), piby4(0.785398163)
{

   if (notation == Kalpha)
   {
      if (a == 1.0) bprime = bx;
      else if (fabs(bx) == 1.0)  bprime = (alpha > 1.0) ? -bx : bx;
      else bprime = tan(piby2 * k(alpha) * bx) / tan(piby2 * alpha);
      beta = bx;
   }
   else if (notation == Chambers || notation == Standard) bprime = bx;
   else if (notation == Default)
   {
      notation = Chambers; bprime = bx;
      if ( bx != 0.0 && alpha != 1 && alpha != 2)
         Throw(Logic_error("Newran: must specify Stable notation"));
   }
   else Throw(Logic_error("Newran: Stable notation not recognised"));
}

//compute (exp(x) - 1) / x
Real Stable::d2(Real z)
{
   const Real p1 = 840.066852536483239;
   const Real p2 = 20.0011141589964569;
   const Real q1 = 1680.13370507296648;
   const Real q2 = 180.013370407390023;
   if (fabs(z) <= 0.1)
   {
      Real zz = z * z;
      Real pv = p1 + zz * p2;
      return 2.0 * pv / (q1 + zz * (q2 + zz) - z * pv);
   }
   else return (exp(z) - 1.0) / z;
}

// compute tan(x) / x
Real Stable::tan2(Real xarg)
{
   const Real p0 =  129.221035;
   const Real p1 = -8.87662377;
   const Real p2 =  0.0528644456;
   const Real q0 =  164.529332;
   const Real q1 = -45.1320561;
   Real x = fabs(xarg);
   if (x <= piby4)
   {
      x /= piby4;
      Real xx = x * x;
      return (p0 + xx * (p1 + xx * p2))
         / (piby4 * (q0 + xx * (q1 + xx)));
   }
   else return tan(xarg) / xarg;
}

Real Stable::k(Real a) { return 1.0 - fabs(1.0 - a); }


Real Stable::Next()
{
   Real u = RNG->Next(); Real w = -log(RNG->Next());
   Real eps = 1.0 - alpha;
   Real phiby2 = piby2 * (u - 0.5);
   Real a = tan(phiby2);
   Real bb = tan2(eps * phiby2);
   Real b = eps * phiby2 * bb;
   Real tau = ( eps > -0.99) ? bprime / (tan2(eps * piby2) * piby2)
      : bprime * eps * tan(alpha * piby2);
   Real a2 = 1.0 - a * a; Real a2p = 1.0 + a * a;
   Real b2 = 1.0 - b * b; Real b2p = 1.0 + b * b;
   Real z = a2p * (b2 + 2.0 * phiby2 * bb * tau) / (w * a2 * b2p);
   Real alogz = log(z) / alpha;
   Real d = d2(eps * alogz) * alogz;
   Real Sprime = (1.0 + eps * d) * 2.0 * ((a - b) * (1.0 + a * b)
      - phiby2 * tau * bb * (b * a2 - 2.0 * a)) / (a2 * b2p) + tau * d;
   
   if (notation == Chambers || bprime == 0.0 || alpha == 1.0 || alpha == 2.0)
      return Sprime;
   else if (notation == Standard) return Sprime + bprime * tan(piby2 * alpha);
   else
   {
      Real arg = piby2 * k(alpha) * beta;
      return ( Sprime + tan(arg) ) * pow( cos(arg), 1.0 / alpha );
   }
}

ExtReal Stable::Mean() const
{
   if (alpha < 1.0) return (bprime == 1.0) ? PlusInfinity :
      (bprime == -1.0) ? MinusInfinity : Indefinite;
   else if (alpha == 1.0) return Indefinite;
   else if (notation == Chambers) return -bprime * tan(piby2 * alpha);
   else return 0.0;
}


ExtReal Stable::Variance() const
{
   return (alpha < 2.0) ? PlusInfinity : 0.5;
}


DiscreteGen::DiscreteGen(int n1, Real* prob)     // discrete generator
						 // values on 0,...,n1-1
{
   #ifdef MONITOR
      cout << "constructing DiscreteGen\n";
   #endif
   Gen(n1, prob); val=0;
   mean=0.0; var=0.0;
   { for (int i=0; i<n; i++) mean = mean + i*prob[i]; }
   { for (int i=0; i<n; i++) var = var + square(i-mean) * prob[i]; }
}

DiscreteGen::DiscreteGen(int n1, Real* prob, Real* val1)
                                                 // discrete generator
                                                 // values on *val
{
   #ifdef MONITOR
      cout << "constructing DiscreteGen\n";
   #endif
   Gen(n1, prob); val = new Real[n1];
   if (!val)  ErrorNoSpace();
   for (int i=0; i<n1; i++) val[i]=val1[i];
   mean=0.0; var=0.0;
   { for (int i=0; i<n; i++) mean = mean + val[i]*prob[i]; }
   { for (int i=0; i<n; i++) var = var + square(val[i]-mean)*prob[i]; }
}


void DiscreteGen::Gen(int n1, Real* prob)
{
   n=n1;                                         // number of values
   p=new Real[n]; ialt=new int[n];
   if (!p || !ialt)  ErrorNoSpace();
   Real rn = 1.0/n; Real px = 0; int i;
   for (i=0; i<n; i++) { p[i]=0.0; ialt[i]=-1; }
   for (i=0; i<n; i++)
   {
      Real pmin=1.0; Real pmax=-1.0; int jmin=-1; int jmax=-1;
      for (int j=0; j<n; j++)
      {
         if (ialt[j]<0)
         {
            px=prob[j]-p[j];
            if (pmax<=px) { pmax=px; jmax=j; }
            if (pmin>=px) { pmin=px; jmin=j; }
         }
      }
      if ((jmax<0) || (jmin<0)) Throw(Runtime_error("Newran: method fails"));
      ialt[jmin]=jmax; px=rn-pmin; p[jmax]+=px; px*=n; p[jmin]=px;
      if ((px>1.00001)||(px<-.00001))
         Throw(Runtime_error("Newran: probs don't add to 1 (a)"));
   }
   if (px>0.00001) Throw(Runtime_error("Newran: probs don't add to 1 (b)"));
}

DiscreteGen::~DiscreteGen()
{
   delete [] p; delete [] ialt; delete [] val;
   #ifdef MONITOR
      cout << "destructing DiscreteGen\n";
   #endif
}

Real DiscreteGen::Next()                  // Next discrete random variable
{
   int i = (int)(n*RNG->Next()); if (RNG->Next()<p[i]) i=ialt[i];
   return val ? val[i] : (Real)i;
}

// log gamma function

static Real ln_gamma_asymp(Real z);
static Real ln_gamma_near_1(Real z);
static Real ln_gamma_near_2(Real z);
static Real ln_gamma_gt_1(Real z);

Real gamma_series(long_Real z)
{
   const long_Real n0 = 1.846356774800897077637235e6L;
   const long_Real n1 = 1.759131712935803984850945e6L;
   const long_Real n2 = 7.542124083269936035445648e5L;
   const long_Real n3 = 1.916219552338091802379555e5L;
   const long_Real n4 = 3.194965924862382624981206e4L;
   const long_Real n5 = 3.652838209061050933543152e3L;
   const long_Real n6 = 2.900228320454639341680104e2L;
   const long_Real n7 = 1.578981962865355560648172e1L;
   const long_Real n8 = 0.564145967416346085128381L;
   const long_Real n9 = 0.119443368011180931171494e-1L;
   const long_Real n10 = 0.113800747608906017093789e-3L;
   const long_Real d0 = 3628800;
   const long_Real d1 = 10628640;
   const long_Real d2 = 12753576;
   const long_Real d3 = 8409500;
   const long_Real d4 = 3416930;
   const long_Real d5 = 902055;
   const long_Real d6 = 157773;
   const long_Real d7 = 18150;
   const long_Real d8 = 1320;
   const long_Real d9 = 55;
   const long_Real d10 = 1;
   
   if (z < 10.0)
   {
      const long_Real num = n0 + z * (n1 + z * (n2 + z * (n3 + z * (n4 + z * (n5 
       + z * (n6 + z * (n7 + z * (n8 + z * (n9 + z * n10)))))))));
      const long_Real den = d0 + z * (d1 + z * (d2 + z * (d3 + z * (d4 + z * (d5 
       + z * (d6 + z * (d7 + z * (d8 + z * (d9 + z)))))))));
      return (Real)(num / den);
   }
   else
   {
      long_Real r = 1.0L / z; 
      const long_Real num = n10 + r * (n9 + r * (n8 + r * (n7 + r * (n6 + r * (n5 
       + r * (n4 + r * (n3 + r * (n2 + r * (n1 + r * n0)))))))));
      const long_Real den = d10 + r * (d9 + r * (d8 + r * (d7 + r * (d6 + r * (d5 
       + r * (d4 + r * (d3 + r * (d2 + r * (d1 + r * d0)))))))));
      return (Real)(num / den);
   }
}

// version for z > 100
Real ln_gamma_asymp(Real z)
{
   long_Real z2 = z * z;
   return
      (z - 0.5) * log(z) - z + 0.9189385332046727417L
      + (1.0L/12.0L + (-1.0L/360.0L + (1.0L/1260.0L - 1.0L/1680.0L/z2)/z2)/z2)/z;
}

// version for z > 0.9
Real ln_gamma_gt_1(Real z)
{
   Tracer et("ln_gamma_gt_1 ");
   if (z >= 100) return ln_gamma_asymp(z);
   if (z >= 2.1) return ln_gamma_lanczos(z);
   if (z >= 1.9) return ln_gamma_near_2(z);   
   if (z >= 1.1) return ln_gamma_lanczos(z);
   if (z >= 0.9) return ln_gamma_near_1(z);
   Throw(Invalid_argument("Argument less than 1"));
#ifndef UseExceptions
   return 0;
#endif
}

// version for z > 1 - Lanczos method
Real ln_gamma_lanczos(Real z)
{
   Real zg5 = gamma_g_value() + z - 0.5;
   return log(gamma_series(z - 1.0)) + (z - 0.5) * (log(zg5) - 1.0);
}

//for z near 1
Real ln_gamma_near_1(Real z)
{
   long_Real z1 = z - 1;
   return
        z1 * (-0.57721566490153286061L
      + z1 * ( 1.64493406684822643647L / 2
      + z1 * (-1.20205690315959428540L / 3
      + z1 * ( 1.08232323371113819152L / 4
      + z1 * (-1.03692775514336992633L / 5
      + z1 * ( 1.01734306198444913971L / 6
      + z1 * (-1.00834927738192282684L / 7
      + z1 * ( 1.00407735619794433938L / 8
      + z1 * (-1.00200838282608221442L / 9
      + z1 * ( 1.00099457512781808534L / 10
      + z1 * (-1.00049418860411946456L / 11
      + z1 * ( 1.00024608655330804830L / 12
      + z1 * (-1.00012271334757848915L / 13
      + z1 * ( 1.00006124813505870483L / 14
      + z1 * (-1.00003058823630702049L / 15
      + z1 * ( 1.00001528225940865187L / 16
      + z1 * (-1.00000763719763789976L / 17
      + z1 * ( 1.00000381729326499984L / 18
      + z1 * (-1.00000190821271655394L / 19
      + z1 *   1.00000095396203387280L / 20 )))))))))))))))))));
}

//for z near 2
Real ln_gamma_near_2(Real z)
{
   long_Real z1 = z - 2;
   return
        z1 * (1 - 0.57721566490153286061L
      + z1 * ( 0.64493406684822643647L / 2
      + z1 * (-0.20205690315959428540L / 3
      + z1 * ( 0.08232323371113819152L / 4
      + z1 * (-0.03692775514336992633L / 5
      + z1 * ( 0.01734306198444913971L / 6
      + z1 * (-0.00834927738192282684L / 7
      + z1 * ( 0.00407735619794433938L / 8
      + z1 * (-0.00200838282608221442L / 9
      + z1 * ( 0.00099457512781808534L / 10
      + z1 * (-0.00049418860411946456L / 11
      + z1 * ( 0.00024608655330804830L / 12
      + z1 * (-0.00012271334757848915L / 13
      + z1 * ( 0.00006124813505870483L / 14
      + z1 * (-0.00003058823630702049L / 15)))))))))))))));
}


Real ln_gamma(Real z, int& sign)
{
   Tracer et("ln_gamma(sign)");
   if (z >= 2.1) { sign = 1; return ln_gamma_gt_1(z); }
   if (z >= 1.9) { sign = 1; return ln_gamma_near_2(z); }   
   if (z >= 1.1) { sign = 1; return ln_gamma_gt_1(z); }
   if (z >= 0.9) { sign = 1; return ln_gamma_near_1(z); }
   else                           // Use reflection formula
   {
      Real cz = ceil(z); Real cz2 = cz / 2.0;
      sign = fabs(ceil(cz2) - cz2) > 0.25 ? 1 : -1;
      Real fz = cz - z;
      if (fz == 0) Throw(Invalid_argument("Non-positive integer argument "));
      Real piz = 3.14159265358979323846 * (1.0 - z);
      return
         log(piz / sin(3.14159265358979323846 * fz)) - ln_gamma_gt_1(2.0 - z);
   }
}

Real ln_gamma(Real z)
{
   Tracer et("ln_gamma");
   int sign;
   Real lg = ln_gamma(z, sign);
   if (sign < 0) Throw(Invalid_argument("Negative gamma value "));
   return lg;
}


Real rbd_gamma(Real z)
{
   Tracer et("rbd_gamma");
   if (z>=1.0)                           // Use reflection formula
   {
      Real zg5 = gamma_g_value() + z - 0.5;
      return gamma_series(z - 1.0)
         * pow(zg5 / 2.7182818284590452353, z - 0.5);
   }
   else
   {
      Real cz = ceil(z); Real cz2 = cz / 2.0;
      Real fz = cz - z;
      if (fz == 0) Throw(Invalid_argument("Non-positive integer argument "));
      Real piz = 3.14159265358979323846 * (1.0 - z);
      Real v = (piz / sin(3.14159265358979323846 * fz)) / rbd_gamma(2.0 - z);
      return fabs(ceil(cz2) - cz2) > 0.25 ? v : -v; 
   }
}


Real NegatedRandom::Next() { return - rv->Next(); }

ExtReal NegatedRandom::Mean() const { return - rv->Mean(); }

ExtReal NegatedRandom::Variance() const { return rv->Variance(); }

Real ScaledRandom::Next() { return rv->Next() * s; }

ExtReal ScaledRandom::Mean() const { return rv->Mean() * s; }

ExtReal ScaledRandom::Variance() const { return rv->Variance() * (s*s); }

Real ShiftedRandom::Next() { return rv->Next() + s; }

ExtReal ShiftedRandom::Mean() const { return rv->Mean() + s; }

ExtReal ShiftedRandom::Variance() const { return rv->Variance(); }

Real ReverseShiftedRandom::Next() { return s - rv->Next(); }

ExtReal ReverseShiftedRandom::Mean() const { return - rv->Mean() + s; }

ExtReal ReverseShiftedRandom::Variance() const { return rv->Variance(); }

Real ReciprocalRandom::Next() { return s / rv->Next(); }

ExtReal RepeatedRandom::Mean() const { return rv->Mean() * (Real)n; }

ExtReal RepeatedRandom::Variance() const { return rv->Variance() * (Real)n; }

RepeatedRandom& Random::operator()(int n)
{
   RepeatedRandom* r = new RepeatedRandom(*this, n);
   if (!r) ErrorNoSpace(); return *r;
}

NegatedRandom& operator-(Random& rv)
{
   NegatedRandom* r = new NegatedRandom(rv);
   if (!r) ErrorNoSpace(); return *r;
}

ShiftedRandom& operator+(Random& rv, Real s)
{
   ShiftedRandom* r = new ShiftedRandom(rv, s);
   if (!r) ErrorNoSpace(); return *r;
}

ShiftedRandom& operator-(Random& rv, Real s)
{
   ShiftedRandom* r = new ShiftedRandom(rv, -s);
   if (!r) ErrorNoSpace(); return *r;
}

ScaledRandom& operator*(Random& rv, Real s)
{
   ScaledRandom* r = new ScaledRandom(rv, s);
   if (!r) ErrorNoSpace(); return *r;
}

ShiftedRandom& operator+(Real s, Random& rv)
{
   ShiftedRandom* r = new ShiftedRandom(rv, s);
   if (!r) ErrorNoSpace(); return *r;
}

ReverseShiftedRandom& operator-(Real s, Random& rv)
{
   ReverseShiftedRandom* r = new ReverseShiftedRandom(rv, s);
   if (!r) ErrorNoSpace(); return *r;
}

ScaledRandom& operator*(Real s, Random& rv)
{
   ScaledRandom* r = new ScaledRandom(rv, s);
   if (!r) ErrorNoSpace(); return *r;
}

ScaledRandom& operator/(Random& rv, Real s)
{
   ScaledRandom* r = new ScaledRandom(rv, 1.0/s);
   if (!r) ErrorNoSpace(); return *r;
}

ReciprocalRandom& operator/(Real s, Random& rv)
{
   ReciprocalRandom* r = new ReciprocalRandom(rv, s);
   if (!r) ErrorNoSpace(); return *r;
}

AddedRandom& operator+(Random& rv1, Random& rv2)
{
   AddedRandom* r = new AddedRandom(rv1, rv2);
   if (!r) ErrorNoSpace(); return *r;
}

MultipliedRandom& operator*(Random& rv1, Random& rv2)
{
   MultipliedRandom* r = new MultipliedRandom(rv1, rv2);
   if (!r) ErrorNoSpace(); return *r;
}

SubtractedRandom& operator-(Random& rv1, Random& rv2)
{
   SubtractedRandom* r = new SubtractedRandom(rv1, rv2);
   if (!r) ErrorNoSpace(); return *r;
}

DividedRandom& operator/(Random& rv1, Random& rv2)
{
   DividedRandom* r = new DividedRandom(rv1, rv2);
   if (!r) ErrorNoSpace(); return *r;
}

Real AddedRandom::Next()
   { Real r1 = rv1->Next(); Real r2 = rv2->Next(); return r1 + r2; }

ExtReal AddedRandom::Mean() const { return rv1->Mean() + rv2->Mean() ; }

ExtReal AddedRandom::Variance() const
   { return rv1->Variance() + rv2->Variance() ; }

Real SubtractedRandom::Next()
   { Real r1 = rv1->Next(); Real r2 = rv2->Next(); return r1 - r2; }

ExtReal SubtractedRandom::Mean() const { return rv1->Mean() - rv2->Mean() ; }

ExtReal SubtractedRandom::Variance() const
   { return rv1->Variance() + rv2->Variance() ; }

Real MultipliedRandom::Next()
   { Real r1 = rv1->Next(); Real r2 = rv2->Next(); return r1 * r2; }

ExtReal MultipliedRandom::Mean() const { return rv1->Mean() * rv2->Mean() ; }

ExtReal MultipliedRandom::Variance() const
{
   ExtReal e1 = square(rv1->Mean()); ExtReal e2 = square(rv2->Mean());
   ExtReal v1 = rv1->Variance(); ExtReal v2 = rv2->Variance();
   ExtReal r=v1*v2+v1*e2+e1*v2;
   return r;
}

Real DividedRandom::Next()
   { Real r1 = rv1->Next(); Real r2 = rv2->Next(); return r1 / r2; }

void Random::load(int*,Real*,Random**)
   { Throw(Logic_error("Newran: illegal combination")); }

void SelectedRandom::load(int* i, Real* probs, Random** rvx)
{
   probs[*i]=p; rvx[*i]=rv; (*i)++;
   delete this;
}

Real SelectedRandom::Next()
{
   Throw(Logic_error("Newran: Next not defined"));
#ifndef UseExceptions
   return 0;
#endif
}

Real AddedSelectedRandom::Next()
{
   Throw(Logic_error("Newran: Next not defined"));
#ifndef UseExceptions
   return 0;
#endif
}

Real RepeatedRandom::Next()
   { Real sum=0.0; for (int i=0; i<n; i++) sum += rv->Next(); return sum; }

MixedRandom::MixedRandom(int nx, Real* probs, Random** rvx)
{
   n = nx;
   rv = new Random*[n]; if (!rv) ErrorNoSpace();
   for (int i=0; i<n; i++) rv[i]=rvx[i];
   Build(probs);
}

MixedRandom::MixedRandom(AddedSelectedRandom& sr)
{
   n = sr.nelems();                       // number of terms;
   Real* probs = new Real[n]; rv = new Random*[n];
   if (!probs || !rv) ErrorNoSpace();
   int i=0; sr.load(&i,probs,rv);
   Build(probs); delete [] probs;
}

void MixedRandom::Build(Real* probs)
{
   int i;
   dg=new DiscreteGen(n,probs);
   if (!dg) ErrorNoSpace();
   mean=0.0; var=0.0;
   for (i=0; i<n; i++) mean = mean + (rv[i])->Mean()*probs[i];
   for (i=0; i<n; i++)
   {
      ExtReal sigsq=(rv[i])->Variance();
      ExtReal mudif=(rv[i])->Mean()-mean;
      var = var + (sigsq+square(mudif))*probs[i];
   }

}

MixedRandom::~MixedRandom()
{
   for (int i=0; i<n; i++) rv[i]->tDelete();
   delete dg; delete [] rv;
}

Real MixedRandom::Next()
   { int i = (int)(dg->Next()); return (rv[i])->Next(); }

int AddedSelectedRandom::nelems() const
   { return rv1->nelems() + rv2->nelems(); }

void AddedSelectedRandom::load(int* i, Real* probs, Random** rvx)
{
   rv1->load(i, probs, rvx); rv2->load(i, probs, rvx);
   delete this;
}

AddedSelectedRandom& operator+(SelectedRandom& rv1,
   SelectedRandom& rv2)
{
   AddedSelectedRandom* r = new AddedSelectedRandom(rv1, rv2);
   if (!r) ErrorNoSpace(); return *r;
}

AddedSelectedRandom& operator+(AddedSelectedRandom& rv1,
   SelectedRandom& rv2)
{
   AddedSelectedRandom* r = new AddedSelectedRandom(rv1, rv2);
   if (!r) ErrorNoSpace(); return *r;
}

AddedSelectedRandom& operator+(SelectedRandom& rv1,
   AddedSelectedRandom& rv2)
{
   AddedSelectedRandom* r = new AddedSelectedRandom(rv1, rv2);
   if (!r) ErrorNoSpace(); return *r;
}

AddedSelectedRandom& operator+(AddedSelectedRandom& rv1,
   AddedSelectedRandom& rv2)
{
   AddedSelectedRandom* r = new AddedSelectedRandom(rv1, rv2);
   if (!r) ErrorNoSpace(); return *r;
}

SelectedRandom& Random::operator()(double p)
{
   SelectedRandom* r = new SelectedRandom(*this, p);
   if (!r) ErrorNoSpace(); return *r;
}




// Identification routines for each class - may not work on all compilers?

const char* Uniform::Name()           { return "Uniform";          }
const char* Constant::Name()          { return "Constant";         }
const char* PosGen::Name()            { return "PosGen";           }
const char* SymGen::Name()            { return "SymGen";           }
const char* AsymGen::Name()           { return "AsymGen";          }
const char* PosGenX::Name()           { return "PosGenX";          }
const char* SymGenX::Name()           { return "SymGenX";          }
const char* AsymGenX::Name()          { return "AsymGenX";         }
const char* Normal::Name()            { return "Normal";           }
const char* ChiSq::Name()             { return "ChiSq";            }
const char* Cauchy::Name()            { return "Cauchy";           }
const char* Exponential::Name()       { return "Exponential";      }
const char* Poisson::Name()           { return "Poisson";          }
const char* Binomial::Name()          { return "Binomial";         }
const char* NegativeBinomial::Name()  { return "NegativeBinomial"; }
const char* Gamma::Name()             { return "Gamma";            }
const char* Pareto::Name()            { return "Pareto";           }
const char* DiscreteGen::Name()       { return "DiscreteGen";      }
const char* SumRandom::Name()         { return "SumRandom";        }
const char* MixedRandom::Name()       { return "MixedRandom";      }
const char* Stable::Name()            { return "Stable";           }
const char* Extreme_Largest::Name()   { return "Extreme Largest";  }
const char* Weibull::Name()       	{ return "Weibull";	     }
const char* Student::Name()		{ return "T locatio-scale";  }


// ********************** permutation generator ***************************

void RandomPermutation::Next(int N, int M, int p[], int start)
{
   // N = size of urn; M = number of draws
   if (N < M) Throw(Logic_error("Newran: N < M in RandomPermutation"));
   int i;
   int* q = new int [N];
   if (!q) ErrorNoSpace();
   for (i = 0; i < N; i++) q[i] = start + i;
   for (i = 0; i < M; i++)
   {
      int k = i + (int)(U.Next() * (N - i));       // uniform on i ... N-1
      p[i] = q[k]; q[k] = q[i];                    // swap i and k terms
                                                   // but put i-th term into p
   }
   delete [] q;
}

/// Shell sort from Sedgewick
void ShellSortAscending(int* a, int N)
{
   int h;
   for (h = 1; h <= N / 9; h = 3*h + 1) {}
   for (; h > 0; h /= 3) for (int i = h; i < N; ++i)
   {
      int v = a[i]; int j = i;
      while (j>=h && a[j-h] > v) { a[j] = a[j-h]; j-= h; }
      a[j] = v;
   }
}

void RandomCombination::SortAscending(int n, int gm[])
{
   ShellSortAscending(gm, n);
}

//***************** Generators with variable parameters ********************

VariPoisson::VariPoisson() : P100(100.0), P200(200.0) {}

Real VariPoisson::Next_very_small(Real mu)
{
   // mu is expected value of Poisson random number
   // Efficient when mu is small
   // generate a Poisson variable with mean mu
   if (mu == 0) return 0;
   Real t = exp(-mu); int k = 0; Real u = U.Next();
   for (Real s = t; s < u; s += t) { ++k; t *= mu / k; }
   return k;
}

Real VariPoisson::Next_small(Real mu)
{
   // mu is expected value of Poisson random number
   // Efficient when mu is reasonably small
   // generate a Poisson variable with mean mu
   // start iteration at mode
   if (mu < 10) return Next_very_small(mu);
   int k_start = (int)mu;  Real u = U.Next();
   Real t1 = exp(k_start * log(mu) - mu - ln_gamma(k_start+1));
   if (t1 > u) return k_start;
   int k1 = k_start; int k2 = k_start; Real t2 = t1; Real s = t1;
   for(;;)
   {
      ++k1; t1 *= mu / k1; s += t1; if (s > u) return k1;
      if (k2 > 0) { t2 *= k2 / mu; --k2; s += t2; if (s > u) return k2; }
   }
}

Real VariPoisson::Next_large(Real mu)
{
   // mu is expected value of Poisson random number
   // reasonably accurate when mu is large, but should try to improve
   // generate a Poisson variable with mean mu
   // method is to start with normal variable X and use X with prob 1/3
   // and X**2 with prob 2/3. In each case X has mean and variance to
   // agree with Poisson rv after adjusting with Sheppard's correction

   const Real sc = 1.0 / 12.0;        // Sheppard correction
   Real k;
   if (U.Next() > 1.0 / 3.0)
   {
      Real musc = 0.5 * (mu - sc); Real meansq = sqrt(mu * mu - musc);
      Real sigma = sqrt(musc / (mu + meansq));
      k = floor(square(N.Next() * sigma + sqrt(meansq)) + 0.5);
   }
   else k = floor(N.Next() * sqrt(mu - sc) + mu + 0.5);
   return k < 0 ? 0 : k;
}

Real VariPoisson::Next(Real mu)
{
   if (mu <= 0)
   {
      if (mu == 0) return 0;
      else Throw(Logic_error("Newran: illegal parameters"));
   }
   if (mu < 10) return Next_very_small(mu);
   else if (mu < 100) return Next_small(mu);
   else if (mu < 200)
   {
      // do in two statements so order of evaluation is preserved
      int i = (int)P100.Next();
      return i + Next_small(mu - 100);
   }
   else if (mu < 300)
   {
      // do in two statements so order of evaluation is preserved
      int i = (int)P200.Next();
      return i + Next_small(mu - 200);
   }
   else return Next_large(mu);
}

int VariPoisson::iNext(Real mu)
   { Tracer et("VariPoisson::iNext "); return Round(Next(mu)); }

VariBinomial::VariBinomial() {}

Real VariBinomial::Next_very_small(int n, Real p)
{
   // Efficient when n * p is small
   // generate a Binomial variable with parameters n and p

   Real q = 1 - p; if (q == 0) return n;
   Real r = p / q; Real t = pow(q, n); Real s = t;
   Real u = U.Next();
   for (int k = 0; k <= n; ++k)
   {
      if (s >= u) return k;
      t *= r * (Real)(n - k) / (Real)(k + 1);
      s += t;
   }
   return 0;    // can happen only if we have round-off error
}

Real VariBinomial::Next_small(int n, Real p)
{
   // Efficient when sqrt(n) * p is small
   // Assume p <= 1/2
   // same as small but start at mode
   // generate a Binomial variable with parameters n and p

   Real q = 1 - p; Real r = p / q; Real u = U.Next();
   int k_start = (int)( (n * r) / (r + 1) );
   Real t1 = exp( ln_gamma(n+1) - ln_gamma(k_start+1) - ln_gamma(n-k_start+1)
      + k_start * log(p) + (n-k_start) * log(q) );
   if (t1 >= u) return k_start;
   Real t2 = t1; Real s = t1;
   int k1 = k_start; int k2 = k_start;
   for (;;)
   {
      t1 *= r * (Real)(n - k1) / (Real)(k1 + 1); ++k1; s += t1;
      if (s >= u) return k1;
      if (k2)
      {
         --k2; t2 *= (Real)(k2 + 1) / (Real)(n - k2) / r; s += t2;
         if (s >= u) return k2;
      }
      else if (k1 == n) return 0;  // can happen only if we have round-off error
   }
}

Real VariBinomial::Next(int n, Real p)
{
   if (p > 0.5) return n - Next(n, 1.0 - p);

   if (n <= 0 || p <= 0)
   {
      if (n < 0 || p < 0) Throw(Logic_error("Newran: illegal parameters"));
      else return 0;
   }

   Real mean = n * p;
   if (mean <= 10) return Next_very_small(n, p);
   else if (mean <= 200) return Next_small(n, p);
   else return Next_large(n,p);

}

// probably not sufficiently accurate
Real VariBinomial::Next_large(int n, Real p)
{
   const Real sc = 1.0 / 12.0;        // Sheppard correction
   Real mean = n * p;
   Real sd = sqrt(mean * (1.0 - p) - sc);
   return floor(N.Next() * sd + mean + 0.5);
}

int VariBinomial::iNext(int n, Real p)
   { Tracer et("VariBinomial::iNext "); return Round(Next(n, p)); }


Real VariLogNormal::Next(Real mean, Real sd)
{
   // should have special version of log for small sd/mean
   Real n_var = log(1 + square(sd / mean));
   return mean * exp(N.Next() * sqrt(n_var) - 0.5 * n_var);
}






#ifdef use_namespace
}
#endif

///@}

