/// \ingroup newran
///@{

/// \file simpstr.cpp
/// A very simple string class - bodies.

#include "include.h"
#include "myexcept.h"
#include "simpstr.h"

#ifdef use_namespace
namespace RBD_COMMON {
#endif


/// Construct an empty SimpleString.
SimpleString::SimpleString() : n(0)
{
   S = new char [1]; if (!S) Throw(Bad_alloc());
   *S = 0;
}

/// Construct SimpleString equal to a SimpleString.
SimpleString::SimpleString(const SimpleString& s) : n(s.n)
{
   S = new char [n+1]; if (!S) Throw(Bad_alloc());
   char* s1 = S; const char* s2 = s.S; unsigned int i = n+1;
   while (i--) *s1++ = *s2++;
}

/// Construct SimpleString equal to a C character string.
SimpleString::SimpleString(const char* c)
{
   const char* s2 = c;
   n = 0; while (*s2++ != 0) ++n;
   S = new char [n+1]; if (!S) Throw(Bad_alloc());
   char* s1 = S; s2 = c; unsigned int i = n+1;
   while (i--) *s1++ = *s2++;
}

/// Construct SimpleString equal to i copies of a character, c.
SimpleString::SimpleString(unsigned int i, char c) : n(i)
{
   S = new char [n+1]; if (!S) Throw(Bad_alloc());
   char* s1 = S;
   while (i--) *s1++ = c; *s1 = 0;
}

/// Set SimpleString equal to a SimpleString.
void SimpleString::operator=(const SimpleString& s)
{
   char* old_S = S; n = s.n;
   S = new char [n+1]; if (!S) Throw(Bad_alloc());
   char* s1 = S; const char* s2 = s.S; unsigned int i = n+1;
   while (i--) *s1++ = *s2++;
   delete [] old_S;
}

/// Set SimpleString equal to a C character string.
void SimpleString::operator=(const char* c)
{
   const char* s2 = c;
   n = 0; while (*s2++ != 0) ++n;
   char* old_S = S;
   S = new char [n+1]; if (!S) Throw(Bad_alloc());
   char* s1 = S; s2 = c; unsigned int i = n+1;
   while (i--) *s1++ = *s2++;
   delete [] old_S;
}

/// Set SimpleString equal to a single character.
void SimpleString::operator=(char c)
{
   char* old_S = S; n = 1;
   S = new char [2]; if (!S) Throw(Bad_alloc());
   S[0] = c; S[1] = 0;
   delete [] old_S;
}

/// Append SimpleString to SimpleString.
void SimpleString::operator+=(const SimpleString& s)
{
   char* old_S = S; unsigned int old_n = n; n += s.n;
   S = new char [n+1]; if (!S) Throw(Bad_alloc());
   char* s1 = S; const char* s2 = old_S; unsigned int i = old_n;
   while (i--) *s1++ = *s2++;
   s2 = s.S; i = s.n+1;
   while (i--) *s1++ = *s2++;
   delete [] old_S;
}

/// Append C character string to SimpleString.
void SimpleString::operator+=(const char* c)
{
   char* old_S = S; unsigned int old_n = n;
   const char* s2 = c;
   int n2 = 0; while (*s2++ != 0) ++n2; n += n2;
   S = new char [n+1]; if (!S) Throw(Bad_alloc());
   char* s1 = S; s2 = old_S; unsigned int i = old_n;
   while (i--) *s1++ = *s2++;
   s2 = c; i = n2+1;
   while (i--) *s1++ = *s2++;
   delete [] old_S;
}

/// Append character to SimpleString.
void SimpleString::operator+=(char c)
{
   char* old_S = S; unsigned int old_n = n; ++n;
   S = new char [n+1]; if (!S) Throw(Bad_alloc());
   char* s1 = S; const char* s2 = old_S; unsigned int i = old_n;
   while (i--) *s1++ = *s2++;
   *s1++ = c; *s1 = 0;
   delete [] old_S;
}

/// Concatenate two SimpleStrings.
SimpleString operator+(const SimpleString& s1, const SimpleString& s2)
{
   SimpleString s(s1.n + s2.n, (char)0);
   const char* c_in = s1.S; unsigned int i = s1.n; char* c_out = s.S;
   while (i--) *c_out++ = *c_in++;
   c_in = s2.S; i = s2.n;
   while (i--) *c_out++ = *c_in++;
   return s;
}

/// Concatenate C character string and SimpleString.
SimpleString operator+(const char* c1, const SimpleString& s2)
{
   const char* c_in = c1;
   int n1 = 0; while (*c_in++ != 0) ++n1;
   SimpleString s(n1 + s2.n, (char)0);
   c_in = c1; unsigned int i = n1; char* c_out = s.S;
   while (i--) *c_out++ = *c_in++;
   c_in = s2.S; i = s2.n;
   while (i--) *c_out++ = *c_in++;
   return s;
}

/// Concatenate SimpleString and C character string.
SimpleString operator+(const SimpleString& s1, const char* c2)
{
   const char* c_in = c2;
   int n2 = 0; while (*c_in++ != 0) ++n2;
   SimpleString s(s1.n + n2, (char)0);
   c_in = s1.S; unsigned int i = s1.n; char* c_out = s.S;
   while (i--) *c_out++ = *c_in++;
   c_in = c2; i = n2;
   while (i--) *c_out++ = *c_in++;
   return s;
}

/// Concatenate character and SimpleString.
SimpleString operator+(char c1, const SimpleString& s2)
{
   SimpleString s(1 + s2.n, (char)0);
   char* c_out = s.S; *c_out++ = c1;
   const char* c_in = s2.S; unsigned int i = s2.n;
   while (i--) *c_out++ = *c_in++;
   return s;
}

/// Concatenate SimpleString and character.
SimpleString operator+(const SimpleString& s1, char c2)
{
   SimpleString s(s1.n + 1, (char)0);
   const char* c_in = s1.S; unsigned int i = s1.n; char* c_out = s.S;
   while (i--) *c_out++ = *c_in++;
   *c_out = c2;
   return s;
}

#ifdef use_namespace
}
#endif

// Descriptions of inline functions for Doxygen

/// \fn unsigned int SimpleString::length() const
/// Number of charcters in a string excluding the terminal 0.

/// \fn unsigned int SimpleString::size() const
/// Number of charcters in a string excluding the terminal 0.

/// \fn const char* SimpleString::c_str() const
/// Pointer to the character string containing the string itself.

/// \fn const char* SimpleString::data() const
/// Pointer to the character string containing the string itself.







///@}




