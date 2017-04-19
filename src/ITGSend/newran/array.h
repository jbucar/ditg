/// \ingroup newran
///@{

/// \file array.h
/// Simple template array class.
/// Based a little on an article by Dag Bruck

#ifndef ARRAY_H
#define ARRAY_H

#include "myexcept.h"

template <class T>
class GeneralArray;

template <class T>
class Array;

template <class T>
class T_Array;

template <class T>
class Array2;

template <class T>
class T_Array2;



// This is a very primitive array class, concerned only
// with the allocation of space.
// Not available to the user; user classes are derived from this.
// It uses templates - so will not run in early versions of C++

/// The base of my other array classes.
/// \internal
template <class T>
class GeneralArray
{
protected:
   T* data;
   int sz;

   GeneralArray(int size);                  // constructors
   GeneralArray();
   GeneralArray(const GeneralArray<T> &);   // copy constructor

   T& El(int);                              // to access elements
   const T& El(int) const;

   GeneralArray<T>& operator= (const T &);
   GeneralArray<T>& operator= (const GeneralArray<T>&);
   GeneralArray<T> operator+(const GeneralArray<T>&) const;

   void ReSize(int size);

public:
   int length() const { return sz; }
   T* base() { return data; }
   const T* base() const { return data; }
   void SortAscending();
   T Sum() const;
   T Sum(const Array<bool>& Condition) const;
   ~GeneralArray();                         // destructor
};

template <class T>
class GeneralArrayIterator
{
public:
   GeneralArrayIterator(GeneralArray<T>& a) : p(a.base()), n(a.length()) {}
   T* operator++()    { return n <= 1 ? p=0 : (--n, ++p); }  // prefix
   T* operator++(int) { return n == 0 ? p=0 : (--n, p++); }  // postfix
   operator T* () { return p; }

private:
   T* p;
   int n;
};

/// Construct array with "size" elements.
template <class T>
GeneralArray<T>::GeneralArray(int size) : sz(size)
{
   if (sz > 0) { data = new T[sz];  if (data == 0) Throw(Bad_alloc("Array: out of space")); }
   else data = 0;
}

/// Construct array with no elements.
template <class T>
GeneralArray<T>::GeneralArray() : sz(0), data(0) {}

/// Copy constructor
template <class T>
GeneralArray<T>::GeneralArray(const GeneralArray<T> & X) : sz(X.sz)
{
   data = new T[sz]; if (data == 0) Throw(Bad_alloc("Array: out of space"));
   int i = sz >> 3; T* d = data; T* x = X.data;
   // should be using copy constructor
   while (i--)
   {
      *d++ = *x++; *d++ = *x++; *d++ = *x++; *d++ = *x++;
      *d++ = *x++; *d++ = *x++; *d++ = *x++; *d++ = *x++;
   }
   i=sz & 7; while (i--) *d++ = *x++;
}

template <class T>
GeneralArray<T>::~GeneralArray()
{ delete [] data; }               // should call individual destructors

/// Element of an array.
template <class T>
inline T& GeneralArray<T>::El(int i)
{ if (!(i >= 0 && i < sz)) Throw(Logic_error("Array: subscript error")); return data[i]; }


/// Element of an array.
template <class T>
inline const T&  GeneralArray<T>::El(int i) const
{ if (!(i >= 0 && i < sz)) Throw(Logic_error("Array: subscript error")); return data[i]; }

template <class T>
GeneralArray<T>& GeneralArray<T>::operator = (const T & x)
{
   int i = sz >> 3; T* d = data;
   while (i--)
   {
      *d++ = x; *d++ = x; *d++ = x; *d++ = x;
      *d++ = x; *d++ = x; *d++ = x; *d++ = x;
   }
   i=sz & 7; while (i--) *d++ = x;
   return *this;
}

/// Set array equal to another array.
template <class T>
GeneralArray<T>& GeneralArray<T>::operator = (const GeneralArray<T>& X)
{
   delete [] data;
   sz=X.sz; data = new T [sz];  if (data == 0) Throw(Bad_alloc("Array: out of space"));
   int i = sz >> 3; T* d = data; T* x = X.data;
   while (i--)
   {
      *d++ = *x++; *d++ = *x++; *d++ = *x++; *d++ = *x++;
      *d++ = *x++; *d++ = *x++; *d++ = *x++; *d++ = *x++;
   }
   i=sz & 7; while (i--) *d++ = *x++;
   return *this;
}

/// Add two arrays.
template<class T>
GeneralArray<T> GeneralArray<T>::operator+(const GeneralArray<T>& A) const
{
   int l = length(); if (l != A.length()) Throw(Logic_error("Array: incompatible subscripts"));
   const T* s1=base(); const T* s2=A.base();
   GeneralArray<T> Sum(l);
   T* s=Sum.base(); int i = l >> 2;
   while (i--)
   {
      *s++ = *s1++ + *s2++; *s++ = *s1++ + *s2++;
      *s++ = *s1++ + *s2++; *s++ = *s1++ + *s2++;
   }
   i=l & 3; while (i--) *s++ = *s1++ + *s2++;
   return Sum;
}

/// Resize and array.
template <class T>
void GeneralArray<T>::ReSize(int size)
{
   delete [] data;
   sz=size;
   if (sz) { data = new T [sz];  if (data == 0) Throw(Bad_alloc("Array: out of space")); }
   else data=0;
}


/// One dimensional array.
template <class T>
class Array : public GeneralArray<T>
{
protected:
   int low;

public:
  Array(int lb, int ub) : GeneralArray<T>(ub-lb+1), low(lb) {}
   Array() : GeneralArray<T>(), low(0) {}
   Array(const Array<T> &X) : GeneralArray<T>(X), low(X.low) {}
   Array(T_Array<T> &X);
   ~Array() {}

   T& operator () (int i) { return this->El(i-low); }
   const T& operator () (int i) const { return this->El(i-low); }

   Array<T>& operator = (const T & X)
      { GeneralArray<T>::operator=(X); return *this; }
   Array<T>& operator = (const Array<T>& X)
      { GeneralArray<T>::operator=(X); low = X.low; return *this; }
   Array<T>& operator = (T_Array<T>& X);

   void ReSize(int lb, int ub)                   ///< Resize an array
      { low=lb; GeneralArray<T>::ReSize(ub-lb+1); }

   int first() const { return low; }             ///< LB of index
   int last() const { return low+this->sz-1; }   ///< UB of index
   T_Array<T> operator+(const Array<T>&) const;
   T_Array<T> operator+(T_Array<T>&) const;
};

/// Temporary 1 dimensional array.
/// \internal
/// This is used for the result of a numeric operation. Its memory can be
/// reused in a subsequent operation reducing the need to copy data.
template <class T>
class T_Array : public Array<T>
{
public:
   T_Array() {}
   T_Array(int lb, int ub) : Array<T>(lb, ub) {}
   T_Array(int lb, int l, T* d) { this->low = lb; this->sz = l; this->data = d; }
   T_Array(T_Array<T>& ta)
      { this->low = ta.low; this->sz = ta.sz; ta.sz = 0; this->data = ta.data; ta.data = 0; }
   T_Array<T> operator+(const Array<T>&);
   ~T_Array() {}
   friend class Array<T>;
};


/// 2 dimensional array.
template <class T>
class Array2 : public GeneralArray<T>
{
private:
   int low1,low2;
   int sz1,sz2;

public:
   Array2(int lb1, int ub1, int lb2, int ub2);
   Array2() : GeneralArray<T>(), low1(0), low2(0), sz1(0), sz2(0) {}
   Array2(const Array2<T> &);
   ~Array2() {}

   T& operator () (int,int);
   const T& operator () (int,int) const;

   Array2<T>& operator = (const T & X)
      { GeneralArray<T>::operator=(X); return *this; }
   Array2<T>& operator = (const Array2<T>&);

   Array2<T> operator+(const Array2<T>&);

   void ReSize(int lb1, int ub1, int lb2, int ub2);

   int first1() const { return low1; }       ///< LB first dimension
   int last1() const { return low1+sz1-1; }  ///< UB first dimension
   int first2() const { return low2; }       ///< LB second dimension
   int last2() const { return low2+sz2-1; }  ///< UB second dimension

};

/// Construct 2D array with given bounds.
template <class T>
Array2<T>::Array2(int lb1, int ub1, int lb2, int ub2)
   : GeneralArray<T>((ub1-lb1+1)*(ub2-lb2+1)),
      low1(lb1), low2(lb2), sz1(ub1-lb1+1), sz2(ub2-lb2+1)
   { if (sz1<=0) Throw(Logic_error("Array: subscript error")); }

/// Construct 2D array equal to 2D array.
template <class T>
Array2<T>::Array2(const Array2<T> & X)
   : GeneralArray<T>(X),
      low1(X.low1), sz1(X.sz1), low2(X.low2), sz2(X.sz2) {}

/// Element of 2D array.
template <class T>
inline T& Array2<T>::operator() (int i, int j)
{
   i -= low1; j -= low2;
   if (!(j>=0 && j < sz2)) Throw(Logic_error("Array: subscript error"));
   return this->El(i*sz2+j);
}

/// Element of 2D array.
template <class T>
inline const T&  Array2<T>::operator() (int i, int j) const
{
   i -= low1; j -= low2;
   if (!(j>=0 && j < sz2)) Throw(Logic_error("Array: subscript error"));
   return this->El(i*sz2+j);
}

/// Set 2D array equal to another 2D array.
template <class T>
Array2<T>& Array2<T>::operator= (const Array2<T>& X)
{
   low1 = X.low1; sz1 = X.sz1; low2 = X.low2; sz2 = X.sz2;
   GeneralArray<T>::operator=(X);
   return *this;
}

/// Resize two dimensional array.
template <class T>
void Array2<T>::ReSize(int lb1, int ub1, int lb2, int ub2)
{
   low1=lb1; low2=lb2;
   if (ub1-lb1+1 < 0) Throw(Logic_error("Array: resize error"));
   sz1=ub1-lb1+1; sz2=ub2-lb2+1;
   GeneralArray<T>::ReSize(sz1*sz2);
}

/// Array + array.
template<class T>
T_Array<T> Array<T>::operator+(const Array<T>& A) const
{
   int lb = first(); if (A.first() != lb) Throw(Logic_error("Array: incompatible subscripts"));
   int l = this->length(); if (l != A.length()) Throw(Logic_error("Array: incompatible subscripts"));
   const T* s1=this->base(); const T* s2=A.base();
   T_Array<T> Sum(lb,l+lb-1);
   T* s=Sum.base(); int i = l >> 2;
   while (i--)
   {
      *s++ = *s1++ + *s2++; *s++ = *s1++ + *s2++;
      *s++ = *s1++ + *s2++; *s++ = *s1++ + *s2++;
   }
   i=l & 3; while (i--) *s++ = *s1++ + *s2++;
   return Sum;
}

/// Set array equal to temporary array.
template<class T>
Array<T>& Array<T>::operator= (T_Array<T>& X)
{
   delete this->data;
   low = X.low; this->sz = X.sz; X.sz = 0; this->data = X.data; X.data = 0;
   return *this;
}

/// Construct array equal to temporary array.
template<class T>
inline Array<T>::Array(T_Array<T>& X) : low(X.low)
   { this->sz = X.sz; X.sz = 0; this->data = X.data; X.data = 0; }

/// Temporary array + temporary array.
template<class T>
T_Array<T> Array<T>::operator+(T_Array<T>& A) const
{
   int lb = this->first(); if (A.first()!=lb) Throw(Logic_error("Array: incompatible subscripts"));
   int l = this->length(); if (l != A.length()) Throw(Logic_error("Array: incompatible subscripts"));
   const T* s1=this->base(); T* s=A.base();
   int i = l >> 2;
   while (i--) { *s++ += *s1++; *s++ += *s1++; *s++ += *s1++; *s++ += *s1++; }
   i=l & 3; while (i--) *s++ += *s1++;
   T_Array<T> Sum(lb, l, A.data);
   A.sz = 0; A.data = 0;
   return Sum;
}

/// Temporary array + array.
template<class T>
T_Array<T> T_Array<T>::operator+(const Array<T>& A)
{
   int lb = this->first(); if (A.first()!=lb) Throw(Logic_error("Array: incompatible subscripts"));
   int l = this->length(); if (l != A.length()) Throw(Logic_error("Array: incompatible subscripts"));
   T* s=this->base(); const T* s2=A.base();
   int i = l >> 2;
   while (i--) { *s++ += *s2++; *s++ += *s2++; *s++ += *s2++; *s++ += *s2++; }
   i=l & 3; while (i--) *s++ += *s2++;
   T_Array<T> Sum(lb, l, this->data);
   this->sz = 0; this->data = 0;
   return Sum;
}


/// Shell sort - from Sedgewick.
template <class T>
void GeneralArray<T>::SortAscending()
{
   int N = sz; T* a = data;
   int h;
   for (h = 1; h <= N / 9; h = 3*h + 1) {}
   for (; h > 0; h /= 3) for (int i = h; i < N; ++i)
   {
      T v = a[i]; int j = i;
      while (j>=h && a[j-h] > v) { a[j] = a[j-h]; j-= h; }
      a[j] = v;
   }
}

/// Sum of elements in an array.
template <class T>
T GeneralArray<T>::Sum() const
{
   T* t = data; int n = sz; T s = 0;
   while (n--) s += *t++;
   return s;
}

/// Sum of subset of elements in an array.
template <class T>
T GeneralArray<T>::Sum(const Array<bool>& Condition) const
{
   T* t = data; const bool* b = Condition.base(); int n = sz; T s = 0;
   while (n--) { if (*b++) s += *t++; else t++; }
   return s;
}

#endif

///@}


