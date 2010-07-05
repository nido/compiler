/*

  Copyright (C) 2000, 2001 Silicon Graphics, Inc.  All Rights Reserved.

  This program is free software; you can redistribute it and/or modify it
  under the terms of version 2 of the GNU General Public License as
  published by the Free Software Foundation.

  This program is distributed in the hope that it would be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

  Further, this software is distributed without any warranty that it is
  free of the rightful claim of any third person regarding infringement 
  or the like.  Any license provided herein, whether implied or 
  otherwise, applies only to this software file.  Patent licenses, if 
  any, provided herein do not apply to combinations of this program with 
  other software, or any other product whatsoever.  

  You should have received a copy of the GNU General Public License along
  with this program; if not, write the Free Software Foundation, Inc., 59
  Temple Place - Suite 330, Boston MA 02111-1307, USA.

  Contact information:  Silicon Graphics, Inc., 1600 Amphitheatre Pky,
  Mountain View, CA 94043, or:

  http://www.sgi.com

  For further information regarding this notice, see:

  http://oss.sgi.com/projects/GenInfo/NoticeExplan

*/


#ifndef PQS_DEFS_included
#define PQS_DEFS_included

#include <stdio.h>
// [HK]
#if __GNUC__ >= 3
#include <set>
#include <algorithm>
#include <vector>
// using std::less;
// using std::set;
#else
#include <set.h>
#include <algo.h>
#include <vector.h>
#endif //  __GNUC__ >= 3


// Trace flags:
// -Wb,-ttpqs:flags
#define PQS_ENTRY     1
#define PQS_HASH      2

// Forward references

// Aliases

//
// Set class template. Based on the STL set with some additional syntactic to make it 
// A little easier to use. 
//
template <class T, class C = std::less<T> >
class PQS_SET {
public:
#ifdef PQS_USE_MEMPOOLS
   typedef mempool_allocator<T> set_allocator_type;
#else
    // [HK]
#if __GNUC__ >=3
    typedef std::allocator<T> set_allocator_type;
#else
   typedef alloc set_allocator_type;
#endif // __GNUC__ >=3
#endif
   typedef std::set<T,C,set_allocator_type> set_type;
    // [HK]
#if __GNUC__ >= 3
   typedef typename set_type::iterator set_iterator_type;
#else
   typedef set_type::iterator set_iterator_type;
#endif // __GNUC__ >= 3
   set_type _set;

   PQS_SET<T,C>() 
#ifdef PQS_USE_MEMPOOLS
      : _set(set_type::key_compare(),set_allocator_type(PQS_mem_pool))
#endif	
   {}
   
   inline static PQS_SET<T,C> Intersection(PQS_SET<T,C> &A,PQS_SET<T,C> &B)
   {
      PQS_SET<T,C> result;
      set_intersection(A._set.begin(),A._set.end(),B._set.begin(),B._set.end(),
		       inserter(result._set,result._set.begin()));
      return result;
   }
   
   inline static PQS_SET<T,C> Union(PQS_SET<T,C> &A,PQS_SET<T,C> &B)
   {
      PQS_SET<T,C> result;
      set_union(A._set.begin(),A._set.end(),B._set.begin(),B._set.end(),
		inserter(result._set,result._set.begin()));
      return result;
   }
   
   inline static PQS_SET<T,C> Diff(PQS_SET<T,C> &A,PQS_SET<T,C> &B)
   {
      PQS_SET<T,C> result;
      set_difference(A._set.begin(),A._set.end(),B._set.begin(),B._set.end(),
		     inserter(result._set,result._set.begin()));
      return result;
   }
   
   inline static BOOL Is_Empty(PQS_SET<T,C> &A) {
      return (A._set.empty());
   }

   inline BOOL Is_Subset(PQS_SET<T,C> &B) {
      return includes(_set.begin(),_set.end(),B._set.begin(),B._set.end());
   }

   inline BOOL Is_Subset(const T &B) {
      return (_set.count(B) != 0);
   }

   inline void Insert(const T &tn) {
      _set.insert(tn);
   }

   inline void Clear() {
      _set.erase(_set.begin(),_set.end());
   }

   inline void Clear(const T &tn) {
      _set.erase(tn);
   }

   inline INT32 Size(void) const {
      return _set.size();
   }

   inline set_iterator_type begin() const {return _set.begin();}
   inline set_iterator_type end() const {return _set.end();}

   void Print(FILE *f=stdout,BOOL newline=TRUE);
};

#endif

