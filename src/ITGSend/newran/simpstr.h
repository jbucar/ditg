/// \ingroup newran
///@{

/// \file simpstr.h
/// A very simple string class - headers.
/// Use where I don't want to include my full string class or
/// the standard string class

#ifndef SIMPLE_STRING_LIB
#define SIMPLE_STRING_LIB

#ifdef use_namespace
namespace RBD_COMMON {
#endif


/// A very simple string class.
class SimpleString
{
   char* S;        ///< pointer to the character string
   unsigned int n; ///< length of string excluding terminal 0
public:
   SimpleString();
   ~SimpleString() { delete [] S; }
   SimpleString(const SimpleString& s);
   SimpleString(const char* c);
   SimpleString(unsigned int i, char c);
   void operator=(const SimpleString& s);
   void operator=(const char*);
   void operator=(char);
   void operator+=(const SimpleString& s);
   void operator+=(const char*);
   void operator+=(char);
   friend SimpleString operator+(const SimpleString& s1, const SimpleString& s2);
   friend SimpleString operator+(const char* c1, const SimpleString& s2);
   friend SimpleString operator+(const SimpleString& s1, const char* c2);
   friend SimpleString operator+(char c1, const SimpleString& s2);
   friend SimpleString operator+(const SimpleString& s1, char c2);
   unsigned int length() const { return n; }
   unsigned int size() const { return n; }
   const char* c_str() const { return S; }
   const char* data() const { return S; }
};

#ifdef use_namespace
}
#endif


#endif                             // SIMPLE_STRING_LIB


// body file: simpstr.cpp


///@}

