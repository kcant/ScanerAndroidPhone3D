// functional_hash.h header -*- C++ -*-

// Copyright (C) 2007, 2008, 2009 Free Software Foundation, Inc.
//
// This file is part of the GNU ISO C++ Library.  This library is free
// software; you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the
// Free Software Foundation; either version 3, or (at your option)
// any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// Under Section 7 of GPL version 3, you are granted additional
// permissions described in the GCC Runtime Library Exception, version
// 3.1, as published by the Free Software Foundation.

// You should have received a copy of the GNU General Public License and
// a copy of the GCC Runtime Library Exception along with this program;
// see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
// <http://www.gnu.org/licenses/>.

/** @file bits/functional_hash.h
 *  This is an internal header file, included by other library headers.
 *  You should not attempt to use it directly.
 */

#ifndef _FUNCTIONAL_HASH_H
#define _FUNCTIONAL_HASH_H 1

#pragma GCC system_header

#ifndef __GXX_EXPERIMENTAL_CXX0X__
# include <c++0x_warning.h>
#endif

#include <string>
#include <system_error>

namespace std
{
  /// Class template hash.
  template<typename _Tp>
    struct hash;

  /// Partial specializations for pointer types.
  template<typename _Tp>
    struct hash<_Tp*> : public std::unary_function<_Tp*, size_t>
    {
      size_t
      operator()(_Tp* __p) const
      { return reinterpret_cast<size_t>(__p); }
    };

  /// Explicit specializations for integer types.
#define _Cxx_hashtable_define_trivial_hash(_Tp) 	       \
  template<>						       \
    struct hash<_Tp> : public std::unary_function<_Tp, size_t> \
    {                                                          \
      size_t                                                   \
      operator()(_Tp __val) const		               \
      { return static_cast<size_t>(__val); }		       \
    };

  _Cxx_hashtable_define_trivial_hash(bool);
  _Cxx_hashtable_define_trivial_hash(char);
  _Cxx_hashtable_define_trivial_hash(signed char);
  _Cxx_hashtable_define_trivial_hash(unsigned char);
  _Cxx_hashtable_define_trivial_hash(wchar_t);
#ifdef _GLIBCXX_USE_C99_STDINT_TR1
  _Cxx_hashtable_define_trivial_hash(char16_t);
  _Cxx_hashtable_define_trivial_hash(char32_t);
#endif
  _Cxx_hashtable_define_trivial_hash(short);
  _Cxx_hashtable_define_trivial_hash(int);
  _Cxx_hashtable_define_trivial_hash(long);
  _Cxx_hashtable_define_trivial_hash(long long);
  _Cxx_hashtable_define_trivial_hash(unsigned short);
  _Cxx_hashtable_define_trivial_hash(unsigned int);
  _Cxx_hashtable_define_trivial_hash(unsigned long);
  _Cxx_hashtable_define_trivial_hash(unsigned long long);

#undef _Cxx_hashtable_define_trivial_hash

  // Fowler / Noll / Vo (FNV) Hash (type FNV-1a)
  // (Used by the next specializations of std::tr1::hash.)

  /// Dummy generic implementation (for sizeof(size_t) != 4, 8).
  template<size_t = sizeof(size_t)>
    struct _Fnv_hash
    {
      static size_t
      hash(const char* __first, size_t __length)
      {
	size_t __result = 0;
	for (; __length > 0; --__length)
	  __result = (__result * 131) + *__first++;
	return __result;
      }
    };

  template<>
    struct _Fnv_hash<4>
    {
      static size_t
      hash(const char* __first, size_t __length)
      {
	size_t __result = static_cast<size_t>(2166136261UL);
	for (; __length > 0; --__length)
	  {
	    __result ^= static_cast<size_t>(*__first++);
	    __result *= static_cast<size_t>(16777619UL);
	  }
	return __result;
      }
    };
  
  template<>
    struct _Fnv_hash<8>
    {
      static size_t
      hash(const char* __first, size_t __length)
      {
	size_t __result =
	  static_cast<size_t>(14695981039346656037ULL);
	for (; __length > 0; --__length)
	  {
	    __result ^= static_cast<size_t>(*__first++);
	    __result *= static_cast<size_t>(1099511628211ULL);
	  }
	return __result;
      }
    };

  /// Explicit specializations for float.
  template<>
    struct hash<float>
    : public std::unary_function<float, size_t>
    {
      size_t
      operator()(float __val) const
      {
	size_t __result = 0;
      
	// 0 and -0 both hash to zero.
	if (__val != 0.0f)
	  __result = _Fnv_hash<>::hash(reinterpret_cast<const char*>(&__val),
				       sizeof(__val));
	return __result;
      }
    };

  /// Explicit specializations for double.
  template<>
    struct hash<double>
    : public std::unary_function<double, size_t>
    {
      size_t
      operator()(double __val) const
      {
	size_t __result = 0;

	// 0 and -0 both hash to zero.
	if (__val != 0.0)
	  __result = _Fnv_hash<>::hash(reinterpret_cast<const char*>(&__val),
				       sizeof(__val));
	return __result;
      }
    };

  /// Explicit specializations for long double.
  template<>
    struct hash<long double>
    : public std::unary_function<long double, size_t>
    {
      size_t
      operator()(long double __val) const
      {
	size_t __result = 0;

	int __exponent;
	__val = __builtin_frexpl(__val, &__exponent);
	__val = __val < 0.0l ? -(__val + 0.5l) : __val;

	const long double __mult =
	  __gnu_cxx::__numeric_traits<size_t>::__max + 1.0l;
	__val *= __mult;

	// Try to use all the bits of the mantissa (really necessary only
	// on 32-bit targets, at least for 80-bit floating point formats).
	const size_t __hibits = (size_t)__val;
	__val = (__val - (long double)__hibits) * __mult;

	const size_t __coeff =
	  __gnu_cxx::__numeric_traits<size_t>::__max / __LDBL_MAX_EXP__;

	__result = __hibits + (size_t)__val + __coeff * __exponent;

	return __result;
      }
    };

  template<>
    struct hash<string>
    : public std::unary_function<string, size_t>
    {
      size_t
      operator()(const string& __s) const
      { return _Fnv_hash<>::hash(__s.data(), __s.length()); }
    };

#ifdef _GLIBCXX_USE_WCHAR_T
  template<>
    struct hash<wstring>
    : public std::unary_function<wstring, size_t>
    {
      size_t
      operator()(const wstring& __s) const
      {
	const char* __p = reinterpret_cast<const char*>(__s.data());
	return _Fnv_hash<>::hash(__p, __s.length() * sizeof(wchar_t));
      }
    };
#endif

#ifdef _GLIBCXX_USE_C99_STDINT_TR1
  template<>
    struct hash<u16string>
    : public std::unary_function<u16string, size_t>
    {
      size_t
      operator()(const u16string& __s) const
      {
	const char* __p = reinterpret_cast<const char*>(__s.data());
	return _Fnv_hash<>::hash(__p, __s.length() * sizeof(char16_t));
      }
    };

  template<>
    struct hash<u32string>
    : public std::unary_function<u32string, size_t>
    {
      size_t
      operator()(const u32string& __s) const
      {
	const char* __p = reinterpret_cast<const char*>(__s.data());
	return _Fnv_hash<>::hash(__p, __s.length() * sizeof(char32_t));
      }
    };
#endif

  template<>
    struct hash<error_code>
    : public std::unary_function<error_code, size_t>
    {
      size_t
      operator()(const error_code& __e) const
      {
	const char* __p = reinterpret_cast<const char*>(&__e);
	return _Fnv_hash<>::hash(__p, sizeof(__e));
      }
    };
}

#endif // _FUNCTIONAL_HASH_H
