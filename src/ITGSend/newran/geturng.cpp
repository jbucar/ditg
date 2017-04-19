/// \ingroup newran
///@{

/// \file geturng.cpp
/// Get binary data from uniform generators for further testing.

#define WANT_STREAM
#define WANT_FSTREAM

#include "include.h"
#include "newran.h"



#ifdef use_namespace
using namespace NEWRAN;
#endif

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

      double N = 2500000;

      bool Bytes = true;             // we take the top byte only
      //bool Bytes = false;             // we take the whole word

      cout << endl;

      Random::SetDirectory("c:\\seed\\");


      {

         LGM_mixed urng;
         //LGM_simple urng;
         //FM urng;
         //MT urng;
         //MotherOfAll urng;
         //MultWithCarry urng;
         //WH urng;
         cout << "Testing " << urng.Name() << ", N = " << N << ", ";
         if (Bytes) cout << "top byte" << endl;
         else cout << "whole word" << endl;
         Random::Set(urng);
         if (copy_seed_from_disk) Random::CopySeedFromDisk(true);

         ofstream os("ran_nos.bin",ios::binary);
         if (Bytes)
         {
            for (int i = 0; i < N * 4; ++i)
            {
               unsigned long x = urng.ulNext();
               os.put((unsigned char)((x >> 24) & 0xFF));
            }
         }
         else
         {
            for (int i = 0; i < N; ++i)
            {
               unsigned long x = urng.ulNext();
               os.put((unsigned char)(x & 0xFF));
               os.put((unsigned char)((x >> 8) & 0xFF));
               os.put((unsigned char)((x >> 16) & 0xFF));
               os.put((unsigned char)((x >> 24) & 0xFF));
            }
         }
         os.close();
      
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


///@}

