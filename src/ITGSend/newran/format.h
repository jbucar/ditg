#ifndef FORMAT_LIB
#define FORMAT_LIB

#include "str.h"

#ifdef use_namespace
namespace RBD_STRING { using namespace RBD_COMMON; }
namespace RBD_LIBRARIES { using namespace RBD_STRING; }
namespace RBD_STRING {
#endif


class Format
{
public:
   enum OUF {E, HASH, ZERO};
   enum A {LEFT, RIGHT, CENTRE, CENTER};
   enum FT {SIG_FIGS, DEC_FIGS, SCIENTIFIC, INTEGER};
   enum POS {PLUS, SPACE, NX};
   enum V {VAR0, VAR1};
private:
   int min_width_val;
   int max_width_val;
   OUF overflow_policy_val;
   OUF underflow_policy_val;
   A alignment_val;
   FT format_type_val;
   int precision_val;
   POS positive_val;
   String prefix_val;
   String suffix_val;
   String separator_val;
   V variant_val;
public:
   Format() : min_width_val(0), max_width_val(12), overflow_policy_val(E),
      underflow_policy_val(ZERO), alignment_val(RIGHT),
      format_type_val(SIG_FIGS), precision_val(2), positive_val(NX),
      prefix_val(""), suffix_val(""), separator_val(","),
      variant_val(VAR0) {}
   int MinWidth() const { return min_width_val; }
   void MinWidth(int mw) { min_width_val = mw; }
   int MaxWidth() const { return max_width_val; }
   void MaxWidth(int mw) { max_width_val = mw; }
   void Width(int w) { max_width_val = min_width_val = w; }
   OUF OverFlowPolicy() const { return overflow_policy_val; }
   void OverFlowPolicy(OUF of) { overflow_policy_val = of; }
   OUF UnderFlowPolicy() const { return underflow_policy_val; }
   void UnderFlowPolicy(OUF uf) { underflow_policy_val = uf; }
   A Alignment() const { return alignment_val; }
   void Alignment (A a) { alignment_val = a; }
   FT FormatType() const { return format_type_val; }
   void FormatType (FT ft) { format_type_val = ft; }
   POS Positive() const { return positive_val; }
   void Positive(POS p) { positive_val = p; }
   int Precision() const { return precision_val; }
   void Precision(int p) { precision_val = p; }
   String Prefix() const { return prefix_val; }
   void Prefix(const String& p) { prefix_val = p; }
   String Suffix() const { return suffix_val; }
   void Suffix(const String& s) { suffix_val = s; }
   String Separator() const { return separator_val; }
   void Separator(const String& s) { separator_val = s; }
   V Variant() const { return variant_val; }
   void Variant(V v) { variant_val = v; }
   
   int min_width() const { return min_width_val; }
   void min_width(int mw) { min_width_val = mw; }
   int max_width() const { return max_width_val; }
   void max_width(int mw) { max_width_val = mw; }
   void width(int w) { max_width_val = min_width_val = w; }
   OUF overflow_policy() const { return overflow_policy_val; }
   void overflow_policy(OUF of) { overflow_policy_val = of; }
   OUF underflow_policy() const { return underflow_policy_val; }
   void underflow_policy(OUF uf) { underflow_policy_val = uf; }
   A alignment() const { return alignment_val; }
   void alignment (A a) { alignment_val = a; }
   FT format_type() const { return format_type_val; }
   void format_type (FT ft) { format_type_val = ft; }
   POS positive() const { return positive_val; }
   void positive(POS p) { positive_val = p; }
   int precision() const { return precision_val; }
   void precision(int p) { precision_val = p; }
   String prefix() const { return prefix_val; }
   void prefix(const String& p) { prefix_val = p; }
   String suffix() const { return suffix_val; }
   void suffix(const String& s) { suffix_val = s; }
   String separator() const { return separator_val; }
   void separator(const String& s) { separator_val = s; }
   V variant() const { return variant_val; }
   void variant(V v) { variant_val = v; }
   
   friend class OstreamWithFormat;
private:
   void SigFig(ostream& os, double value, int nsig) const;
   void DecFig(ostream& os, double value, int ndec, bool force_decimal=false)
      const;
   void IntegerPrint(ostream& os, double value) const;
   void Scientific(ostream& os, double value, int nsig) const;

   void FormatOut(ostream& os, double value) const;

   void StringPrint(ostream& os, const char* value, int size) const;

   void ErrorOut(ostream& os) const;
};

class OstreamWithFormat
{
   ostream& os;
   const Format& f;
public:
   OstreamWithFormat(ostream& osx, const Format& fx)
      : os(osx), f(fx) {}
      
   OstreamWithFormat(OstreamWithFormat const& owf)
      : os(owf.os), f(owf.f) {}

   OstreamWithFormat& operator<<(const char* value);

   OstreamWithFormat& operator<<(const String& value)
      { f.StringPrint(os, value.data(), value.size()); return *this; }

   OstreamWithFormat& operator<<(int value)
      { f.IntegerPrint(os, (double)value); return *this; }

   OstreamWithFormat& operator<<(unsigned int value)
      { f.IntegerPrint(os, (double)value); return *this; }

   OstreamWithFormat& operator<<(double value)
      { f.FormatOut(os, value); return *this; }

   OstreamWithFormat& operator<<(float value)
      { return operator<<((double)value); }

   OstreamWithFormat& operator<<(long value)
      { f.IntegerPrint(os, (double)value); return *this; }

   OstreamWithFormat& operator<<(unsigned long value)
      { f.IntegerPrint(os, (double)value); return *this; }

   OstreamWithFormat& operator<<(char value) { os << value; return *this; }

   OstreamWithFormat& operator<< (OstreamWithFormat& (*_f)(OstreamWithFormat&))
      { _f(*this); return *this; }
      
   const Format& format() const { return f; }
   
   friend OstreamWithFormat
      operator<<(OstreamWithFormat& osx, const Format& fx);

   friend OstreamWithFormat& endl(OstreamWithFormat&);
   friend OstreamWithFormat& ends(OstreamWithFormat&);
   friend OstreamWithFormat& flush(OstreamWithFormat&);
   
};

inline OstreamWithFormat operator<<(ostream& osx, const Format& fx)
   { return OstreamWithFormat(osx, fx); }

inline OstreamWithFormat operator<<(OstreamWithFormat& osx, const Format& fx)
   { return OstreamWithFormat(osx.os, fx); }

OstreamWithFormat& endl(OstreamWithFormat&);
OstreamWithFormat& ends(OstreamWithFormat&);
OstreamWithFormat& flush(OstreamWithFormat&);

   
   
#ifdef use_namespace
}
#endif


#endif

// body file: format.cpp



