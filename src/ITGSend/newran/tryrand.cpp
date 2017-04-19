/// \ingroup newran
///@{

/// \file tryrand.cpp
/// Test of newran - main program and functions.

#define WANT_STREAM
#define WANT_MATH
#define WANT_TIME

#include "include.h"
#include "newran.h"
#include "format.h"
#include "utility.h"
#include "tryrand.h"


int main()
{
   bool copy_seed_from_disk = false;

   time_lapse tl;      // measure program run time

   //MultWithCarry urng(0.46875);
   //MotherOfAll urng(0.46875);
   LGM_mixed urng(0.46875);

   Random::Set(urng);
   Random::SetDirectory("c:\\seed\\");
   if (copy_seed_from_disk) Random::CopySeedFromDisk(true);

   Real* s1; Real* s2; Real* s3; Real* s4;
   cout << "\nBegin test\n";   // Forces cout to allocate memory at beginning
   cout << "Now print a real number: " << 3.14159265 << endl;
   { s1 = new Real[8000]; delete [] s1; }
   { s3 = new Real; delete s3;}


   Try
   {


      long n = 200000;
      long n_large = 1000000;

      test1(n);
      test2(n);
      test3(n_large);
      test4(n);
      test5(n_large);
      test6(n);
      

      cout << "\nEnd of tests\n";

   }
   Catch(BaseException)
   {
      cout << "\nTest program fails - exception generated\n\n";
      cout << BaseException::what() << "\n";
   }
   CatchAll
   {
      cout << "\nTest program fails - exception generated\n\n"; 
   }


   { s2 = new Real[8000]; delete [] s2; }
   cout << "\n(The following memory checks are probably not valid with all\n";
   cout << "compilers - see documentation)\n";
   cout << "\nChecking for lost memory: "
      << (unsigned long)s1 << " " << (unsigned long)s2 << " ";
   if (s1 != s2) cout << " - error\n"; else cout << " - ok\n";
   { s4 = new Real; delete s4;}
   cout << "\nChecking for lost memory: "
      << (unsigned long)s3 << " " << (unsigned long)s4 << " ";
   if (s3 != s4) cout << " - error\n"; else cout << " - ok\n";
   cout << endl;

   return 0;
}


//***************** print ExtReal with format ***********************

OstreamWithFormat operator<<(OstreamWithFormat os, const ExtReal& er)
{
   switch (er.Code())
   {
      case Finite:        os << er.Value();       break;
      case PlusInfinity:  os << "plus-inf";       break;
      case MinusInfinity: os << "minus-inf";      break;
      case Indefinite:    os << "indef";          break;
      case Missing:       os << "missing";        break;
   }
   return os;
}


//*********************** histogram function *************************

void Histogram(Random* rx, int n)          // draw histogram with n obsv
{
   int i,j; int count[20];
   Real* a = new Real[n];
   if (!a) { cout << "\nNo memory for Histogram\n"; return; }
   for (i = 0; i < n; i++) a[i] = rx->Next();
   Real amax = a[0]; Real amin = a[0]; Real mean = a[0]; Real sd = 0;
   for (i = 1; i < n; i++)
   {
      if (amin > a[i]) amin = a[i]; else if (amax < a[i]) amax = a[i];
      mean += a[i];
   }
   mean /= n;
   for (i = 0; i < 20; i++) count[i]=0;
   for (i = 0; i < n; i++)
   {
      Real rat= (amax != amin) ? (a[i] - amin)/(amax - amin) : 1.0;
      j = (int)( 19.999 * rat );
      if (j >= 0 && j < 20) count[j]++;
      Real diff = a[i] - mean; sd += diff*diff;
   }
   sd = sqrt(sd/(n-1));
   j = 0;
   for (i = 0; i < 20; i++) { if (j < count[i]) j = count[i]; }
   if (j > 70) { for (i = 0; i < 20; i++) count[i] = (int)((70L*count[i])/j); }
   cout << "\n";
   for (i = 0; i < 20; i++)
      { cout << "\n|"; for (j = 1; j < count[i]; j = j+1) cout << "*"; }
   cout << "\n" << rx->Name() << "\n";
   Format F; F.FormatType(Format::SIG_FIGS); F.Precision(5); F.Width(10);
   cout << "p. mean = " << F << rx->Mean();
   cout << ", p. var = " << F << rx->Variance() << endl;
   cout << "s. mean = " << F << mean;
   cout << ", s. var = " << F << sd*sd;
   cout << ", max = " << F << amax;
   cout << ", min = " << F << amin;
   cout << endl;
   delete a;
}

///@}

