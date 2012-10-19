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

#ifndef SQUASH_COMMON_H
#define SQUASH_COMMON_H

#define	_CRT_SECURE_NO_WARNINGS
#define	_CRT_NONSTDC_NO_DEPRECATE

#define _USE_MATH_DEFINES
#include <math.h>
#include <assert.h>
#include <intrin.h>

/* ------------------------------------------------------------------------------------
 */
#include "config.h"

#if	(_MSC_VER == 1700)
#pragma warning (disable : 4005)
#include <d3d11.h>
#include <d3dx11.h>
#elif	(_MSC_VER == 1600)
#include <d3d9.h>
#include <d3dx9.h>
#else
#include <d3d9.h>
#include <d3dx9.h>
#endif

#include "../../globals.h"
#include "squash.h"
#include "format.h"

namespace squash {

  bool TextureConvert(int minlevel, LPDIRECT3DTEXTURE *tex, bool dither, bool gamma, TEXFORMAT target);
  bool TextureCollapse(LPDIRECT3DTEXTURE *tex, TEXFORMAT target, bool swizzle);

  template<                               const int format>
  void TextureMatte(RESOURCEINFO &texo, ULONG sPch, ULONG *sTex);

  template<typename UTYPE, typename type, const int format, const int A>
  bool TextureQuantizeRAW(int minlevel, LPDIRECT3DTEXTURE *tex, bool optimize = true);
  template<typename UTYPE, typename type, const int format>
  bool TextureConvertRAW (int minlevel, LPDIRECT3DTEXTURE *tex, bool optimize = true);
  template<typename UTYPE, typename type, const int format>
  bool TextureCompressDXT(int minlevel, LPDIRECT3DTEXTURE *tex, bool optimize = true);

} // namespace squash

/* ####################################################################################
 */

#define TCOMPRESS_L		0
#define TCOMPRESS_a		1
#define TCOMPRESS_A		2
#define TCOMPRESS_La		3
#define TCOMPRESS_LA		4
#define TCOMPRESS_LH		5
#define TCOMPRESS_RGB		6
#define TCOMPRESS_RGBa		7
#define TCOMPRESS_RGBV		8
#define TCOMPRESS_RGBA		9
#define TCOMPRESS_RGBH		10
#define	TCOMPRESS_COLOR(fmt)	(((fmt) >= TCOMPRESS_L) && ((fmt) <= TCOMPRESS_RGBH))
#define	TCOMPRESS_GREYS(fmt)	(((fmt) == TCOMPRESS_L) || ((fmt) == TCOMPRESS_La) || ((fmt) == TCOMPRESS_LA) || ((fmt) == TCOMPRESS_LH))
#define	TCOMPRESS_TRANS(fmt)	(((fmt) == TCOMPRESS_a) || ((fmt) == TCOMPRESS_A ) || ((fmt) == TCOMPRESS_La) || ((fmt) == TCOMPRESS_LA) || ((fmt) == TCOMPRESS_RGBa) || ((fmt) == TCOMPRESS_RGBA))
#define	TCOMPRESS_TRESH(fmt)	(((fmt) == TCOMPRESS_a) || ((fmt) == TCOMPRESS_La) || ((fmt) == TCOMPRESS_RGBa))

#define TCOMPRESS_xy		11
#define TCOMPRESS_xyZ		12
#define TCOMPRESS_XY		13
#define TCOMPRESS_XYz		14
#define TCOMPRESS_xyz		15
#define TCOMPRESS_xyzV		16
#define TCOMPRESS_xyzD		17
#define TCOMPRESS_XYZ		18
#define TCOMPRESS_XZY		19
#define TCOMPRESS_XYZV		20
#define TCOMPRESS_XZYV		21
#define TCOMPRESS_XYZD		22
#define TCOMPRESS_XZYD		23
#define TCOMPRESS_XYCD		24
#define	TCOMPRESS_NINDEP(fmt)	(((fmt) >= TCOMPRESS_xy ) && ((fmt) <= TCOMPRESS_xyzD))
#define	TCOMPRESS_SWIZZL(fmt)	(((fmt) == TCOMPRESS_XZY) || ((fmt) == TCOMPRESS_XZYD) || ((fmt) == TCOMPRESS_XZYV))
#define	TCOMPRESS_NORMAL(fmt)	(((fmt) >= TCOMPRESS_xy ) && ((fmt) <= TCOMPRESS_XYCD))

/* do we have a side-stream? */
#define	TCOMPRESS_SIDES(fmt)	((((fmt) != TCOMPRESS_L) && ((fmt) != TCOMPRESS_RGB) && ((fmt) != TCOMPRESS_RGBV)) ||	\
				  ((fmt) == TCOMPRESS_xyzD) || ((fmt) >= TCOMPRESS_XYZD))

#define	TCOMPRESS_CHANNELS(fmt)			\
  (						\
    ((fmt) == TCOMPRESS_L	? 1 :		\
    ((fmt) == TCOMPRESS_a	? 1 :		\
    ((fmt) == TCOMPRESS_A	? 1 :		\
    ((fmt) == TCOMPRESS_La	? 2 :		\
    ((fmt) == TCOMPRESS_LA	? 2 :		\
    ((fmt) == TCOMPRESS_LH	? 2 :		\
    ((fmt) == TCOMPRESS_RGB	? 3 :		\
    ((fmt) == TCOMPRESS_RGBa	? 4 :		\
    ((fmt) == TCOMPRESS_RGBV	? 4 :		\
    ((fmt) == TCOMPRESS_RGBA	? 4 :		\
    ((fmt) == TCOMPRESS_RGBH	? 4 :		\
						\
    ((fmt) == TCOMPRESS_xy	? 2 :		\
    ((fmt) == TCOMPRESS_XY	? 2 :		\
    ((fmt) == TCOMPRESS_xyZ	? 1 :		\
    ((fmt) == TCOMPRESS_XYz	? 2 :		\
    ((fmt) == TCOMPRESS_xyz	? 3 :		\
    ((fmt) == TCOMPRESS_xyzV	? 4 :		\
    ((fmt) == TCOMPRESS_xyzD	? 4 :		\
    ((fmt) == TCOMPRESS_XYZ	? 3 :		\
    ((fmt) == TCOMPRESS_XZY	? 3 :		\
    ((fmt) == TCOMPRESS_XYZV	? 4 :		\
    ((fmt) == TCOMPRESS_XZYV	? 4 :		\
    ((fmt) == TCOMPRESS_XYZD	? 4 :		\
    ((fmt) == TCOMPRESS_XZYD	? 4 :		\
    ((fmt) == TCOMPRESS_XYCD	? 4 :		\
				  0		\
    )))))))))))))))))))))))))			\
  )

/* in Oblivion the encoding to DXT1 apparently turns
 * off the shader which handle the alpha-information
 * to prevent that just code to DXT1 when the information
 * is neutral:
 * - for normals, black means no specular
 * - for colors, white means no transparency/parallax
 */
#define CollapseAlpha(fmt, blk, wht)	(ignorewhitealpha ? (TCOMPRESS_NORMAL(fmt) ? blk       : wht      ) :        wht      )
#define MinimalAlpha(fmt, blk, wht, bw)	(ignorewhitealpha ? (TCOMPRESS_NORMAL(fmt) ? blk || bw : wht || bw) : blk || wht || bw)
#define ExistAlpha(fmt, org)		((TCOMPRESS_CHANNELS(fmt) > 3) && (org != TEXFMT_DXT1))

#include "math.h"
#include "filter.h"

#endif // !TEXTURE_DDS_COMMON_H
