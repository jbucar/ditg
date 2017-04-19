// Character string manipulation library
// 19 June, 2004
// Copyright (c) R B Davies 1996

// The is a string library that is intended to be compatible with the
// "class string" library in the April 1995 draft of the C++ standard.


#ifndef STRING_LIB
#define STRING_LIB

#ifndef INCLUDE_LIB
#include "include.h"
#endif

#include "myexcept.h"

#ifdef use_namespace
namespace RBD_STRING { using namespace RBD_COMMON; }
namespace RBD_LIBRARIES { using namespace RBD_STRING; }
namespace RBD_STRING {
#endif

typedef unsigned int uint;


// helper classes defined later containing different forms of string

class StrBase;                    // base of remaining classes (virtual)
   class CharSeq;                 // pointer to character sequence
      class StrRep;               // string as char seq; no copy on write
         class StrRepMult;        // string as char seq; copy on write OK
         class StrRepCap;         // string as char seq; with capacity
         class StrRepNullTerm;    // null terminated string
            class StrNull;        // null string
   class CharSingle;              // single character
      class CharRepeated;         // repeated character
   class StrSumBase;              // base for sum classes
      class StrSum;               // sum of two strings
      class StrSumLC;             // character+sum
      class StrSumRC;             // sum+character

// the string class seen by the user

class String : public Janitor
// the class Janitor is from my simulated exception package; you may
// remove the dependence on Janitor if you do not want to use my
// exception package
{
   StrBase *px;                   // pointer to a StrBase

   char* GetData() const;         // unprotected pointer to s
   StrBase* Clone() const;	  // "copy" a StrBase
   StrRep* GetStrRep() const;     // get its StrRep
   StrRep* GetStrRepW();          // get its StrRep, writing allowed
   StrRep* Protect() const;       // get protected StrRep
				  // (change StrRepMult to StrRep)
   bool HasCapacity() const;      // does it have capacity defined
   String(StrBase* sb);           // string pointing to StrBase
   static uint s_index;           // used in searches
   static StrNull* ASN;           // pointer to null string - only one instance

public:
   unsigned int refcount() const; // ref count (for debugging)
   StrBase* WhereIsIt() const
      { return px; }              // for debugging
   const char* StringType() const;// for debugging
   void CleanUp();                // for my exception simulator

   static uint npos;              // set to (uint)(-1)

   String();
   String(const String& str);     // not explicitly in standard
   String(const String& str, uint pos, uint n = npos);
   String(const char* s, uint n);
   String(const char* s);
   String(uint n, char c);
   ~String();

   String& operator=(const String& str);
   String& operator=(const char* s);
   String& operator=(const char c);

   uint size() const;
   uint length() const;
   uint max_size() const;
   void resize(uint n, char c = 0);    // std does default separately
   uint capacity() const;
   void reserve(uint res_arg = 0);
   void clear();
   bool empty() const;

   char operator[](uint pos) const;
   char& operator[](uint pos);
   char at(uint n) const;
   char& at(uint n);

   String& operator+=(const String& rhs);
   String& operator+=(const char* s);
   String& operator+=(char c);
   void push_back(char c) { operator+=(c); }

   String& append(const String& str);  // not explicitly in standard
   String& append(const String& str, uint pos, uint n = npos);
   String& append(const char* s, uint n);
   String& append(const char* s);
   String& append(uint n, char c = 0);

   String& assign(const String& str);  // not explicitly in standard
   String& assign(const String& str, uint pos, uint n = npos);
   String& assign(const char* s, uint n);
   String& assign(const char* s);
   String& assign(uint n, char c = 0);

   String& insert(uint pos1, const String& str);
                                       // not explicitly in standard
   String& insert(uint pos1, const String& str, uint pos2, uint n = npos);
   String& insert(uint pos, const char* s, uint n = npos);
                                       // std does default separately
   String& insert(uint pos, uint n, char c = 0);

   String& erase(uint pos = 0, uint n = npos);

   String& replace(uint pos1, uint n1, const String& str);
   String& replace(uint pos1, uint n1, const String& str, uint pos2, uint n2);
   String& replace(uint pos, uint n1, const char* s, uint n2 = npos);
                                       // std does default separately
   String& replace(uint pos, uint n1, uint n2, char c = 0);

   uint copy(char* s, uint n, uint pos = 0) const;
   void swap(String&);

   const char* c_str() const;
   const char* data() const;

   uint find(const String& str, uint pos = 0) const;
   uint find(const char* s, uint pos, uint n) const;
   uint find(const char* s, uint pos = 0) const;
   uint find(const char c, uint pos = 0) const;

   uint rfind(const String& str, uint pos = npos) const;
   uint rfind(const char* s, uint pos, uint n) const;
   uint rfind(const char* s, uint pos = npos) const;
   uint rfind(const char c, uint pos = npos) const;

   uint find_first_of(const String& str, uint pos = 0) const;
   uint find_first_of(const char* s, uint pos, uint n) const;
   uint find_first_of(const char* s, uint pos = 0) const;
   uint find_first_of(const char c, uint pos = 0) const;

   uint find_last_of(const String& str, uint pos = npos) const;
   uint find_last_of(const char* s, uint pos, uint n) const;
   uint find_last_of(const char* s, uint pos = npos) const;
   uint find_last_of(const char c, uint pos = npos) const;

   uint find_first_not_of(const String& str, uint pos = 0) const;
   uint find_first_not_of(const char* s, uint pos, uint n) const;
   uint find_first_not_of(const char* s, uint pos = 0) const;
   uint find_first_not_of(const char c, uint pos = 0) const;

   uint find_last_not_of(const String& str, uint pos = npos) const;
   uint find_last_not_of(const char* s, uint pos, uint n) const;
   uint find_last_not_of(const char* s, uint pos = npos) const;
   uint find_last_not_of(const char c, uint pos = npos) const;

   String substr(uint pos = 0, uint n = npos) const;

   int compare(const String& str) const;
   int compare(uint pos1, uint n1, const String& str) const;
   int compare(uint pos1, uint n1, const String& str, uint pos2, uint n2) const;
   int compare(const char* s) const;
   int compare(uint pos, uint n1, const char* s, uint n2 = npos) const;

   friend String operator+(const String& lhs, const String& rhs);
   friend String operator+(const char* lhs, const String& rhs);
   friend String operator+(char lhs, const String& rhs);
   friend String operator+(const String& lhs, const char* rhs);
   friend String operator+(const String& lhs, char rhs);

   friend bool operator==(const String& lhs, const String& rhs);
   friend bool operator==(const char* lhs, const String& rhs);
   friend bool operator==(const String& lhs, const char* rhs);

   friend bool operator!=(const String& lhs, const String& rhs);
   friend bool operator!=(const char* lhs, const String& rhs);
   friend bool operator!=(const String& lhs, const char* rhs);

   friend bool operator<(const String& lhs, const String& rhs);
   friend bool operator<(const char* lhs, const String& rhs);
   friend bool operator<(const String& lhs, const char* rhs);

   friend bool operator>(const String& lhs, const String& rhs);
   friend bool operator>(const char* lhs, const String& rhs);
   friend bool operator>(const String& lhs, const char* rhs);

   friend bool operator<=(const String& lhs, const String& rhs);
   friend bool operator<=(const char* lhs, const String& rhs);
   friend bool operator<=(const String& lhs, const char* rhs);

   friend bool operator>=(const String& lhs, const String& rhs);
   friend bool operator>=(const char* lhs, const String& rhs);
   friend bool operator>=(const String& lhs, const char* rhs);

   friend void swap(String& A, String& B);

   friend istream& operator>>(istream& is, String& str);
   friend ostream& operator<<(ostream& os, const String& str);
   friend istream& getline(istream& is, String& str, char delim = '\n');

   friend class CharSeq;
   friend class StrSum;
   friend class StringPackageInitialise;

private:

   // these are called by the official string functions
   int compare(uint pos, uint n, const CharSeq& sb) const;
   String& my_append(const StrBase& sb);
   String& my_assign(const StrBase& sb);
   String& my_insert(uint pos, const StrBase& sb);
   String& my_replace(uint pos1, uint n1, const StrBase& str);
   uint find(const CharSeq& str, uint pos) const;
   uint rfind(const CharSeq& str, uint pos) const;
   uint find_first_of(const CharSeq& str, uint pos) const;
   uint find_last_of(const CharSeq& str, uint pos) const;
   uint find_first_not_of(const CharSeq& str, uint pos) const;
   uint find_last_not_of(const CharSeq& str, uint pos) const;

   // this is for linking in with the exception package and can be removed
   // if you do not want the simulated exceptions
   FREE_CHECK(String)
};


// the helper classes


class StrBase
{
public:
   virtual uint size() const = 0;
   virtual uint capacity() const;
   virtual void Load(char*& ps) const = 0; // get data from a StrBase
   virtual StrBase* Clone(StrBase**) = 0;  // "copy" a StrBase
   virtual StrRep* GetStrRep(StrBase**);   // convert to StrRep
   virtual StrRep* GetStrRepW(StrBase**);  // convert to StrRep - writing OK
   virtual StrRep* Protect(StrBase**);     // convert to prot StrRep
   virtual void Drop();
   virtual StrBase* UnProtect();           // StrRep to StrRepMult
   virtual void WithCapacity(StrBase**);   // convert to StrRepCap
   virtual const char* NullTerminate(StrBase**);
					   // convert to StrRepNullTerm
   virtual bool HasCapacity() const;       // is capacity defined
   virtual unsigned int refcount() const;  // for debugging
   virtual ~StrBase() {}
   virtual const char* StringType() const = 0; // return storage mode of string
};


// point to a character sequence

class CharSeq : public StrBase
{
protected:
   CharSeq(uint SZ) : sz(SZ) {}
public:
   uint sz;                            // size of string
   char* s;                            // actual character string
   CharSeq() : sz(0), s(0) {}
   CharSeq(char* S);
   CharSeq(char* S, uint SZ);
   CharSeq(const String& str);
   CharSeq(const String& str, uint pos, uint n);
   uint size() const;
   uint capacity() const;
   void Load(char*& ps) const;
   StrBase* Clone(StrBase**);
   const char* StringType() const { return "CharSeq"; }
};


// the class to manage the actual contiguous character string
// this version does not allow copy on write

class StrRep : public CharSeq
{
public:
   StrRep() {}
   StrRep(uint SZ);
   StrRep(uint n, char c);
   StrRep(const char* x);
   StrRep(const char* x, uint len);
   StrRep(const char* x, uint len, uint n, char c);
			     // len characters from x and n-len copies of c
   ~StrRep();

   StrRep* GetStrRep(StrBase**);
   StrRep* GetStrRepW(StrBase**);
   StrRep* Protect(StrBase**);
   void WithCapacity(StrBase**);
   StrBase* Clone(StrBase**);
   StrBase* UnProtect();
   const char* StringType() const { return "StrRep"; }
};


// the class to manage the actual contiguous character string
// this version does allow copy on write
// ie more than one string can be pointing at an object in this class

class StrRepMult : public StrRep
{
public:
   unsigned int refcnt;      // 0 means one string assigned
   StrRepMult() : refcnt(0) {}
   StrRepMult(uint SZ);
   StrRepMult(uint n, char c);
   StrRepMult(const char* x);
   StrRepMult(const char* x, uint len);
   StrRepMult(const char* x, uint len, uint n, char c);
			     // len characters from x and n-len copies of c
   void Drop();              // delete if refcnt==0 otherwise --refcnt

   StrRep* GetStrRepW(StrBase**);
   StrRep* Protect(StrBase**);
   void WithCapacity(StrBase**);
   StrBase* Clone(StrBase**);
   unsigned int refcount() const;
   StrBase* UnProtect();
   const char* StringType() const { return "StrRepMult"; }
};


// a StrRep which can have capacity() > size()

class StrRepCap : public StrRep
{
public:
   uint cpx;                            // allocated space for string
   StrRepCap() : StrRep(), cpx(0) {}
   StrRepCap(uint len, uint cp);        // string with length & capacity
   StrRepCap(const char* s, uint len);  // length and capacity = len
   uint capacity() const;
   StrBase* UnProtect();
   void WithCapacity(StrBase**);
   const char* NullTerminate(StrBase**);
   bool HasCapacity() const;            // is capacity defined
   const char* StringType() const { return "StrRepCap"; }
};

// a StrRep stored as a null-terminated string

class StrRepNullTerm : public StrRep
{
public:
   StrRepNullTerm(uint len);
   void WithCapacity(StrBase**);
   StrBase* UnProtect();
   const char* NullTerminate(StrBase**);
   const char* StringType() const { return "StrRepNullTerm"; }
};


// single character

class CharSingle : public StrBase
{
protected:
   char c;
public:
   CharSingle(char C) : c(C) {}
   uint size() const;
   void Load(char*& ps) const;
   StrBase* Clone(StrBase**);
   const char* StringType() const { return "CharSingle"; }
};


// repeated character

class CharRepeated : public CharSingle
{
protected:
   uint n;
public:
   CharRepeated(int N, char C);
   uint size() const;
   void Load(char*& ps) const;
   StrBase* Clone(StrBase**);
   const char* StringType() const { return "CharRepeated"; }
};


// the base for the sum of string classes

class StrSumBase : public StrBase
{
protected:
   bool ref;                      // true if already accessed
   StrSumBase();
public:
   StrBase* Clone(StrBase**);
   unsigned int refcount() const;
   const char* StringType() const { return "StrSumBase"; }
};


// string + string

class StrSum : public StrSumBase
{
protected:
   StrBase* lhs;
   StrBase* rhs;
public:
   StrSum(StrBase* L, StrBase* R);
   uint size() const;
   void Load(char*& ps) const;
   void Drop();
   const char* StringType() const { return "StrSum"; }
};


// character + string

class StrSumLC : public StrSumBase
{
protected:
   char lhs;
   StrBase* rhs;
public:
   StrSumLC(char L, StrBase* R);
   uint size() const;
   void Load(char*& ps) const;
   void Drop();
   const char* StringType() const { return "StrSumLC"; }
};


// string + character

class StrSumRC : public StrSumBase
{
protected:
   StrBase* lhs;
   char rhs;
public:
   StrSumRC(StrBase* L, char R);
   uint size() const;
   void Load(char*& ps) const;
   void Drop();
   const char* StringType() const { return "StrSumRC"; }
};


// null string

class StrNull : public StrRepNullTerm
{
public:
   StrNull();
   uint size() const;
   void Load(char*& ps) const;
   StrBase* Clone(StrBase**);
   StrBase* UnProtect();
   void Drop();
   const char* StringType() const { return "StrNull"; }
};

// global inline

inline void swap(String& A, String& B) { A.swap(B); }


// initialise string package
// to make sure String::npos = -1 and the null string is initialised
// before anything happens

class StringPackageInitialise
{
public:
   StringPackageInitialise();
};

static StringPackageInitialise DoStringPackageInitialise;


#ifdef use_namespace
}
#endif


#endif                                 // STRING_LIB


// body file: str.cpp



