// character string manipulation library
// Copyright (c) R B Davies 1996

// The is a string library that is intended to be compatible with the
// "class string" library in the April 1995 draft of the C++ standard.

// Parts are based on Tony Hansen s "The C++ Answer Book"


#define WANT_STREAM
#define WANT_MATH
#define WANT_STRING

#include "include.h"

#include "str.h"


#ifdef use_namespace
namespace RBD_STRING { using namespace RBD_COMMON; }
namespace RBD_LIBRARIES { using namespace RBD_STRING; }
namespace RBD_STRING {
#endif


//******************************* globals ********************************

uint String::npos = (uint)(-1);
                        // also initialised to -1 by StringPackageInitialise
uint String::s_index;
StrNull* String::ASN;

// string bodies

const char* String::StringType() const { return px->StringType(); }

StrBase* String::Clone() const { return px->Clone(&(StrBase*&)px); }

unsigned int String::refcount() const { return px->refcount(); }

void String::CleanUp() { if (px) px->Drop(); px = ASN; }

StrRep* String::GetStrRep() const
   { return px->GetStrRep(&(StrBase*&)px); }

StrRep* String::GetStrRepW() { return px->GetStrRepW(&px); }

char* String::GetData() const { return GetStrRep()->s; }

StrRep* String::Protect() const { return px->Protect(&(StrBase*&)px); }

bool String::HasCapacity() const { return px->HasCapacity(); }

String::String(StrBase* sb) : px(sb) {}

String::String() : px(ASN) {}

String::String(const String& str) { px = ASN; px = str.Clone(); }

String::String(const String& str, uint pos, uint n)
{
   px = ASN;                            // in case of later failure
   StrRep* str_p = str.GetStrRep();
   uint sz = str.size();
   if (pos > sz) Throw(Out_of_range("string index error\n"));
   sz -=  pos;                          // length of new string
   if (n < sz) sz = n;
   if (sz == 0) px = ASN;
   else px = new StrRepMult(str_p->s+pos, sz);
}

String::String(const char* s, uint n)
{
   px = ASN;                            // in case of later failure
   if (!s) Throw(Invalid_argument("string constructor: char* s = 0\n"));
   uint sz = 0; const char* S = s;
   while (*S++) { if (sz >= n) break; sz++; }
   if (sz == 0) px = ASN;
   else px = new StrRepMult(s, sz);
}

String::String(const char* s)
{
   px = ASN;                            // in case of later failure
   if (!s) Throw(Invalid_argument("string constructor: char* s = 0\n"));
   uint sz = strlen(s);
   if (sz == 0) px = ASN;
   else px = new StrRepMult(s, sz);
}

String::String(uint n, char c)
{
   px = ASN;                            // in case of later failure
   if (n == npos) Throw(Length_error("string length = npos\n"));
   if (n == 0) px = ASN;
   else if (n == 1) px = new CharSingle(c);
   else px = new CharRepeated(n, c);
}

String::~String() { if (px) px->Drop(); }

// do the operator= separately from assign because we do not ever
// want to just copy into existing memory space. ie we do not
// maintain capacity when we use operator= .

String& String::operator=(const String& str)
   { StrBase* p = px; px = str.Clone(); p->Drop(); return *this; }

String& String::operator=(const char* s)
{
   StrBase* p = px;
   if (strlen(s) == 0) px = ASN;
   else px = new StrRepMult(s);
   p->Drop(); return *this;
}


String& String::operator=(const char c)
{
   px->Drop(); px = new CharSingle(c);
   return *this;
}

uint String::size() const { return px->size(); }

uint String::length() const { return px->size(); }

uint String::max_size() const { return npos-1; }

void String::resize(uint n, char c)
{
   if (n == npos) Throw(Length_error("string length = npos\n"));
   if (n == 0) { px->Drop(); px = ASN; return; }
   StrRep* p = GetStrRep();
   if (n <= p->sz) px = new StrRepMult(p->s, n);
   else px = new StrRepMult(p->s, p->sz, n, c);
   p->Drop();
}

uint String::capacity() const { return px->capacity(); }

void String::reserve(uint res_arg)
{
   if (res_arg == 0)                        // remove capacity
   {
      StrRep* p = new StrRepMult(size());
      char* target = p->s; px->Load(target); px->Drop(); px = p;
   }
   else if ( res_arg > size() || capacity() != size() )
                                            // need to change storage
   {
      if (res_arg < size()) res_arg = size();
      StrRep* p = new StrRepCap(size(), res_arg);
      char* target = p->s; px->Load(target); px->Drop(); px = p;
   }
   else px->WithCapacity(&(StrBase*&)px);   // convert to string with capacity

}

void String::clear() { if (px) px->Drop(); px = ASN; }

bool String::empty() const { return px->size() == 0; }

char String::operator[](uint pos) const
{
   StrRep* p = GetStrRep();
   if (pos >= p->size())
   {
      if (pos == p->size()) return 0;
      else Throw(Out_of_range("string index error\n"));
   }
   return (p->s)[pos];
}

char& String::operator[](uint pos)
{
   StrRep* p = GetStrRepW();
   if (pos >= p->size()) Throw(Out_of_range("string index error\n"));
   return p->s[pos];
}

char String::at(uint n) const
{
   StrRep* p = GetStrRep();
   if (n >= p->size())
   {
      if (n == p->size()) return 0;
      else Throw(Out_of_range("string index error\n"));
   }
   return p->s[n];
}

char& String::at(uint n)
{
   StrRep* p = GetStrRepW();
   if (n >= p->size()) Throw(Out_of_range("string index error\n"));
   return p->s[n];
}


//******************** the line editing functions ************************

String& String::operator+=(const String& rhs) { return append(rhs); }

String& String::operator+=(const char* s) { return append(s); }

String& String::operator+=(char c)
{
   if (HasCapacity()) return my_append(CharSingle(c));
   else
   {
      if (size() >= npos - 1) Throw(Length_error("string length >= npos\n"));
      px = new StrSumRC(px ,c);
      return *this;
   }
}


String& String::my_append(const StrBase& sb)
{
   StrRep* p = GetStrRep();
   uint sz = size(); uint rlen =sb.size();
   if (sz >= npos - rlen) Throw(Length_error("string length >= npos\n"));
   rlen += sz;
   if (rlen > capacity() || refcount() != 0)
   {
      // must make new string
      StrRep* oldp = p; p = new StrRepMult(rlen);
      memcpy(p->s, oldp->s, sz); oldp->Drop(); px = p;
   }
   char* target = p->s+sz; sb.Load(target); p->sz = rlen; return *this;
}

String& String::append(const String& str)
{
   if (HasCapacity()) return my_append(CharSeq(str));
   else
   {
      if (size() >= npos - str.size())
         Throw(Length_error("string length >= npos\n"));
      px = new StrSum(px, str.Clone());
      return *this;
   }
}

String& String::append(const String& str, uint pos, uint n)
{
   if (HasCapacity()) return my_append(CharSeq(str, pos, n));
   else
   {
      CharSeq CS(str, pos, n);
      if (size() >= npos - CS.sz)
         Throw(Length_error("string length >= npos\n"));
      StrBase* rhs = new StrRep(CS.s, CS.sz);
      px = new StrSum(px, rhs);
      return *this;
   }
}

String& String::append(const char* s, uint n)
{
   if (HasCapacity()) return my_append(CharSeq((char*)s, n));
   else
   {
      StrBase* rhs = new StrRep(s, n);
      if (size() >= npos - rhs->size())
      {
         delete rhs;
         Throw(Length_error("string length >= npos\n"));
      }
      px = new StrSum(px, rhs);
      return *this;
   }
}

String& String::append(const char* s)
{
   if (HasCapacity()) return my_append(CharSeq((char*)s));
   else
   {
      StrBase* rhs = new StrRep(s);
      if (size() >= npos - rhs->size())
      {
         delete rhs;
         Throw(Length_error("string length >= npos\n"));
      }
      px = new StrSum(px, rhs);
      return *this;
   }
}

String& String::append(uint n, char c)
{
   if (size() >= npos - n)
      Throw(Length_error("string length >= npos\n"));

   if (HasCapacity()) return my_append(CharRepeated(n, c));
   else if (n == 1)
   {
      px = new StrSumRC(px ,c);
   }
   else
   {
      StrBase* rhs = new CharRepeated(n, c);
      px = new StrSum(px, rhs);
   }
   return *this;
}

// assign with argument being all or part of the target string
// should not be a problem under HasCapacity since Load uses
// memmove.

String& String::assign(const String& str)
{
   if (HasCapacity() && str.size() <= capacity())
   {
      StrRep* p = GetStrRep(); char* target = p->s;
      str.px->Load(target); p->sz = str.size();
   }
   else if (px != str.px) { px->Drop(); px = str.Clone(); }
   return *this;
}

String& String::my_assign(const StrBase& sb)
{
   uint sz = sb.size(); StrRep* p;
   if (HasCapacity() && sz <= capacity())
   {
      p = GetStrRep(); char* target = p->s;
      sb.Load(target); p->sz = sz; return *this;
   }
   else if (sz == 0) p = ASN;
   else
   {
      p = new StrRepMult(sb.size());
      char* target = p->s; sb.Load(target);
   }
   px->Drop(); px = p; return *this;
}

String& String::assign(const String& str, uint pos, uint n)
   { return my_assign(CharSeq(str, pos, n)); }

String& String::assign(const char* s, uint n)
   { return my_assign(CharSeq((char*)s, n)); }

String& String::assign(const char* s)
   { return my_assign(CharSeq((char*)s)); }

String& String::assign(uint n, char c)
   { return my_assign(CharRepeated(n, c)); }

String& String::my_insert(uint pos, const StrBase& sb)
{
   uint sz = size(); uint rlen =sb.size();
   if (pos > sz) Throw(Out_of_range("string index error\n"));
   if (sz >= npos - rlen) Throw(Length_error("string length >= npos\n"));
   StrRep* oldp = GetStrRep();
   if (HasCapacity() && sz + rlen <= capacity())
   {
      // must allow for possibility that sb is part of *this
      char* target = oldp->s + pos;
      StrRep tail(target, sz-pos);
      sb.Load(target); tail.Load(target); oldp->sz = sz + rlen;
   }
   else
   {
      StrRep* p = new StrRepMult(sz + rlen); px = p;
      char* target = p->s; memcpy(target, oldp->s, pos);
      target += pos; sb.Load(target);
      memcpy(target, oldp->s + pos, sz-pos);
      oldp->Drop();
   }
   return *this;
}

String& String::insert(uint pos1, const String& str)
   { return my_insert(pos1, *(str.px)); }

String& String::insert(uint pos1, const String& str, uint pos2, uint n)
   { return my_insert(pos1, CharSeq(str, pos2, n)); }

String& String::insert(uint pos, const char* s, uint n)
   { return my_insert(pos, CharSeq((char*)s, n)); }

String& String::insert(uint pos, uint n, char c)
   { return my_insert(pos, CharRepeated(n, c)); }

String& String::erase(uint pos, uint n)
{
   uint sz = size();
   if (pos > sz) Throw(Out_of_range("string index error\n"));
   uint xlen = sz-pos; if (xlen > n) xlen = n; sz -= xlen;
   if (sz == 0)
   {
      if (HasCapacity()) GetStrRep()->sz = 0;
      else { px->Drop(); px = ASN; return *this; }
   }
   StrRep* oldp = GetStrRep();
   if (HasCapacity())
   {
      char* target = oldp->s + pos;
      memmove(target, target + xlen, sz-pos);
      oldp->sz = sz;
   }
   else
   {
      StrRep* p = new StrRepMult(sz);
      px = p;
      char* target = p->s; memcpy(target, oldp->s, pos);
      target += pos; memcpy(target, oldp->s+pos+xlen, sz-pos);
      oldp->Drop();
   }
   return *this;
}

String& String::my_replace(uint pos, uint n, const StrBase& sb)
{
   uint sz = size(); uint rlen =sb.size();
   if (pos > sz) Throw(Out_of_range("string index error\n"));
   uint xlen = sz-pos; if (xlen > n) xlen = n; sz -= xlen;
   if (sz >= npos - rlen) Throw(Length_error("string length >= npos\n"));
   StrRep* oldp = GetStrRep();
   if (rlen == xlen && refcount() == 0)          // in-place replace OK
      { char* target = oldp->s+pos; sb.Load(target); }
   else
   if (HasCapacity() && sz + rlen <= capacity())
   {
      // must allow for possibility that sb is part of *this
      char* target = oldp->s + pos;
      StrRep tail(target + xlen, sz-pos);
      sb.Load(target); tail.Load(target); oldp->sz = sz + rlen;
   }
   else if (sz + rlen == 0) { px->Drop(); px = ASN; return *this; }
   else
   {
      StrRep* p = new StrRepMult(sz+rlen); px = p;
      char* target = p->s; memcpy(target, oldp->s, pos);
      target += pos; sb.Load(target);
      memcpy(target, oldp->s+pos+xlen, sz-pos);
      oldp->Drop();
   }
   return *this;
}

String& String::replace(uint pos1, uint n1, const String& str)
   { return my_replace(pos1, n1, *(str.px)); }

String& String::replace(uint pos1, uint n1, const String& str, uint pos2,
   uint n2)
   { return my_replace(pos1, n1, CharSeq(str, pos2, n2)); }

String& String::replace(uint pos, uint n1, const char* s, uint n2)
   { return  my_replace(pos, n1, CharSeq((char*)s, n2)); }

String& String::replace(uint pos, uint n1, uint n2, char c)
   { return my_replace(pos, n1, CharRepeated(n2, c)); }


//************************* some odds and sods ***************************

uint String::copy(char* s, uint n, uint pos) const
{
   StrRep* p = GetStrRep(); uint rlen = size();
   if (pos > rlen) Throw(Out_of_range("string index error\n"));
   rlen -= pos; if (rlen > n) rlen = n;
   memcpy(s, p->s+pos, rlen); return rlen;
}

void String::swap(String& str)
{
   StrBase* oldp = px->UnProtect(); px = str.px->UnProtect(); str.px = oldp;
}

const char* String::c_str() const
   { return px->NullTerminate(&(StrBase*&)px); }

const char* String::data() const
   { StrRep* p = Protect(); ((String&)*this).px = p; return p->s; }


//***************************** the find functions ***********************

uint String::find(const CharSeq& str, uint pos) const
{
   uint str_sz = str.size(); uint sz = size();
   if (sz < str_sz || pos > sz - str_sz) return npos;
      // need both checks to avoid overflows or negatives
   if (str_sz == 0) return pos;
   const char* start = GetData(); const char* newstart = start+pos;
   const char* str_loc = str.s;
   if (str_sz == 1)
   {
      newstart = (const char*)memchr(newstart, *str_loc, sz-pos);
      if (!newstart) return npos; else return (uint)(newstart-start);
   }
   uint sindx = s_index;   // otherwise may get problems if multithreading
   for (;;)
   {
      if (sindx >= str_sz) sindx = 0;
      uint m = sz+1-str_sz-(uint)(newstart-start);
      if (m==0) { s_index = sindx; return npos; }
      newstart = (const char*)memchr(newstart+sindx, str_loc[sindx], m);
      if (!newstart) { s_index = sindx; return npos; }
      newstart -= sindx;
      if (memcmp(newstart, str_loc, str_sz) == 0)
         { s_index = sindx; return (uint)(newstart-start); }
      sindx++; newstart++;
   }
}

uint String::find(const String& str, uint pos) const
   { return find(*(str.GetStrRep()), pos); }

uint String::find(const char* s, uint pos, uint n) const
   { return find(CharSeq((char*)s,n),pos); }

uint String::find(const char* s, uint pos) const
   { return find(CharSeq((char*)s),pos); }

uint String::find(const char c, uint pos) const
{
   uint sz = size();
   if (pos >= sz) return npos;     // also ensures sz != 0
   const char* start = GetData(); const char* newstart = start+pos;
   newstart = (const char*)memchr(newstart, c, sz-pos);
   if (!newstart) return npos; else return (uint)(newstart-start);
}

uint String::rfind(const CharSeq& str, uint pos) const
{
   // can return sz if searching for a zero length string
   // is this what the committee wants?
   uint str_sz = str.size(); uint sz = size();
   if (sz < str_sz) return npos;
   sz -= str_sz; if (pos > sz) pos = sz;
   if (str_sz == 0) return pos;
   const char* start = GetData(); const char* newstart = start+pos;
   const char* str_loc = str.s;
   uint sindx = s_index;
   for (;;)
   {
      if (sindx >= str_sz) sindx = 0;
      uint i = (uint)(newstart-start);
      char c = str_loc[sindx]; newstart += sindx;
      while (*newstart != c)
      {
         if (i-- == 0) { s_index = sindx; return npos; }
         newstart--;
      }
      newstart -= sindx;
      if (!memcmp(newstart, str_loc, str_sz))
         { s_index = sindx; return (uint)(newstart-start); }
      if (--newstart < start) { s_index = sindx; return npos; }
      sindx++;
   }
}

uint String::rfind(const String& str, uint pos) const
   { return rfind(*(str.GetStrRep()), pos); }

uint String::rfind(const char* s, uint pos, uint n) const
   { return rfind(CharSeq((char*)s,n),pos); }

uint String::rfind(const char* s, uint pos) const
   { return rfind(CharSeq((char*)s),pos); }

uint String::rfind(const char c, uint pos) const
{
   uint i = size(); if (i > pos) i = pos + 1;
   char* s = GetData() + i;
   while (i-- > 0) { if (*(--s) == c) return i; }
   return npos;
}

uint String::find_first_of(const CharSeq& str, uint pos) const
{
   // this should be done without memchr at least when str is long
   uint sz = size(); uint str_sz = str.size();
   if (str_sz == 0 || pos >= sz) return npos;  // also ensures sz != 0
   const char* start = GetData(); const char* newstart = start+pos;
   const char* str_loc = str.s; sz -= pos;
   while (sz--)
   {
      if (memchr(str_loc, *newstart, str_sz)) return (uint)(newstart-start);
      newstart++;
   }
   return npos;
}

uint String::find_first_of(const String& str, uint pos) const
   { return find_first_of(*(str.GetStrRep()), pos); }

uint String::find_first_of(const char* s, uint pos, uint n) const
   { return find_first_of(CharSeq((char*)s,n),pos); }

uint String::find_first_of(const char* s, uint pos) const
   { return find_first_of(CharSeq((char*)s),pos); }

uint String::find_first_of(const char c, uint pos) const
   { return find(c, pos); }

uint String::find_last_of(const CharSeq& str, uint pos) const
{
   uint sz = size(); if (sz > pos) sz = pos + 1;
   uint str_sz = str.size(); if (str_sz == 0) return npos;
   const char* start = GetData(); const char* newstart = start+sz;
   const char* str_loc = str.s;
   while (sz--)
   {
      if (memchr(str_loc, *(--newstart), str_sz))
         return (uint)(newstart-start);
   }
   return npos;
}

uint String::find_last_of(const String& str, uint pos) const
   { return find_last_of(*(str.GetStrRep()), pos); }

uint String::find_last_of(const char* s, uint pos, uint n) const
   { return find_last_of(CharSeq((char*)s,n),pos); }

uint String::find_last_of(const char* s, uint pos) const
   { return find_last_of(CharSeq((char*)s),pos); }

uint String::find_last_of(const char c, uint pos) const
   { return rfind(c, pos); }

uint String::find_first_not_of(const CharSeq& str, uint pos) const
{
   uint sz = size(); uint str_sz = str.size();
   if (pos >= sz) return npos;  // also ensures sz != 0
   if (str_sz == 0 ) return pos;
   const char* start = GetData(); const char* newstart = start+pos;
   const char* str_loc = str.s; sz -= pos;
   while (sz--)
   {
      if (!memchr(str_loc, *newstart, str_sz)) return (uint)(newstart-start);
      newstart++;
   }
   return npos;
}

uint String::find_first_not_of(const String& str, uint pos) const
   { return find_first_not_of(*(str.GetStrRep()), pos); }

uint String::find_first_not_of(const char* s, uint pos, uint n) const
   { return find_first_not_of(CharSeq((char*)s,n),pos); }

uint String::find_first_not_of(const char* s, uint pos) const
   { return find_first_not_of(CharSeq((char*)s),pos); }

uint String::find_first_not_of(const char c, uint pos) const
{
   uint sz = size();
   if (pos >= sz) return npos;     // also ensures sz != 0
   char* s = GetData() + pos; uint i = sz - pos;
   while (i > 0) { if (*(s++) != c) return sz - i; i--; }
   return npos;
}

uint String::find_last_not_of(const CharSeq& str, uint pos) const
{
   uint sz = size(); if (sz > pos) sz = pos + 1;
   if (sz == 0) return npos;
   uint str_sz = str.size(); if (str_sz == 0) return sz - 1;
   const char* start = GetData(); const char* newstart = start+sz;
   const char* str_loc = str.s;
   while (sz--)
   {
      if (!memchr(str_loc, *(--newstart), str_sz))
         return (uint)(newstart-start);
   }
   return npos;
}

uint String::find_last_not_of(const String& str, uint pos) const
   { return find_last_not_of(*(str.GetStrRep()), pos); }

uint String::find_last_not_of(const char* s, uint pos, uint n) const
   { return find_last_not_of(CharSeq((char*)s,n),pos); }

uint String::find_last_not_of(const char* s, uint pos) const
   { return find_last_not_of(CharSeq((char*)s),pos); }

uint String::find_last_not_of(const char c, uint pos) const
{
   uint i = size(); if (i > pos) i = pos + 1;
   char* s = GetData() + i;
   while (i-- > 0) { if (*(--s) != c) return i; }
   return npos;
}

String String::substr(uint pos, uint n) const
   { return String(*this, pos, n); }


//***************************** the booleans *****************************

int String::compare(uint pos, uint n, const CharSeq& sb) const
{
   StrRep* p = GetStrRep(); uint sz = size(); uint str_sz = sb.size();
   if (pos > sz) Throw(Out_of_range("string index error\n"));
   sz -= pos;
   if (sz > n) sz = n; else n = sz;
   if (n > str_sz) n = str_sz;
   int m = memcmp(p->s+pos, sb.s, n); if (m) return m;
   if (sz < str_sz) return -1;
   if (sz > str_sz) return 1;
   return 0;
}

int String::compare(const String& str) const
   { return compare(0, npos, CharSeq(str)); }

int String::compare(uint pos1, uint n1, const String& str) const
   { return compare(pos1, n1, CharSeq(str)); }

int String::compare(uint pos1, uint n1,
   const String& str, uint pos2, uint n2) const
   { return compare(pos1, n1, CharSeq(str, pos2, n2)); }

int String::compare(uint pos, uint n1, const char* s, uint n2) const
   { return compare(pos, n1, CharSeq((char*)s, n2)); }

int String::compare(const char* s) const
   { return compare(0, npos, CharSeq((char*)s)); }


bool operator==(const String& lhs, const String& rhs)
   { return lhs.compare(rhs) == 0; }

bool operator==(const char* lhs, const String& rhs)
   { return rhs.compare(lhs) == 0; }

bool operator==(const String& lhs, const char* rhs)
   { return lhs.compare(rhs) == 0; }

bool operator!=(const String& lhs, const String& rhs)
   { return lhs.compare(rhs) != 0; }

bool operator!=(const char* lhs, const String& rhs)
   { return rhs.compare(lhs) != 0; }

bool operator!=(const String& lhs, const char* rhs)
   { return lhs.compare(rhs) != 0; }

bool operator<(const String& lhs, const String& rhs)
   { return lhs.compare(rhs) < 0; }

bool operator<(const char* lhs, const String& rhs)
   { return rhs.compare(lhs) > 0; }

bool operator<(const String& lhs, const char* rhs)
   { return lhs.compare(rhs) < 0; }

bool operator>(const String& lhs, const String& rhs)
   { return lhs.compare(rhs) > 0; }

bool operator>(const char* lhs, const String& rhs)
   { return rhs.compare(lhs) < 0; }

bool operator>(const String& lhs, const char* rhs)
   { return lhs.compare(rhs) > 0; }

bool operator<=(const String& lhs, const String& rhs)
   { return lhs.compare(rhs) <= 0; }

bool operator<=(const char* lhs, const String& rhs)
   { return rhs.compare(lhs) >= 0; }

bool operator<=(const String& lhs, const char* rhs)
   { return lhs.compare(rhs) <= 0; }

bool operator>=(const String& lhs, const String& rhs)
   { return lhs.compare(rhs) >= 0; }

bool operator>=(const char* lhs, const String& rhs)
   { return rhs.compare(lhs) <= 0; }

bool operator>=(const String& lhs, const char* rhs)
   { return lhs.compare(rhs) >= 0; }


//****************** the io routines (need more work) ********************

istream& operator>>(istream& is, String& str)
{
   char buf[1024];
   is.width(1024);
   is >> buf; str = buf;
   is.width(0);
   return is;
}

ostream& operator<<(ostream& os, const String& str)
{ return os << str.c_str(); }


// from Alain Decamps
istream& getline(istream& is, String& str, char delim)
{
  int token;
  str.reserve(2048);
  str.erase();  // with current implementation no reallocation here
  while (((token = is.get()) != EOF) && (token != delim))
     { str += char(token); }
  if (token == EOF)
     { is.clear(ios::eofbit|is.rdstate()); }
  // str.reserve(0); // waste of time if used in a "while not EOF"
  return is;
}


//******************** bodies of the helper classes **********************

// the virtual functions - do not inline

uint StrSum::size() const { return lhs->size() + rhs->size(); }
uint StrSumLC::size() const { return 1 + rhs->size(); }
uint StrSumRC::size() const { return lhs->size() + 1; }
uint CharSeq::size() const { return sz; }
uint CharSingle::size() const { return 1; }
uint CharRepeated::size() const { return n; }
uint StrNull::size() const { return 0; }


uint StrBase::capacity() const { return size(); }
uint CharSeq::capacity() const { return sz; }
uint StrRepCap::capacity() const { return cpx; }


void StrSum::Load(char*& ps) const { lhs->Load(ps); rhs->Load(ps); }
void StrSumLC::Load(char*& ps) const { *ps++ = lhs; rhs->Load(ps); }
void StrSumRC::Load(char*& ps) const { lhs->Load(ps); *ps++ = rhs; }
void CharSeq::Load(char*& ps) const { memmove(ps, s, sz); ps += sz; }
void CharSingle::Load(char*& ps) const { *ps++ = c; }
void CharRepeated::Load(char*& ps) const { memset(ps, c, n); ps += n; }
void StrNull::Load(char*&) const {}


StrRep* StrBase::GetStrRep(StrBase** px)
{
   StrRep* p = new StrRepMult(size()); char* target = p->s;
   Load(target); (*px)->Drop(); *px = p; return p;
}

StrRep* StrRep::GetStrRep(StrBase**) { return this; }

StrRep* StrBase::GetStrRepW(StrBase** px)
{
   StrRep* p = new StrRepMult(size()); char* target = p->s;
   Load(target); (*px)->Drop(); *px = p; return p;
}

StrRep* StrRep::GetStrRepW(StrBase**) { return this; }

StrRep* StrRepMult::GetStrRepW(StrBase** px)
{
   if (refcnt == 0) return this;
   StrRep* p = new StrRepMult(s, sz);
   (*px)->Drop(); *px = p; return p;
}

StrRep* StrBase::Protect(StrBase** px)
{
   StrRep* p = new StrRep(size()); char* target = p->s;
   Load(target); (*px)->Drop(); *px = p; return p;
}

StrRep* StrRep::Protect(StrBase**) { return this; }

StrRep* StrRepMult::Protect(StrBase** px)
{
   StrRep* p;
   if (refcnt == 0)
   {
      p = new StrRep();
      p->sz = sz; sz = 0; char* s1 = p->s; p->s = s; s = s1;
   }
   else
   {
      p = new StrRep(s, sz);
   }
   (*px)->Drop(); *px = p; return p;
}

StrBase* StrBase::UnProtect() { return this; }

StrBase* StrRep::UnProtect()
{
   StrRepMult* p = new StrRepMult();
   p->sz = sz; sz = 0; char* s1 = p->s; p->s = s; s = s1;
   p->refcnt = 0; delete this; return p;
}

StrBase* StrRepMult::UnProtect() { return this; }

StrBase* StrRepCap::UnProtect()
{
   StrRepMult* p = new StrRepMult(s, sz);
   delete this; return p;
}

StrBase* StrRepNullTerm::UnProtect()
{
   StrRepMult* p = new StrRepMult(s, sz);
   delete this; return p;
}

StrBase* StrNull::UnProtect() { return this; }

void StrBase::WithCapacity(StrBase** px)
{
   uint sz1 = size();
   StrRepCap* p = new StrRepCap(sz1,sz1); char* target = p->s;
   Load(target); (*px)->Drop(); *px = p;
}

void StrRepCap::WithCapacity(StrBase**) {}

void StrRep::WithCapacity(StrBase** px)
{
   StrRepCap* p = new StrRepCap();
   p->cpx = p->sz = sz;  sz = 0;  char* s1 = p->s;  p->s = s;  s = s1;
   (*px)->Drop(); *px = p;
}

void StrRepMult::WithCapacity(StrBase** px)
{
   StrRepCap* p;
   if (refcnt == 0)
   {
      p = new StrRepCap();
      p->cpx = p->sz = sz; sz = 0; char* s1 = p->s; p->s = s; s = s1;
   }
   else
   {
      p = new StrRepCap(s, sz);
   }
   (*px)->Drop(); *px = p;
}

void StrRepNullTerm::WithCapacity(StrBase** px)
{
   StrRepCap* p = new StrRepCap(s, sz);
   (*px)->Drop(); *px = p;
}


const char* StrBase::NullTerminate(StrBase** px)
{
   StrRep* p = new StrRepNullTerm(size());
   char* target = p->s; char* t = target; Load(target); *target = 0;
   (*px)->Drop(); *px = p; return t;
}

const char* StrRepNullTerm::NullTerminate(StrBase**) { return s; }

const char* StrRepCap::NullTerminate(StrBase** px)
{
   if (cpx > sz) { s[sz] = 0; return s; }
   else return StrBase::NullTerminate(px);
}


void StrSum::Drop()
   { if (ref) ref = false; else { lhs->Drop(); rhs->Drop(); delete this; } }

void StrSumLC::Drop()
   { if (ref) ref = false; else { rhs->Drop(); delete this; } }

void StrSumRC::Drop()
   { if (ref) ref = false; else { lhs->Drop(); delete this; } }

void StrBase::Drop() { delete this; }
void StrRepMult::Drop() { if (refcnt-- == 0) delete this; }
void StrNull::Drop() {}


StrBase* StrSumBase::Clone(StrBase** px)
{
   if (ref)
   {
      StrRepMult* p = new StrRepMult(size());
      p->refcnt++;
      char* target = p->s; Load(target); (*px)->Drop(); *px = p;
      return p;
   }
   else { ref = true; return this; }
}

StrBase* CharSeq::Clone(StrBase**)
{
   StrBase* sb = new StrRep(s,sz);
   return sb;
}

StrBase* StrRep::Clone(StrBase**)
{
   StrRepMult* p = new StrRepMult(s, size());
   return p;
}

StrBase* StrRepMult::Clone(StrBase**) { refcnt++; return this; }

StrBase* CharSingle::Clone(StrBase**)
{
   StrBase* sb = new CharSingle(c);
   return sb;
}

StrBase* CharRepeated::Clone(StrBase**)
{
   StrBase* sb = new CharRepeated(n,c);
   return sb;
}

StrBase* StrNull::Clone(StrBase**) { return this; }

bool StrBase::HasCapacity() const { return false; }
bool StrRepCap::HasCapacity() const { return true; }


unsigned int StrBase::refcount() const { return 0; }
unsigned int StrRepMult::refcount() const { return refcnt; }
unsigned int StrSumBase::refcount() const { return (ref) ? 1 : 0; }



//***************************** constructors *****************************

StrRep::StrRep(uint SZ) : CharSeq(SZ)
{
   s = new char[sz];
}

StrRep::StrRep(uint n, char c) : CharSeq(n)
{
   s = new char[sz];
   memset(s, c, sz);
}

StrRep::StrRep(const char* x)
{
   sz = strlen(x);
   s = new char[sz];
   memcpy(s, x, sz);
}

StrRep::StrRep(const char* x, uint len) : CharSeq(len)
{
   s = new char[sz];
   memcpy(s, x, sz);
}

StrRep::StrRep(const char* x, uint len, uint n, char c)
   : CharSeq(n)
{
   s = new char[sz];
   memcpy(s, x, len); memset(s+len, c, n-len);
}

StrRep::~StrRep() { if (s) delete [] s; }


StrRepMult::StrRepMult(uint SZ) : StrRep(SZ), refcnt(0) {}
StrRepMult::StrRepMult(uint n, char c) : StrRep(n, c), refcnt(0) {}
StrRepMult::StrRepMult(const char* x) : StrRep(x), refcnt(0) {}

StrRepMult::StrRepMult(const char* x, uint len)
   : StrRep(x, len), refcnt(0) {}

StrRepMult::StrRepMult(const char* x, uint len, uint n, char c)
   : StrRep(x, len, n, c), refcnt(0) {}


StrRepCap::StrRepCap(uint len, uint cp) : StrRep(cp), cpx(cp) { sz = len; }

StrRepCap::StrRepCap(const char* x, uint len) : StrRep(x, len), cpx(len) {}


StrRepNullTerm::StrRepNullTerm(uint len) : StrRep(len + 1) { sz = len; }

StrNull::StrNull() : StrRepNullTerm(0) { *s = 0; }

CharSeq::CharSeq(char* S) : sz(strlen(S)), s(S) {}

CharSeq::CharSeq(char* S, uint n) : s(S)
{ uint i = 0; while (*S++) { if (i >= n) break; i++; }  sz = i; }

CharSeq::CharSeq(const String& str) : sz(str.size()), s(str.GetData())  {}

CharSeq::CharSeq(const String& str, uint pos, uint n)
{
   sz =str.size(); if (pos > sz) Throw(Out_of_range("string index error\n"));
   sz -= pos; if (sz > n) sz = n;
   s = str.GetData()+pos;
}

CharRepeated::CharRepeated(int N, char C) : CharSingle(C), n(N)
   { if (n==String::npos) Throw(Length_error("string length = npos\n")); }

StrSum::StrSum(StrBase* L, StrBase* R) : lhs(L), rhs(R) {}
StrSumLC::StrSumLC(char L, StrBase* R) : lhs(L), rhs(R) {}
StrSumRC::StrSumRC(StrBase* L, char R) : lhs(L), rhs(R) {}
StrSumBase::StrSumBase() : ref(false) {}


//******************************** operator+ *****************************

String operator+(const String& lhs, const String& rhs)
{
   StrSum* sos = new StrSum(lhs.Clone(), rhs.Clone());
   return String(sos);
}

String operator+(const char* lhs, const String& rhs)
{
   StrRep* lhscs = new StrRep((char*)lhs);
   StrSum* sos = new StrSum(lhscs, rhs.Clone());
   return String(sos);
}

String operator+(char lhs, const String& rhs)
{
   StrSumLC* sos = new StrSumLC(lhs, rhs.Clone());
   return String(sos);
}

String operator+(const String& lhs, const char* rhs)
{
   StrRep* rhscs = new StrRep((char*)rhs);
   StrSum* sos = new StrSum(lhs.Clone(), rhscs);
   return String(sos);
}

String operator+(const String& lhs, char rhs)
{
   StrSumRC* sos = new StrSumRC(lhs.Clone(), rhs);
   return String(sos);
}

//******************** initialise string package **********************

// this is called whenever str.h is loaded; this ensures that npos is set to
// -1  and ASN is initialised before any globals involving strings get
// initialised.

StringPackageInitialise::StringPackageInitialise()
{
   if (!String::ASN)
      { String::ASN = new StrNull; String::npos = (uint)(-1); }
}

//*********************************************************************


#ifdef use_namespace
}
#endif
