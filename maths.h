/* Version: MPL 1.1/LGPL 3.0
 *
 * "The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
 * License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is squash.
 *
 * The Initial Developer of the Original Code is
 * Ethatron <niels@paradice-insight.us>. Portions created by The Initial
 * Developer are Copyright (C) 2011-2012 The Initial Developer.
 * All Rights Reserved.
 *
 * Alternatively, the contents of this file may be used under the terms
 * of the GNU Library General Public License Version 3 license (the
 * "LGPL License"), in which case the provisions of LGPL License are
 * applicable instead of those above. If you wish to allow use of your
 * version of this file only under the terms of the LGPL License and not
 * to allow others to use your version of this file under the MPL,
 * indicate your decision by deleting the provisions above and replace
 * them with the notice and other provisions required by the LGPL License.
 * If you do not delete the provisions above, a recipient may use your
 * version of this file under either the MPL or the LGPL License."
 */

#ifndef SQUASH_MATH_H
#define SQUASH_MATH_H

#include "config.h"

#if	defined(SQUASH_USE_AMP) && !defined(SQUASH_USE_AMP_DEBUG)
inline float powf(_In_ float _X, _In_ float _Y) amp_restricted
  {return Concurrency::fast_math::powf(_X, _Y); }
inline float sqrtf(_In_ float _X) amp_restricted
  {return Concurrency::fast_math::sqrtf(_X); }
inline float atan2f(_In_ float _X, _In_ float _Y) amp_restricted
  {return Concurrency::fast_math::atan2f(_X, _Y); }
inline float floorf(_In_ float _X) amp_restricted
  {return Concurrency::fast_math::floorf(_X); }
inline float fabsf(_In_ float _X) amp_restricted
  {return Concurrency::fast_math::fabsf(_X); }
inline float ceilf(_In_ float _X) amp_restricted
  {return Concurrency::fast_math::ceilf(_X); }
inline float sinf(_In_ float _X) amp_restricted
  {return Concurrency::fast_math::sinf(_X); }
inline float cosf(_In_ float _X) amp_restricted
  {return Concurrency::fast_math::cosf(_X); }
#endif

#if	defined(SQUASH_USE_AMP)
class longlong {
public:
    signed long hi;
  unsigned long lo;

  longlong() ccr_restricted {}
  longlong(int v) ccr_restricted {hi=0, lo=v;}

  longlong& operator = (const int &v) ccr_restricted {hi=0,lo=v;return *this;}
  longlong& operator += (const int &v) ccr_restricted {unsigned long cc = lo;lo+=v;hi+=(lo < cc ? 1 : 0);return *this;}
  longlong& operator *= (const int &v) ccr_restricted {/*TODO*/return *this;}
  longlong& operator /= (const int &v) ccr_restricted {/*TODO*/return *this;}
  longlong& operator |= (const longlong &v) ccr_restricted {hi|=v.hi;lo|=v.lo;return *this;}

  friend longlong operator / (const longlong &l, const int &d) ccr_restricted {
    unsigned long q1 = l.hi / d;
    unsigned long r1 = ((l.hi - (q1 * d)) << 16) + (l.lo >> 16);
    unsigned long q2 = r1 / d;
    unsigned long r2 = (( r1  - (q2 * d)) << 16) + (l.lo & 0xFFFF);
    unsigned long q3 = r2 / d;

    longlong res = 0;

    res += q3;
    res += q2 << 16;
    res.hi += q2 >> 16;
    res.hi += q1;

    return res;
  }

  longlong operator - () ccr_restricted {
    longlong res;

    res.lo = ~lo;
    res.hi = ~hi;
    res += 1;

    return res;
  }

  friend longlong operator - (const longlong &a, const longlong &b) ccr_restricted {
    unsigned long cc = a.lo;
    longlong res = a;

    res.lo -= b.lo;
    res.hi -= b.hi;
    res.hi -= (res.lo < cc ? 1 : 0);

    return res;
  }

  friend longlong operator + (const longlong &a, const int &b) ccr_restricted {
    unsigned long cc = a.lo;
    longlong res = a;

    res.lo += b;
    res.hi += (res.lo < cc ? 1 : 0);

    return res;
  }

  friend longlong operator + (const longlong &a, const longlong &b) ccr_restricted {
    unsigned long cc = a.lo;
    longlong res = a;

    res.lo += b.lo;
    res.hi += b.hi;
    res.hi += (res.lo < cc ? 1 : 0);

    return res;
  }

  friend ULONG operator >> (const longlong &a, const int &s) ccr_restricted {
    longlong res;

    res.hi  = a.hi >> s;
    res.lo  = a.hi << (32 - s);
    res.lo |= a.lo >> s;

    return res.lo;
  }

  friend bool operator > (const longlong &a, const longlong &b) ccr_restricted {
    if (a.hi >  b.hi)  return true;
    if (a.hi == b.hi)  return
       (a.lo >  b.lo); return false;
  }

  friend bool operator < (const longlong &a, const longlong &b) ccr_restricted {
    if (a.hi <  b.hi)  return true;
    if (a.hi == b.hi)  return
       (a.lo <  b.lo); return false;
  }

  friend bool operator >= (const longlong &a, const longlong &b) ccr_restricted {
    if (a.hi >  b.hi)  return true;
    if (a.hi == b.hi)  return
       (a.lo >= b.lo); return false;
  }

  friend longlong abs(const longlong &a) ccr_restricted {
    longlong res;

    res.hi = res.hi ^ (res.hi < 0 ? -1 : 0);
    res.lo = res.lo ^ (res.hi < 0 ? -1 : 0);
    res    = res    + (res.hi < 0 ?  1 : 0);

    return res;
  }
};
#else
typedef	long long	longlong;
#endif

#if	!defined(SQUASH_USE_SSE) || (SQUASH_USE_SSE < 2)
/* ------------------------------------------------------------------- */
#define flt(f)		((float)(f))
#define qfloor(x)	((long)floorf(x))
#define qtrunc(x)	(floorf(x))
#define qsign(x)	((x) >= 0 ? 1.0f : -1.0f)
#define qhalf(x)	((x) >= 0 ? 0.5f : -0.5f)
#define qabs(x)		((x) >= 0 ? (x) : -(x))
#define qmin(a,b)	min(a,b)
#define qmax(a,b)	max(a,b)
#define qsqr (l)	((l) * (l))
#define qsqrt(l)	(1.0f * sqrtf((float)(l)))
#define rcp(l)		(1.0f / (l))
#define rsqrt(l)	(1.0f / sqrtf((float)(l)))

template<const long range>
static doinline long passreg rint(float n) ccr_restricted {
  /* lattice quantizer */
  return (long)qmax(qmin(qfloor(n + 0.5f), (float)range), 0.0f);

//if (n >= 0.0f)
//  return (long)qmax(floor(n + 0.5f), 255);
//if (n <= 0.0f)
//  return (long)qmin( ceil(n - 0.5f),   0);
}

template<const long range>
static doinline long passreg sint(float n) ccr_restricted {
  /* lattice quantizer */
  return (long)qmax(qmin(qfloor(n + 0.5f), (float)range), 0.0f);

  const float r = (range) >> 1;

  /* dead-zone quantizer */
  if (n > r)
    return (long)(    qmin(qfloor(    n + 0.5f), (float)range    ));
//if (n <= 128.0f)
    return (long)(r - qmin(qfloor(r - n + 0.5f), (float)range - r));
}
#else
/* ------------------------------------------------------------------- */
static doinline long     passreg qabs(long     l            ) { return abs(l   ); }
static doinline longlong passreg qabs(longlong l            ) { return abs(l   ); }

static doinline int      passreg qmin(int      m, int      n) { return min(m, n); }
static doinline int      passreg qmax(int      m, int      n) { return max(m, n); }
static doinline long     passreg qmin(long     m, long     n) { return min(m, n); }
static doinline long     passreg qmax(long     m, long     n) { return max(m, n); }
static doinline longlong passreg qmin(longlong m, longlong n) { return min(m, n); }
static doinline longlong passreg qmax(longlong m, longlong n) { return max(m, n); }

static doinline float passreg flt(long n) {
  __m128 f = _mm_cvt_si2ss(_mm_setzero_ps(), n);

  return f.m128_f32[0];
}

static doinline float passreg qabs(float l) {
  __m128 x = _mm_set_ss(l);
  __m128 a = _mm_castsi128_ps(_mm_cvtsi32_si128(~0x80000000));
  __m128 s = _mm_and_ps(x, a);

  return s.m128_f32[0];
}

static doinline __m128 passreg _qq_qabs_ps(__m128 &l) {
  __m128 a = _mm_castsi128_ps(_mm_set1_epi32(~0x80000000));
  __m128 s = _mm_and_ps(l, a);

  return s;
}

static doinline float passreg qsign(float l) {
  __m128 x = _mm_set_ss(l);
  __m128 a = _mm_castsi128_ps(_mm_cvtsi32_si128(0x80000000));
  __m128 s = _mm_and_ps(x, a);

  return s.m128_f32[0];
}

static doinline float passreg qhalf(float l) {
  __m128 x = _mm_set_ss(l);
  __m128 a = _mm_castsi128_ps(_mm_cvtsi32_si128(0x80000000));
  __m128 b = _mm_set_ss(0.5f);
  __m128 s = _mm_or_ps(_mm_and_ps(x, a), b);

  return s.m128_f32[0];
}

static doinline long passreg qfloor(float l) {
  __m128  x = _mm_set_ss(l);
  __m128i t = _mm_cvttps_epi32(x);
  __m128i a = _mm_cvtsi32_si128(0x80000000);
          a = _mm_and_si128(a, _mm_castps_si128(x));
          a = _mm_srli_epi32(a, 31);
  __m128i s = _mm_sub_epi32(t, a);

  return s.m128i_u32[0];
}

static doinline float passreg qtrunc(float l) {
  __m128 x = _mm_set_ss(l);
  __m128 s = _mm_cvtepi32_ps(_mm_cvttps_epi32(x));

  return s.m128_f32[0];
}

static doinline float passreg qmin(float m, long n) {
  __m128 x = _mm_set_ss(m);
  __m128 y = _mm_cvt_si2ss(_mm_setzero_ps(), n);
  __m128 s = _mm_min_ss(x, y);

  return s.m128_f32[0];
}

static doinline float passreg qmax(float m, long n) {
  __m128 x = _mm_set_ss(m);
  __m128 y = _mm_cvt_si2ss(_mm_setzero_ps(), n);
  __m128 s = _mm_max_ss(x, y);

  return s.m128_f32[0];
}

static doinline float passreg qmin(float m, float n) {
  __m128 x = _mm_set_ss(m);
  __m128 y = _mm_set_ss(n);
  __m128 s = _mm_min_ss(x, y);

  return s.m128_f32[0];
}

static doinline float passreg qmax(float m, float n) {
  __m128 x = _mm_set_ss(m);
  __m128 y = _mm_set_ss(n);
  __m128 s = _mm_max_ss(x, y);

  return s.m128_f32[0];
}

static doinline long  passreg qsqr(long l) {
  return l * l;
}

static doinline float passreg qsqr(float l) {
  return l * l;
}

static doinline float passreg qsqrt(float l) {
  __m128 x = _mm_set_ss(l);
  __m128 s = _mm_sqrt_ss(x);

  return s.m128_f32[0];
}

static doinline float passreg rcp(float l) {
  __m128 x = _mm_set_ss(l);
  __m128 s = _mm_rcp_ss(x);

  // Newton-Raphson step
  __m128 t = _mm_set_ss(1.0f);
  __m128 n;

  n = _mm_mul_ss(s, x);	  // s*x
  t = _mm_sub_ss(t, n);	  // 1.0f - s*x
  t = _mm_mul_ss(t, s);	  // (1.0f - s*x) * s
  s = _mm_add_ss(s, t);	  // s + (1.0f - s*x) * s

  return s.m128_f32[0];
}

/* checkput: we may get away with one iteration here */
static doinline float passreg rcp(int l) {
  __m128 x = _mm_cvt_si2ss(_mm_setzero_ps(), l);
  __m128 s = _mm_rcp_ss(x);

  // Newton-Raphson step
  __m128 t = _mm_set_ss(1.0f);
  __m128 n;

  n = _mm_mul_ss(s, x);	  // s*x
  t = _mm_sub_ss(t, n);	  // 1.0f - s*x
  t = _mm_mul_ss(t, s);	  // (1.0f - s*x) * s
  s = _mm_add_ss(s, t);	  // s + (1.0f - s*x) * s

  return s.m128_f32[0];
}

/* checkput: we may get away with one iteration here */
static doinline __m128 passreg _qq_rcp_ss(int l) {
  __m128 x = _mm_cvt_si2ss(_mm_setzero_ps(), l);
  __m128 s = _mm_rcp_ss(x);

  // Newton-Raphson step
  __m128 t = _mm_set_ss(1.0f);
  __m128 n;

  n = _mm_mul_ss(s, x);	  // s*x
  t = _mm_sub_ss(t, n);	  // 1.0f - s*x
  t = _mm_mul_ss(t, s);	  // (1.0f - s*x) * s
  s = _mm_add_ss(s, t);	  // s + (1.0f - s*x) * s

  return s;
}

/* checkput: we may get away with one iteration here */
static doinline __m128 passreg _qq_rcp_ss(__m128 &l) {
  __m128 s = _mm_rcp_ss(l);

  // Newton-Raphson step
  __m128 t = _mm_set_ss(1.0f);
  __m128 n;

  n = _mm_mul_ss(s, l);	  // s*x
  t = _mm_sub_ss(t, n);	  // 1.0f - s*x
  t = _mm_mul_ss(t, s);	  // (1.0f - s*x) * s
  s = _mm_add_ss(s, t);	  // s + (1.0f - s*x) * s

  return s;
}

/* checkput: we may get away with one iteration here */
static doinline __m128 passreg _qq_rcp_ps(int l) {
  __m128 s = _qq_rcp_ss(l);

  return _mm_shuffle_ps(s, s, _MM_SHUFFLE(0,0,0,0));
}

/* checkput: we may get away with one iteration here */
static doinline __m128 passreg _qq_rcp_ps(__m128 &l) {
  __m128 s = _mm_rcp_ps(l);

  // Newton-Raphson step
  __m128 t = _mm_set_ps1(1.0f);
  __m128 n;

  n = _mm_mul_ps(s, l);	  // s*x
  t = _mm_sub_ps(t, n);	  // 1.0f - s*x
  t = _mm_mul_ps(t, s);	  // (1.0f - s*x) * s
  s = _mm_add_ps(s, t);	  // s + (1.0f - s*x) * s

  return s;
}

static doinline float passreg rsqrt(float l) {
  __m128 x = _mm_set_ss(l);
  __m128 s = _mm_rsqrt_ss(x);

  // Newton-Raphson step
  __m128 t = _mm_set_ss(3.0f);
  __m128 h = _mm_set_ss(0.5f);
  __m128 n;

  n = _mm_mul_ss(s, s);	  // s*s
  n = _mm_mul_ss(n, x);	  // s*s*l
  t = _mm_sub_ss(t, n);	  // 3.0f - s*s*l
  t = _mm_mul_ss(t, h);	  // (3.0f - s*s*l) * 0.5f
  s = _mm_mul_ss(s, t);	  // (3.0f - s*s*l) * 0.5f * s

  return s.m128_f32[0];
}

static doinline __m128 passreg _qq_rsqrt_ps(__m128 &l) {
  __m128 x = l;
  __m128 s = _mm_rsqrt_ps(x);

  // Newton-Raphson step
  __m128 t = _mm_set_ps1(3.0f);
  __m128 h = _mm_set_ps1(0.5f);
  __m128 n;

  n = _mm_mul_ps(s, s);	  // s*s
  n = _mm_mul_ps(n, x);	  // s*s*l
  t = _mm_sub_ps(t, n);	  // 3.0f - s*s*l
  t = _mm_mul_ps(t, h);	  // (3.0f - s*s*l) * 0.5f
  s = _mm_mul_ps(s, t);	  // (3.0f - s*s*l) * 0.5f * s

  return s;
}

/* checkput: we may get away with one iteration here */
static doinline float passreg rsqrt(long l) {
  __m128 x = _mm_cvt_si2ss(_mm_setzero_ps(), l);
  __m128 s = _mm_rsqrt_ss(x);

  // Newton-Raphson step
  __m128 t = _mm_set_ss(3.0f);
  __m128 h = _mm_set_ss(0.5f);
  __m128 n;

  n = _mm_mul_ss(s, s);	  // s*s
  n = _mm_mul_ss(n, x);	  // s*s*l
  t = _mm_sub_ss(t, n);	  // 3.0f - s*s*l
  t = _mm_mul_ss(t, h);	  // (3.0f - s*s*l) * 0.5f
  s = _mm_mul_ss(s, t);	  // (3.0f - s*s*l) * 0.5f * s

  return s.m128_f32[0];
}

/* checkput: we may get away with one iteration here */
static doinline __m128 passreg _qq_rsqrt_ps(__m128i &l) {
  __m128 x = _mm_cvtepi32_ps(l);
  __m128 s = _mm_rsqrt_ps(x);

  // Newton-Raphson step
  __m128 t = _mm_set_ss(3.0f);
  __m128 h = _mm_set_ss(0.5f);
  __m128 n;

  n = _mm_mul_ss(s, s);	  // s*s
  n = _mm_mul_ss(n, x);	  // s*s*l
  t = _mm_sub_ss(t, n);	  // 3.0f - s*s*l
  t = _mm_mul_ss(t, h);	  // (3.0f - s*s*l) * 0.5f
  s = _mm_mul_ss(s, t);	  // (3.0f - s*s*l) * 0.5f * s

  return s;
}

template<const long range>
static doinline long passreg rint(float n) {
  /* lattice quantizer */
  return (long)qmax(qmin(qtrunc(n + 0.5f), (float)range), 0.0f);
}

template<const long range>
static doinline __m128i passreg _qq_rint_ps(__m128 &n) {
  __m128  h = _mm_add_ps(n, _mm_set_ps1(0.5f));
  __m128i i = _mm_cvttps_epi32(h);

  /* epi32 is SSE4 */
  i = _mm_max_epi16(i, _mm_set1_epi32(0));
  i = _mm_min_epi16(i, _mm_set1_epi32(range));

  /* lattice quantizer */
  return i;
}

template<const long range>
static doinline long passreg sint(float n) {
  /* lattice quantizer */
  return (long)qmax(qmin(qtrunc(n + 0.5f), (float)range), 0.0f);
}

template<const long range>
static doinline __m128i passreg _qq_sint_ps(__m128 &n) {
  __m128 h = _mm_add_ps(n, _mm_set_ps1(0.5f));
  __m128i i = _mm_cvttps_epi32(h);

  /* epi32 is SSE4 */
  i = _mm_max_epi16(i, _mm_set1_epi32(0));
  i = _mm_min_epi16(i, _mm_set1_epi32(range));

  /* lattice quantizer */
  return i;
}

#ifdef	USESSE2_POW_VARIANT1
/**
 * See http://www.devmaster.net/forums/showthread.php?p=43580
 */
#define POLY0(x, c0) _mm_set1_ps(c0)
#define POLY1(x, c0, c1) _mm_add_ps(_mm_mul_ps(POLY0(x, c1), x), _mm_set1_ps(c0))
#define POLY2(x, c0, c1, c2) _mm_add_ps(_mm_mul_ps(POLY1(x, c1, c2), x), _mm_set1_ps(c0))
#define POLY3(x, c0, c1, c2, c3) _mm_add_ps(_mm_mul_ps(POLY2(x, c1, c2, c3), x), _mm_set1_ps(c0))
#define POLY4(x, c0, c1, c2, c3, c4) _mm_add_ps(_mm_mul_ps(POLY3(x, c1, c2, c3, c4), x), _mm_set1_ps(c0))
#define POLY5(x, c0, c1, c2, c3, c4, c5) _mm_add_ps(_mm_mul_ps(POLY4(x, c1, c2, c3, c4, c5), x), _mm_set1_ps(c0))

#define EXP_POLY_DEGREE 3
#define LOG_POLY_DEGREE 5

static doinline __m128 passreg _qq_exp2f4_ps(__m128 &x) {
  __m128i ipart;
  __m128 fpart, expipart, expfpart;

  x = _mm_min_ps(x, _mm_set1_ps( 129.00000f));
  x = _mm_max_ps(x, _mm_set1_ps(-126.99999f));

  /* ipart = int(x - 0.5) */
  ipart = _mm_cvtps_epi32(_mm_sub_ps(x, _mm_set1_ps(0.5f)));

  /* fpart = x - ipart */
  fpart = _mm_sub_ps(x, _mm_cvtepi32_ps(ipart));

  /* expipart = (float) (1 << ipart) */
  expipart = _mm_castsi128_ps(_mm_slli_epi32(_mm_add_epi32(ipart, _mm_set1_epi32(127)), 23));

   /* minimax polynomial fit of 2**x, in range [-0.5, 0.5[ */
#if	EXP_POLY_DEGREE == 5
  expfpart = POLY5(fpart, 9.9999994e-1f, 6.9315308e-1f, 2.4015361e-1f, 5.5826318e-2f, 8.9893397e-3f, 1.8775767e-3f);
#elif	EXP_POLY_DEGREE == 4
  expfpart = POLY4(fpart, 1.0000026f   , 6.9300383e-1f, 2.4144275e-1f, 5.2011464e-2f, 1.3534167e-2f);
#elif	EXP_POLY_DEGREE == 3
  expfpart = POLY3(fpart, 9.9992520e-1f, 6.9583356e-1f, 2.2606716e-1f, 7.8024521e-2f);
#elif	EXP_POLY_DEGREE == 2
  expfpart = POLY2(fpart, 1.0017247f   , 6.5763628e-1f, 3.3718944e-1f);
#else
#error
#endif

  return _mm_mul_ps(expipart, expfpart);
}

static doinline __m128 passreg _qq_log2f4_ps(__m128 &x) {
  __m128i expmask = _mm_set1_epi32(0x7f800000);
  __m128i mantmask = _mm_set1_epi32(0x007fffff);
  __m128 one = _mm_set1_ps(1.0f);

  __m128i i = _mm_castps_si128(x);

  /* exp = (float) exponent(x) */
  __m128 exp = _mm_cvtepi32_ps(_mm_sub_epi32(_mm_srli_epi32(_mm_and_si128(i, expmask), 23), _mm_set1_epi32(127)));

  /* mant = (float) mantissa(x) */
  __m128 mant = _mm_or_ps(_mm_castsi128_ps(_mm_and_si128(i, mantmask)), one);

  __m128 logmant;

  /* Minimax polynomial fit of log2(x)/(x - 1), for x in range [1, 2[
   * These coefficients can be generate with
   * http://www.boost.org/doc/libs/1_36_0/libs/math/doc/sf_and_dist/html/math_toolkit/toolkit/internals2/minimax.html
   */
#if LOG_POLY_DEGREE == 6
  logmant = POLY5(mant, 3.11578814719469302614f, -3.32419399085241980044f, 2.59883907202499966007f , -1.23152682416275988241f , 0.318212422185251071475f , -0.0344359067839062357313f);
#elif LOG_POLY_DEGREE == 5
  logmant = POLY4(mant, 2.8882704548164776201f , -2.52074962577807006663f, 1.48116647521213171641f , -0.465725644288844778798f, 0.0596515482674574969533f);
#elif LOG_POLY_DEGREE == 4
  logmant = POLY3(mant, 2.61761038894603480148f, -1.75647175389045657003f, 0.688243882994381274313f, -0.107254423828329604454f);
#elif LOG_POLY_DEGREE == 3
  logmant = POLY2(mant, 2.28330284476918490682f, -1.04913055217340124191f, 0.204446009836232697516f);
#else
#error
#endif

  /* This effectively increases the polynomial degree by one, but ensures that log2(1) == 0*/
  logmant = _mm_mul_ps(logmant, _mm_sub_ps(mant, one));

  return _mm_add_ps(logmant, exp);
}
#else
#define madv(x, y, z) _mm_add_ps(_mm_mul_ps(x, y), z)
#define madd(x, y, c1) _mm_add_ps(_mm_mul_ps(x, y), _mm_set1_ps(c1))
#define madc(x, c0, c1) _mm_add_ps(_mm_mul_ps(x, _mm_set1_ps(c0)), _mm_set1_ps(c1))

/*
 * FUNCTION
 *	__m128 _exp2f4(__m128 x)
 *
 * DESCRIPTION
 *	The _exp2f4 function computes 2 raised to the input vector x.
 *      Computation is performed by observing the 2^(a+b) = 2^a * 2^b.
 *	We decompose x into a and b (above) by letting.
 *	a = ceil(x), b = x - a;
 *
 *	2^a is easilty computed by placing a into the exponent
 *	or a floating point number whose mantissa is all zeros.
 *
 *	2^b is computed using the following polynomial approximation.
 *	(C. Hastings, Jr, 1955).
 *
 *                __7__
 *		  \
 *		   \
 *	2^(-x) =   /     Ci*x^i
 *                /____
 *                 i=1
 *
 *	for x in the range 0.0 to 1.0
 *
 *	C0 =  1.0
 *	C1 = -0.9999999995
 *	C2 =  0.4999999206
 *	C3 = -0.1666653019
 *	C4 =  0.0416573475
 *	C5 = -0.0083013598
 *	C6 =  0.0013298820
 *	C7 = -0.0001413161
 *
 */
static doinline __m128 passreg _qq_exp2f4_ps(__m128 &x) {
  __m128 SM_LN2 = _mm_set1_ps(0.69314718055994530942f);	/* log_e 2 */
  __m128 frac, frac2, frac4;
  __m128 result;
  __m128 hi, lo;

  __m128 exp_int, exp_frac;
  __m128i ix;

  /* for pow([0.0,1.0],[0.0,X])
   * -> log2([0.0,1.0]) * [0.0,X]
   * -> [-INF,-0.0] * [0.0,X]
   *
   * -INF  -> bias = 0x00000000, ix = -0x80000000, underflow = f, overflow = f
   * -5.00 -> bias = 0x00000000, ix = -5         , underflow = t, overflow = f
   * -0.20 -> bias = 0x00000000, ix =  0         , underflow = t, overflow = f
   * -0.00 -> bias = 0x00000000, ix =  0         , underflow = t, overflow = f
   *  0.00 -> bias = 0x3F7FFFFF, ix =  0         , underflow = t, overflow = f
   *
   * maximum "log2(smallest value not 0)" for 1/256 is -8
   * maximum "log2(smallest value not 0)" for 1/65536 is -16
   * maximum "log2(0)" for 0 is -127
   */

  /* Break in the input x into two parts ceil(x), x - ceil(x).
   */
#ifdef	EXP_POSITIVE_X
  __m128 bias;
//__m128i bias;

  bias = _mm_cmpge_ps(x, _mm_set1_ps(0.0f));	    // -0 => 0
  bias = _mm_and_ps(bias, _mm_castsi128_ps(_mm_set1_epi32(0x3F7FFFFF)));
  ix   = _mm_cvtps_epi32(_mm_add_ps(x, bias));

//bias = _mm_srai_epi32(_mm_cvtps_epi32(x), 31);    // -0 => 0
//bias = _mm_andnot_si128(bias, _mm_set1_epi32(0x3F7FFFFF));
//ix   = _mm_cvtps_epi32(_mm_add_ps(x, _mm_castsi128_ps(bias)));

//bias = cast<float>(sse2<int32_t>(x) >> 31);
//bias = cast<float>(andc(sse2<int32_t>(0x3F7FFFFF), cast<int32_t>(bias)));
//ix   = sse2<int32_t>(x + bias);
#else
  ix   = _mm_cvtps_epi32(x);
#endif

  frac = _mm_sub_ps(_mm_cvtepi32_ps(ix), x);  // frac = __m128(ix) - x;
  frac = _mm_mul_ps(frac, SM_LN2);	      // frac *= SM_LN2;

#if   defined(EXP_POSITIVE_X)
  __m128i underflow, overflow;

  underflow =                _mm_cmpgt_epi32(ix, _mm_set1_epi32(-128))    ;
  overflow  = _mm_srli_epi32(_mm_cmpgt_epi32(ix, _mm_set1_epi32( 128)), 1);
//underflow = cast<uint32_t>(ix > -128);
//overflow  = cast<uint32_t>(ix >  128) >> 1;

  exp_int = _mm_castsi128_ps(_mm_and_si128(_mm_slli_epi32(_mm_add_epi32(ix, _mm_set1_epi32(127)), 23), underflow));
//exp_int = cast<float>(cast<uint32_t>((ix + 127) << 23) & underflow);
#elif   defined(EXP_HUGE_X)
  __m128i underflow;

  underflow =                _mm_cmpgt_epi32(ix, _mm_set1_epi32(-128))    ;
//underflow = cast<uint32_t>(ix > -128);

  exp_int = _mm_castsi128_ps(_mm_and_si128(_mm_slli_epi32(_mm_add_epi32(ix, _mm_set1_epi32(127)), 23), underflow));
#else
  __m128i exponent;

  /* log(0) -> -127 * 258 -> -32766 -> -127
   * log(0) -> -127 * 259 ->  32643 -> 32643
   *
   * maximum is 0^258
   */
  exponent = _mm_max_epi16(ix, _mm_set1_epi32(-127));
  exponent = _mm_slli_epi32(exponent, 23);
  exponent = _mm_add_epi32(exponent, _mm_set1_epi32(127 << 23));
  exp_int  = _mm_castsi128_ps(exponent);
#endif

  /* Instruction counts can be reduced if the polynomial was
   * computed entirely from nested (dependent) fma's. However,
   * to reduce the number of pipeline stalls, the polygon is evaluated
   * in two halves (hi amd lo).
   */
  hi = madc(frac, -0.0001413161f, 0.0013298820f);
  lo = madc(frac, -0.1666653019f, 0.4999999206f);

  frac2 = _mm_mul_ps(frac , frac );	  //frac2 = frac  * frac;

  hi = madd(frac, hi, -0.0083013598f);
  lo = madd(frac, lo, -0.9999999995f);

  frac4 = _mm_mul_ps(frac2, frac2);	  //frac4 = frac2 * frac2;

  hi = madd(frac, hi,  0.0416573475f);
  lo = madd(frac, lo,  1.0f);

  exp_frac = madv(frac4, hi, lo);

  result = _mm_mul_ps(exp_frac, exp_int);   //result = exp_frac * exp_int;

#ifdef	EXP_POSITIVE_X
  /* Handle overflow (+INF) */
  result = _mm_or_ps(result, _mm_castsi128_ps(overflow));	    //result |= overflow;
#endif

  return (result);
}

/*
 * FUNCTION
 *	vector float _log2f4(vector float x)
 *
 * DESCRIPTION
 *	The _log2f4 function computes log (base 2) on a vector if inputs
 *      values x. The _log2f4 function is approximated as a polynomial of
 *      order 8 (C. Hastings, Jr, 1955).
 *
 *                   __8__
 *		     \
 *		      \
 *	log2f(1+x) =  /     Ci*x^i
 *                   /____
 *                    i=1
 *
 *	for x in the range 0.0 to 1.0
 *
 *	C1 =  1.4426898816672
 *	C2 = -0.72116591947498
 *	C3 =  0.47868480909345
 *	C4 = -0.34730547155299
 *	C5 =  0.24187369696082
 *	C6 = -0.13753123777116
 *	C7 =  0.052064690894143
 *	C8 = -0.0093104962134977
 *
 *	This function assumes that x is a non-zero positive value.
 *
 */
static doinline __m128 passreg _qq_log2f4_ps(__m128 &x) {
  __m128i exponent, uxponent;
  __m128 result;
  __m128 x2, x4;
  __m128 hi, lo;
  __m128 y;

  /* for pow([0.0,1.0],[0.0,X])
   * -> log2([0.0,1.0])
   * -> [-INF,-0.0]
   *
   * 1.00 -> exponent =    0, y = 0.0
   * 0.25 -> exponent =   -2, y = 0.0
   * 0.01 -> exponent =   -7, y = 0.28
   * 0.00 -> exponent = -127, y = 0.0
   */

  /* Extract the exponent from the input X.
   */
//exponent = _mm_castps_si128(_mm_andnot_ps(_mm_set1_ps(-0.0f), x));
  exponent = _mm_and_si128(_mm_castps_si128(x), _mm_set1_epi32(0x7f800000));
  uxponent = _mm_sub_epi32(exponent, _mm_set1_epi32(127 << 23));
  exponent = _mm_srai_epi32(uxponent, 23);
//exponent  = cast<int32_t>(cast<uint32_t>(andc(x, -0.0f)) >> 23);
//exponent -= 127;

  /* Compute the remainder after removing the exponent.
   */
  y = _mm_castsi128_ps(_mm_sub_epi32(_mm_castps_si128(x), uxponent));
//__m128 y = cast<float>(cast<int32_t>(x) - (exponent << 23));

  /* Calculate the log2 of the remainder using the polynomial
   * approximation.
   */
  y = _mm_sub_ps(y, _mm_set1_ps(1.0f));	  //y -= 1.0f;

  /* Instruction counts can be reduced if the polynomial was
   * computed entirely from nested (dependent) fma's. However,
   * to reduce the number of pipeline stalls, the polygon is evaluated
   * in two halves (hi amd lo).
   */
  hi = madc(y, -0.0093104962134977f,  0.052064690894143f);
  lo = madc(y,  0.47868480909345f  , -0.72116591947498f );

  x2 = _mm_mul_ps(y , y );  //x2 = y  * y;

  hi = madd(y, hi, -0.13753123777116f);
  lo = madd(y, lo,  1.4426898816672f );

  x4 = _mm_mul_ps(x2, x2);  //x4 = x2 * x2;

  hi = madd(y, hi,  0.24187369696082f);
  lo = _mm_mul_ps(y, lo);   //lo = y * lo;

  hi = madd(y, hi, -0.34730547155299f);

  result = madv(x4, hi, lo);

  /* Add the exponent back into the result.
   */
  result = _mm_add_ps(result, _mm_cvtepi32_ps(exponent));

  return (result);
}
#endif

static doinline __m128 passreg _qq_powf4_ps(__m128 &x, __m128 &y) {
  /* for pow([0.0,1.0],[0.0,X])
    * -> log2([0.0,1.0]) * [0.0,X]
    * -> log2(0,00390625) * 2.2	  -> -8
    * -> log2(0,00390625) * 0.45  -> -8
    * -> [-INF,-0.0] * [0.0,X]
    * -> [-127,-0.0] * [0.0,X]
    */
  __m128 l = _qq_log2f4_ps(x);
  __m128 m = _mm_mul_ps(l, y);
  return     _qq_exp2f4_ps(m);
}

static doinline __m128 passreg _qq_powf4_ps(__m128 &x, float n) {
  __m128 y; y = _mm_set_ps1(n);

  return _qq_powf4_ps(x, y);
}

static doinline __m128 passreg _qq_powf4_ps(__m128 &x, long n) {
  __m128 y = _mm_cvt_si2ss(_mm_setzero_ps(), n);

  return _qq_powf4_ps(x, y.m128_f32[0]);
}

static doinline __m128i passreg _qq_unpack_epi32(const ULONG &n) {
  __m128i up = _mm_setzero_si128();
  __m128i pu = _mm_cvtsi32_si128(n);

  /* ARGB -> RGBA */
  pu = _mm_unpacklo_epi8 (pu, up);
  pu = _mm_unpacklo_epi16(pu, up);
  pu = _mm_shuffle_epi32 (pu, _MM_SHUFFLE(2,1,0,3));

  return pu;
}

static doinline __m128 passreg _qq_unpack_ps(const ULONG &n) {
  __m128i pu = _qq_unpack_epi32(n);
  __m128  fu = _mm_cvtepi32_ps (pu);

  return fu;
}

static doinline long passreg _qq_pack_epi32(__m128i &n) {
  __m128i s, b;

  /* RGBA -> ABGR */
  s = _mm_shuffle_epi32(n, _MM_SHUFFLE(0,1,2,3));
  b = _mm_packs_epi32  (s, s);
  b = _mm_packus_epi16 (b, b);	/* epi32 is SSE4 */

  return _mm_cvtsi128_si32(b);
}

static doinline long passreg _qq_pack_ps(__m128 &n) {
  __m128i s;

  s = _qq_rint_ps<0xFF>(n);
  return _qq_pack_epi32(s);
}
#endif

#endif // !TEXTURE_MATH
