/// \ingroup newran
///@{

/// \file newran1.cpp
/// Uniform random number generators.

//#define WANT_STREAM
#define WANT_MATH
#define WANT_STRING
#define WANT_FSTREAM

#include "include.h"

#include "newran.h"

#ifdef use_namespace
namespace NEWRAN { using namespace RBD_COMMON; }
namespace RBD_LIBRARIES { using namespace NEWRAN; }
namespace NEWRAN {
#endif

static char hex(unsigned long s, int n)
{
   unsigned long sn = (s >> n) & 0xF;
   if (sn < 10) return (char)('0' + sn);
   else return (char)('A' + (sn - 10));
}

static unsigned long unhex(char c, int n)
{
   if (c < '0') Throw(Runtime_error("newran: invalid character in unhex"));
   if (c <= '9') return ((unsigned long)(c - '0')) << n;
   if (c < 'A') Throw(Runtime_error("newran: invalid character in unhex"));
   if (c <= 'F') return ((unsigned long)(c - 'A' + 10)) << n;
   Throw(Runtime_error("newran: invalid character in unhex"));
#ifndef UseExceptions
   return 0;
#endif
}



//***************************** Base RNG *********************************

const char* Random::Name() { return "Random"; }

Random* Random::RNG = new DummyRNG; // so we get an error message if accessed
Random* Random::Dummy = RNG;        // so we can get back DummyRNG back
SimpleString Random::Dir;           // directory where seed is stored

Real Random::Density(Real) const
{
   Throw(Logic_error("Newran: density function not available"));
#ifndef UseExceptions
   return 0;
#endif
}

unsigned long Random::ulNext()
{
   Throw(Logic_error("Newran: ulNext function not available"));
#ifndef UseExceptions
   return 0;
#endif
}

void Random::Set(Random& r)   // set urng type
{
   RNG = &r;
}

void Random::CopySeedFromDisk(bool update)
{
   if (Dir.size() == 0) Throw(Logic_error("Newran: directory not defined"));
   RNG->CSFD(update);
}

void Random::CopySeedToDisk()
{
   if (Dir.size() == 0) Throw(Logic_error("Newran: directory not defined"));
   RNG->CSTD();
}

void Random::SetDirectory(const char* dir)
{
   Dir = dir;
}

//***************** verify we are dealing with a seed file *********************

static void ReadVerify(ifstream& in, bool missing_ok = false)
{
   if (!in || in.eof())
   {
      if (missing_ok) return;
      Throw(Runtime_error("Newran: can't open seed file"));
   }
   const char* v_string = "!Newran03!"; char c;
   for (int i = 0; i < 10; ++i)
   {
      in >> c;
      if (in.eof())
      {
         if (i==0 && missing_ok) return;
         Throw(Runtime_error("Newran: seed file verification fails"));
      }
      if (c != v_string[i])
         Throw(Runtime_error("Newran: seed file verification fails"));
   }
}

static void WriteVerify(ofstream& out)
{
   if (!out) Throw(Runtime_error("Newran: can't access seed file"));
   const char* v_string = "!Newran03!";
   for (int i = 0; i < 10; ++i) out << v_string[i];
}


//*********************** base of LGM Generators ******************************

const char* LGM_base::Name() { return "LGM_base"; }

LGM_base::LGM_base(double s)
{
   if (s>=1.0 || s<=0.0)
      Throw(Logic_error("Newran: seed out of range"));
   seed = (long)(s*2147483648UL);
   Update = false;
}

void LGM_base::NextValue()
{
   // m = 2147483647 = 2^31 - 1; a = 16807;
   // 127773 = m div a; 2836 = m mod a
   long hi = seed / 127773L;                 // integer division
   long lo = seed - hi * 127773L;            // modulo
   seed = 16807 * lo - 2836 * hi;
   if (seed <= 0) seed += 2147483647L;
}

Real LGM_base::Next() { NextValue(); return (double)seed / 2147483648.0; }

unsigned long LGM_base::ulNext() { NextValue(); return (unsigned long)seed << 1; }


//*********************** mixed LGM Generator ******************************

const char* LGM_mixed::Name() { return "LGM_mixed"; }

LGM_mixed::LGM_mixed(double s) : LGM_base(s)
{
   for (int i = 0; i<128; i++) { LGM_base::NextValue(); Buffer[i] = seed; }
}

void LGM_mixed::NextValue()                    // get new mixed random number
{
   int i = (int)(LGM_base::Next()*128);       // 0 <= i < 128
   LGM_base::NextValue();
   seed_mixed = Buffer[i]; Buffer[i] = seed;
}

Real LGM_mixed::Next() { NextValue(); return (double)seed_mixed / 2147483648.0; }

unsigned long LGM_mixed::ulNext()
{ NextValue(); return (unsigned long)seed_mixed << 1; }


void LGM_mixed::CSFD(bool update)
{
   Update = false;
   ifstream in((Dir+"lgm_mix.txt").c_str());
   ReadVerify(in);
   char x; seed = 0;
   for (int n = 28; n >= 0; n -= 4) { in >> x; seed |= unhex(x, n); }
   ReadVerify(in);
   in.close();
   for (int i = 0; i<128; i++) { LGM_base::NextValue(); Buffer[i] = seed; }
   Update = update;
}

void LGM_mixed::CSTD()
{
   ifstream in; in.open((Dir+"lgm_mix.txt").c_str(), ios::in);
   ReadVerify(in, true); if (in) in.close();
   ofstream out((Dir+"lgm_mix.txt").c_str());
   WriteVerify(out);
   for (int n = 28; n >= 0; n -= 4) out << hex(seed, n);
   WriteVerify(out);
   out << endl;
   out.close();
}


LGM_mixed::~LGM_mixed()
{
   if (Update) CSTD();
   if (RNG == this) RNG = Dummy;
}

//*********************** simple LGM Generator ******************************

const char* LGM_simple::Name() { return "LGM_simple"; }

LGM_simple::LGM_simple(double s) : LGM_base(s) {}

void LGM_simple::CSFD(bool update)
{
   Update = false;
   ifstream in((Dir+"lgm.txt").c_str());
   ReadVerify(in);
   seed = 0;
   char x;
   for (int n = 28; n >= 0; n -= 4) { in >> x; seed |= unhex(x, n); }
   ReadVerify(in);
   in.close();
   Update = update;
}

void LGM_simple::CSTD()
{
   ifstream in; in.open((Dir+"lgm_mix.txt").c_str(), ios::in);
   ReadVerify(in, true); if (in) in.close();
   ofstream out((Dir+"lgm.txt").c_str());
   WriteVerify(out);
   for (int n = 28; n >= 0; n -= 4) out << hex(seed, n);
   WriteVerify(out);
   out << endl;
   out.close();
}


LGM_simple::~LGM_simple()
{
   if (Update) CSTD();
   if (RNG == this) RNG = Dummy;
}


//*********************** Wichmann Hill Generator ******************************

//B.A. Wichmann and I. D. Hill (1982). Algorithm AS 183: An Efficient and Portable
//Pseudo-random Number Generator, Applied Statistics, 31, 188-190;
//Remarks: 34,p.198 and 35, p.89. 


const char* WH::Name() { return "Wichmann Hill"; }

WH::WH(double s)
{
   if (s>=1.0 || s<=0.0)
      Throw(Logic_error("Newran: seed out of range"));
   seed1 = (unsigned long)(s*30269);
   seed2 = 1234;
   seed3 = 5678;
   Update = false;
}

Real WH::Next()                           // get new uniform random number
{
   seed1 = 171 * seed1 % 30269; 
   seed2 = 172 * seed2 % 30307; 
   seed3 = 170 * seed3 % 30323;
   double x = seed1 / 30269.0 + seed2 / 30307.0 + seed3 / 30323.0;
   if (x < 1) return x;
   else if (x < 2) return x - 1;
   else return x - 2; 
}

unsigned long WH::ulNext() { return (unsigned long)floor(WH::Next() * 4294967296.0); }

void WH::CSFD(bool update)
{
   Update = false;
   ifstream in((Dir+"wh.txt").c_str());
   ReadVerify(in);
   seed1 = seed2 = seed3 = 0;
   char x; int n;
   for (n = 28; n >= 0; n -= 4) { in >> x; seed1 |= unhex(x, n); }
   for (n = 28; n >= 0; n -= 4) { in >> x; seed2 |= unhex(x, n); }
   for (n = 28; n >= 0; n -= 4) { in >> x; seed3 |= unhex(x, n); }
   ReadVerify(in);
   in.close();
   Update = update;
}

void WH::CSTD()
{
   ifstream in; in.open((Dir+"lgm_mix.txt").c_str(), ios::in);
   ReadVerify(in, true); if (in) in.close();
   ofstream out((Dir+"wh.txt").c_str());
   WriteVerify(out);
   int n;
   for (n = 28; n >= 0; n -= 4) out << hex(seed1, n);
   for (n = 28; n >= 0; n -= 4) out << hex(seed2, n);
   for (n = 28; n >= 0; n -= 4) out << hex(seed3, n);
   WriteVerify(out);
   out << endl;
   out.close();
}


WH::~WH()
{
   if (Update) CSTD();
   if (RNG == this) RNG = Dummy;
}




//***************** The Mother of all random number generators **********

const char* MotherOfAll::Name() { return "MotherOfAll"; }



MotherOfAll::MotherOfAll(double s) : mStart(1)
{
   // updated 27 September 2005
   if (s>=1.0 || s<=0.0)
      Throw(Logic_error("Newran: seed out of range"));
   seed = (unsigned long)(s*2147483648.0);
   // check again to make sure it hasn't rounded to an invalid value
   if (seed>2147483647 || seed==0)
      Throw(Logic_error("Newran: seed out of range"));
   Update = false;
}


#define m16Long 65536L           /* 2^16 */
#define m16Mask 0xFFFF           /* mask for lower 16 bits */
#define m15Mask 0x7FFF           /* mask for lower 15 bits */
#define m31Mask 0x7FFFFFFF       /* mask for 31 bits */
#define m32Double  4294967295.0  /* 2^32-1 */



// George Marsaglia's The mother of all random number generators
//     producing uniformly distributed pseudo random 32 bit values with
//               period about 2^250.
// The text of Marsaglia's posting is appended at the end of the function.
//
// The arrays mother1 and mother2 store carry values in their
//     first element, and random 16 bit numbers in elements 1 to 8.
//     These random numbers are moved to elements 2 to 9 and a new
//     carry and number are generated and placed in elements 0 and 1.
// The arrays mother1 and mother2 are filled with random 16 bit values
//     on first call of Mother by another generator.  mStart is the switch.
//
// Returns:
//     A 32 bit random number is obtained by combining the output of the
//     two generators and returned in *pSeed.  It is also scaled by
//     2^32-1 and returned as a double between 0 and 1
//
//     SEED:
//     The initial value of *pSeed may be any long value
//
//     Bob Wheeler 8/8/94 <bwheeler (at) ssnet.com>
//


void MotherOfAll::Mother()
{
   unsigned long  number, number1, number2;
   short n, *p;
   unsigned short sNumber;

   // Initialize motheri with 9 random values the first time
   if (mStart)
   {
      sNumber = (unsigned short)(seed & m16Mask);   // The low 16 bits
      number = seed & m31Mask;                      // Only want 31 bits

      p = mother1;
      for (n = 18; n--;)
      {
         number = 30903*sNumber + (number>>16); // One line multiply-with-carry
         *p++ = sNumber = (unsigned short)(number & m16Mask);
         if (n==9) p = mother2;
      }
      // make cary 15 bits
      mother1[0] &= m15Mask; mother2[0] &= m15Mask; mStart = 0;
   }

   // Move elements 1 to 8 to 2 to 9
   memmove(mother1+2, mother1+1, 8*sizeof(short));
   memmove(mother2+2, mother2+1, 8*sizeof(short));

   // Put the carry values in numberi
   number1 = mother1[0]; number2 = mother2[0];

   // Form the linear combinations
   number1 += 1941*mother1[2]+1860*mother1[3]+1812*mother1[4]+1776*mother1[5]+
      1492*mother1[6]+1215*mother1[7]+1066*mother1[8]+12013*mother1[9];
   number2 += 1111*mother2[2]+2222*mother2[3]+3333*mother2[4]+4444*mother2[5]+
      5555*mother2[6]+6666*mother2[7]+7777*mother2[8]+9272*mother2[9];

   // Save the high bits of numberi as the new carry
   mother1[0] = (short)(number1 / m16Long);
   mother2[0] = (short)(number2 / m16Long);
   // Put the low bits of numberi into motheri[1]
   mother1[1] = (short)(m16Mask & number1);
   mother2[1] = (short)(m16Mask & number2);

   // Combine the two 16 bit random numbers into one 32 bit
   seed = (((long)mother1[1]) << 16) + (long)mother2[1];
   seed &= 0xFFFFFFFF;

}

Real MotherOfAll::Next() { Mother(); return ((double)seed + 0.5) / 4294967296.0; }

unsigned long MotherOfAll::ulNext() { Mother(); return seed; }


void MotherOfAll::CSFD(bool update)
{
   Update = false;
   ifstream in((Dir+"mother.txt").c_str());
   ReadVerify(in);
   char x;
   for (int i = 0; i < 10; ++i)
   {
      int n; unsigned long m1 = 0, m2 = 0;
      for (n = 12; n >= 0; n -= 4) { in >> x; m1 |= unhex(x, n); }
      for (n = 12; n >= 0; n -= 4) { in >> x; m2 |= unhex(x, n); }
      mother1[i] = (short)m1;
      mother2[i] = (short)m2;
   }
   ReadVerify(in);
   in.close();
   mStart = 0;
   Update = update;
}

void MotherOfAll::CSTD()
{
   ifstream in; in.open((Dir+"lgm_mix.txt").c_str(), ios::in);
   ReadVerify(in, true); if (in) in.close();
   ofstream out((Dir+"mother.txt").c_str());
   WriteVerify(out);
   for (int i = 0; i < 10; ++i)
   {
      int n; unsigned long m1 = mother1[i], m2 = mother2[i];
      for (n = 12; n >= 0; n -= 4) out << hex(m1, n);
      for (n = 12; n >= 0; n -= 4) out << hex(m2, n);
   }
   WriteVerify(out);
   out << endl;
   out.close();
}


MotherOfAll::~MotherOfAll()
{
   if (Update) CSTD();
   if (RNG == this) RNG = Dummy;
}

#undef m16Long
#undef m16Mask
#undef m15Mask
#undef m31Mask
#undef m32Double




//*************** Multiply with carry generator *************************

const char* MultWithCarry::Name() { return "MultWithCarry"; }

MultWithCarry::MultWithCarry(double s)
{
   if (s>=1.0 || s<=0.0)
      Throw(Logic_error("Newran: seed out of range"));
   x = (unsigned long)(s * 4294967296.0);
   crry = 1234567;
   Update = false;
}


#ifdef HAS_INT64

void MultWithCarry::NextValue()
{
   unsigned _int64 axc = (unsigned _int64)x * 2083801278 + crry;
   crry = (unsigned long)(axc >> 32);
   x = (unsigned long)axc;
}

#else

// carry out 32bit * 32bit multiply in software

void MultWithCarry::NextValue()
{
   unsigned long  mult = 2083801278;
   unsigned long  m_hi = mult >> 16;
   unsigned long  m_lo = mult & 0xFFFF;

   unsigned long  x_hi = x >> 16;
   unsigned long  x_lo = x & 0xFFFF;

   unsigned long  c_hi = crry >> 16;
   unsigned long  c_lo = crry & 0xFFFF;

   x = x_lo * m_lo + c_lo;
   unsigned long axc = x_lo * m_hi + x_hi * m_lo + c_hi + (x >> 16);
   crry = x_hi * m_hi + (axc >> 16);

   x = (x & 0xFFFF) + ((axc << 16) & 0xFFFFFFFF);

}

#endif


Real MultWithCarry::Next() { NextValue(); return ((double)x + 0.5) / 4294967296.0; }

unsigned long MultWithCarry::ulNext() { NextValue(); return x; }



void MultWithCarry::CSFD(bool update)
{
   Update = false;
   ifstream in((Dir+"multwc.txt").c_str());
   ReadVerify(in);
   x = 0; crry = 0;
   char xx;
   int n;
   for (n = 28; n >= 0; n -= 4) { in >> xx; x |= unhex(xx, n); }
   for (n = 28; n >= 0; n -= 4) { in >> xx; crry |= unhex(xx, n); }
   ReadVerify(in);
   in.close();
   Update = update;
}

void MultWithCarry::CSTD()
{
   ifstream in; in.open((Dir+"lgm_mix.txt").c_str(), ios::in);
   ReadVerify(in, true); if (in) in.close();
   ofstream out((Dir+"multwc.txt").c_str());
   WriteVerify(out);
   int n;
   for (n = 28; n >= 0; n -= 4) out << hex(x, n);
   for (n = 28; n >= 0; n -= 4) out << hex(crry, n);
   WriteVerify(out);
   out << endl;
   out.close();
}

MultWithCarry::~MultWithCarry()
{
   if (Update) CSTD();
   if (RNG == this) RNG = Dummy;
}


//***************************** MT generator ****************************

/* 
   A C-program for MT19937, with initialization improved 2002/1/26.
   Coded by Takuji Nishimura and Makoto Matsumoto.

   Before using, initialize the state by using init_genrand(seed)  
   or init_by_array(init_key, key_length).

   Copyright (C) 1997 - 2002, Makoto Matsumoto and Takuji Nishimura,
   All rights reserved.                          

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

     1. Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.

     2. Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.

     3. The names of its contributors may not be used to endorse or promote 
        products derived from this software without specific prior written 
        permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


   Any feedback is very welcome.
   http://www.math.keio.ac.jp/matumoto/emt.html
   email: matumoto (at) math.keio.ac.jp
*/


// Period parameters  
#define N 624
#define M 397
#define MATRIX_A 0x9908b0dfUL   // constant vector a
#define UPPER_MASK 0x80000000UL // most significant w-r bits
#define LOWER_MASK 0x7fffffffUL // least significant r bits

static unsigned long mt[N]; // the array for the state vector
static int mti=N+1;         // mti==N+1 means mt[N] is not initialized

// initializes mt[N] with a seed
void MT::init_genrand(unsigned long s)
{
    mt[0]= s & 0xffffffffUL;
    for (mti=1; mti<N; mti++)
    {
        mt[mti] = (1812433253UL * (mt[mti-1] ^ (mt[mti-1] >> 30)) + mti); 
        // See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier.
        // In the previous versions, MSBs of the seed affect
        // only MSBs of the array mt[].
        // 2002/01/09 modified by Makoto Matsumoto
        mt[mti] &= 0xffffffffUL;
        // for >32 bit machines
    }
}

// generates a random number on [0,0xffffffff]-interval

unsigned long MT::genrand_int32()
{
    unsigned long y;
    static unsigned long mag01[2]={0x0UL, MATRIX_A};
    // mag01[x] = x * MATRIX_A  for x=0,1 

    if (mti >= N)   // generate N words at one time
    {
        int kk;

        if (mti == N+1)           // if init_genrand() has not been called,
            init_genrand(5489UL); // a default initial seed is used

        for (kk=0;kk<N-M;kk++)
        {
            y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
            mt[kk] = mt[kk+M] ^ (y >> 1) ^ mag01[y & 0x1UL];
        }

        for (;kk<N-1;kk++)
        {
            y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
            mt[kk] = mt[kk+(M-N)] ^ (y >> 1) ^ mag01[y & 0x1UL];
        }

        y = (mt[N-1]&UPPER_MASK)|(mt[0]&LOWER_MASK);
        mt[N-1] = mt[M-1] ^ (y >> 1) ^ mag01[y & 0x1UL];

        mti = 0;
    }
  
    y = mt[mti++];

    // Tempering
    y ^= (y >> 11);
    y ^= (y << 7) & 0x9d2c5680UL;
    y ^= (y << 15) & 0xefc60000UL;
    y ^= (y >> 18);

    return y;
}


void MT::CSFD(bool update)
{
   Update = false;
   ifstream in((Dir+"mt19937.txt").c_str());
   ReadVerify(in);

   char xx;
   for (int i = 0; i < N; ++i)
   {
      unsigned long x = 0;
      for (int n = 28; n >= 0; n -= 4) { in >> xx; x |= unhex(xx, n); }
      mt[i] = x; 
   }
   mti = 0;
   for (int n = 28; n >= 0; n -= 4) { in >> xx; mti |= unhex(xx, n); }
   ReadVerify(in);
   in.close();
   Update = update;
}

void MT::CSTD()
{
   ifstream in; in.open((Dir+"lgm_mix.txt").c_str(), ios::in);
   ReadVerify(in, true); if (in) in.close();
   ofstream out((Dir+"mt19937.txt").c_str());
   WriteVerify(out);
   for (int i = 0; i < N; ++i)
   {
      unsigned long x = mt[i];
      for (int n = 28; n >= 0; n -= 4) out << hex(x, n);
   }
   for (int n = 28; n >= 0; n -= 4) out << hex(mti, n);

   WriteVerify(out);
   out << endl;
   out.close();
}

MT::MT(double s)
{
   init_genrand((unsigned long)(s*4294967296.0));
   Update = false;
}


Real MT::Next()
{
   return ((double)genrand_int32() + 0.5) / 4294967296.0; 
   // divided by 2^32
}


const char* MT::Name() { return "MT19937"; }

MT::~MT()
{
   if (Update) CSTD();
   if (RNG == this) RNG = Dummy;
}

#undef N
#undef M
#undef MATRIX_A
#undef UPPER_MASK
#undef LOWER_MASK


//*************************** Fishman Moore generator *********************

//References
//G.S. Fishman and L.R. Moore (1986), An exhaustive analysis of multiplicative
//    congruential random number generators with modulus 2^31-1, SIAM J Sci.
//    Stat. Comput., 7, pp. 24-45.

//Based on code supplied by Tony Cooper (written by him?)									*/

#define D31M1	2147483647L	/* Mersenne prime 2^31-1 		*/
#define a	950706376.0	/* congruential multiplier = 0x38AAA0C8 */
#define a1	0x000038AAL	/*  14506 = a >> 16 			*/
#define a0	0x0000A0C8L	/*  41160 = a & 0x0000FFFF 		*/
#define	a1ma0	0xFFFF97E2L	/* -26654 = (a1 minus a0) 		*/

void FM::NextValue()
{
    unsigned long x = seed;
    unsigned long a1x1;
    unsigned long a0x0;
    unsigned long a1x0pa0x1;

    a1x1 = a1 * (x >> 16);
    a0x0 = a0 * (x & 0x0000FFFF);
    a1x0pa0x1 = a1ma0 * ((x & 0x0000FFFF) - (x >> 16) ) + a1x1 + a0x0;
    a1x0pa0x1 &= 0xFFFFFFFF;
    a1x1 += (a1x0pa0x1 >> 16);
    unsigned long ax = (a1x0pa0x1 << 16) & 0xFFFFFFFF;
    a0x0 += ax;
    if (a0x0 < ax) a1x1++;
    x = (a1x1 << 1) + (a0x0 >> 31) + (a0x0 & 0x7FFFFFFF);
    if (x & 0x80000000) x -= D31M1;
    seed = x;
}

Real FM::Next() { NextValue(); return (double)seed / 2147483648.0; }

unsigned long FM::ulNext() { NextValue(); return seed << 1; }


const char* FM::Name() { return "Fishman Moore"; }

FM::FM(double s)
{
   if (s>=1.0 || s<=0.0)
      Throw(Logic_error("Newran: seed out of range"));
   seed = (unsigned long)(s*2147483648UL);
   Update = false;
}

void FM::CSFD(bool update)
{
   Update = false;
   ifstream in((Dir+"fm.txt").c_str());
   ReadVerify(in);
   unsigned long iseed = 0;
   char x;
   for (int n = 28; n >= 0; n -= 4) { in >> x; iseed |= unhex(x, n); }
   ReadVerify(in);
   in.close();
   seed = iseed;
   Update = update;
}

void FM::CSTD()
{
   ifstream in; in.open((Dir+"lgm_mix.txt").c_str(), ios::in);
   ReadVerify(in, true); if (in) in.close();
   ofstream out((Dir+"fm.txt").c_str());
   WriteVerify(out);
   unsigned long iseed = (unsigned long)seed;
   for (int n = 28; n >= 0; n -= 4) out << hex(iseed, n);
   WriteVerify(out);
   out << endl;
   out.close();
}


FM::~FM()
{
   if (Update) CSTD();
   if (RNG == this) RNG = Dummy;
}

#undef D31M1
#undef a
#undef a1
#undef a0
#undef a1ma0






//***************** Dummy for loading at startup ************************

const char* DummyRNG::Name() { return "DummyRNG"; }

Real DummyRNG::Next()
{
   Throw(Runtime_error("Newran: uniform rng not setup"));
#ifndef UseExceptions
   return 0;
#endif
}

void DummyRNG::CSFD(bool)
{
   Throw(Runtime_error("Newran: uniform rng not setup"));
}

void DummyRNG::CSTD()
{
   Throw(Runtime_error("Newran: uniform rng not setup"));
}


#ifdef use_namespace
}
#endif

// Additional documentation for newran.
// Information that could have been included in newran.h but is put here to
// avoid making newran.h too complicated.

/// \fn static void Random::Set(Random &r)
/// Set the uniform rng to be used by subsequent calls to rngs.

/// \fn static void Random::CopySeedFromDisk(bool update=false)
/// Copy the seed from disk.
/// Boolean argument determines whether the seed stored on the disk will
/// be restored when the current block exits

/// \fn static void Random::CopySeedToDisk()
/// Copy the current value of the seed to disk.

/// \fn static void Random::SetDirectory(const char* dir)
/// Set the directory where seeds are stored.

/// \fn virtual Real Random::Next()
/// Return a new random number.

/// \fn virtual unsigned long Random::ulNext()
/// Return a new random number as an unsigned long.

/// \fn virtual const char* Random::Name()
/// Return a pointer to a character string identifying an rng.

/// \fn virtual ExtReal Random::Mean() const
/// Return the expectation (mean) of an rng.

/// \fn virtual ExtReal Random::Variance() const
/// Return the variance of an rng. 





///@}

