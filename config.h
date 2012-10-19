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

#ifndef SQUASH_CONFIG_H
#define SQUASH_CONFIG_H

/* spill level X's intermediate sums for level X+1 to use for calculation (and up)
 *
 * we start spilling when the i/o of the spill is less than the accumulation-cost
 * of reading from the original buffer (ignoring ALU cost):
 * level 2:
 *  read   8x 8 * sizeof(ULONG)x1 =  8x 8x4x1 =  256x1 =  256
 *  write  4x 4 * sizeof( long)x8 =  4x 4x4x8 =  256x8 = 1024
 *  read   8x 8 * sizeof( long)x8 =  8x 8x4x8 =  256x8 = 2048
 * level 3:
 *  read  16x16 * sizeof(ULONG)x1 = 16x16x4x1 = 1024x1 = 1024
 *  write  4x 4 * sizeof( long)x8 =  4x 4x4x8 =  256x8 = 1024
 *  read   8x 8 * sizeof( long)x8 =  8x 8x4x8 =  256x8 = 2048
 * level 4:
 *  read  32x32 * sizeof(ULONG)x1 = 32x32x4x1 = 4096x1 = 4096
 *  write  4x 4 * sizeof( long)x8 =  4x 4x4x8 =  256x8 = 1024
 *  read   8x 8 * sizeof( long)x8 =  8x 8x4x8 =  256x8 = 2048
 */
#define	SQUASH_INTERMEDIATES	3

#if	(_MSC_VER == 1700)
#define	SQUASH_USE_CCR
#undef	SQUASH_USE_AMP
#undef	SQUASH_USE_OMP
#elif	(_MSC_VER == 1600)
#define	SQUASH_USE_CCR
#else
#endif

// Set to 1 or 2 or 3 or 4 when building squish to use SSE or SSE2, SSE3 or SSE4 instructions.
#ifndef SQUASH_USE_SSE
#define SQUASH_USE_SSE 3
#endif

// Set to 3 or 4 when building squish to use SSSE3 or SSE4A instructions.
#ifndef SQUASH_USE_XSSE
#define SQUASH_USE_XSSE 0
#endif

// Internally et SQUASH_USE_SIMD when either Altivec or SSE is available.
#if SQUASH_USE_SSE
#define SQUASH_USE_SIMD 1
#ifdef __GNUC__
#define a16		__attribute__ ((__aligned__ (16)))
typedef long long	__int64;
#else
#define a16		__declspec(align(16))
#endif
#else
#define SQUASH_USE_SIMD 0
#ifdef __GNUC__
#define a16		__attribute__ ((__aligned__ (4)))
typedef unsigned long long unsigned__int64;
#else
#define a16		__declspec(align(4))
typedef unsigned __int64 unsigned__int64;
#endif
#endif

/* *****************************************************************************
 * Turn explicit vectorization of in case AMP or DirectCompute is requested
 */
#if	defined(SQUASH_USE_AMP) || defined(SQUASH_USE_COMPUTE)
#undef	SQUASH_USE_SSE
#undef	SQUASH_USE_SIMD

#define SQUASH_USE_SSE		0
#define SQUASH_USE_SIMD		0
#endif

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#if	defined(SQUASH_USE_CCR) && !defined(SQUASH_USE_AMP)
#include <array>  // TR1
#include <ppl.h>
//#include "concrt/ppl_extras.h"
#define	parallel_for_fixed	parallel_for

using namespace Concurrency;
#define	omp_set_nested(x)
#define	ccr_restricted
#define	amp_restricted
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#elif	defined(SQUASH_USE_CCR) && defined(SQUASH_USE_AMP) && 0
#include <array>  // TR1
#include <ppl.h>
//#include "concrt/ppl_extras.h"
#define	parallel_for_fixed	parallel_for

namespace Concurrency {
template<typename type, const int dim>
class array_view {
public:
  int w, h, d; type *arr;
  array_view(        int ww, int hh, type *aa, bool discard = false) { w = ww; h = hh; d =  1; arr = aa; }
  array_view(int dd, int hh, int ww, type *aa, bool discard = false) { w = ww; h = hh; d = dd; arr = aa; }
  type& operator() (              const int &y, const int &x) { return arr[        (y*w)+(x)]; }
  type& operator() (const int &z, const int &y, const int &x) { return arr[(z*w*h)+(y*w)+(x)]; }
};
}

#define tile_static

//accelerator my_acc(accelerator::direct3d_ref);
using namespace Concurrency;
#define	omp_set_nested(x)
#define	ccr_restricted		restrict(cpu)
#define	amp_restricted		restrict(cpu)
#undef	SQUASH_USE_SSE
#define	SQUASH_USE_PRE
#define SQUISH_USE_SSE 0
#define	SQUASH_USE_AMP_DEBUG
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#elif	defined(SQUASH_USE_AMP)
#include <array>  // TR1
#include <amp.h>
#include <amp_math.h>
#include <amp_graphics.h>
#include <amp_short_vectors.h>
#define	parallel_for_fixed	parallel_for

using namespace Concurrency;
#define	omp_set_nested(x)
#define	ccr_restricted		restrict(cpu,amp)
#define	amp_restricted		restrict(amp)
#undef	SQUASH_USE_SSE
#define	SQUASH_USE_PRE
#define SQUISH_USE_SSE 0
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#else
#include <omp.h>
#define	ccr_restricted
#define	amp_restricted
#endif

/* *****************************************************************************
*/
#ifdef __GNUC__
#define assume
#define doinline
#define	passreg		__fastcall
typedef long long	longlong;
#else
#define assume		__assume
#define doinline	__forceinline
#define	passreg		__fastcall
typedef __int64		longlong;
#endif

#endif // ndef SQUASH_CONFIG_H
