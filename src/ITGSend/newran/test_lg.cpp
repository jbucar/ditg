/// \ingroup newran
///@{

/// \file test_lg.cpp
/// Test log gamma function.

#define WANT_STREAM
#define WANT_MATH

#include "include.h"
#include "format.h"

#include "newran.h"

#ifdef use_namespace
using namespace RBD_LIBRARIES;
#endif

// compare values calculated by ln_gamma_lanczos and ln_gamma near 0 and 1
// ln_gamma_lanczos doesn't use special methods so this provides a cross-check
// of the special methods
Real spot_check(Real z)
{
   Real z0 = floor(z + 0.5); 
   Real v1 = ln_gamma(z);
   Real v2 = ln_gamma_lanczos(z) - ln_gamma_lanczos(z0);
   return fabs((v1 - v2) / v2);
}

// need this for CC
inline Real fabsx(long_Real x) { return fabs((Real)x); }

int my_main()
{
   Tracer et("my_main");
   int i; Real diff;
   
   Format SCI2;
   SCI2.FormatType(Format::SCIENTIFIC);
   SCI2.Precision(2);
   SCI2.Width(10);

   
   cout << "Test of production code for gamma function" << endl << endl;
   Real max_error_pos = 0.0;
   Real max_error_neg = 0.0;
   
   const long_Real gam_1_3 = 2.6789385347077476336L;
   const long_Real gam_1_2 = 1.7724538509055160272L;
   const long_Real gam_2_3 = 1.3541179394264004169L;
   
   long_Real Xp = 1;
   long_Real X1_3p = gam_1_3;
   long_Real X1_3n = gam_1_3;
   long_Real X1_2p = gam_1_2;
   long_Real X1_2n = gam_1_2;
   long_Real X2_3p = gam_2_3;
   long_Real X2_3n = gam_2_3;
   
   
   cout << "Test values from -75 to 75" << endl;
   for (i = 1; i <= 75; ++i)
   {
      int i1 = i - 1;
      X1_3p *= (long_Real)i1 + 1.0L/3.0L;
      X1_2p *= (long_Real)i1 + 1.0L/2.0L;
      X2_3p *= (long_Real)i1 + 2.0L/3.0L;
      Real diffp = fabsx( (Xp - rbd_gamma((Real)i)) / Xp );
      Xp *= i;
      Real diff1_3p = fabsx( (X1_3p - rbd_gamma((Real)i + 1.0/3.0)) / X1_3p);
      Real diff1_2p = fabsx( (X1_2p - rbd_gamma((Real)i + 1.0/2.0)) / X1_2p);
      Real diff2_3p = fabsx( (X2_3p - rbd_gamma((Real)i + 2.0/3.0)) / X2_3p);
      Real diff1_3n = fabsx( (X1_3n - rbd_gamma((Real)(-i1) + 1.0/3.0)) / X1_3n);
      Real diff1_2n = fabsx( (X1_2n - rbd_gamma((Real)(-i1) + 1.0/2.0)) / X1_2n);
      Real diff2_3n = fabsx( (X2_3n - rbd_gamma((Real)(-i1) + 2.0/3.0)) / X2_3n);

      X1_3n /= (long_Real)(-i) + 1.0L/3.0L;
      X1_2n /= (long_Real)(-i) + 1.0L/2.0L;
      X2_3n /= (long_Real)(-i) + 2.0L/3.0L;
      
      if (max_error_pos < diffp) max_error_pos = diffp;
      if (max_error_pos < diff1_3p) max_error_pos = diff1_3p;
      if (max_error_pos < diff1_2p) max_error_pos = diff1_2p;
      if (max_error_pos < diff2_3p) max_error_pos = diff2_3p;
      if (max_error_neg < diff1_3n) max_error_neg = diff1_3n;
      if (max_error_neg < diff1_2n) max_error_neg = diff1_2n;
      if (max_error_neg < diff2_3n) max_error_neg = diff2_3n;
   
   
   }
   cout << endl;
   cout << "Maximum relative error" << endl;
   cout << "   arguments >= 1:       " << SCI2 << max_error_pos << endl;
   cout << "   arguments <  1:       " << SCI2 << max_error_neg << endl;
   cout << endl;

   cout << "Now do logs" << endl;
   Xp = 1;
   X1_3p = gam_1_3;
   X1_3n = gam_1_3;
   X1_2p = gam_1_2;
   X1_2n = gam_1_2;
   X2_3p = gam_2_3;
   X2_3n = gam_2_3;

   max_error_pos = max_error_neg = 0.0; int sign_error = 0;
   for (i = 1; i <= 75; ++i)
   {
      int i1 = i - 1;
      X1_3p *= (long_Real)i1 + 1.0L/3.0L;
      X1_2p *= (long_Real)i1 + 1.0L/2.0L;
      X2_3p *= (long_Real)i1 + 2.0L/3.0L;
      Real LXp = log((Real)Xp);
      Real LX1_3p = log((Real)X1_3p);
      Real LX1_2p = log((Real)X1_2p);
      Real LX2_3p = log((Real)X2_3p);
      Real LX1_3n = log(fabsx(X1_3n));
      Real LX1_2n = log(fabsx(X1_2n));
      Real LX2_3n = log(fabsx(X2_3n));
      Real diffp = 0;
      if (i > 2) diffp = fabsx( (LXp - ln_gamma((Real)i)) / LXp);
      Xp *= i;
      Real diff1_3p = fabsx( (LX1_3p - ln_gamma((Real)i + 1.0/3.0)) / LX1_3p);
      Real diff1_2p = fabsx( (LX1_2p - ln_gamma((Real)i + 1.0/2.0)) / LX1_2p);
      Real diff2_3p = fabsx( (LX2_3p - ln_gamma((Real)i + 2.0/3.0)) / LX2_3p);
      int s1_3, s1_2, s2_3;
      Real diff1_3n
         = fabs( (LX1_3n - ln_gamma((Real)(-i1) + 1.0/3.0, s1_3)) / LX1_3n);
      Real diff1_2n
         = fabs( (LX1_2n - ln_gamma((Real)(-i1) + 1.0/2.0, s1_2)) / LX1_2n);
      Real diff2_3n
         = fabs( (LX2_3n - ln_gamma((Real)(-i1) + 2.0/3.0, s2_3)) / LX2_3n);
      if (s1_3 * X1_3n < 0 || s1_2 * X1_2n < 0 || s2_3 * X2_3n < 0)
         ++sign_error;
      
      X1_3n /= (long_Real)(-i) + 1.0L/3.0L;
      X1_2n /= (long_Real)(-i) + 1.0L/2.0L;
      X2_3n /= (long_Real)(-i) + 2.0L/3.0L;
      
      if (max_error_pos < diffp) max_error_pos = diffp;
      if (max_error_pos < diff1_3p) max_error_pos = diff1_3p;
      if (max_error_pos < diff1_2p) max_error_pos = diff1_2p;
      if (max_error_pos < diff2_3p) max_error_pos = diff2_3p;
      if (max_error_neg < diff1_3n) max_error_neg = diff1_3n;
      if (max_error_neg < diff1_2n) max_error_neg = diff1_2n;
      if (max_error_neg < diff2_3n) max_error_neg = diff2_3n;
   
   
   }
   cout << endl;
   cout << "Maximum relative error" << endl;
   cout << "   arguments >= 1:       " << SCI2 << max_error_pos << endl;
   cout << "   arguments <  1:       " << SCI2 << max_error_neg << endl;
   cout << "error at 1 (should be 0):" << SCI2 << ln_gamma(1.0) << endl;
   cout << "error at 2 (should be 0):" << SCI2 << ln_gamma(2.0) << endl;
   cout << endl;
   cout << "Number of sign errors (should be 0) = " << sign_error << endl;
   cout << endl;
   
   cout << "Test with integer z in range 100 to 10000" << endl << endl;
   // compare with Lanczos method
   max_error_pos = 0.0;
   for (i = 100; i <= 10000; i += 100)
   {
      Real z = i;
      Real lgl = ln_gamma_lanczos(z);
      Real diff = fabs(ln_gamma(z) - lgl) / lgl; 
      if (max_error_pos < diff) max_error_pos = diff;
   }
   cout << "Maximum relative error = " << SCI2 << max_error_pos << endl;
   cout << endl;
   
   cout << "Test for z near 1 or 2" << endl;
   max_error_pos = 0.0;
   diff = spot_check(0.901); if (max_error_pos < diff) max_error_pos = diff;
   diff = spot_check(0.95); if (max_error_pos < diff) max_error_pos = diff;
   diff = spot_check(0.99); if (max_error_pos < diff) max_error_pos = diff;
   diff = spot_check(1.01); if (max_error_pos < diff) max_error_pos = diff;
   diff = spot_check(1.05); if (max_error_pos < diff) max_error_pos = diff;
   diff = spot_check(1.099); if (max_error_pos < diff) max_error_pos = diff;
   diff = spot_check(1.901); if (max_error_pos < diff) max_error_pos = diff;
   diff = spot_check(1.95); if (max_error_pos < diff) max_error_pos = diff;
   diff = spot_check(1.99); if (max_error_pos < diff) max_error_pos = diff;
   diff = spot_check(2.01); if (max_error_pos < diff) max_error_pos = diff;
   diff = spot_check(2.05); if (max_error_pos < diff) max_error_pos = diff;
   diff = spot_check(2.099); if (max_error_pos < diff) max_error_pos = diff;
   
   cout << "Maximum relative error = " << SCI2 << max_error_pos << endl;
   cout << endl;
      
   
   return 0;

}







int main()
{

   Try
   {
      return my_main();
   }
   Catch(Exception)
   {
      cout << "\nProgram fails - exception generated\n\n";
      cout << Exception::what() << "\n";
   }
   CatchAll
   {
      cout << "\nProgram fails - exception generated\n\n"; 
   }

   return 0;
}

///@}




