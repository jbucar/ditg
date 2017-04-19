/// \ingroup newran
///@{

/// \file nr_ex.cpp
/// Simple example - get 10 normal random variables.

#define WANT_STREAM

#include "newran.h"

#ifdef use_namespace
using namespace NEWRAN;
#endif


int main()
{

   Try
   {
      bool copy_seed_from_disk = false;    // set to true to get seed from disk file


      Random::SetDirectory("c:\\seed\\");  // set directory for seed control
 
      MotherOfAll urng;                    // declare uniform random number generator

      Random::Set(urng);                   // set urng as generator to be used

      if (copy_seed_from_disk)
         Random::CopySeedFromDisk(true);   // get seed information from disk

      Normal normal;                       // declare normal generator
     
      for (int i = 1; i <= 10; ++i)        // print 10 normal random numbers
         cout << setprecision(5) << setw(10) << normal.Next() << endl;

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

   return 0;
}

///@}


