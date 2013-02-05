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

#pragma	warning (disable : 4100)

#include "maths.h"

/* maximum dimensionality of the "pixel"-vector or vexel :^P */
#define	DIM	8

/* -AWSOME: --------------------------------------------------------------------------------------
 * http://aras-p.info/texts/CompactNormalStorage.html
 */

/* make normals occupy the full[0,255] color-cube */
#define	NORMALS_CUBESPACE

/* http://www.gamedev.net/topic/539608-mip-mapping-normal-maps/page__whichpage__1%25EF%25BF%25BD */
#define	NORMALS_SCALEBYLEVEL	normalsteepness
#define	ALPHAS_SCALEBYLEVEL	alphaopaqueness

/* different ways of encoding the normals */
#undef	NORMALS_INTEGER
#undef	NORMALS_FLOAT_XYZ
#undef	NORMALS_FLOAT_XYZ_TANGENTSPACE
#undef	NORMALS_FLOAT_XY_TANGENTSPACE
#define	NORMALS_FLOAT_DXDY_TANGENTSPACE	0.5f

#define	NORMALS_SIGNED_BIASf	(1.0f * 128.0f)
#define	NORMALS_SIGNED_SCALEf	(1.0f / 127.0f)
#define	NORMALS_SIGNED_BIASr	(1.0f * 128.0f)
#define	NORMALS_SIGNED_SCALEr	(1.0f * 127.0f)

#define	NORMALS_UNSIGNED_BIASf	(1.0f * 127.5f)
#define	NORMALS_UNSIGNED_SCALEf	(1.0f / 127.5f)
#define	NORMALS_UNSIGNED_BIASr	(1.0f * 127.5f)
#define	NORMALS_UNSIGNED_SCALEr	(1.0f * 127.5f)

/* http://diaryofagraphicsprogrammer.blogspot.com/2009/01/partial-derivative-normal-maps.html
 *
 * The idea is to store the paritial derivate of the normal in two channels of the map like this
 *
 * dx = (-nx/nz);
 * dy = (-ny/nz);
 *
 * Then you can reconstruct the normal like this:
 *
 * nx = -dx;
 * ny = -dy;
 * nz = 1;
 * normalize(n);
 *
 * The advantage is that you do not have to reconstruct Z, so you can skip one instruction in
 * each pixel shader that uses normal maps.
 */
#undef	NORMALS_FLOAT_AZ_TANGENTSPACE

/* http://www.gamedev.net/topic/535230-storing-normals-as-spherical-coordinates/
 *
 * Encode:
 *  return (float2(atan2(nrmNorm.y, nrmNorm.x) / M_PI, nrmNorm.z) + 1.0f) * 0.5f;
 *
 * Decode:
 *  float2 spGPUAngles = spherical.xy * 2.0f - 1.0f;
 *  float2 sincosTheta; sincos(spGPUAngles.x * M_PI, sincosTheta.x, sincosTheta.y);
 *  float2 sincosPhi = float2(sqrt(1.0f - spGPUAngles.y * spGPUAngles.y), spGPUAngles.y);
 *
 * return float3(sincosTheta.y * sincosPhi.x, sincosTheta.x * sincosPhi.x, sincosPhi.y);
 *
 * Storing z instead of acos(z) just saves some ops the decoding, you still need sin(phi) and cos(phi) to reconstruct XY. You just happen to already have cos(acos(z)), and the trig identity: '1 - sin(x)^2 = cos(x)^2' does the rest.
 *
 * Edit:
 *  Didn't answer the question I guess. You are seeing odd results because the conversion back from spherical is missing a step. You are computing sincos(theta) but not sincos(phi). The reason why I say store just normal.z and not acos(normal.z) is because the length of the vector is 1.0f (did I mention this method of encode/decode only works on normalized vectors) and so doing acos(normal.z/1) and then recovering cos(acos(normal.z/1)) is a very silly thing to do. Instead I store normal.z, and then compute sin(phi) by using the law of sines.
 */

#define ACCUMODE_LINEAR		( 0 << 0)	// RGBA/RGBH
#define ACCUMODE_GAMMA		( 1 << 0)	// RGBA/RGBH
#define ACCUMODE_FLAT		( 0 << 0)	// XYZD
#define ACCUMODE_SCALE		( 1 << 0)	// XYZD

#define NORMMODE_LINEAR		( 0 << 0)	// RGBA/RGBH
#define NORMMODE_GAMMA		( 1 << 0)	// RGBA/RGBH

#define TRGTNORM_CUBESPACE	( 1 << 0)	// XYZD
#define TRGTMODE_CODING		(15 << 1)
#define TRGTMODE_CODING_RGB	( 0 << 1)
#define TRGTMODE_CODING_XYZt	( 1 << 1)	// "t" for tangent space
#define TRGTMODE_CODING_XYt	( 2 << 1)
#define TRGTMODE_CODING_DXDYt	( 3 << 1)	// fixed Z
#define TRGTMODE_CODING_DXDYdZt ( 4 << 1)	// 4x4 adaptive Z
#define TRGTMODE_CODING_DXDYDZt ( 5 << 1)	// 1x1 adaptive Z
#define TRGTMODE_CODING_AZt	( 6 << 1)
#define TRGTMODE_CODING_XYZ	( 7 << 1)
#define TRGTMODE_CODING_XY	( 8 << 1)

/* ####################################################################################
 */

template<const int mode>
static doinline void AccuRGBA(long (&bs)[DIM], const ULONG &b,
			      const float colorgamma, const float alphacontrast) ccr_restricted {
  /* separate the channels and build the sum */
#if	defined(SQUASH_USE_SSE) && (SQUASH_USE_SSE >= 2)
  __m128i *s = (__m128i *)bs;
  __m128i pu;

  pu = _qq_unpack_epi32  (b);
  *s = _mm_add_epi32     (*s, pu);
#else
  long vec[4];

  /* ARGB -> RGBA */
  vec[0] = (b >> 24) & 0xFF; /*a*/
  vec[1] = (b >>  0) & 0xFF; /*b*/
  vec[2] = (b >>  8) & 0xFF; /*g*/
  vec[3] = (b >> 16) & 0xFF; /*r*/

  bs[0] += vec[0]; /*a*/
  bs[1] += vec[1]; /*b*/
  bs[2] += vec[2]; /*g*/
  bs[3] += vec[3]; /*r*/
#endif
}

template<const int mode>
static doinline void AccuRGBH(long (&bs)[DIM], const ULONG &b,
			      const float colorgamma) ccr_restricted {
  /* separate the channels and build the sum */
#if	defined(SQUASH_USE_SSE) && (SQUASH_USE_SSE >= 2)
  __m128i *s = (__m128i *)bs;
  __m128i pu;

  pu = _qq_unpack_epi32  (b);
  *s = _mm_add_epi32     (*s, pu);
#else
  /* HRGB -> RGBH */
  bs[0] += (b >> 24) & 0xFF; /*h*/
  bs[1] += (b >>  0) & 0xFF; /*b*/
  bs[2] += (b >>  8) & 0xFF; /*g*/
  bs[3] += (b >> 16) & 0xFF; /*r*/
#endif
}

template<const int mode>
static doinline void AccuRGBM(long (&bs)[DIM], const ULONG &b, int level, int l,
			      const float colorgamma) ccr_restricted {
  /* separate the channels and build the sum */
#if	defined(SQUASH_USE_SSE) && (SQUASH_USE_SSE >= 2)
  __m128i *s = (__m128i *)bs;
  __m128i pu;
  __m128i mk = _mm_set_epi32(0,0,0,-1);
  __m128i mx;

  pu = _qq_unpack_epi32  (b);
  mx = _mm_max_epi16     (pu, *s);
  mx = _mm_sub_epi32     (mx, *s);  // r = x - s
  mx = _mm_and_si128     (mk, mx);
  pu = _mm_andnot_si128  (mk, pu);
  pu = _mm_or_si128      (mx, pu);
  *s = _mm_add_epi32     (*s, pu);  // s = (x - s) + s
#else
  /* MRGB -> RGBM */
  bs[0]  = qmax(bs[0], (long)
	   (b >> 24) & 0xFF); /*m*/
  bs[1] += (b >>  0) & 0xFF ; /*b*/
  bs[2] += (b >>  8) & 0xFF ; /*g*/
  bs[3] += (b >> 16) & 0xFF ; /*r*/
#endif
}

template<const int mode>
static doinline void AccuRGBA(longlong (&bs)[DIM], const ULONG &b,
			      const float colorgamma, const float alphacontrast) ccr_restricted {
  /* separate the channels and build the sum */
  long vec[4];

  /* ARGB -> RGBA */
  vec[0] = (b >> 24) & 0xFF; vec[0] += vec[0] << 8; /*a*/
  vec[1] = (b >>  0) & 0xFF; vec[1] += vec[1] << 8; /*b*/
  vec[2] = (b >>  8) & 0xFF; vec[2] += vec[2] << 8; /*g*/
  vec[3] = (b >> 16) & 0xFF; vec[3] += vec[3] << 8; /*r*/

  bs[0] += vec[0]; /*a*/
  bs[1] += vec[1]; /*b*/
  bs[2] += vec[2]; /*g*/
  bs[3] += vec[3]; /*r*/
}

template<const int mode>
static doinline void AccuRGBH(longlong (&bs)[DIM], const ULONG &b,
			      const float colorgamma) ccr_restricted {
  /* separate the channels and build the sum */
  long vec[4];

  /* HRGB -> RGBH */
  vec[0] = (b >> 24) & 0xFF; vec[0] += vec[0] << 8; /*h*/
  vec[1] = (b >>  0) & 0xFF; vec[1] += vec[1] << 8; /*b*/
  vec[2] = (b >>  8) & 0xFF; vec[2] += vec[2] << 8; /*g*/
  vec[3] = (b >> 16) & 0xFF; vec[3] += vec[3] << 8; /*r*/

  bs[0] += vec[0]; /*h*/
  bs[1] += vec[1]; /*b*/
  bs[2] += vec[2]; /*g*/
  bs[3] += vec[3]; /*r*/
}

template<const int mode>
static doinline void AccuRGBM(longlong (&bs)[DIM], const ULONG &b, int level, int l,
			      const float colorgamma) ccr_restricted {
  /* separate the channels and build the sum */
  long vec[4];

  /* MRGB -> RGBM */
  vec[0] = (b >> 24) & 0xFF; vec[0] += vec[0] << 8; /*m*/
  vec[1] = (b >>  0) & 0xFF; vec[1] += vec[1] << 8; /*b*/
  vec[2] = (b >>  8) & 0xFF; vec[2] += vec[2] << 8; /*g*/
  vec[3] = (b >> 16) & 0xFF; vec[3] += vec[3] << 8; /*r*/

  bs[0]  = qmax(bs[0],
           vec[0]); /*m*/
  bs[1] += vec[1] ; /*b*/
  bs[2] += vec[2] ; /*g*/
  bs[3] += vec[3] ; /*r*/
}

template<const int mode>
static doinline void AccuRGBA(float (&bs)[DIM], const ULONG &b,
			      const float colorgamma, const float alphacontrast) ccr_restricted {
  /* separate the channels and build the sum */
#if	defined(SQUASH_USE_SSE) && (SQUASH_USE_SSE >= 2)
  const float rnrm = 1.0f / 0xFF;
  const float colg = colorgamma;
  const float alpc = alphacontrast;

  __m128 *s = (__m128 *)bs;
  __m128 hf = _mm_set_ss(0.5f);
  __m128 nm = _mm_set_ps(rnrm, rnrm, rnrm, rnrm);
  __m128 ml, pw, fu, sg, st, sh, rv;

  fu = _qq_unpack_ps     (b);
  ml = _mm_mul_ps        (fu, nm);

  /* raise a every level */
  if (mode & ACCUMODE_SCALE) {
    __m128 ex = _mm_set_ps(colg, colg, colg, 1.0f);

    ml = _qq_powf4_ps(ml, ex);
  }
  else {
    __m128 ex = _mm_set_ps(colg, colg, colg, alpc);

    sg = _mm_cmpgt_ss(ml, hf);
    sg = _mm_castsi128_ps(_mm_slli_epi32(_mm_castps_si128(sg), 31));
    st = _mm_xor_ps(_mm_set_ss(2.0f), sg);
    sh = _mm_xor_ps(hf, sg);

    /* sigma function */
//  s * 2.0f * (0.5f - rn[0])
    rv = _mm_sub_ss  (hf, ml);
    ml = _mm_move_ss (ml, rv);
    ml = _mm_mul_ss  (ml, st);

    pw = _qq_powf4_ps(ml, ex);

//  0.5f - (s * 0.5f * rn[0])
    pw = _mm_mul_ss  (pw, sh);
    rv = _mm_sub_ss  (hf, pw);
    ml = _mm_move_ss (ml, rv);
  }

  *s = _mm_add_ps  (*s, ml);
#else
  const float rnrm = 1.0f / 0xFF;
  long vec[4];
  float rn[4];

  /* ARGB -> RGBA */
  vec[0] = (b >> 24) & 0xFF; /*a*/
  vec[1] = (b >>  0) & 0xFF; /*b*/
  vec[2] = (b >>  8) & 0xFF; /*g*/
  vec[3] = (b >> 16) & 0xFF; /*r*/

  rn[0] = (float)vec[0] * rnrm; /*a*/
  rn[1] = (float)vec[1] * rnrm; /*b*/
  rn[2] = (float)vec[2] * rnrm; /*g*/
  rn[3] = (float)vec[3] * rnrm; /*r*/

  /* raise a every level */
  if (mode & ACCUMODE_SCALE) {
    rn[0] =                                           rn[0]                 ; /*a*/
    rn[1] =                   powf(                   rn[1] , colorgamma   ); /*b*/
    rn[2] =                   powf(                   rn[2] , colorgamma   ); /*g*/
    rn[3] =                   powf(                   rn[3] , colorgamma   ); /*r*/
  }
  else {
    float s = 1.0f;
    if (rn[0] > 0.5f)
      s = -s;

    /* sigma function */
    rn[0] = 0.5f - s * 0.5f * powf(s * 2.0f * (0.5f - rn[0]), alphacontrast); /*a*/
//  rn[0] =                   powf(                   rn[0] , alphacontrast); /*a*/
    rn[1] =                   powf(                   rn[1] , colorgamma   ); /*b*/
    rn[2] =                   powf(                   rn[2] , colorgamma   ); /*g*/
    rn[3] =                   powf(                   rn[3] , colorgamma   ); /*r*/
  }

  bs[0] += rn[0]; /*a*/
  bs[1] += rn[1]; /*b*/
  bs[2] += rn[2]; /*g*/
  bs[3] += rn[3]; /*r*/
#endif
}

template<const int mode>
static doinline void AccuRGBH(float (&bs)[DIM], const ULONG &b,
			      const float colorgamma) ccr_restricted {
  /* separate the channels and build the sum */
#if	defined(SQUASH_USE_SSE) && (SQUASH_USE_SSE >= 2)
  const float rnrm = 1.0f / 0xFF;
  const float colg = colorgamma;

  __m128 *s = (__m128 *)bs;
  __m128 nm = _mm_set_ps(rnrm, rnrm, rnrm, rnrm);
  __m128 ex = _mm_set_ps1(colg);
  __m128 pw, fu;

  fu = _qq_unpack_ps     (b);
  pw = _mm_mul_ps        (fu, nm);
  pw = _qq_powf4_ps      (pw, ex);

  pw = _mm_move_ss       (pw, fu);
  *s = _mm_add_ps        (*s, pw);
#else
  const float rnrm = 1.0f / 0xFF;
  long vec[4];

  /* HRGB -> RGBH */
  vec[0] = (b >> 24) & 0xFF; /*h*/
  vec[1] = (b >>  0) & 0xFF; /*b*/
  vec[2] = (b >>  8) & 0xFF; /*g*/
  vec[3] = (b >> 16) & 0xFF; /*r*/

  bs[0] +=             vec[0]                    ; /*h*/
  bs[1] += powf((float)vec[1] * rnrm, colorgamma); /*b*/
  bs[2] += powf((float)vec[2] * rnrm, colorgamma); /*g*/
  bs[3] += powf((float)vec[3] * rnrm, colorgamma); /*r*/
#endif
}

template<const int mode>
static doinline void AccuRGBM(float (&bs)[DIM], const ULONG &b, int level, int l,
			      const float colorgamma) ccr_restricted {
  /* separate the channels and build the sum */
#if	defined(SQUASH_USE_SSE) && (SQUASH_USE_SSE >= 2)
  const float rnrm = 1.0f / 0xFF;
  const float colg = colorgamma;

  __m128 *s = (__m128 *)bs;
  __m128 nm = _mm_set_ps(rnrm, rnrm, rnrm, rnrm);
  __m128 ex = _mm_set_ps1(colg);
  __m128 pw, fu;

  fu = _qq_unpack_ps     (b);
  pw = _mm_mul_ps        (fu, nm);
  pw = _qq_powf4_ps      (pw, ex);

  pw = _mm_add_ps        (pw, *s);
  pw = _mm_move_ss       (pw, fu);
  pw = _mm_max_ss        (pw, *s);

  *s = pw;
#else
  const float rnrm = 1.0f / 0xFF;
  long vec[4];

  /* MRGB -> RGBM */
  vec[0] = (*b >> 24) & 0xFF; /*m*/
  vec[1] = (*b >>  0) & 0xFF; /*b*/
  vec[2] = (*b >>  8) & 0xFF; /*g*/
  vec[3] = (*b >> 16) & 0xFF; /*r*/

  bs[0]  = qmax(bs[0],
                (long )vec[0]                   ); /*m*/
  bs[1] += powf((float)vec[1] * rnrm, colorgamma); /*b*/
  bs[2] += powf((float)vec[2] * rnrm, colorgamma); /*g*/
  bs[3] += powf((float)vec[3] * rnrm, colorgamma); /*r*/
#endif
}

template<const int mode>
static doinline void AccuXYZD(long (&ns)[DIM], const ULONG &n) ccr_restricted {
#if	defined(SQUASH_USE_SSE) && (SQUASH_USE_SSE >= 2)
  __m128i *s = (__m128i *)ns;
  __m128i sb = _mm_set_epi32(0x80, 0x80, 0x80, 0x00);
  __m128i pu;

  pu = _qq_unpack_epi32  (n);
  pu = _mm_sub_epi32     (pu, sb);
  *s = _mm_add_epi32     (*s, pu);
#else
  long vec[4];

  /* DXYZ -> XYZD */
  vec[0] = ((n >> 24) & 0xFF) - 0x00; /*d[ 0,1]*/
  vec[1] = ((n >>  0) & 0xFF) - 0x80; /*z[-1,1]*/
  vec[2] = ((n >>  8) & 0xFF) - 0x80; /*y[-1,1]*/
  vec[3] = ((n >> 16) & 0xFF) - 0x80; /*x[-1,1]*/

  ns[0] += vec[0];
  ns[1] += vec[1];
  ns[2] += vec[2];
  ns[3] += vec[3];
#endif
}

template<const int mode>
static doinline void AccuXYZD(longlong (&ns)[DIM], const ULONG &n) ccr_restricted {
  long vec[4];

  /* DXYZ -> XYZD */
  vec[0] = ((n >> 24) & 0xFF) - 0x00; /*d[ 0,1]*/
  vec[1] = ((n >>  0) & 0xFF) - 0x80; /*z[-1,1]*/
  vec[2] = ((n >>  8) & 0xFF) - 0x80; /*y[-1,1]*/
  vec[3] = ((n >> 16) & 0xFF) - 0x80; /*x[-1,1]*/

  ns[0] += vec[0];
  ns[1] += vec[1];
  ns[2] += vec[2];
  ns[3] += vec[3];
}

template<const int mode>
static doinline void AccuXYZD(float (&nn)[DIM], const ULONG &n) ccr_restricted {
  const float hoff = -NORMALS_UNSIGNED_BIASf;
  const float rnrm =  NORMALS_UNSIGNED_SCALEf;

#if	defined(SQUASH_USE_SSE) && (SQUASH_USE_SSE >= 2)
  __m128 *o = (__m128 *)nn;
  __m128 hf = _mm_set_ps(hoff, hoff, hoff, 0.0f);
  __m128 nm = _mm_set_ps(rnrm, rnrm, rnrm, 1.0f);
  __m128 fu, ml, ln, l1, l2;

  fu = _qq_unpack_ps     (n);
  ml = _mm_add_ps        (fu, hf);
  ml = _mm_mul_ps        (ml, nm);

  /* prevent singularity */
  ln = _mm_mul_ps        (ml, ml);
  l1 = _mm_shuffle_ps    (ln, ln, _MM_SHUFFLE(1,3,2,0));
  l2 = _mm_shuffle_ps    (ln, ln, _MM_SHUFFLE(2,1,3,0));
  ln = _mm_add_ps        (ln, l1);
  ln = _mm_add_ps        (ln, l2);
  if (_mm_comieq_ss(_mm_movehl_ps(ln, ln), _mm_setzero_ps())) {
    ml = _mm_set_ps      (0.0f, 0.0f, 1.0f, 0x01);
    ln = _mm_set_ps      (1.0f, 1.0f, 1.0f, 0x01);
  }
  else
    ln = _qq_rsqrt_ps    (ln);

  ml = _mm_mul_ps        (ml, ln);
  ml = _mm_move_ss       (ml, fu);
  *o = _mm_add_ps        (*o, ml);
#else
  float vec[4], len;

  /* DXYZ -> XYZD */
  vec[0] = (float)((n >> 24) & 0xFF);
  vec[1] = (float)((n >>  0) & 0xFF); vec[1] += hoff; vec[1] *= rnrm;
  vec[2] = (float)((n >>  8) & 0xFF); vec[2] += hoff; vec[2] *= rnrm;
  vec[3] = (float)((n >> 16) & 0xFF); vec[3] += hoff; vec[3] *= rnrm;

  /* prevent singularity */
  len = vec[1] * vec[1] + vec[2] * vec[2] + vec[3] * vec[3];
  if (!len)
    len = vec[1] = 1.0f, vec[2] = vec[3] = 0.0f;
  else
    len = rsqrt(len);

  vec[1] *= len;
  vec[2] *= len;
  vec[3] *= len;

  nn[0] += vec[0];
  nn[1] += vec[1];
  nn[2] += vec[2];
  nn[3] += vec[3];
#endif
}

template<const int mode>
static doinline void AccuXYCD(long (&nn)[DIM], const ULONG &n) ccr_restricted {
  /* doesn't exist, just for fullfilling templated case */;
}

template<const int mode>
static doinline void AccuXYCD(longlong (&nn)[DIM], const ULONG &n) ccr_restricted {
  /* doesn't exist, just for fullfilling templated case */;
}

template<const int mode>
static doinline void AccuXYCD(float (&nn)[DIM], const ULONG &n) ccr_restricted {
  const float hoff = -NORMALS_UNSIGNED_BIASf;
  const float rnrm =  NORMALS_UNSIGNED_SCALEf;
  float vec[5], len;

  /* DXYC -> XYCD */
  vec[0] = (float)((n >> 24) & 0xFF);
  vec[1] = (float)((n >>  0) & 0xFF);
  vec[2] = (float)((n >>  8) & 0xFF); vec[2] += hoff; vec[2] *= rnrm;
  vec[3] = (float)((n >> 16) & 0xFF); vec[3] += hoff; vec[3] *= rnrm;
  vec[4] = qsqrt(1.0f - qmin(1.0f, vec[2] * vec[2] + vec[3] * vec[3]));

  /* prevent singularity */
  len = vec[4] * vec[4] + vec[2] * vec[2] + vec[3] * vec[3];
  if (!len)
    len = vec[4] = 1.0f, vec[2] = vec[3] = 0.0f;
  else
    len = rsqrt(len);

  vec[2] *= len;
  vec[3] *= len;
  vec[4] *= len;

  nn[0] += vec[0];
  nn[1] += vec[1];
  nn[2] += vec[2];
  nn[3] += vec[3];
  nn[4] += vec[4];
}

/* ####################################################################################
 */

template<const int rmode>
static doinline void ReduceRGBA(long (&bs)[DIM], long (&bt)[DIM]) ccr_restricted {
  /* separate the channels and build the sum */
#if	defined(SQUASH_USE_SSE) && (SQUASH_USE_SSE >= 2)
  __m128i *t = (__m128i *)bt;
  __m128i *s = (__m128i *)bs;

  *s = _mm_add_epi32(*s, *t);
#else
  bs[0] += bt[0]; /*a*/
  bs[1] += bt[1]; /*b*/
  bs[2] += bt[2]; /*g*/
  bs[3] += bt[3]; /*r*/
#endif
}

template<const int rmode>
static doinline void ReduceRGBH(long (&bs)[DIM], long (&bt)[DIM]) ccr_restricted {
  /* separate the channels and build the sum */
#if	defined(SQUASH_USE_SSE) && (SQUASH_USE_SSE >= 2)
  __m128i *t = (__m128i *)bt;
  __m128i *s = (__m128i *)bs;

  *s = _mm_add_epi32(*s, *t);
#else
  bs[0] += bt[0]; /*h*/
  bs[1] += bt[1]; /*b*/
  bs[2] += bt[2]; /*g*/
  bs[3] += bt[3]; /*r*/
#endif
}

template<const int rmode>
static doinline void ReduceRGBM(long (&bs)[DIM], long (&bt)[DIM]) ccr_restricted {
  /* separate the channels and build the sum */
#if	defined(SQUASH_USE_SSE) && (SQUASH_USE_SSE >= 2)
  __m128i *t = (__m128i *)bt;
  __m128i *s = (__m128i *)bs;
  __m128i ma, mx;

  ma = _mm_add_epi32(*s, *t);
//mx = _mm_max_epi32(*s, *t);
  mx = _mm_max_epi16(*s, *t);

  ma.m128i_i32[0] = mx.m128i_i32[0];

  *s = ma;
#else
  bs[0]  = qmax(bs[0], bt[0]); /*h*/
  bs[1] +=             bt[1] ; /*b*/
  bs[2] +=             bt[2] ; /*g*/
  bs[3] +=             bt[3] ; /*r*/
#endif
}

template<const int rmode>
static doinline void ReduceRGBA(longlong (&bs)[DIM], longlong (&bt)[DIM]) ccr_restricted {
  bs[0] += bt[0]; /*a*/
  bs[1] += bt[1]; /*b*/
  bs[2] += bt[2]; /*g*/
  bs[3] += bt[3]; /*r*/
}

template<const int rmode>
static doinline void ReduceRGBH(longlong (&bs)[DIM], longlong (&bt)[DIM]) ccr_restricted {
  bs[0] += bt[0]; /*h*/
  bs[1] += bt[1]; /*b*/
  bs[2] += bt[2]; /*g*/
  bs[3] += bt[3]; /*r*/
}

template<const int rmode>
static doinline void ReduceRGBM(longlong (&bs)[DIM], longlong (&bt)[DIM]) ccr_restricted {
  bs[0] += bt[0]; /*m*/
  bs[1] += bt[1]; /*b*/
  bs[2] += bt[2]; /*g*/
  bs[3] += bt[3]; /*r*/
}

template<const int rmode>
static doinline void ReduceRGBA(float (&bs)[DIM], float (&bt)[DIM]) ccr_restricted {
  /* separate the channels and build the sum */
#if	defined(SQUASH_USE_SSE) && (SQUASH_USE_SSE >= 2)
  __m128 *t = (__m128 *)bt;
  __m128 *s = (__m128 *)bs;

  *s = _mm_add_ps(*s, *t);
#else
  bs[0] += bt[0]; /*a*/
  bs[1] += bt[1]; /*b*/
  bs[2] += bt[2]; /*g*/
  bs[3] += bt[3]; /*r*/
#endif
}

template<const int rmode>
static doinline void ReduceRGBH(float (&bs)[DIM], float (&bt)[DIM]) ccr_restricted {
  /* separate the channels and build the sum */
#if	defined(SQUASH_USE_SSE) && (SQUASH_USE_SSE >= 2)
  __m128 *t = (__m128 *)bt;
  __m128 *s = (__m128 *)bs;

  *s = _mm_add_ps(*s, *t);
#else
  bs[0] += bt[0]; /*h*/
  bs[1] += bt[1]; /*b*/
  bs[2] += bt[2]; /*g*/
  bs[3] += bt[3]; /*r*/
#endif
}

template<const int rmode>
static doinline void ReduceRGBM(float (&bs)[DIM], float (&bt)[DIM]) ccr_restricted {
  /* separate the channels and build the sum */
#if	defined(SQUASH_USE_SSE) && (SQUASH_USE_SSE >= 2)
  __m128 *t = (__m128 *)bt;
  __m128 *s = (__m128 *)bs;
  __m128 ma;

  ma = _mm_add_ps (*s, *t);
  ma = _mm_move_ss(ma, *s);
  ma = _mm_max_ss (ma, *t);

  *s = ma;
#else
  bs[0]  = qmax(bs[0], bt[0]); /*h*/
  bs[1] +=             bt[1] ; /*b*/
  bs[2] +=             bt[2] ; /*g*/
  bs[3] +=             bt[3] ; /*r*/
#endif
}

template<const int rmode>
static doinline void ReduceXYZD(long (&ns)[DIM], long (&nt)[DIM]) ccr_restricted {
  /* separate the channels and build the sum */
#if	defined(SQUASH_USE_SSE) && (SQUASH_USE_SSE >= 2)
  __m128i *t = (__m128i *)nt;
  __m128i *s = (__m128i *)ns;

  *s = _mm_add_epi32(*s, *t);
#else
  ns[0] += nt[0]; /*d[ 0,1]*/
  ns[1] += nt[1]; /*z[-1,1]*/
  ns[2] += nt[2]; /*y[-1,1]*/
  ns[3] += nt[3]; /*x[-1,1]*/
#endif
}

template<const int rmode>
static doinline void ReduceXYZD(longlong (&ns)[DIM], longlong (&nt)[DIM]) ccr_restricted {
  /* separate the channels and build the sum */
  ns[0] += nt[0]; /*d[ 0,1]*/
  ns[1] += nt[1]; /*z[-1,1]*/
  ns[2] += nt[2]; /*y[-1,1]*/
  ns[3] += nt[3]; /*x[-1,1]*/
}

template<const int rmode>
static doinline void ReduceXYZD(float (&nn)[DIM], float (&nt)[DIM]) ccr_restricted {
  /* separate the channels and build the sum */
#if	defined(SQUASH_USE_SSE) && (SQUASH_USE_SSE >= 2)
  __m128 *t = (__m128 *)nt;
  __m128 *n = (__m128 *)nn;

  *n = _mm_add_ps(*n, *t);
#else
  nn[0] += nt[0]; /*d[ 0,1]*/
  nn[1] += nt[1]; /*z[-1,1]*/
  nn[2] += nt[2]; /*y[-1,1]*/
  nn[3] += nt[3]; /*x[-1,1]*/
#endif
}

template<const int rmode>
static doinline void ReduceXYCD(long (&nn)[DIM], long (&nt)[DIM]) ccr_restricted {
  abort();
}

template<const int rmode>
static doinline void ReduceXYCD(longlong (&nn)[DIM], longlong (&nt)[DIM]) ccr_restricted {
  abort();
}

template<const int rmode>
static doinline void ReduceXYCD(float (&nn)[DIM], float (&nt)[DIM]) ccr_restricted {
  /* separate the channels and build the sum */
#if	defined(SQUASH_USE_SSE) && (SQUASH_USE_SSE >= 2)
  __m128 *t = (__m128 *)nt;
  __m128 *n = (__m128 *)nn;

  *n = _mm_add_ps(*n, *t); n++; t++;
  *n = _mm_add_ps(*n, *t);
#else
  nn[0] += nt[0]; /*c[ 0,1]*/
  nn[1] += nt[1]; /*d[ 0,1]*/
  nn[2] += nt[2]; /*z[-1,1]*/
  nn[3] += nt[3]; /*y[-1,1]*/
  nn[4] += nt[4]; /*x[-1,1]*/
#endif
}

/* ####################################################################################
 */

template<const int mode>
static doinline void NormRGBA(long (&obs)[DIM], long (&bs)[DIM], int av, int level, int l, const float ALPHAS_SCALEBYLEVEL,
				 const float colorgammainv, const float alphacontrastinv) ccr_restricted {
  /* build average of each channel an join */
  if ((mode & ACCUMODE_SCALE) && (l > 0)) {
    long bs_0_;

    /* raise a every level */
    bs_0_  = bs[0];      /*a*/
    bs_0_ *= 2 + l;
    bs_0_ /= 2 * av;
    bs_0_  = qmin(bs_0_, (long)0xFF);

    obs[0] = bs_0_     ; /*b*/
    obs[1] = bs[1] / av; /*b*/
    obs[2] = bs[2] / av; /*g*/
    obs[3] = bs[3] / av; /*r*/
  }
  else {
    obs[0] = bs[0] / av; /*a*/
    obs[1] = bs[1] / av; /*b*/
    obs[2] = bs[2] / av; /*g*/
    obs[3] = bs[3] / av; /*r*/
  }
}

template<const int mode>
static doinline void NormRGBH(long (&obs)[DIM], long (&bs)[DIM], int av,
				 const float colorgammainv) ccr_restricted {
  /* build average of each channel an join */
  obs[0] = bs[0] / av; /*h*/
  obs[1] = bs[1] / av; /*b*/
  obs[2] = bs[2] / av; /*g*/
  obs[3] = bs[3] / av; /*r*/
}

template<const int mode>
static doinline void NormRGBM(long (&obs)[DIM], long (&bs)[DIM], int av,
				 const float colorgammainv) ccr_restricted {
  /* build average of each channel an join */
  obs[0] = bs[0]     ; /*m*/
  obs[1] = bs[1] / av; /*b*/
  obs[2] = bs[2] / av; /*g*/
  obs[3] = bs[3] / av; /*r*/
}

template<const int mode>
static doinline void NormRGBA(longlong (&obs)[DIM], longlong (&bs)[DIM], int av, int level, int l, const float ALPHAS_SCALEBYLEVEL,
				 const float colorgammainv, const float alphacontrastinv) ccr_restricted {
  /* build average of each channel an join */
  if ((mode & ACCUMODE_SCALE) && (l > 0)) {
    longlong bs_0_;

    /* raise a every level */
    bs_0_  = bs[0];      /*a*/
    bs_0_ *= 2 + l;
    bs_0_ /= 2 * av;
    bs_0_  = qmin(bs_0_, (longlong)0xFFFF);

    obs[0] = bs_0_     ; /*b*/
    obs[1] = bs[1] / av; /*b*/
    obs[2] = bs[2] / av; /*g*/
    obs[3] = bs[3] / av; /*r*/
  }
  else {
    obs[0] = bs[0] / av; /*a*/
    obs[1] = bs[1] / av; /*b*/
    obs[2] = bs[2] / av; /*g*/
    obs[3] = bs[3] / av; /*r*/
  }
}

template<const int mode>
static doinline void NormRGBH(longlong (&obs)[DIM], longlong (&bs)[DIM], int av,
				 const float colorgammainv) ccr_restricted {
  /* build average of each channel an join */
  obs[0] = bs[0] / av; /*h*/
  obs[1] = bs[1] / av; /*b*/
  obs[2] = bs[2] / av; /*g*/
  obs[3] = bs[3] / av; /*r*/
}

template<const int mode>
static doinline void NormRGBM(longlong (&obs)[DIM], longlong (&bs)[DIM], int av,
				 const float colorgammainv) ccr_restricted {
  /* build average of each channel an join */
  obs[0] = bs[0]     ; /*m*/
  obs[1] = bs[1] / av; /*b*/
  obs[2] = bs[2] / av; /*g*/
  obs[3] = bs[3] / av; /*r*/
}

template<const int mode>
static doinline void NormRGBA(float (&obs)[DIM], float (&bs)[DIM], int av, int level, int l, const float ALPHAS_SCALEBYLEVEL,
				 const float colorgammainv, const float alphacontrastinv) ccr_restricted {
  /* build average of each channel an join */
#if	defined(SQUASH_USE_SSE) && (SQUASH_USE_SSE >= 2)
  const float fnrm = 1.0f * 0xFF;
  const float colg = colorgammainv;
  const float alph = alphacontrastinv;

  __m128 rv = _qq_rcp_ps(av);
  __m128 nm = _mm_set_ps(fnrm, fnrm, fnrm, fnrm);
  __m128 ex = _mm_set_ps(colg, colg, colg, alph);

  __m128 hf = _mm_set_ss(0.5f);
  __m128 *o = (__m128 *)obs;
  __m128 *s = (__m128 *) bs;
  __m128 ml, pw, sg, st, sh;

  ml = _mm_mul_ps    (*s, rv);

  if (mode & ACCUMODE_SCALE) {
    __m128 ex = _mm_set_ps(colg, colg, colg, 1.0f);

    if (l > 0) {
      __m128
      on = _mm_set_ss (1.0f);
      rv = _mm_cvt_si2ss(rv, l);

      rv = _mm_mul_ss (rv, _mm_set_ss(ALPHAS_SCALEBYLEVEL));
      rv = _mm_add_ss (rv, on);
      ml = _mm_mul_ss (ml, rv);
      ml = _mm_min_ss (ml, on);
    }

    ml = _qq_powf4_ps(ml, ex);
  }
  else {
    sg = _mm_cmpgt_ss(ml, hf);
    sg = _mm_castsi128_ps(_mm_slli_epi32(_mm_castps_si128(sg), 31));
    st = _mm_xor_ps  (_mm_set_ss(2.0f), sg);
    sh = _mm_xor_ps  (hf, sg);

    /* sigma function */
    //s * 2.0f * (0.5f - rn[0])
    rv = _mm_sub_ss  (hf, ml);
    ml = _mm_move_ss (ml, rv);
    ml = _mm_mul_ss  (ml, st);

    pw = _qq_powf4_ps(ml, ex);

    //0.5f - (s * 0.5f * rn[0])
    pw = _mm_mul_ss  (pw, sh);
    rv = _mm_sub_ss  (hf, pw);
    ml = _mm_move_ss (ml, rv);
  }

  *o = _mm_mul_ps    (ml, nm);
#else
  const float rav = rcp(av);
  float rn[4];

  rn[0] = bs[0] * rav; /*a*/
  rn[1] = bs[1] * rav; /*b*/
  rn[2] = bs[2] * rav; /*g*/
  rn[3] = bs[3] * rav; /*r*/

  if (mode & ACCUMODE_SCALE) {
    if (l > 0) {
      rn[0] *= 1.0f + (0.5f * l);
      rn[0]  = qmin(rn[0], 1.0f);
    }

    rn[0] =                                            rn[0]                     ; /*a*/
    rn[1] =                    powf(                   rn[1] , colorgammainv   ) ; /*b*/
    rn[2] =                    powf(                   rn[2] , colorgammainv   ) ; /*g*/
    rn[3] =                    powf(                   rn[3] , colorgammainv   ) ; /*r*/
  }
  else {
    float s = 1.0f;
    if (rn[0] > 0.5f)
      s = -s;

    /* sigma function */
    rn[0] = (0.5f - s * 0.5f * powf(s * 2.0f * (0.5f - rn[0]), alphacontrastinv)); /*a*/
  //rn[0] =                    powf(                   rn[0] , alphacontrastinv) ; /*a*/
  //rn[0] =                                            rn[0]                     ; /*a*/
    rn[1] =                    powf(                   rn[1] , colorgammainv   ) ; /*b*/
    rn[2] =                    powf(                   rn[2] , colorgammainv   ) ; /*g*/
    rn[3] =                    powf(                   rn[3] , colorgammainv   ) ; /*r*/
  }

  obs[0] = rn[0] * 0xFF; /*a[0,1]*/
  obs[1] = rn[1] * 0xFF; /*b[0,1]*/
  obs[2] = rn[2] * 0xFF; /*g[0,1]*/
  obs[3] = rn[3] * 0xFF; /*r[0,1]*/
#endif
}

template<const int mode>
static doinline void NormRGBH(float (&obs)[DIM], float (&bs)[DIM], int av,
				 const float colorgammainv) ccr_restricted {
  /* build average of each channel an join */
#if	defined(SQUASH_USE_SSE) && (SQUASH_USE_SSE >= 2)
  const float fnrm = 1.0f * 0xFF;
  const float colg = colorgammainv;

  __m128 rv = _qq_rcp_ps(av);
  __m128 nm = _mm_set_ps(fnrm, fnrm, fnrm, fnrm);
  __m128 ex = _mm_set_ps1(colg);
  __m128 *o = (__m128 *)obs;
  __m128 *s = (__m128 *) bs;
  __m128 pw, ml;

  ml = _mm_mul_ps     (*s, rv);
  pw = _qq_powf4_ps   (ml, ex);
  pw = _mm_mul_ps     (pw, nm);
  pw = _mm_move_ss    (pw, ml);

  *o = pw;
#else
  const float rav = rcp(av);

  obs[0] =     (bs[0] * rav               )       ; /*h[0,1]*/
  obs[1] = powf(bs[1] * rav, colorgammainv) * 0xFF; /*b[0,1]*/
  obs[2] = powf(bs[2] * rav, colorgammainv) * 0xFF; /*g[0,1]*/
  obs[3] = powf(bs[3] * rav, colorgammainv) * 0xFF; /*r[0,1]*/
#endif
}

template<const int mode>
static doinline void NormRGBM(float (&obs)[DIM], float (&bs)[DIM], int av,
				 const float colorgammainv) ccr_restricted {
  /* build average of each channel an join */
#if	defined(SQUASH_USE_SSE) && (SQUASH_USE_SSE >= 2)
  const float fnrm = 1.0f * 0xFF;
  const float colg = colorgammainv;

  __m128 rv = _qq_rcp_ps(av);
  __m128 nm = _mm_set_ps(fnrm, fnrm, fnrm, fnrm);
  __m128 ex = _mm_set_ps1(colg);
  __m128 *o = (__m128 *)obs;
  __m128 *s = (__m128 *) bs;
  __m128 pw, ml;

  ml = _mm_mul_ps     (*s, rv);
  pw = _qq_powf4_ps   (ml, ex);
  pw = _mm_mul_ps     (pw, nm);
  pw = _mm_move_ss    (pw, *s);

  *o = pw;
#else
  const float rav = rcp(av);

  obs[0] =      bs[0]                             ; /*m*/
  obs[1] = powf(bs[1] * rav, colorgammainv) * 0xFF; /*b*/
  obs[2] = powf(bs[2] * rav, colorgammainv) * 0xFF; /*g*/
  obs[3] = powf(bs[3] * rav, colorgammainv) * 0xFF; /*r*/
#endif
}

template<const int mode>
static doinline void NormXYZD(long (&ons)[DIM], long (&ns)[DIM], int av, int level, int l, const int NORMALS_SCALEBYLEVEL) ccr_restricted {
  if ((mode & ACCUMODE_SCALE) && (l > 0)) {
    /* lower z (heighten the virtual displacement) every level */
    ons[0] = ns[0] / av ; /*d[ 0,1]*/
    ons[1] = ns[1] * ((level * NORMALS_SCALEBYLEVEL) - l)
                   / ((level * NORMALS_SCALEBYLEVEL) *
                     av); /*z[-1,1]*/
    ons[2] = ns[2] / av ; /*y[-1,1]*/
    ons[3] = ns[3] / av ; /*x[-1,1]*/
  }
  else {
    ons[0] = ns[0] / av ; /*d[ 0,1]*/
    ons[1] = ns[1] / av ; /*z[-1,1]*/
    ons[2] = ns[2] / av ; /*y[-1,1]*/
    ons[3] = ns[3] / av ; /*x[-1,1]*/
  }
}

template<const int mode>
static doinline void NormXYZD(longlong (&ons)[DIM], longlong (&ns)[DIM], int av, int level, int l, const int NORMALS_SCALEBYLEVEL) ccr_restricted {
#if 0
  if ((mode & ACCUMODE_SCALE) && (l > 0)) {
    /* lower z (heighten the virtual displacement) every level */
    ons[0] = ns[0] / av ; /*d[ 0,1]*/
    ons[1] = ns[1] * ((level * NORMALS_SCALEBYLEVEL) - l)
                   / ((level * NORMALS_SCALEBYLEVEL) *
                     av); /*z[-1,1]*/
    ons[2] = ns[2] / av ; /*y[-1,1]*/
    ons[3] = ns[3] / av ; /*x[-1,1]*/
  }
  else
#endif
  {
    ons[0] = ns[0] / av ; /*d[ 0,1]*/
    ons[1] = ns[1] / av ; /*z[-1,1]*/
    ons[2] = ns[2] / av ; /*y[-1,1]*/
    ons[3] = ns[3] / av ; /*x[-1,1]*/
  }
}

template<const int mode>
static doinline void NormXYZD(float (&onn)[DIM], float (&nn)[DIM], int av, int level, int l, const int NORMALS_SCALEBYLEVEL) ccr_restricted {
#if	defined(SQUASH_USE_SSE) && (SQUASH_USE_SSE >= 2)
  __m128 rv = _qq_rcp_ss(av);
  __m128 *o = (__m128 *)onn;
  __m128 *n = (__m128 *) nn;
  __m128 ml = *n, ln, l1, l2;

  if ((mode & ACCUMODE_SCALE) && (l > 0)) {
    const int aa = (level * NORMALS_SCALEBYLEVEL) - l;
    const int ab = (level * NORMALS_SCALEBYLEVEL)    ;

    __m128 on = _mm_set_ps(1.0f, 1.0f, 1.0f, 1.0f);
    __m128 vb = _mm_setzero_ps();

    vb = _mm_cvt_si2ss (vb, ab);
    on = _mm_cvt_si2ss (on, aa);

    vb = _qq_rcp_ss    (vb);
    on = _mm_mul_ss    (on, vb);
    on = _mm_shuffle_ps(on, on, _MM_SHUFFLE(3,2,0,1));

    /* lower z (heighten the virtual displacement) every level */
    ml = _mm_mul_ps    (ml, on);
  }

  /* prevent singularity */
  ln = _mm_mul_ps    (ml, ml);
  l1 = _mm_shuffle_ps(ln, ln, _MM_SHUFFLE(1,3,2,0));
  l2 = _mm_shuffle_ps(ln, ln, _MM_SHUFFLE(2,1,3,0));
  ln = _mm_add_ps    (ln, l1);
  ln = _mm_add_ps    (ln, l2);
  if (_mm_comieq_ss(_mm_movehl_ps(ln, ln), _mm_setzero_ps())) {
    ml = _mm_set_ps  (0.0f, 0.0f, 1.0f, 0x01);
    ln = _mm_set_ps  (1.0f, 1.0f, 1.0f, 0x01);
  }
  else
    ln = _qq_rsqrt_ps(ln);

  ml = _mm_move_ss   (ml, *n);
  ln = _mm_move_ss   (ln, rv);
  *o = _mm_mul_ps    (ml, ln);
#else
  const float rav = rcp(av);
  float len, nn_1_ = nn[1];

  if ((mode & ACCUMODE_SCALE) && (l > 0)) {
    /* lower z (heighten the virtual displacement) every level */
    nn_1_ *= flt((level * NORMALS_SCALEBYLEVEL) - l);
    nn_1_ *= rcp((level * NORMALS_SCALEBYLEVEL)    );
  }

  /* prevent singularity */
  len = nn_1_ * nn_1_ + nn[2] * nn[2] + nn[3] * nn[3];
  if (!len)
    len = nn_1_ = 1.0f, nn[2] = nn[3] = 0.0f;
  else
    len = rsqrt(len);

  onn[0] = nn[0] * rav; /*d[ 0,1]*/
  onn[1] = nn_1_ * len; /*z[-1,1]*/
  onn[2] = nn[2] * len; /*y[-1,1]*/
  onn[3] = nn[3] * len; /*x[-1,1]*/
#endif
}

template<const int mode>
static doinline void NormXYCD(long (&onn)[DIM], long (&nn)[DIM], int av, int level, int l, const int NORMALS_SCALEBYLEVEL) ccr_restricted {
  /* doesn't exist, just for fullfilling templated case */;
}

template<const int mode>
static doinline void NormXYCD(longlong (&onn)[DIM], longlong (&nn)[DIM], int av, int level, int l, const int NORMALS_SCALEBYLEVEL) ccr_restricted {
  /* doesn't exist, just for fullfilling templated case */;
}

template<const int mode>
static doinline void NormXYCD(float (&onn)[DIM], float (&nn)[DIM], int av, int level, int l, const int NORMALS_SCALEBYLEVEL) ccr_restricted {
  float len, nn_4_ = nn[4];

  if ((mode & ACCUMODE_SCALE) && (l > 0)) {
    /* lower z (heighten the virtual displacement) every level */
    nn_4_ *= flt((level * NORMALS_SCALEBYLEVEL) - l);
    nn_4_ *= rcp((level * NORMALS_SCALEBYLEVEL)    );
  }

  /* prevent singularity */
  len = nn_4_ * nn_4_ + nn[2] * nn[2] + nn[3] * nn[3];
  if (!len)
    len = nn_4_ = 1.0f, nn[2] = nn[3] = 0.0f;
  else
    len = rsqrt(len);

#if	defined(SQUASH_USE_SSE) && (SQUASH_USE_SSE >= 2)
  __m128 *o = (__m128 *)onn;
  __m128 *n = (__m128 *) nn;
  __m128 rv = _qq_rcp_ps(av);
  __m128 ln = _mm_set_ss(len);

  rv = _mm_shuffle_ps (rv, ln, _MM_SHUFFLE(0,0,0,0));
  *o = _mm_mul_ps     (*n, rv);
#else
  const float rav = rcp(av);

  onn[0] = nn[0] * rav; /*d[ 0,1]*/
  onn[1] = nn[1] * rav; /*c[-1,1]*/
  onn[2] = nn[2] * len; /*y[-1,1]*/
  onn[3] = nn[3] * len; /*x[-1,1]*/
//onn[4] = nn_4_ * len; /*z[-1,1]*/
#endif
}

/* ####################################################################################
 */

template<const int mode>
static doinline void LookRGBA(long (&bs)[DIM], long (&br)[DIM]) ccr_restricted {
  /* collect magnitudes */
#if	defined(SQUASH_USE_SSE) && (SQUASH_USE_SSE >= 2)
  __m128i *s = (__m128i *)bs;
  __m128i *r = (__m128i *)br;

//*r = _mm_max_epi32(*s, *r);
  *r = _mm_max_epi16(*s, *r);
#else
  br[0] = qmax(bs[0], br[0]); /*a*/
  br[1] = qmax(bs[1], br[1]); /*b*/
  br[2] = qmax(bs[2], br[2]); /*g*/
  br[3] = qmax(bs[3], br[3]); /*r*/
#endif
}

template<const int mode>
static doinline void LookRGBH(long (&bs)[DIM], long (&br)[DIM]) ccr_restricted {
  /* collect magnitudes */
#if	defined(SQUASH_USE_SSE) && (SQUASH_USE_SSE >= 2)
  __m128i *s = (__m128i *)bs;
  __m128i *r = (__m128i *)br;

//*r = _mm_max_epi32(*s, *r);
  *r = _mm_max_epi16(*s, *r);
#else
  br[0] = qmax(bs[0], br[0]); /*h*/
  br[1] = qmax(bs[1], br[1]); /*b*/
  br[2] = qmax(bs[2], br[2]); /*g*/
  br[3] = qmax(bs[3], br[3]); /*r*/
#endif
}

template<const int mode>
static doinline void LookRGBA(longlong (&bs)[DIM], longlong (&br)[DIM]) ccr_restricted {
  /* collect magnitudes */
  br[0] = qmax(bs[0], br[0]); /*a*/
  br[1] = qmax(bs[1], br[1]); /*b*/
  br[2] = qmax(bs[2], br[2]); /*g*/
  br[3] = qmax(bs[3], br[3]); /*r*/
}

template<const int mode>
static doinline void LookRGBH(longlong (&bs)[DIM], longlong (&br)[DIM]) ccr_restricted {
  br[0] = qmax(bs[0], br[0]); /*h*/
  br[1] = qmax(bs[1], br[1]); /*b*/
  br[2] = qmax(bs[2], br[2]); /*g*/
  br[3] = qmax(bs[3], br[3]); /*r*/
}

template<const int mode>
static doinline void LookRGBA(float (&bs)[DIM], float (&br)[DIM]) ccr_restricted {
  /* collect magnitudes */
#if	defined(SQUASH_USE_SSE) && (SQUASH_USE_SSE >= 2)
  __m128 *s = (__m128 *)bs;
  __m128 *r = (__m128 *)br;

  *r = _mm_max_ps(*s, *r);
#else
  br[0] = qmax(bs[0], br[0]); /*a*/
  br[1] = qmax(bs[1], br[1]); /*b*/
  br[2] = qmax(bs[2], br[2]); /*g*/
  br[3] = qmax(bs[3], br[3]); /*r*/
#endif
}

template<const int mode>
static doinline void LookRGBH(float (&bs)[DIM], float (&br)[DIM]) ccr_restricted {
  /* collect magnitudes */
#if	defined(SQUASH_USE_SSE) && (SQUASH_USE_SSE >= 2)
  __m128 *s = (__m128 *)bs;
  __m128 *r = (__m128 *)br;

  *r = _mm_max_ps(*s, *r);
#else
  br[0] = qmax(bs[0], br[0]); /*h*/
  br[1] = qmax(bs[1], br[1]); /*b*/
  br[2] = qmax(bs[2], br[2]); /*g*/
  br[3] = qmax(bs[3], br[3]); /*r*/
#endif
}

template<const int mode>
static doinline void LookXYZD(long (&ns)[DIM], long (&nr)[DIM]) ccr_restricted {
  /* collect magnitudes */
#if	defined(SQUASH_USE_SSE) && (SQUASH_USE_SSE >= 2)
  __m128i *s = (__m128i *)ns;
  __m128i *r = (__m128i *)nr;
  __m128i ab, mk;

  mk = _mm_srai_epi32(*s, 31);
  ab = _mm_add_epi32 (mk, *s);
  ab = _mm_xor_si128 (ab, mk);
  ab.m128i_u32[0] = r->m128i_u32[0];

//*r = _mm_max_epi32 (ab, *r);
  *r = _mm_max_epi16 (ab, *r);
#else
  nr[1] = qmax(qabs(ns[1]), nr[1]); /*z[-1,1]*/
  nr[2] = qmax(qabs(ns[2]), nr[2]); /*y[-1,1]*/
  nr[3] = qmax(qabs(ns[3]), nr[3]); /*x[-1,1]*/
#endif
}

template<const int mode>
static doinline void LookXYZD(longlong (&ns)[DIM], longlong (&nr)[DIM]) ccr_restricted {
  nr[1] = qmax(qabs(ns[1]), nr[1]); /*z[-1,1]*/
  nr[2] = qmax(qabs(ns[2]), nr[2]); /*y[-1,1]*/
  nr[3] = qmax(qabs(ns[3]), nr[3]); /*x[-1,1]*/
}

template<const int mode>
static doinline void LookXYZD(float (&nn)[DIM], float (&nr)[DIM]) ccr_restricted {
  if ((mode & TRGTMODE_CODING) == TRGTMODE_CODING_DXDYdZt) {
#if	defined(SQUASH_USE_SSE) && (SQUASH_USE_SSE >= 2)
    __m128 *n = (__m128 *)nn;
    __m128 *r = (__m128 *)nr;
    __m128 nm = *n, ab, dr, mx;

    mx = _mm_max_ps    (nm, _mm_set_ps(-1.0f, -1.0f, 0.001f, -1.0f));
    ab = _qq_qabs_ps   (nm);
    mx = _qq_rcp_ps    (mx);
    ab = _mm_movehl_ps (ab, ab);
    dr = _mm_shuffle_ps(ab, ab, _MM_SHUFFLE(3,2,0,1));
    ab = _mm_max_ps    (ab, dr);
    ab = _mm_mul_ps    (ab, mx);

    ab = _mm_move_ss   (ab, *r);
    *r = _mm_max_ps    (ab, *r);
#else
    /* calculate maximum partial derivative */
    float rel = (
      qmax(
	qabs(nn[2]),
	qabs(nn[3])
      )
      /
      qmax(
	    (nn[1]),
	    (0.001f)
      )
    );

    nr[1] = qmax(     rel   , nr[1]); /*r[ 0,inf]*/
    nr[2] = qmax(qabs(nn[2]), nr[2]); /*y[-1,1]*/
    nr[3] = qmax(qabs(nn[3]), nr[3]); /*x[-1,1]*/
#endif
  }
  else if (((mode & TRGTMODE_CODING) != TRGTMODE_CODING_XY) &&
           ((mode & TRGTMODE_CODING) != TRGTMODE_CODING_XYZ)) {
    /* collect magnitudes */
#if	defined(SQUASH_USE_SSE) && (SQUASH_USE_SSE >= 2)
    __m128 *n = (__m128 *)nn;
    __m128 *r = (__m128 *)nr;
    __m128 ab;

    ab = _qq_qabs_ps (*n);
    ab = _mm_move_ss(ab, *r);
    *r = _mm_max_ps (ab, *r);
#else
    nr[1] = qmax(qabs(nn[1]), nr[1]); /*z[-1,1]*/
    nr[2] = qmax(qabs(nn[2]), nr[2]); /*y[-1,1]*/
    nr[3] = qmax(qabs(nn[3]), nr[3]); /*x[-1,1]*/
#endif
  }
}

template<const int mode>
static doinline void LookXYCD(long (&nn)[DIM], long (&nr)[DIM]) ccr_restricted {
  /* doesn't exist, just for fullfilling templated case */;
}

template<const int mode>
static doinline void LookXYCD(longlong (&nn)[DIM], longlong (&nr)[DIM]) ccr_restricted {
  /* doesn't exist, just for fullfilling templated case */;
}

template<const int mode>
static doinline void LookXYCD(float (&nn)[DIM], float (&nr)[DIM]) ccr_restricted {
  /* collect magnitudes */
#if	defined(SQUASH_USE_SSE) && (SQUASH_USE_SSE >= 2)
  __m128 *n = (__m128 *)nn;
  __m128 *r = (__m128 *)nr;
  __m128 mk = _mm_castsi128_ps(_mm_set1_epi32(~0x80000000));
  __m128 ab;

  ab = _mm_movehl_ps  (mk, *n);
  ab = _mm_and_ps     (ab, mk);
  ab = _mm_unpacklo_ps(ab, *r);
  *r = _mm_max_ps     (ab, *r);
#else
  nr[2] = qmax(qabs(nn[2]), nr[2]); /*y[-1,1]*/
  nr[3] = qmax(qabs(nn[3]), nr[3]); /*x[-1,1]*/
#endif
}

/* ####################################################################################
 */

template<const int mode>
static doinline void CodeRGBA(long (&bs)[DIM], long (&br)[DIM]) ccr_restricted {
}

template<const int mode>
static doinline void CodeRGBH(long (&bs)[DIM], long (&br)[DIM]) ccr_restricted {
}

template<const int mode>
static doinline void CodeRGBA(longlong (&bs)[DIM], longlong (&br)[DIM]) ccr_restricted {
}

template<const int mode>
static doinline void CodeRGBH(longlong (&bs)[DIM], longlong (&br)[DIM]) ccr_restricted {
}

template<const int mode>
static doinline void CodeRGBA(float (&bs)[DIM], float (&br)[DIM]) ccr_restricted {
}

template<const int mode>
static doinline void CodeRGBH(float (&bs)[DIM], float (&br)[DIM]) ccr_restricted {
}

template<const int mode, const int zprec>
static doinline void CodeXYZD(long (&ns)[DIM], long (&nr)[DIM]) ccr_restricted {
}

template<const int mode, const int zprec>
static doinline void CodeXYZD(longlong (&ns)[DIM], longlong (&nr)[DIM]) ccr_restricted {
}

template<const int mode, const int zprec>
static doinline void CodeXYZD(float (&nn)[DIM], float (&nr)[DIM]) ccr_restricted {
#if	defined(SQUASH_USE_SSE) && (SQUASH_USE_SSE >= 2)
  __m128 *n = (__m128 *)nn;
  __m128 *r = (__m128 *)nr;
  __m128 on = _mm_set_ps(1.0f, 1.0f, 1.0f, 1.0f);
  __m128 db = _mm_set_ps(
	NORMALS_FLOAT_DXDY_TANGENTSPACE,
	NORMALS_FLOAT_DXDY_TANGENTSPACE,
	NORMALS_FLOAT_DXDY_TANGENTSPACE,
	NORMALS_FLOAT_DXDY_TANGENTSPACE);		// [0.5f,1.0f]
  __m128 dr;
  __m128 nm;

  /* ################################################################# */
  if (((mode & TRGTMODE_CODING) == TRGTMODE_CODING_DXDYt) ||
      ((mode & TRGTMODE_CODING) == TRGTMODE_CODING_DXDYdZt) ||
      ((mode & TRGTMODE_CODING) == TRGTMODE_CODING_DXDYDZt)) {
    __m128 up, dx, mx;

    /* no negative side allowed, and no singularity */
    nm = _mm_max_ps(*n, _mm_set_ps(-1.0f, -1.0f, 0.001f, -1.0f));

    /**/ if ((mode & TRGTMODE_CODING) == TRGTMODE_CODING_DXDYdZt) {
      dx = _qq_rcp_ps    (*r);				// [...,1.0f]
    }
    else if ((mode & TRGTMODE_CODING) == TRGTMODE_CODING_DXDYDZt) {
      dx = _qq_qabs_ps   (nm);
      mx = _qq_rcp_ps    (nm);
      dx = _mm_movehl_ps (dx, dx);
      dr = _mm_shuffle_ps(dx, dx, _MM_SHUFFLE(3,2,0,1));
      dx = _mm_max_ps    (dx, dr);
      dx = _mm_mul_ps    (dx, mx);
      dx = _qq_rcp_ps    (dx);				// [...,1.0f]
    }
    else if ((mode & TRGTMODE_CODING) == TRGTMODE_CODING_DXDYt) {
      dx =                on ;				// [...,1.0f]
    }

    /* clamp derivative to [min-occur, 1.0] */
    db = _mm_max_ps(db, dx);
    db = _mm_min_ps(db, on);

    /* [0,1.0] -> int([0,zprec]) -> [0,1.0] */
    db = _mm_mul_ps(db, _mm_set_ps1(1.0000f * ((1 << zprec) - 1)));
    db = _mm_add_ps(db, _mm_set_ps(0.0f, 0.0f, 0.5f, 0.0f));
    db = _mm_cvtepi32_ps(_mm_cvttps_epi32(db));
    db = _mm_mul_ps(db, _mm_set_ps1(1.0000f / ((1 << zprec) - 1)));

    // derivb / vec[1], derivb * rcp(vec[1])
    dr = _mm_mul_ps(db, _qq_rcp_ps(nm));
    dr =                _mm_shuffle_ps(dr, dr, _MM_SHUFFLE(1,1,1,1)) ;

    // derivb / vec[1] * qmax(qabs(vec[2]), qabs(vec[3]))
    up = _qq_qabs_ps(nm);
    up = _mm_movehl_ps(up, up);
    up = _mm_max_ps(up, _mm_shuffle_ps(up, up, _MM_SHUFFLE(0,1,0,1)));
    mx = up;
    up = _mm_mul_ps(up, dr);

    // up > upl
    if (_mm_comigt_ss(up, on)) {
      up = _qq_rcp_ps  (up);
      nm = _mm_mul_ps  (nm, _mm_movehl_ps(up, on));

      // derivb / vec[1] * qmax(qabs(vec[2]), qabs(vec[3]))
      up = _mm_mul_ps  (up, mx);
      up = _mm_mul_ps  (up, dr);

      // up > upl
      if (_mm_comigt_ss(up, on)) {
	up = _qq_rcp_ps  (up);
	nm = _mm_mul_ps  (nm, _mm_movehl_ps(up, on));
      }
    }
  }
  else if (((mode & TRGTMODE_CODING) != TRGTMODE_CODING_XY) &&
           ((mode & TRGTMODE_CODING) != TRGTMODE_CODING_XYZ)) {
    /* no negative side allowed */
    nm = _mm_max_ps(*n, _mm_set_ps(-1.0f, -1.0f, 0.0f, -1.0f));
  }
  else {
    nm = *n;
  }

  /* ################################################################# */
  if (((mode & TRGTMODE_CODING) == TRGTMODE_CODING_XYZ) ||
      ((mode & TRGTMODE_CODING) == TRGTMODE_CODING_XYZt)) {
    __m128 ln, mn, mx;

//  if (mode & TRGTNORM_CUBESPACE)
//    len = rcp(qmax(vec[1], qmax(vec[2], vec[3])));
//  else
//    len = rsqrt(vec[1] * vec[1] + vec[2] * vec[2] + vec[3] * vec[3]);

    if (mode & TRGTNORM_CUBESPACE) {
      ln = _mm_shuffle_ps(nm, nm, _MM_SHUFFLE(1,3,2,0));
      mx = _mm_shuffle_ps(nm, nm, _MM_SHUFFLE(2,1,3,0));
      ln = _mm_max_ps    (ln, nm);
      ln = _mm_max_ps    (ln, mx);
      ln = _qq_rcp_ps    (ln);
    }
    else {
      mn = _mm_mul_ps    (nm, nm);
      ln = _mm_shuffle_ps(mn, mn, _MM_SHUFFLE(1,3,2,0));
      mx = _mm_shuffle_ps(mn, mn, _MM_SHUFFLE(2,1,3,0));
      ln = _mm_add_ps    (ln, mn);
      ln = _mm_add_ps    (ln, mx);
      ln = _qq_rsqrt_ps  (ln);
    }

    ln = _mm_move_ss(ln, on);
    nm = _mm_mul_ps (nm, ln);
  }
  /* ################################################################# */
  else if (((mode & TRGTMODE_CODING) == TRGTMODE_CODING_XY) ||
           ((mode & TRGTMODE_CODING) == TRGTMODE_CODING_XYt)) {
    __m128 ln, mn, mx, my, mz, ll;

//  len = rsqrt(vec[1] * vec[1] + vec[2] * vec[2] + vec[3] * vec[3]);
//  if (mode & TRGTNORM_CUBESPACE) {
//    float lnn = rsqrt(vec[2] * vec[2] + vec[3] * vec[3]);
//    float factor = (2.0f - qmax(vec[2] * lnn, vec[3] * lnn)) * len;

    mn = _mm_mul_ps    (nm, nm);
    mx = _mm_shuffle_ps(mn, mn, _MM_SHUFFLE(1,3,2,0));
    my = _mm_shuffle_ps(mn, mn, _MM_SHUFFLE(2,1,3,0));
    mz = _mm_max_ps    (my, mx);
    ll = _mm_add_ps    (my, mx);
    ln = _mm_add_ps    (ll, mn);
    ln = _qq_rsqrt_ps  (ln);

    if (mode & TRGTNORM_CUBESPACE) {
      ll = _qq_rsqrt_ps  (ll);
      mz = _mm_mul_ps    (mz, ll);
      mz = _mm_sub_ps    (_mm_set_ps(2.0f, 2.0f, 2.0f, 2.0f), mz);
      mz = _mm_shuffle_ps(mz, mz, _MM_SHUFFLE(1,1,1,1));
      mz = _mm_movehl_ps (mz, on);
      ln = _mm_mul_ps    (ln, mz);
    }

    ln = _mm_mul_ps   (ln, nm);
    ln = _mm_movehl_ps(ln, on);
    nm = _mm_move_ss  (ln, nm);
  }
  /* ################################################################# */
  else if (((mode & TRGTMODE_CODING) == TRGTMODE_CODING_DXDYt) ||
	   ((mode & TRGTMODE_CODING) == TRGTMODE_CODING_DXDYdZt) ||
	   ((mode & TRGTMODE_CODING) == TRGTMODE_CODING_DXDYDZt)) {
    __m128 ln, mn, mx, my, mz, ll;

    ln = _mm_move_ss(dr, on);

    if (mode & TRGTNORM_CUBESPACE) {
      mn = _mm_mul_ps    (nm, nm);
      mx = _mm_shuffle_ps(mn, mn, _MM_SHUFFLE(1,3,2,0));
      my = _mm_shuffle_ps(mn, mn, _MM_SHUFFLE(2,1,3,0));
      ll = _mm_add_ps    (my, mx);

      ll = _qq_rsqrt_ps  (ll);
      mz = _mm_mul_ps    (mz, ll);
      mz = _mm_sub_ps    (_mm_set_ps(2.0f, 2.0f, 2.0f, 2.0f), mz);
      mz = _mm_shuffle_ps(mz, mz, _MM_SHUFFLE(1,1,1,1));
      mz = _mm_movehl_ps (mz, on);
      ln = _mm_mul_ps    (ln, mz);
    }

    nm = _mm_mul_ps(nm, ln);
  }
  /* ################################################################# */
  else if ((mode & TRGTMODE_CODING) == TRGTMODE_CODING_AZt) {
    float ang;

//  ang = atan2f(nm.m128_f32[2], nm.m128_f32[3]) / (float)       M_PI ;
    ang = atan2f(nm.m128_f32[2], nm.m128_f32[3]) * (float)(1.0 / M_PI);

    nm.m128_f32[2] = ang;
    nm.m128_f32[3] = 1.0f;
  }

  *n = nm;
#else
  a16 float vec[8]; float len;
  float derivb = NORMALS_FLOAT_DXDY_TANGENTSPACE;		// [0.5f,1.0f]

  vec[0] = nn[0];
  vec[1] = nn[1];
  vec[2] = nn[2];
  vec[3] = nn[3];

  /* ################################################################# */
  if (((mode & TRGTMODE_CODING) == TRGTMODE_CODING_DXDYt) ||
      ((mode & TRGTMODE_CODING) == TRGTMODE_CODING_DXDYdZt) ||
      ((mode & TRGTMODE_CODING) == TRGTMODE_CODING_DXDYDZt)) {
    /* no negative side allowed, and no singularity */
    vec[1] = qmax(vec[1], 0.001f);

    float derivl = 1.0f;
    float derivx;

    /**/ if ((mode & TRGTMODE_CODING) == TRGTMODE_CODING_DXDYdZt) {
      /* get the minimum divider we have to support
       * based on the vectors in the 4x4 block
       *
       * this will select a constant z-multiplier over
       * the full 4x4 block with that we calculate the
       * partial derivative:
       *
       *  x / z * multiplier;
       *  y / z * multiplier;
       *  z / z * multiplier;
       */
      derivx = rcp(nr[1]);				// [...,1.0f]
    }
    else if ((mode & TRGTMODE_CODING) == TRGTMODE_CODING_DXDYDZt) {
      /* get the minimum divider we have to support
       * based on the vectors in the 1x1 block
       */
      derivx = rcp(
	qmax(
	  qabs(nn[2]),
	  qabs(nn[3])
	)
	/
	qmax(
	  (nn[1]),
	  (0.001f)
	)
      );
    }
    else if ((mode & TRGTMODE_CODING) == TRGTMODE_CODING_DXDYt) {
      /* no z, just z==1 implicit
       */
      derivx =      1.0f ;				// [...,1.0f]
    }

    /* clamp derivative to [min-occur, 1.0] */
    if (derivb < derivx)
      derivb = derivx;
    if (derivb > derivl)
      derivb = derivl;

    /* anticipate the quantization of the derivative
     * and directly provide on the destination lattice
     * of values
     * this gives a "huge" quality jump for DXT!
     *
     * [0,1.0] -> int([0,zprec]) -> [0,1.0]
     */
    {
      const float fprec = 1.0000f * ((1 << zprec) - 1);
      const float iprec = 1.0001f / ((1 << zprec) - 1);

      derivb = qtrunc(derivb * fprec + 0.5f) * iprec;
    }

#if 0
    vec[1] =
      qmax(
	qabs(vec[1]),
      qmax(
	qabs(vec[2]) * derivb,
	qabs(vec[3]) * derivb
      ));
#else
    float up =
    derivb *
    (
      qmax(
	qabs(vec[2]),
	qabs(vec[3])
      )
      /      vec[1]
    );

    /* if Z is below our threshold we have to
      * normalize to make Z our desired value
      *
      * vector * (0.5 / Z)
      */
    float upl = 1.0f;
    if (up > upl) {
      up = rcp(up);

      vec[2] *= up;
      vec[3] *= up;

      /* if the resulting vector overflows the range
	* of the directional components (that is when
	* Z was very low or negative)
	*/
      up =
      derivb *
      (
	qmax(
	  qabs(vec[2]),
	  qabs(vec[3])
	)
	/      vec[1]
      );

      /* this rather complex seemingly identical double-check
	* allows us to create vectors outside the unit-circle
	* sqrt(X²+Y²) at Z which more exactly match the original
	* vector which was below the smallest permitted Z.
	*
	* At the most extreme corner with values [1.0,1.0,0.5]
	* we still can produce vectors [0.66,0.66,0.33] which are
	* below 0.5.
	*/
      if (up > upl) {
	up = rcp(up);

	vec[2] *= up;
	vec[3] *= up;
      }
    }
#endif
  }
  else if (((mode & TRGTMODE_CODING) != TRGTMODE_CODING_XY) &&
           ((mode & TRGTMODE_CODING) != TRGTMODE_CODING_XYZ)) {
    /* no negative side allowed */
    vec[1] = qmax(vec[1], 0.0f);
  }

  /* ################################################################# */
  /**/ if (((mode & TRGTMODE_CODING) == TRGTMODE_CODING_XYZ) ||
           ((mode & TRGTMODE_CODING) == TRGTMODE_CODING_XYZt)) {
    if (mode & TRGTNORM_CUBESPACE)
      len = rcp(qmax(vec[1], qmax(vec[2], vec[3])));
    else
      len = rsqrt(vec[1] * vec[1] + vec[2] * vec[2] + vec[3] * vec[3]);

    vec[1] *= len;
    vec[2] *= len;
    vec[3] *= len;
  }
  /* ################################################################# */
  else if (((mode & TRGTMODE_CODING) == TRGTMODE_CODING_XY) ||
           ((mode & TRGTMODE_CODING) == TRGTMODE_CODING_XYt)) {
    len = rsqrt(vec[1] * vec[1] + vec[2] * vec[2] + vec[3] * vec[3]);

    if (mode & TRGTNORM_CUBESPACE) {
      float lnn = rsqrt(vec[2] * vec[2] + vec[3] * vec[3]);
      float factor = (2.0f - qmax(vec[2], vec[3]) * lnn) * len;

      vec[1] *= len;
      vec[2] *= factor;
      vec[3] *= factor;
    }
    else {
      vec[1] *= len;
      vec[2] *= len;
      vec[3] *= len;
    }

#ifndef SQUASH_USE_AMP
    assert(fabsf(vec[1] - 1.0f) < 0.001f);
#endif
  }
  /* ################################################################# */
  else if (((mode & TRGTMODE_CODING) == TRGTMODE_CODING_DXDYt) ||
	   ((mode & TRGTMODE_CODING) == TRGTMODE_CODING_DXDYdZt) ||
	   ((mode & TRGTMODE_CODING) == TRGTMODE_CODING_DXDYDZt)) {
    /* this format is a bit special */
    len = vec[1] / derivb;
    len = rcp(len);
    /* == equals == */
    len = derivb /     vec[1] ;
//  len = derivb * rcp(vec[1]);

    if (mode & TRGTNORM_CUBESPACE) {
      float lnn = rsqrt(vec[2] * vec[2] + vec[3] * vec[3]);
      float factor = (2.0f - qmax(vec[2], vec[3]) * lnn) * len;

      vec[1] *= len;
      vec[2] *= factor;
      vec[3] *= factor;

#if 0
      if (1) {
	float chk[4], cln, fct;

	chk[2] = vec[2];
	chk[3] = vec[3];

	cln = sqrt(chk[2] * chk[2] + chk[3] * chk[3]);

	fct = 2.0f - max(chk[2] / cln, chk[3] / cln);

	chk[1]  = 1.0f * derivb;
	chk[2] /= fct;
	chk[3] /= fct;

	cln = sqrt(chk[1] * chk[1] + chk[2] * chk[2] + chk[3] * chk[3]);

	chk[1] /= cln;
	chk[2] /= cln;
	chk[3] /= cln;

	cln = 1;
      }
#endif
    }
    else {
      vec[1] *= len;
      vec[2] *= len;
      vec[3] *= len;
    }

#ifndef SQUASH_USE_AMP
    assert(fabsf(vec[1] - derivb) < 0.001f);
#endif
  }
  /* ################################################################# */
  else if ((mode & TRGTMODE_CODING) == TRGTMODE_CODING_AZt) {
    float ang;

    len = qsqrt(vec[1] * vec[1] + vec[2] * vec[2] + vec[3] * vec[3]);
//  ang = atan2f(vec[2], vec[3]) / (float)       M_PI ; vec[2] = ang;
    ang = atan2f(vec[2], vec[3]) * (float)(1.0 / M_PI); vec[2] = ang;

    vec[3] = 1.0f;
  }

  nn[0] = vec[0];
  nn[1] = vec[1];
  nn[2] = vec[2];
  nn[3] = vec[3];
#endif
}

template<const int mode, const int zprec>
static doinline void CodeXYCD(long (&nn)[DIM], long (&nr)[DIM]) ccr_restricted {
  /* doesn't exist, just for fullfilling templated case */;
}

template<const int mode, const int zprec>
static doinline void CodeXYCD(longlong (&nn)[DIM], longlong (&nr)[DIM]) ccr_restricted {
  /* doesn't exist, just for fullfilling templated case */;
}

template<const int mode, const int zprec>
static doinline void CodeXYCD(float (&nn)[DIM], float (&nr)[DIM]) ccr_restricted {
  a16 float vec[8]; float len;

  vec[0] = nn[0];
  vec[1] = nn[1];
  vec[2] = nn[2];
  vec[3] = nn[3];
  vec[4] = qsqrt(1.0f  - qmin(1.0f, vec[2] * vec[2] + vec[3] * vec[3]));

  len = rsqrt(vec[4] * vec[4] + vec[2] * vec[2] + vec[3] * vec[3]);

#if	defined(SQUASH_USE_SSE) && (SQUASH_USE_SSE >= 2)
  __m128 *v = (__m128 *)vec;
  __m128 l1 = _mm_set_ps(len , len , 1.0f, 1.0f);
//__m128 l2 = _mm_set_ps(0.0f, 0.0f, len , len );

  *(v + 0) = _mm_mul_ps(*(v + 0), l1);
//*(v + 1) = _mm_mul_ps(*(v + 1), l2);
#else
  vec[2] *= len;
  vec[3] *= len;
//vec[4] *= len;
#endif

  nn[0] = vec[0];
  nn[1] = vec[1];
  nn[2] = vec[2];
  nn[3] = vec[3];
}

/* ####################################################################################
 */

template<const int mode>
static doinline void RangeRGBA(int (&bo)[DIM], long (&bi)[DIM]) ccr_restricted {
  bo[0] = bi[0]; /*a*/
  bo[1] = bi[1]; /*b*/
  bo[2] = bi[2]; /*g*/
  bo[3] = bi[3]; /*r*/
}

template<const int mode>
static doinline void RangeRGBH(int (&bo)[DIM], long (&bi)[DIM]) ccr_restricted {
  bo[0] = bi[0]; /*h*/
  bo[1] = bi[1]; /*b*/
  bo[2] = bi[2]; /*g*/
  bo[3] = bi[3]; /*r*/
}

template<const int mode>
static doinline void RangeRGBA(int (&bo)[DIM], longlong (&bi)[DIM]) ccr_restricted {
  abort();
}

template<const int mode>
static doinline void RangeRGBH(int (&bo)[DIM], longlong (&bi)[DIM]) ccr_restricted {
  abort();
}

template<const int mode>
static doinline void RangeRGBA(int (&bo)[DIM], float (&bi)[DIM]) ccr_restricted {
#if	defined(SQUASH_USE_SSE) && (SQUASH_USE_SSE >= 2)
  __m128  *i = (__m128  *)bi;
  __m128i *o = (__m128i *)bo;

  *o = _qq_rint_ps<0xFF>(*i);
#else
  bo[0] = rint<0xFF>(bi[0]); /*a*/
  bo[1] = rint<0xFF>(bi[1]); /*b*/
  bo[2] = rint<0xFF>(bi[2]); /*g*/
  bo[3] = rint<0xFF>(bi[3]); /*r*/
#endif
}

template<const int mode>
static doinline void RangeRGBH(int (&bo)[DIM], float (&bi)[DIM]) ccr_restricted {
#if	defined(SQUASH_USE_SSE) && (SQUASH_USE_SSE >= 2)
  __m128  *i = (__m128  *)bi;
  __m128i *o = (__m128i *)bo;

  *o = _qq_rint_ps<0xFF>(*i);
#else
  bo[0] = rint<0xFF>(bi[0]); /*h*/
  bo[1] = rint<0xFF>(bi[1]); /*b*/
  bo[2] = rint<0xFF>(bi[2]); /*g*/
  bo[3] = rint<0xFF>(bi[3]); /*r*/
#endif
}

template<const int mode>
static doinline void RangeXYZD(int (&no)[DIM], long (&ni)[DIM]) ccr_restricted {
  /* build average of each channel an join */
#if	defined(SQUASH_USE_SSE) && (SQUASH_USE_SSE >= 2)
  __m128i *i = (__m128i *)ni;
  __m128i *o = (__m128i *)no;
  __m128i ad = _mm_set_epi32(0x80, 0x80, 0x80, 0x00);
  __m128i sm;

  sm = _mm_add_epi32(*i, ad);
  *o = _qq_rint_ps<0xFF>(sm);
#else
  no[0] = ni[0] + 0x00; /*d*/
  no[1] = ni[1] + 0x80; /*z*/
  no[2] = ni[2] + 0x80; /*y*/
  no[3] = ni[3] + 0x80; /*x*/
#endif
}

template<const int mode>
static doinline void RangeXYZD(int (&no)[DIM], longlong (&ni)[DIM]) ccr_restricted {
  abort();
}

template<const int mode>
static doinline void RangeXYZD(int (&no)[DIM], float (&ni)[DIM]) ccr_restricted {
  const float hoff = NORMALS_UNSIGNED_BIASr;
  const float fnrm = NORMALS_UNSIGNED_SCALEr;

#if	defined(SQUASH_USE_SSE) && (SQUASH_USE_SSE >= 2)
  ULONG n = 0;
  a16 float vec[4];

  /* ################################################################# */
  /**/ if (((mode & TRGTMODE_CODING) == TRGTMODE_CODING_XYZ) ||
           ((mode & TRGTMODE_CODING) == TRGTMODE_CODING_XYZt) ||
           ((mode & TRGTMODE_CODING) == TRGTMODE_CODING_XY) ||
           ((mode & TRGTMODE_CODING) == TRGTMODE_CODING_XYt)) {
    *((__m128  *)vec) =
      _mm_add_ps(
      _mm_mul_ps(*ni, _mm_set_ps(fnrm, fnrm, fnrm, 1.0f)),
                      _mm_set_ps(hoff, hoff, hoff, 0.0f));
  }
  /* ################################################################# */
  else if (((mode & TRGTMODE_CODING) == TRGTMODE_CODING_DXDYt) ||
	   ((mode & TRGTMODE_CODING) == TRGTMODE_CODING_DXDYdZt) ||
	   ((mode & TRGTMODE_CODING) == TRGTMODE_CODING_DXDYDZt)) {
    *((__m128  *)vec) =
      _mm_add_ps(
      _mm_mul_ps(*ni, _mm_set_ps(fnrm, fnrm, fnrm, 1.0f)),
                      _mm_set_ps(hoff, hoff, hoff, 0.0f));
  }
  /* ################################################################# */
  else if ((mode & TRGTMODE_CODING) == TRGTMODE_CODING_AZt) {
    *((__m128  *)vec) =
      _mm_add_ps(
      _mm_mul_ps(*ni, _mm_set_ps(0xFF, fnrm, 1.0f, 1.0f)),
                      _mm_set_ps(0x00, hoff, 0.0f, 0.0f));
  }
#else
  ULONG n = 0;
  a16 float vec[4];

  vec[0] = ni[0];
  vec[1] = ni[1];
  vec[2] = ni[2];
  vec[3] = ni[3];

  /* ################################################################# */
  /**/ if (((mode & TRGTMODE_CODING) == TRGTMODE_CODING_XYZ) ||
	   ((mode & TRGTMODE_CODING) == TRGTMODE_CODING_XYZt) ||
	   ((mode & TRGTMODE_CODING) == TRGTMODE_CODING_XY) ||
	   ((mode & TRGTMODE_CODING) == TRGTMODE_CODING_XYt)) {
    vec[1] *= fnrm; vec[1] += hoff;
    vec[2] *= fnrm; vec[2] += hoff;
    vec[3] *= fnrm; vec[3] += hoff;
  }
  /* ################################################################# */
  else if (((mode & TRGTMODE_CODING) == TRGTMODE_CODING_DXDYt) ||
	   ((mode & TRGTMODE_CODING) == TRGTMODE_CODING_DXDYdZt) ||
	   ((mode & TRGTMODE_CODING) == TRGTMODE_CODING_DXDYDZt)) {
    vec[1] *= fnrm; vec[1] += hoff;
    vec[2] *= fnrm; vec[2] += hoff;
    vec[3] *= fnrm; vec[3] += hoff;
  }
  /* ################################################################# */
  else if ((mode & TRGTMODE_CODING) == TRGTMODE_CODING_AZt) {
    vec[1] *= 0xFF; vec[1] += 0x00;
    vec[2] *= fnrm; vec[2] += hoff;
  }
#endif

  /* simple quantization (we can do better though!) */
  if (((mode & TRGTMODE_CODING) != TRGTMODE_CODING_XYZ) &&
      ((mode & TRGTMODE_CODING) != TRGTMODE_CODING_XYZt) &&
      ((mode & TRGTMODE_CODING) != TRGTMODE_CODING_DXDYt) &&
//    ((mode & TRGTMODE_CODING) != TRGTMODE_CODING_DXDYdZt) &&	// for this Z isn't allowd to move
      ((mode & TRGTMODE_CODING) != TRGTMODE_CODING_DXDYDZt)) {
#if	defined(SQUASH_USE_SSE) && (SQUASH_USE_SSE >= 2)
    __m128i *o = (__m128i *)no;
    __m128  *v = (__m128  *)vec;

    *o = _qq_rint_ps<0xFF>(*v);
#else
    no[0] = rint<0xFF>(vec[0]); /*d[ 0,1]*/
    no[1] = sint<0xFF>(vec[1]); /*z[-1,1]*/
    no[2] = sint<0xFF>(vec[2]); /*y[-1,1]*/
    no[3] = sint<0xFF>(vec[3]); /*x[-1,1]*/
#endif
  }
  else {
    a16 float normal[4];
    a16 long sides[4][2];
    a16 long sidep[4],
	     bestp[4];
    a16 float bbox[DIM];
    a16 float best;
    
    /* the original vector in up-scaled quantized space (half-normal, length 0.5) */
    normal[3] = (vec[3] - hoff);
    normal[2] = (vec[2] - hoff);
    normal[1] = (vec[1] - hoff);

    /* the quantized opposing corner vectors in up-scaled quantized space */
    sides[3][0] = qfloor(normal[3] - 0.5f) * 2 + 1;
    sides[2][0] = qfloor(normal[2] - 0.5f) * 2 + 1;
    sides[1][0] = qfloor(normal[1] - 0.5f) * 2 + 1;
    /* walk only on the odd lattice */
    sides[3][1] = min(sides[3][0] + 2, 0xFFL);
    sides[2][1] = min(sides[2][0] + 2, 0xFFL);
    sides[1][1] = min(sides[1][0] + 2, 0xFFL);

#define	qcpy(a, b)    a[3] = b[3] , a[2] = b[2] , a[1] = b[1]
#define	qlen(z,y,x)   z[3] * z[3] + y[2] * y[2] + x[1] * x[1]

    /* "normal" is a normalized vector (though in up-scaled space)
     * and regardless the length doesn't need to be rescaled as it's
     * length gets canceled out (being in every dot-product as a constant)
     */
#define	qdot(a, b, z, y, x)	(		\
	(a[3] * (sidep[3] = b[3][z])) + 	\
	(a[2] * (sidep[2] = b[2][y])) + 	\
	(a[1] * (sidep[1] = b[1][x]))		\
  ) * rsqrt(b[3][z] * b[3][z] + b[2][y] * b[2][y] + b[1][x] * b[1][x])

    /* calculate the angles between each of the eight corners of the cube on
     * the lattice which are around the end-position of the original normal-vector
     * (the point on the unit-sphere)
     *
     * dot -> cos(angle) -> cos(0) => 1 -> select largest dot
     */
    bbox[0] = qdot(normal, sides, 0, 0, 0);                     best = bbox[0], qcpy(bestp, sidep);
    bbox[1] = qdot(normal, sides, 0, 0, 1); if (best < bbox[1]) best = bbox[1], qcpy(bestp, sidep);
    bbox[2] = qdot(normal, sides, 0, 1, 0); if (best < bbox[2]) best = bbox[2], qcpy(bestp, sidep);
    bbox[3] = qdot(normal, sides, 0, 1, 1); if (best < bbox[3]) best = bbox[3], qcpy(bestp, sidep);
    
    // for this Z isn't allowd to move
    if ((mode & TRGTMODE_CODING) != TRGTMODE_CODING_DXDYt) {

    bbox[4] = qdot(normal, sides, 1, 0, 0); if (best < bbox[4]) best = bbox[4], qcpy(bestp, sidep);
    bbox[5] = qdot(normal, sides, 1, 0, 1); if (best < bbox[5]) best = bbox[5], qcpy(bestp, sidep);
    bbox[6] = qdot(normal, sides, 1, 1, 0); if (best < bbox[6]) best = bbox[6], qcpy(bestp, sidep);
    bbox[7] = qdot(normal, sides, 1, 1, 1); if (best < bbox[7]) best = bbox[7], qcpy(bestp, sidep);

    }

#undef	qcpy
#undef	qlen
#undef	qdot

    /* put the 0.5 back (8 bit -> 0x80 etc.) */
    bestp[1] = ((bestp[1] - 1) >> 1) + (0xFFL - (0xFFL >> 1));
    bestp[2] = ((bestp[2] - 1) >> 1) + (0xFFL - (0xFFL >> 1));
    bestp[3] = ((bestp[3] - 1) >> 1) + (0xFFL - (0xFFL >> 1));

    no[0] = rint<0xFF>(vec[0]); /*d[ 0,1]*/
    no[1] = qmax(bestp[1], 0L); /*z[-1,1]*/
    no[2] = qmax(bestp[2], 0L); /*y[-1,1]*/
    no[3] = qmax(bestp[3], 0L); /*x[-1,1]*/
  }
}

template<const int mode>
static doinline void RangeXYCD(int (&no)[DIM], long (&ni)[DIM]) ccr_restricted {
  /* doesn't exist, just for fullfilling templated case */;
}

template<const int mode>
static doinline void RangeXYCD(int (&no)[DIM], longlong (&ni)[DIM]) ccr_restricted {
  /* doesn't exist, just for fullfilling templated case */;
}

template<const int mode>
static doinline void RangeXYCD(int (&no)[DIM], float (&ni)[DIM]) ccr_restricted {
  const float hoff = NORMALS_UNSIGNED_BIASr;
  const float fnrm = NORMALS_UNSIGNED_SCALEr;

  ULONG n = 0;
  a16 float vec[4];

  vec[0] = ni[0];
  vec[1] = ni[1];
  vec[2] = ni[2];
  vec[3] = ni[3];

#if	defined(SQUASH_USE_SSE) && (SQUASH_USE_SSE >= 2)
  __m128i *o = (__m128i *)no;
  __m128  *v = (__m128  *)vec;
  __m128 h = _mm_set_ps(fnrm, fnrm, 0x01, 0x01);
  __m128 o = _mm_set_ps(hoff, hoff, 0x00, 0x00);
  __m128 m;

  m = _mm_add_ps(_mm_mul_ps(*v, h), o);
  *o = _qq_rint_ps<0xFF>(m);
#else
  vec[2] *= fnrm; vec[2] += hoff;
  vec[3] *= fnrm; vec[3] += hoff;

  no[0] = rint<0xFF>(vec[0]); /*d[ 0,1]*/
  no[1] = rint<0xFF>(vec[1]); /*c[-1,1]*/
  no[2] = sint<0xFF>(vec[2]); /*y[-1,1]*/
  no[3] = sint<0xFF>(vec[3]); /*x[-1,1]*/
#endif
}

/* ####################################################################################
 */

template<const int mode>
static doinline ULONG JoinRGBA(long (&bs)[DIM], long (&br)[DIM]) ccr_restricted {
  /* build average of each channel an join */
#if	defined(SQUASH_USE_SSE) && (SQUASH_USE_SSE >= 2)
  __m128i *s = (__m128i *)bs;

  return _qq_pack_epi32(*s);
#else
  ULONG b = 0;

  /* RGBA -> ABGR */
  b += (bs[0] << 24); /*a*/
  b += (bs[1] << 16); /*b*/
  b += (bs[2] <<  8); /*g*/
  b += (bs[3] <<  0); /*r*/

  return b;
#endif
}

template<const int mode>
static doinline ULONG JoinRGBH(long (&bs)[DIM], long (&br)[DIM]) ccr_restricted {
  /* build average of each channel an join */
#if	defined(SQUASH_USE_SSE) && (SQUASH_USE_SSE >= 2)
  __m128i *s = (__m128i *)bs;

  return _qq_pack_epi32(*s);
#else
  ULONG b = 0;

  /* RGBH -> HBGR */
  b += (bs[0] << 24); /*h*/
  b += (bs[1] << 16); /*b*/
  b += (bs[2] <<  8); /*g*/
  b += (bs[3] <<  0); /*r*/

  return b;
#endif
}

template<const int mode>
static doinline ULONG JoinRGBA(longlong (&bs)[DIM], longlong (&br)[DIM]) ccr_restricted {
  abort();
}

template<const int mode>
static doinline ULONG JoinRGBH(longlong (&bs)[DIM], longlong (&br)[DIM]) ccr_restricted {
  abort();
}

template<const int mode>
static doinline ULONG JoinRGBA(float (&bs)[DIM], float (&br)[DIM]) ccr_restricted {
  /* build average of each channel an join */
#if	defined(SQUASH_USE_SSE) && (SQUASH_USE_SSE >= 2)
  __m128 *s = (__m128 *)bs;

  return _qq_pack_ps(*s);
#else
  ULONG b = 0;

  /* RGBA -> ABGR */
  b += (rint<0xFF>(bs[0]) << 24); /*a[ 0,1]*/
  b += (rint<0xFF>(bs[1]) << 16); /*b[ 0,1]*/
  b += (rint<0xFF>(bs[2]) <<  8); /*g[ 0,1]*/
  b += (rint<0xFF>(bs[3]) <<  0); /*r[ 0,1]*/

  return b;
#endif
}

template<const int mode>
static doinline ULONG JoinRGBH(float (&bs)[DIM], float (&br)[DIM]) ccr_restricted {
  /* build average of each channel an join */
#if	defined(SQUASH_USE_SSE) && (SQUASH_USE_SSE >= 2)
  __m128 *s = (__m128 *)bs;

  return _qq_pack_ps(*s);
#else
  ULONG b = 0;

  /* RGBH -> HBGR */
  b += (rint<0xFF>(bs[0]) << 24); /*h[ 0,1]*/
  b += (rint<0xFF>(bs[1]) << 16); /*b[ 0,1]*/
  b += (rint<0xFF>(bs[2]) <<  8); /*g[ 0,1]*/
  b += (rint<0xFF>(bs[3]) <<  0); /*r[ 0,1]*/

  return b;
#endif
}

template<const int mode>
static doinline ULONG JoinXYZD(long (&ns)[DIM], long (&nr)[DIM]) ccr_restricted {
  /* build average of each channel an join */
#if	defined(SQUASH_USE_SSE) && (SQUASH_USE_SSE >= 2)
  __m128i *s = (__m128i *)ns;
  __m128i ad = _mm_set_epi32(0x80, 0x80, 0x80, 0x00);
  __m128i sm;

  sm = _mm_add_epi32(*s, ad);
  return _qq_pack_epi32(sm);
#else
  ULONG n = 0;

  /* XYZD -> DZYX */
  n += (ns[0] + 0x00) << 24; /*d[ 0,1]*/
  n += (ns[1] + 0x80) << 16; /*z[-1,1]*/
  n += (ns[2] + 0x80) <<  8; /*y[-1,1]*/
  n += (ns[3] + 0x80) <<  0; /*x[-1,1]*/

  return n;
#endif
}

template<const int mode>
static doinline ULONG JoinXYZD(longlong (&ns)[DIM], longlong (&nr)[DIM]) ccr_restricted {
  /* build average of each channel an join */
  ULONG n = 0;

  /* XYZD -> DZYX */
  n += (ns[0] + 0x00) << 24; /*d[ 0,1]*/
  n += (ns[1] + 0x80) << 16; /*z[-1,1]*/
  n += (ns[2] + 0x80) <<  8; /*y[-1,1]*/
  n += (ns[3] + 0x80) <<  0; /*x[-1,1]*/

  return n;
}

template<const int mode>
static doinline ULONG JoinXYZD(float (&nn)[DIM], float (&nr)[DIM]) ccr_restricted {
  const float hoff = NORMALS_UNSIGNED_BIASr;
  const float fnrm = NORMALS_UNSIGNED_SCALEr;

#if	defined(SQUASH_USE_SSE) && (SQUASH_USE_SSE >= 2)
  ULONG n = 0;
  a16 float vec[4];

  /* ################################################################# */
  /**/ if (((mode & TRGTMODE_CODING) == TRGTMODE_CODING_XYZ) ||
	   ((mode & TRGTMODE_CODING) == TRGTMODE_CODING_XYZt) ||
	   ((mode & TRGTMODE_CODING) == TRGTMODE_CODING_XY) ||
	   ((mode & TRGTMODE_CODING) == TRGTMODE_CODING_XYt)) {
    *((__m128  *)vec) =
      _mm_add_ps(
      _mm_mul_ps(*((__m128  *)nn), _mm_set_ps(fnrm, fnrm, fnrm, 1.0f)),
                                   _mm_set_ps(hoff, hoff, hoff, 0.0f));
  }
  /* ################################################################# */
  else if (((mode & TRGTMODE_CODING) == TRGTMODE_CODING_DXDYt) ||
	   ((mode & TRGTMODE_CODING) == TRGTMODE_CODING_DXDYdZt) ||
	   ((mode & TRGTMODE_CODING) == TRGTMODE_CODING_DXDYDZt)) {
    *((__m128  *)vec) =
      _mm_add_ps(
      _mm_mul_ps(*((__m128  *)nn), _mm_set_ps(fnrm, fnrm, fnrm, 1.0f)),
                                   _mm_set_ps(hoff, hoff, hoff, 0.0f));
  }
  /* ################################################################# */
  else if ((mode & TRGTMODE_CODING) == TRGTMODE_CODING_AZt) {
    *((__m128  *)vec) =
      _mm_add_ps(
      _mm_mul_ps(*((__m128  *)nn), _mm_set_ps(0xFF, fnrm, 1.0f, 1.0f)),
                                   _mm_set_ps(0x00, hoff, 0.0f, 0.0f));
  }
#else
  ULONG n = 0;
  a16 float vec[4];

  vec[0] = nn[0];
  vec[1] = nn[1];
  vec[2] = nn[2];
  vec[3] = nn[3];

  /* ################################################################# */
  /**/ if (((mode & TRGTMODE_CODING) == TRGTMODE_CODING_XYZ) ||
           ((mode & TRGTMODE_CODING) == TRGTMODE_CODING_XYZt) ||
           ((mode & TRGTMODE_CODING) == TRGTMODE_CODING_XY) ||
           ((mode & TRGTMODE_CODING) == TRGTMODE_CODING_XYt)) {
    vec[1] *= fnrm; vec[1] += hoff;
    vec[2] *= fnrm; vec[2] += hoff;
    vec[3] *= fnrm; vec[3] += hoff;
  }
  /* ################################################################# */
  else if (((mode & TRGTMODE_CODING) == TRGTMODE_CODING_DXDYt) ||
	   ((mode & TRGTMODE_CODING) == TRGTMODE_CODING_DXDYdZt) ||
	   ((mode & TRGTMODE_CODING) == TRGTMODE_CODING_DXDYDZt)) {
    if (mode & TRGTNORM_CUBESPACE) {
      vec[1] *= fnrm; vec[1] += hoff;
      vec[2] *= fnrm; vec[2] += hoff;
      vec[3] *= fnrm; vec[3] += hoff;
    }
    else {
      vec[1] *= fnrm; vec[1] += hoff;
      vec[2] *= fnrm; vec[2] += hoff;
      vec[3] *= fnrm; vec[3] += hoff;
    }
  }
  /* ################################################################# */
  else if ((mode & TRGTMODE_CODING) == TRGTMODE_CODING_AZt) {
    vec[1] *= 0xFF; vec[1] += 0x00;
    vec[2] *= fnrm; vec[2] += hoff;
  }
#endif

  /* simple quantization (we can do better though!) */
  if (((mode & TRGTMODE_CODING) != TRGTMODE_CODING_XYZ) &&
      ((mode & TRGTMODE_CODING) != TRGTMODE_CODING_XYZt) &&
      ((mode & TRGTMODE_CODING) != TRGTMODE_CODING_DXDYt) &&
//    ((mode & TRGTMODE_CODING) != TRGTMODE_CODING_DXDYdZt) &&	// for this Z isn't allowd to move
      ((mode & TRGTMODE_CODING) != TRGTMODE_CODING_DXDYDZt)) {
#if	defined(SQUASH_USE_SSE) && (SQUASH_USE_SSE >= 2)
    __m128 *v = (__m128 *)vec;

    n = _qq_pack_ps(*v);
#else
    /* XYZD -> DZYX */
    n += (rint<0xFF>(vec[0]) << 24); /*d[ 0,1]*/
    n += (sint<0xFF>(vec[1]) << 16); /*z[-1,1]*/
    n += (sint<0xFF>(vec[2]) <<  8); /*y[-1,1]*/
    n += (sint<0xFF>(vec[3]) <<  0); /*x[-1,1]*/
#endif
  }
  else {
    a16 float normal[4];
    a16 long sides[4][2];
    a16 long sidep[4],
	     bestp[4];
    a16 float bbox[DIM];
    a16 float best;
    
    /* the original vector in up-scaled quantized space (half-normal, length 0.5) */
    normal[3] = (vec[3] - hoff);
    normal[2] = (vec[2] - hoff);
    normal[1] = (vec[1] - hoff);

    /* the quantized opposing corner vectors in up-scaled quantized space */
    sides[3][0] = qfloor(normal[3] - 0.5f) * 2 + 1;
    sides[2][0] = qfloor(normal[2] - 0.5f) * 2 + 1;
    sides[1][0] = qfloor(normal[1] - 0.5f) * 2 + 1;
    /* walk only on the odd lattice */
    sides[3][1] = min(sides[3][0] + 2, 0xFFL);
    sides[2][1] = min(sides[2][0] + 2, 0xFFL);
    sides[1][1] = min(sides[1][0] + 2, 0xFFL);

#define	qcpy(a, b)    a[3] = b[3] , a[2] = b[2] , a[1] = b[1]
#define	qlen(z,y,x)   z[3] * z[3] + y[2] * y[2] + x[1] * x[1]

    /* "normal" is a normalized vector (though in up-scaled space)
     * and regardless the length doesn't need to be rescaled as it's
     * length gets canceled out (being in every dot-product as a constant)
     */
#define	qdot(a, b, z, y, x)	(		\
	(a[3] * (sidep[3] = b[3][z])) + 	\
	(a[2] * (sidep[2] = b[2][y])) + 	\
	(a[1] * (sidep[1] = b[1][x]))		\
  ) * rsqrt(b[3][z] * b[3][z] + b[2][y] * b[2][y] + b[1][x] * b[1][x])

    /* calculate the angles between each of the eight corners of the cube on
     * the lattice which are around the end-position of the original normal-vector
     * (the point on the unit-sphere)
     *
     * dot -> cos(angle) -> cos(0) => 1 -> select largest dot
     */
    bbox[0] = qdot(normal, sides, 0, 0, 0);                     best = bbox[0], qcpy(bestp, sidep);
    bbox[1] = qdot(normal, sides, 0, 0, 1); if (best < bbox[1]) best = bbox[1], qcpy(bestp, sidep);
    bbox[2] = qdot(normal, sides, 0, 1, 0); if (best < bbox[2]) best = bbox[2], qcpy(bestp, sidep);
    bbox[3] = qdot(normal, sides, 0, 1, 1); if (best < bbox[3]) best = bbox[3], qcpy(bestp, sidep);

    // for this Z isn't allowd to move
    if ((mode & TRGTMODE_CODING) != TRGTMODE_CODING_DXDYt) {

    bbox[4] = qdot(normal, sides, 1, 0, 0); if (best < bbox[4]) best = bbox[4], qcpy(bestp, sidep);
    bbox[5] = qdot(normal, sides, 1, 0, 1); if (best < bbox[5]) best = bbox[5], qcpy(bestp, sidep);
    bbox[6] = qdot(normal, sides, 1, 1, 0); if (best < bbox[6]) best = bbox[6], qcpy(bestp, sidep);
    bbox[7] = qdot(normal, sides, 1, 1, 1); if (best < bbox[7]) best = bbox[7], qcpy(bestp, sidep);

    }

#undef	qcpy
#undef	qlen
#undef	qdot

    /* put the 0.5 back (8 bit -> 0x80 etc.) */
    bestp[1] = ((bestp[1] - 1) >> 1) + (0xFFL - (0xFFL >> 1));
    bestp[2] = ((bestp[2] - 1) >> 1) + (0xFFL - (0xFFL >> 1));
    bestp[3] = ((bestp[3] - 1) >> 1) + (0xFFL - (0xFFL >> 1));

    /* XYZD -> DZYX */
    n += rint<0xFF>(vec[0]) << 24; /*d[ 0,1]*/
    n += qmax(bestp[1], 0L) << 16; /*z[-1,1]*/
    n += qmax(bestp[2], 0L) <<  8; /*y[-1,1]*/
    n += qmax(bestp[3], 0L) <<  0; /*x[-1,1]*/
  }

  return n;
}

template<const int mode>
static doinline ULONG JoinXYCD(long (&nn)[DIM], long (&nr)[DIM]) ccr_restricted {
  /* doesn't exist, just for fullfilling templated case */; return 0;
}

template<const int mode>
static doinline ULONG JoinXYCD(longlong (&nn)[DIM], longlong (&nr)[DIM]) ccr_restricted {
  /* doesn't exist, just for fullfilling templated case */; return 0;
}

template<const int mode>
static doinline ULONG JoinXYCD(float (&nn)[DIM], float (&nr)[DIM]) ccr_restricted {
  const float hoff = NORMALS_UNSIGNED_BIASr;
  const float fnrm = NORMALS_UNSIGNED_SCALEr;

  ULONG n = 0;
  a16 float vec[4];

  vec[0] = nn[0];
  vec[1] = nn[1];
  vec[2] = nn[2];
  vec[3] = nn[3];

#if	defined(SQUASH_USE_SSE) && (SQUASH_USE_SSE >= 2)
  __m128 *v = (__m128 *)vec;
  __m128 h = _mm_set_ps(fnrm, fnrm, 1.0f, 1.0f);
  __m128 o = _mm_set_ps(hoff, hoff, 0.0f, 0.0f);
  __m128 m;

  m = _mm_add_ps(_mm_mul_ps(*v, h), o);
  n = _qq_pack_ps(m);
#else
  vec[2] *= fnrm; vec[2] += hoff;
  vec[3] *= fnrm; vec[3] += hoff;

  /* XYCD -> DCYX */
  n += (rint<0xFF>(vec[0]) << 24); /*d[ 0,1]*/
  n += (rint<0xFF>(vec[1]) << 16); /*c[-1,1]*/
  n += (sint<0xFF>(vec[2]) <<  8); /*y[-1,1]*/
  n += (sint<0xFF>(vec[3]) <<  0); /*x[-1,1]*/
#endif

  return n;
}

/* ####################################################################################
 */
#pragma	warning ( disable : 4293 )

template<const int mode, const int A, const int R, const int G, const int B>
static doinline ULONG QuntRGBA(long (&bs)[DIM], long (&br)[DIM]) ccr_restricted {
  /* build average of each channel an join */
  ULONG b = 0;

  /* RGBA -> ARGB */
  b <<= A; b += (bs[0] >> (8 - A)); /*a*/
  b <<= R; b += (bs[3] >> (8 - R)); /*r*/
  b <<= G; b += (bs[2] >> (8 - G)); /*g*/
  b <<= B; b += (bs[1] >> (8 - B)); /*b*/

  return b;
}

template<const int mode, const int H, const int R, const int G, const int B>
static doinline ULONG QuntRGBH(long (&bs)[DIM], long (&br)[DIM]) ccr_restricted {
  /* build average of each channel an join */
  ULONG b = 0;

  /* RGBH -> HRGB */
  b <<= H; b += (bs[0] >> (8 - H)); /*h*/
  b <<= R; b += (bs[3] >> (8 - R)); /*r*/
  b <<= G; b += (bs[2] >> (8 - G)); /*g*/
  b <<= B; b += (bs[1] >> (8 - B)); /*b*/

  return b;
}

template<const int mode, const int A, const int R, const int G, const int B>
static doinline ULONG QuntRGBA(longlong (&bs)[DIM], longlong (&br)[DIM]) ccr_restricted {
  /* build average of each channel an join */
  ULONG b = 0;

  /* RGBA -> ARGB */
  b <<= A; b += (ULONG)(bs[0] >> (16 - A)); /*a*/
  b <<= R; b += (ULONG)(bs[3] >> (16 - R)); /*r*/
  b <<= G; b += (ULONG)(bs[2] >> (16 - G)); /*g*/
  b <<= B; b += (ULONG)(bs[1] >> (16 - B)); /*b*/

  return b;
}

template<const int mode, const int H, const int R, const int G, const int B>
static doinline ULONG QuntRGBH(longlong (&bs)[DIM], longlong (&br)[DIM]) ccr_restricted {
  /* build average of each channel an join */
  ULONG b = 0;

  /* RGBH -> HRGB */
  b <<= H; b += (ULONG)(bs[0] >> (16 - H)); /*h*/
  b <<= R; b += (ULONG)(bs[3] >> (16 - R)); /*r*/
  b <<= G; b += (ULONG)(bs[2] >> (16 - G)); /*g*/
  b <<= B; b += (ULONG)(bs[1] >> (16 - B)); /*b*/

  return b;
}

template<const int mode, const int A, const int R, const int G, const int B>
static doinline ULONG QuntRGBA(float (&bs)[DIM], float (&br)[DIM]) ccr_restricted {
  /* build average of each channel an join */
  const float rnrm = 1.0f / 0xFF;
  ULONG b = 0;

  /* RGBA -> ARGB */
  b <<= A; b += (rint<(1 << A) - 1>(bs[0] * ((1 << A) - 1) * rnrm)); /*a[ 0,1]*/
  b <<= R; b += (rint<(1 << R) - 1>(bs[3] * ((1 << R) - 1) * rnrm)); /*r[ 0,1]*/
  b <<= G; b += (rint<(1 << G) - 1>(bs[2] * ((1 << G) - 1) * rnrm)); /*g[ 0,1]*/
  b <<= B; b += (rint<(1 << B) - 1>(bs[1] * ((1 << B) - 1) * rnrm)); /*b[ 0,1]*/

  return b;
}

template<const int mode, const int H, const int R, const int G, const int B>
static doinline ULONG QuntRGBH(float (&bs)[DIM], float (&br)[DIM]) ccr_restricted {
  /* build average of each channel an join */
  const float rnrm = 1.0f / 0xFF;
  ULONG b = 0;

  /* RGBH -> HRGB */
  b <<= H; b += (rint<(1 << H) - 1>(bs[0] * ((1 << H) - 1) * rnrm)); /*h[ 0,1]*/
  b <<= R; b += (rint<(1 << R) - 1>(bs[3] * ((1 << R) - 1) * rnrm)); /*r[ 0,1]*/
  b <<= G; b += (rint<(1 << G) - 1>(bs[2] * ((1 << G) - 1) * rnrm)); /*g[ 0,1]*/
  b <<= B; b += (rint<(1 << B) - 1>(bs[1] * ((1 << B) - 1) * rnrm)); /*b[ 0,1]*/

  return b;
}

template<const int mode, int D, const int X, const int Y, const int Z>
static doinline ULONG QuntXYZD(long (&ns)[DIM], long (&nr)[DIM]) ccr_restricted {
  /* build average of each channel an join */
  ULONG n = 0;

  /* XYZD -> DXYZ */
  n <<= D; n += ((ns[0] + 0x00) >> (8 - D)); /*d*/
  n <<= X; n += ((ns[3] + 0x80) >> (8 - X)); /*x*/
  n <<= Y; n += ((ns[2] + 0x80) >> (8 - Y)); /*y*/
  n <<= Z; n += ((ns[1] + 0x80) >> (8 - Z)); /*z*/

  return n;
}

template<const int mode, int D, const int X, const int Y, const int Z>
static doinline ULONG QuntXYZD(longlong (&ns)[DIM], longlong (&nr)[DIM]) ccr_restricted {
  /* build average of each channel an join */
  ULONG n = 0;

  /* XYZD -> DXYZ */
  n <<= D; n += (ULONG)((ns[0] + 0x0000) >> (16 - D)); /*d*/
  n <<= X; n += (ULONG)((ns[3] + 0x8000) >> (16 - X)); /*x*/
  n <<= Y; n += (ULONG)((ns[2] + 0x8000) >> (16 - Y)); /*y*/
  n <<= Z; n += (ULONG)((ns[1] + 0x8000) >> (16 - Z)); /*z*/

  return n;
}

template<const int mode, int D, const int X, const int Y, const int Z>
static doinline ULONG QuntXYZD(float (&nn)[DIM], float (&nr)[DIM]) ccr_restricted {
  const float hZ = (1 << Z) - 0.5f * (1 << Z);
  const float hY = (1 << Y) - 0.5f * (1 << Y);
  const float hX = (1 << X) - 0.5f * (1 << X);
  const float nZ = (1 << Z) - hZ;
  const float nY = (1 << Y) - hY;
  const float nX = (1 << X) - hX;
  const long  mD = (1 << D) - 1;
  const long  mZ = (1 << Z) - 1;
  const long  mY = (1 << Y) - 1;
  const long  mX = (1 << X) - 1;
  const float rD = (float)mD / 0xFF;
  const float rZ = (float)mZ / 0xFF;
//const float rY = (float)mY / 0xFF;
  const float rX = (float)mX / 0xFF;
#pragma warning (disable : 4244)

#if	defined(SQUASH_USE_SSE) && (SQUASH_USE_SSE >= 2)
  ULONG n = 0;
  a16 float vec[4];

  /* ################################################################# */
  /**/ if (((mode & TRGTMODE_CODING) == TRGTMODE_CODING_XYZ) ||
           ((mode & TRGTMODE_CODING) == TRGTMODE_CODING_XYZt) ||
           ((mode & TRGTMODE_CODING) == TRGTMODE_CODING_XY) ||
           ((mode & TRGTMODE_CODING) == TRGTMODE_CODING_XYt)) {
    *((__m128  *)vec) =
      _mm_add_ps(
      _mm_mul_ps(*((__m128  *)nn), _mm_set_ps(nZ, nY, nX,  rD )),
				   _mm_set_ps(hZ, hY, hX, 0.0f));
  }
  /* ################################################################# */
  else if (((mode & TRGTMODE_CODING) == TRGTMODE_CODING_DXDYt) ||
           ((mode & TRGTMODE_CODING) == TRGTMODE_CODING_DXDYdZt) ||
           ((mode & TRGTMODE_CODING) == TRGTMODE_CODING_DXDYDZt)) {
    *((__m128  *)vec) =
      _mm_add_ps(
      _mm_mul_ps(*((__m128  *)nn), _mm_set_ps(nZ, nY, nX,  rD )),
				   _mm_set_ps(hZ, hY, hX, 0.0f));
  }
  /* ################################################################# */
  else if ((mode & TRGTMODE_CODING) == TRGTMODE_CODING_AZt) {
    *((__m128  *)vec) =
      _mm_add_ps(
      _mm_mul_ps(*((__m128  *)nn), _mm_set_ps( rZ , nY,  rX ,  rD )),
				   _mm_set_ps(0.0f, hY, 0.0f, 0.0f));
  }
#else
  ULONG n = 0;
  float vec[4];

  vec[0] = nn[0];
  vec[1] = nn[1];
  vec[2] = nn[2];
  vec[3] = nn[3];

  /* ################################################################# */
  /**/ if (((mode & TRGTMODE_CODING) == TRGTMODE_CODING_XYZ) ||
           ((mode & TRGTMODE_CODING) == TRGTMODE_CODING_XYZt) ||
           ((mode & TRGTMODE_CODING) == TRGTMODE_CODING_XY) ||
           ((mode & TRGTMODE_CODING) == TRGTMODE_CODING_XYt)) {
    vec[0] *= rD; vec[0] += 0x00;
    vec[1] *= nX; vec[1] += hX;
    vec[2] *= nY; vec[2] += hY;
    vec[3] *= nZ; vec[3] += hZ;
  }
  /* ################################################################# */
  else if (((mode & TRGTMODE_CODING) == TRGTMODE_CODING_DXDYt) ||
	   ((mode & TRGTMODE_CODING) == TRGTMODE_CODING_DXDYdZt) ||
	   ((mode & TRGTMODE_CODING) == TRGTMODE_CODING_DXDYDZt)) {
    vec[0] *= rD; vec[0] += 0x00;
    vec[1] *= nX; vec[1] += hX;
    vec[2] *= nY; vec[2] += hY;
    vec[3] *= nZ; vec[3] += hZ;
  }
  /* ################################################################# */
  else if ((mode & TRGTMODE_CODING) == TRGTMODE_CODING_AZt) {
    vec[0] *= rD; vec[0] += 0x00;
    vec[1] *= rX; vec[1] += 0x00;
    vec[2] *= nY; vec[2] += hY;
  }
#endif

  /* simple quantization (we can do better though!) */
  if (((mode & TRGTMODE_CODING) != TRGTMODE_CODING_XYZ) &&
      ((mode & TRGTMODE_CODING) != TRGTMODE_CODING_XYZt) &&
      ((mode & TRGTMODE_CODING) != TRGTMODE_CODING_DXDYt) &&
//    ((mode & TRGTMODE_CODING) != TRGTMODE_CODING_DXDYdZt) &&	// for this Z isn't allowd to move
      ((mode & TRGTMODE_CODING) != TRGTMODE_CODING_DXDYDZt)) {
#if	defined(SQUASH_USE_SSE) && (SQUASH_USE_SSE >= 2)
    __m128 *v = (__m128 *)vec;
    __m128i rg = _mm_set_epi32(mD, mZ, mY, mX);

    __m128 m;
    __m128i i;

    /* lattice quantizer */
    m = _mm_add_ps   (*v, _mm_set_ps1(0.5f));
    i = _mm_cvttps_epi32(m);
    i = _mm_max_epi16(i, _mm_set1_epi32(0));
    i = _mm_min_epi16(i, rg);

    /* XYZD -> DXYZ */
    n <<= D; n += i.m128i_i32[0]; /*d[ 0,1]*/
    n <<= X; n += i.m128i_i32[3]; /*x[-1,1]*/
    n <<= Y; n += i.m128i_i32[2]; /*y[-1,1]*/
    n <<= Z; n += i.m128i_i32[1]; /*z[-1,1]*/
#else
    /* XYZD -> DXYZ */
    n <<= D; n += (rint<(1 << D) - 1>(vec[0]); /*d[ 0,1]*/
    n <<= X; n += (sint<(1 << X) - 1>(vec[3]); /*x[-1,1]*/
    n <<= Y; n += (sint<(1 << Y) - 1>(vec[2]); /*y[-1,1]*/
    n <<= Z; n += (sint<(1 << Z) - 1>(vec[1]); /*z[-1,1]*/
#endif
  }
  else {
    float normal[4];
    long sides[4][2];
    long sidep[4],
	 bestp[4];
    float bbox[DIM];
    float best = -FLT_MAX;
    long  len;

    /* the original vector in up-scaled quantized space (half-normal, length 0.5) */
    normal[3] = (vec[3] - hZ);
    normal[2] = (vec[2] - hY);
    normal[1] = (vec[1] - hX);

    /* make an iteration for each available lattice-point, divided by 2 (don't
     * check positions twice because of overlapping bboxes, rough approximation)
     */
    int   lattice =
      max(mZ,
      max(mY,
          mX));
    float expansn =       qmax(
      qabs(nn[3] * 0.5f), qmax(
      qabs(nn[2] * 0.5f),
      qabs(nn[1] * 0.5f)));
//  expansn = 0.5f /     expansn ;
    expansn = 0.5f * rcp(expansn);

    int iterations = lattice, bi,
        expansions = (long)(iterations * (expansn - 1.0f));

    for (int i = (iterations + expansions); i >= (iterations - expansions); i--) {
      /* shorten the vector per iteration */
//    float rit = (float)i / iterations;
      float rit = rcp(iterations) * i;

      /* the quantized opposing corner vectors in up-scaled quantized space */
      sides[3][0] = qfloor(normal[3] * rit - 0.5f) * 2 + 1;
      sides[2][0] = qfloor(normal[2] * rit - 0.5f) * 2 + 1;
      sides[1][0] = qfloor(normal[1] * rit - 0.5f) * 2 + 1;
      /* walk only on the odd lattice */
      sides[3][1] = min(sides[3][0] + 2, mZ);
      sides[2][1] = min(sides[2][0] + 2, mY);
      sides[1][1] = min(sides[1][0] + 2, mX);

#define	qcpy(a, b)    a[3] = b[3] , a[2] = b[2] , a[1] = b[1]
#define	qlen(z,y,x)   z[3] * z[3] + y[2] * y[2] + x[1] * x[1]

      /* "normal" is a normalized vector (though in up-scaled space and len 0.5)
       * and regardless the length doesn't need to be rescaled as it's
       * length gets canceled out (being in every dot-product as a constant)
       */
#define	qdot(a, b, z, y, x)	(		\
	(a[3] * (sidep[3] = b[3][z])) + 	\
	(a[2] * (sidep[2] = b[2][y])) + 	\
	(a[1] * (sidep[1] = b[1][x]))		\
  ) * rsqrt(len = (b[3][z] * b[3][z] + b[2][y] * b[2][y] + b[1][x] * b[1][x]))

      /* calculate the angles between each of the eight corners of the cube on
       * the lattice which are around the end-position of the original normal-vector
       * (the point on the unit-sphere)
       *
       * dot -> cos(angle) -> cos(0) => 1 -> select largest dot
       */
      bbox[0] = qdot(normal, sides, 0, 0, 0); if (len && (best < bbox[0])) { best = bbox[0], qcpy(bestp, sidep); bi = i; }
      bbox[1] = qdot(normal, sides, 0, 0, 1); if (len && (best < bbox[1])) { best = bbox[1], qcpy(bestp, sidep); bi = i; }
      bbox[2] = qdot(normal, sides, 0, 1, 0); if (len && (best < bbox[2])) { best = bbox[2], qcpy(bestp, sidep); bi = i; }
      bbox[3] = qdot(normal, sides, 0, 1, 1); if (len && (best < bbox[3])) { best = bbox[3], qcpy(bestp, sidep); bi = i; }

      // for this Z isn't allowd to move
      if ((mode & TRGTMODE_CODING) != TRGTMODE_CODING_DXDYt) {

      bbox[4] = qdot(normal, sides, 1, 0, 0); if (len && (best < bbox[4])) { best = bbox[4], qcpy(bestp, sidep); bi = i; }
      bbox[5] = qdot(normal, sides, 1, 0, 1); if (len && (best < bbox[5])) { best = bbox[5], qcpy(bestp, sidep); bi = i; }
      bbox[6] = qdot(normal, sides, 1, 1, 0); if (len && (best < bbox[6])) { best = bbox[6], qcpy(bestp, sidep); bi = i; }
      bbox[7] = qdot(normal, sides, 1, 1, 1); if (len && (best < bbox[7])) { best = bbox[7], qcpy(bestp, sidep); bi = i; }

      }
    }

#undef	qcpy
#undef	qlen
#undef	qdot

    /* put the 0.5 back (8 bit -> 0x80 etc.) */
    bestp[1] = ((bestp[1] - 1) >> 1) + (mX - (mX >> 1));
    bestp[2] = ((bestp[2] - 1) >> 1) + (mY - (mY >> 1));
    bestp[3] = ((bestp[3] - 1) >> 1) + (mZ - (mZ >> 1));

    /* XYZD -> DXYZ */
    n <<= D; n += (rint<(1 << D) - 1>(vec[0])); /*d[ 0,1]*/
    n <<= X; n += qmax(bestp[3], 0L); /*x[-1,1]*/
    n <<= Y; n += qmax(bestp[2], 0L); /*y[-1,1]*/
    n <<= Z; n += qmax(bestp[1], 0L); /*z[-1,1]*/
  }
  
#pragma warning (default : 4244)
  return n;
}

template<const int mode, int D, const int C, const int Y, const int X>
static doinline ULONG QuntXYCD(long (&nn)[DIM], long (&nr)[DIM]) ccr_restricted {
  /* doesn't exist, just for fullfilling templated case */; return 0;
}

template<const int mode, int D, const int C, const int Y, const int X>
static doinline ULONG QuntXYCD(longlong (&nn)[DIM], longlong (&nr)[DIM]) ccr_restricted {
  /* doesn't exist, just for fullfilling templated case */; return 0;
}

template<const int mode, int D, const int C, const int Y, const int X>
static doinline ULONG QuntXYCD(float (&nn)[DIM], float (&nr)[DIM]) ccr_restricted {
  /* doesn't exist, just for fullfilling templated case */; return 0;
}

/* ####################################################################################
 */

template<typename type>
static doinline void Spill(type (*d)[DIM], type (&s)[DIM]) ccr_restricted {
#if	defined(SQUASH_USE_SSE) && (SQUASH_USE_SSE >= 2)
  _mm_stream_ps(((float *)d) + 0, *((__m128 *)&s[0]));
  _mm_stream_ps(((float *)d) + 4, *((__m128 *)&s[4]));
#else
  memcpy(d, &s, sizeof(s));
#endif
}

/* ####################################################################################
 */

template<typename type>
static doinline void Clear(type (&t)[DIM]) ccr_restricted {
#if	defined(SQUASH_USE_SSE) && (SQUASH_USE_SSE >= 2)
  _mm_store_ps(((float *)t) + 0, _mm_setzero_ps());
  _mm_store_ps(((float *)t) + 4, _mm_setzero_ps());
#else
  memset(&t, 0, sizeof(t));
#endif
}

#pragma	warning (default : 4100)
