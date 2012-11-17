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

#include "common.h"

/* tile-dimensions */
#define	TX	1
#define	TY	1

/* ------------------------------------------------------------------------------------
 */

namespace squash {

template<const int format>
void TextureMatte(RESOURCEINFO &texo, ULONG sPch, ULONG *sTex) {

    /**/ if (TCOMPRESS_NORMAL(format)) {
      /* forward/backward-z */
      ULONG v = 0x00808000 | (TCOMPRESS_NINDEP(format) ? 0x00UL : 0xFFUL);
      ULONG c =              (TCOMPRESS_SIDES (format) ? 0x00UL : 0xFFUL) << 24;

#if	defined(SQUASH_USE_AMP) && !defined(SQUASH_USE_AMP_DEBUG)
      Concurrency::extent<1> num(texo.Width * texo.Height);
      Concurrency::array_view<ULONG, 1> sArr(num, sTex);

      Concurrency::parallel_for_each(num, [=](index<1> elm) restrict(amp) {
#elif	defined(SQUASH_USE_CCR)
      ULONG *sArr = sTex;
      parallel_for_fixed(0, (int)(texo.Width * texo.Height), [=](int elm) {
	int y = elm / texo.Width;
	int x = elm % texo.Width;
#else
      ULONG *sArr = sTex;
#pragma omp parallel for schedule(dynamic, 4)		\
			 shared(sTex)
      for (int y = 0; y < (int)texo.Height; y += TX) {
      for (int x = 0; x < (int)texo.Width ; x += TY) {
	int elm = (y * texo.Width) + x;
#endif
	ULONG t = sTex[y * sPch + x] | c;

	/* black matte to forward-z */
	/**/ if (( t & 0x00FFFFFF) == 0)
	  t = (t | v);
	/* white matte to forward-z (can't be done here, as white is valid for partial derivatives)
	else if ((~t & 0x00FFFFFF) == 0)
	  t = (t & v); */

	sArr[y * sPch + x] = t;
#if	defined(SQUASH_USE_AMP) && !defined(SQUASH_USE_AMP_DEBUG)
      });

      sArr.synchronize();
#elif	defined(SQUASH_USE_CCR)
      });
#else
      }}
#endif
    }

    else if (TCOMPRESS_GREYS(format)) {
      ULONG c =              (TCOMPRESS_SIDES (format) ? 0x00UL : 0xFFUL) << 24;

#if	defined(SQUASH_USE_AMP) && !defined(SQUASH_USE_AMP_DEBUG)
      Concurrency::extent<1> num(texo.Width * texo.Height);
      Concurrency::array_view<ULONG, 1> sArr(num, sTex);

      Concurrency::parallel_for_each(num, [=](index<1> elm) restrict(amp) {
#elif	defined(SQUASH_USE_CCR)
      ULONG *sArr = sTex;
      parallel_for_fixed(0, (int)(texo.Width * texo.Height), [=](int elm) {
#else
      ULONG *sArr = sTex;
#pragma omp parallel for schedule(dynamic, 4)		\
			 shared(sTex)
      for (int y = 0; y < (int)texo.Height; y += TX) {
      for (int x = 0; x < (int)texo.Width ; x += TY) {
	int elm = (y * texo.Width) + x;
#endif
	ULONG t = sArr[elm] | c;
	ULONG a = (t >> 24) & 0xFF; /*a*/
	ULONG r = (t >> 16) & 0xFF; /*a*/
	ULONG g = (t >>  8) & 0xFF; /*a*/
	ULONG b = (t >>  0) & 0xFF; /*a*/

	g = ((r * 5) + (g * 8) + (b * 3) + 8) >> 4;
	t = (a << 24) | (g << 16) | (g << 8) | (g << 0);

	sArr[elm] = t;
#if	defined(SQUASH_USE_AMP) && !defined(SQUASH_USE_AMP_DEBUG)
      });

      sArr.synchronize();
#elif	defined(SQUASH_USE_CCR)
      });
#else
      }}
#endif
    }

    else if (!TCOMPRESS_SIDES(format)) {
      ULONG c =              (TCOMPRESS_SIDES (format) ? 0x00UL : 0xFFUL) << 24;

#if	defined(SQUASH_USE_AMP) && !defined(SQUASH_USE_AMP_DEBUG)
      Concurrency::extent<1> num(texo.Width * texo.Height);
      Concurrency::array_view<ULONG, 1> sArr(num, sTex);

      Concurrency::parallel_for_each(num, [=](index<1> elm) restrict(amp) {
#elif	defined(SQUASH_USE_CCR)
      ULONG *sArr = sTex;
      parallel_for_fixed(0, (int)(texo.Width * texo.Height), [=](int elm) {
#else
      ULONG *sArr = sTex;
#pragma omp parallel for schedule(dynamic, 4)		\
			 shared(sTex)
      for (int y = 0; y < (int)texo.Height; y += TX) {
      for (int x = 0; x < (int)texo.Width ; x += TY) {
	int elm = (y * texo.Width) + x;
#endif
	sArr[elm] |= c;
#if	defined(SQUASH_USE_AMP) && !defined(SQUASH_USE_AMP_DEBUG)
      });

      sArr.synchronize();
#elif	defined(SQUASH_USE_CCR)
      });
#else
      }}
#endif
    }
}

} // namespace squash

#undef	TX
#undef	TY

/* ------------------------------------------------------------------------------------
 */

namespace squash {

void _TextureMatte(RESOURCEINFO &texo, ULONG sPch, ULONG *sTex) {
  TextureMatte<TCOMPRESS_L   >(texo, sPch, sTex);
  TextureMatte<TCOMPRESS_a   >(texo, sPch, sTex);
  TextureMatte<TCOMPRESS_A   >(texo, sPch, sTex);
  TextureMatte<TCOMPRESS_La  >(texo, sPch, sTex);
  TextureMatte<TCOMPRESS_LA  >(texo, sPch, sTex);
  TextureMatte<TCOMPRESS_LH  >(texo, sPch, sTex);
  TextureMatte<TCOMPRESS_RGB >(texo, sPch, sTex);
  TextureMatte<TCOMPRESS_RGBa>(texo, sPch, sTex);
  TextureMatte<TCOMPRESS_RGBV>(texo, sPch, sTex);
  TextureMatte<TCOMPRESS_RGBA>(texo, sPch, sTex);
  TextureMatte<TCOMPRESS_RGBH>(texo, sPch, sTex);

  TextureMatte<TCOMPRESS_xy  >(texo, sPch, sTex);
  TextureMatte<TCOMPRESS_XY  >(texo, sPch, sTex);
  TextureMatte<TCOMPRESS_xyZ >(texo, sPch, sTex);
  TextureMatte<TCOMPRESS_XYz >(texo, sPch, sTex);
  TextureMatte<TCOMPRESS_xyz >(texo, sPch, sTex);
  TextureMatte<TCOMPRESS_xyzV>(texo, sPch, sTex);
  TextureMatte<TCOMPRESS_xyzD>(texo, sPch, sTex);
  TextureMatte<TCOMPRESS_XYZ >(texo, sPch, sTex);
  TextureMatte<TCOMPRESS_XZY >(texo, sPch, sTex);
  TextureMatte<TCOMPRESS_XYZV>(texo, sPch, sTex);
  TextureMatte<TCOMPRESS_XZYV>(texo, sPch, sTex);
  TextureMatte<TCOMPRESS_XYZD>(texo, sPch, sTex);
  TextureMatte<TCOMPRESS_XZYD>(texo, sPch, sTex);
}

} // namespace squash
