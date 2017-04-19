/// \ingroup newran
///@{

/// \file tryurng.cpp
/// Test uniform random number generators - main program.

#define WANT_STREAM
#define WANT_MATH

#include "include.h"
#include "newran.h"
#include "format.h"
#include "tryurng.h"
#include "utility.h"
#include "test_out.h"




int main()
{

   Real* s1; Real* s2; Real* s3; Real* s4;
   cout << "\nBegin test\n";   // Forces cout to allocate memory at beginning
   cout << "Now print a real number: " << 3.14159265 << endl;
   { s1 = new Real[8000]; delete [] s1; }
   { s3 = new Real; delete s3;}


   Try
   {
      bool copy_seed_from_disk = false;


      double N = 10000000;
      //double N = 100000000;
      //double N = 1000000000;
      //double N = 5000000000.0;         // needs 500 megbytes of memory
      cout << endl;

      Random::SetDirectory("c:\\seed\\");

      if (N < 1000000000)
      {
         Format FN; FN.FormatType(Format::SCIENTIFIC);
         time_lapse tl;      // measure program run time
         LGM_mixed urng;
         cout << "Testing " << urng.Name() << ", N = " << FN << N << endl;
         cout << BaseTest::Header << endl;
         Random::Set(urng);
         if (copy_seed_from_disk) Random::CopySeedFromDisk(true);
         test1(N);
      }

      if (N < 1000000000)
      {
         Format FN; FN.FormatType(Format::SCIENTIFIC);
         time_lapse tl;      // measure program run time
         LGM_simple urng;
         cout << "Testing " << urng.Name() << ", N = " << FN << N << endl;
         cout << BaseTest::Header << endl;
         Random::Set(urng);
         if (copy_seed_from_disk) Random::CopySeedFromDisk(true);
         test1(N);
      }

      if (N < 1000000000)
      {
         Format FN; FN.FormatType(Format::SCIENTIFIC);
         time_lapse tl;      // measure program run time
         FM urng;
         cout << "Testing " << urng.Name() << ", N = " << FN << N << endl;
         cout << BaseTest::Header << endl;
         Random::Set(urng);
         if (copy_seed_from_disk) Random::CopySeedFromDisk(true);
         test1(N);
      }

      {
         Format FN; FN.FormatType(Format::SCIENTIFIC);
         time_lapse tl;      // measure program run time
         MT urng;
         cout << "Testing " << urng.Name() << ", N = " << FN << N << endl;
         cout << BaseTest::Header << endl;
         Random::Set(urng);
         if (copy_seed_from_disk) Random::CopySeedFromDisk(true);
         test1(N);
      }

      {
         Format FN; FN.FormatType(Format::SCIENTIFIC);
         time_lapse tl;      // measure program run time
         WH urng;
         cout << "Testing " << urng.Name() << ", N = " << FN << N << endl;
         cout << BaseTest::Header << endl;
         Random::Set(urng);
         if (copy_seed_from_disk) Random::CopySeedFromDisk(true);
         test1(N);
      }

      {
         Format FN; FN.FormatType(Format::SCIENTIFIC);
         time_lapse tl;      // measure program run time
         MotherOfAll urng;
         cout << "Testing " << urng.Name() << ", N = " << FN << N << endl;
         cout << BaseTest::Header << endl;
         Random::Set(urng);
         if (copy_seed_from_disk) Random::CopySeedFromDisk(true);
         test1(N);
      }

      {
         Format FN; FN.FormatType(Format::SCIENTIFIC);
         time_lapse tl;      // measure program run time
         MultWithCarry urng;
         cout << "Testing " << urng.Name() << ", N = " << FN << N << endl;
         cout << BaseTest::Header << endl;
         Random::Set(urng);
         if (copy_seed_from_disk) Random::CopySeedFromDisk(true);
         test1(N);
      }

      Random::SetDirectory("");
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




