

#define WANT_STREAM
#define WANT_MATH
#define WANT_STRING


#include "include.h"
#include "str.h"
#include "format.h"

#ifdef use_namespace
namespace RBD_STRING { using namespace RBD_COMMON; }
namespace RBD_LIBRARIES { using namespace RBD_STRING; }
namespace RBD_STRING {
#endif


#ifdef use_namespace

OstreamWithFormat& endl(OstreamWithFormat& osf)
   { osf.os << ::endl; return osf; }

OstreamWithFormat& ends(OstreamWithFormat& osf)
   { osf.os << ::ends; return osf; }

OstreamWithFormat& flush(OstreamWithFormat& osf)
   { osf.os << ::flush; return osf; }

#else

OstreamWithFormat& endl(OstreamWithFormat& osf)
   { osf.os << endl; return osf; }

OstreamWithFormat& ends(OstreamWithFormat& osf)
   { osf.os << ends; return osf; }

OstreamWithFormat& flush(OstreamWithFormat& osf)
   { osf.os << flush; return osf; }

#endif


OstreamWithFormat& OstreamWithFormat::operator<<(const char* value)
   { f.StringPrint(os, value, strlen(value)); return *this; }


void Format::FormatOut(ostream& os, double value) const
{
   os << prefix_val;
   switch (format_type_val)
   {
      case Format::SIG_FIGS:
         SigFig(os, value, precision_val);
         break;

      case Format::DEC_FIGS:
         DecFig(os, value, precision_val);
         break;

      case Format::SCIENTIFIC:
         Scientific(os, value, precision_val);
         break;

      case Format::INTEGER:
         DecFig(os, value, 0);
         break;
   }
   os << suffix_val;
}

void Format::IntegerPrint(ostream& os, double value) const
   { os << prefix_val; DecFig(os, value, 0); os << suffix_val; }


void Format::StringPrint(ostream& os, const char* value, int size) const
{
   int i;

   // prefix
   os << prefix_val;

   // will it fit
   if (size > max_width_val) { ErrorOut(os); return; }

   // padding
   int spaces;
   if (size < min_width_val) spaces = min_width_val - size;  else spaces = 0;
   int s1, s2;                                // leading and following spaces
   if (alignment_val == Format::LEFT) { s1 = 0; s2 = spaces; }
   else if (alignment_val == Format::RIGHT) { s1 = spaces; s2 = 0; }
   else { s1 = spaces / 2; s2 = spaces - s1; }

   // leading spaces
   for (i = 0; i < s1; i++) os << ' ';

   // the string
   for (i = 0; i < size; i++) os << value[i];

   // trailing blanks
   for (i = 0; i < s2; i++) os << ' ';

   // suffix
   os << suffix_val;
}

void Format::SigFig(ostream& os, double value, int nsig) const
{
   bool variant1 = (variant_val == Format::VAR1);
   double v = value;
   if (v == 0.0)
   {
      if (variant1) DecFig(os, 0.0, nsig);
      else DecFig(os, 0.0, nsig-1);
      return;
   }

   // sign
   bool neg;
   if (v < 0) { v = -v; neg = true; }
   else neg = false;

   // exponent - first try
   int ndec = nsig - (int)floor(log10(v)) - 1;    // number of decimals

   // round
   double p = pow(10.0, ndec);                    // 10**ndec
   double rv = floor(v * p + 0.5);                // rounded value

   // exponent - adjust
   if (rv >= pow(10.0, nsig)) { ndec--; p /= 10; rv = floor(v * p + 0.5); }

   // make rounded value
   v = rv / p;
   if (neg) v = -v;

   // width for decimal format
   int w = nsig;
   if (ndec > 0)
   {
      w++;                                        // for decimal
      if (ndec >= nsig)
      {
         if (!variant1) ++w;                      // leading zero
         w += (ndec - nsig);                      // additional zeros
      }
   }
   else { w -= ndec; ndec = 0; }                  // additional zeros
   if (neg || positive_val != Format::NX) w++;    // for sign

   // use scientific or decimal format
   // use Scientific
   //    if number is large and won't fit
   //    number is small and won't fit and underflow policy is to use sci.
   if (w > max_width_val)
   {
      if (ndec == 0) Scientific(os, value, nsig);
      else if (underflow_policy_val == Format::E)
         Scientific(os, value, nsig);
      else DecFig(os, value, ndec, true);
   }
   else DecFig(os, v, ndec, true);
}

void Format::DecFig(ostream& os, double value, int ndec,
   bool force_decimal) const
{
   int i;
   double v = value;
   bool variant1 = (variant_val == Format::VAR1);

   // sign
   bool neg;
   if (v < 0) { v = -v; neg = true; } else neg = false;

   // round and break into integer and decimal bits
   // iv will contain the integer part
   // v will contain the rounded decimal part, multiplied up to make an integer
   double p = pow(10.0,ndec);                  // 10 ** ndec
   double iv;                                  // integer part
   v = modf(v, &iv);                           // decimal parts
   v = floor(v * p + 0.5);                     // rounded decimal part
   if (v >= p) { iv += 1.0; v -= p; }          // decimal part over 1

   // check for underflow
   if ( iv == 0 &&
        value != 0.0 &&
        underflow_policy_val == Format::E &&
        !force_decimal &&
        (!variant1 || v < 0.1) )
      { Scientific(os, value, ndec+1); return; }
      
   // space available for integer part
   int li = max_width_val - ndec;                 // max space for integer part
   int lj = min_width_val - ndec;                 // min space for integer part

   // space for sign
   if (neg || positive_val != Format::NX) { li--; lj--; }

   // are we going to show the decimal point
   bool dp = (ndec > 0);
   if (dp) { li--; lj--; }                        // space for decimal point

   // try again with reduced number of decimals
   if (variant1)
   {                                               
      if (li < 0)
         { DecFig(os, value, ndec+li, force_decimal); return; }
   }
   else
   {                                               
      if (li < 1)
         { DecFig(os, value, ndec+li-1, force_decimal); return; }
   }

   String si(li, ' ');
   int nchar = 0;
   for (i = li-1; i >= 0; i--)
   {
      if (iv < 0.5) break;
      nchar++;                                    // chars in integer part
      double x; x = modf(iv/10, &iv);
      si[i] = (char)('0' + (int)floor(x * 10 + 0.5));
   }
   if (iv > 0.5)                                  // won't fit
   {
      if (overflow_policy_val == Format::HASH) ErrorOut(os);
      else  Scientific(os, value, max_width_val-4);// maybe do precision better
      return;
   }
   if (nchar == 0 && !variant1)
      { nchar = 1; si[li-1] = '0'; }              // integer part is zero

   int spaces = lj - nchar;                       // number of spaces
   if (spaces < 0) spaces = 0;
   int s1, s2;                                    // leading and following sp
   if (alignment_val == Format::LEFT) { s1 = 0; s2 = spaces; }
   else if (alignment_val == Format::RIGHT) { s1 = spaces; s2 = 0; }
   else { s1 = spaces / 2; s2 = spaces - s1; }
   
   // in VAR1 variant, if there are no decimals but there is room for
   // the decimal point, put it in.
   if (ndec == 0 && variant1 && s1 > 0) { dp = true; --s1; }

   // leading spaces
   for (i = 0; i < s1; i++) os << ' ';

   // sign
   if (neg) os << '-';
   else if (positive_val == Format::SPACE) os << ' ';
   else if (positive_val == Format::PLUS) os << '+';

   // integer part
   for (i = 0; i < nchar; i++) os << si[li - nchar + i];

   // decimal part
   if (dp)
   {
      os << '.';
      String sv(ndec, '0');
      for (i = ndec-1;  i >= 0; i--)
      {
         double x; x = modf(v/10, &v);
         sv[i] = (char)('0' + (int)floor(x * 10 + 0.5));
      }
      for (i = 0; i < ndec; i++) os << sv[i];
   }

   // trailing blanks
   for (i = 0; i < s2; i++) os << ' ';

}

void Format::Scientific(ostream& os, double value, int nsig) const
{
   int i;
   double v = value;

   // check nsig >= 0
   if (nsig < 0) { ErrorOut(os); return; }

   // sign
   bool neg;
   if (v < 0) { v = -v; neg = true; }
   else neg = false;

   // exponent - first try
   int lv;
   if (v == 0.0) lv = 0;
   else lv = (int)floor(log10(v));           // power of 10 - first try
   v /= pow(10.0,lv);                        // normalise by power of 10

   // round
   double p = pow(10.0,nsig-1);              // aiming at nsig-1 decimal places
   double rv = floor(v * p + 0.5);           // rounded value * p

   // exponent - adjust
   if (rv >= 10.0 * p)
      { lv++; v /= 10.0; rv = floor(v * p + 0.5); }

   // will it fit
   int w = nsig;
   if (nsig > 1) w++;                          // for decimal
   if (neg || positive_val != Format::NX) w++; // for sign
   int np;                                     // decimals in power part
   if (lv >=100 || lv <= -100)
      { if (lv >=1000 || lv <= -1000) np = 4; else np = 3; }
   else np = 2;
   w += np + 2;                                // +2 is for e and sign
   if (w > max_width_val)
   {
      int ov = w - max_width_val;              // overflow
      // can rounding shift us from E-100 to E-99
      if ((lv == -100 || lv == -1000) && ov == 1)
      {
         double p1 = p / 10.0; 
         double rv1 = floor(v * p1 + 0.5);
         if (rv1 >= p)
         {
            double v1 = rv1 * pow(10.0, lv) / p1;
            if (neg) v1 = -v1;
            Scientific(os, v1, nsig);    
            return;
         }
      }
      if (overflow_policy_val == Format::HASH) { ErrorOut(os); return; }
      if (ov > 1) --ov;                        // in case characteristic length
                                               // drops when we round
      if (ov > 1) --ov;                        // in case we can drop decimal
      nsig -= ov;                              // how many sig figures
      if (nsig <= 0) { ErrorOut(os); return; } // can't print
      Scientific(os, value, nsig);             // try again with reduced nsig
      return;
   }

   // spaces
   int spaces = min_width_val - w;
   if (spaces < 0) spaces = 0;
   int s1, s2;                                    // leading and following sp
   if (alignment_val == Format::LEFT) { s1 = 0; s2 = spaces; }
   else if (alignment_val == Format::RIGHT) { s1 = spaces; s2 = 0; }
   else { s1 = spaces / 2; s2 = spaces - s1; }

   // leading spaces
   for (i = 0; i < s1; i++) os << ' ';

   // sign
   if (neg) os << '-';
   else if (positive_val == Format::SPACE) os << ' ';
   else if (positive_val == Format::PLUS) os << '+';

   // decimal part
   String sv; sv.resize(nsig);
   for (i = nsig-1;  i >= 0; i--)
   {
      double x; x = modf(rv/10, &rv);
      sv[i] = (char)('0' + (int)floor(x * 10 + 0.5));
   }
   os << sv[0];                            // first digit
   if (nsig > 1)                           // need decimal
      { os << '.'; for (int i = 1; i < nsig; i++) os << sv[i]; }

   // power part
   os << 'e';
   if (lv >= 0) os << '+'; else { os << '-'; lv = -lv; }
   sv.resize(np);
   for (i = np-1; i >= 0; i--)
   {
      int lv1 = lv / 10; int x = lv - lv1 * 10; lv = lv1;
      sv[i] = (char)('0' + x);
   }
   for (i = 0; i < np; i++) os << sv[i];

   // trailing blanks
   for (i = 0; i < s2; i++) os << ' ';
}

void Format::ErrorOut(ostream& os) const
{
   if (min_width_val == 0) os << '#';
   else { for (int i = 0; i < min_width_val; i++) os << '#'; }
}


#ifdef use_namespace
}
#endif

