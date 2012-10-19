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

/* tile-dimensions (tied to 4x4 tile per block size) */
#define	TX	4
#define	TY	4

/* ------------------------------------------------------------------------------------
 */

/* only way to do this, <format> from template argument to preprocessor check */
#if	defined(SQUASH_USE_AMP) && defined(SQUASH_USE_PRE)

/* namespace to hide multiple defined symbols */
#include "squish/singlecolourlookup_ccr.inl"
#include "squish/degeneracy_ccr.inl"
namespace bc3 {
#include "squish.h"
}

using namespace bc3;

#define	fiting_max	0

#define	strprete(i)		#i
#define	stringify(i)		strprete(i)
#define	ff(s,f,LVL,FIT)		ff2(s,f,LVL,FIT)
#define	ff2(s,f,LVL,FIT)	s ## _ ## f ## _ ## BC ## LVL ## _ ## FIT

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#if	(fiting_max >= 0)
#define	fiting	0
#define	coding	3
//efine	format	TCOMPRESS_La
//nclude "compress_amp.cpp"		// DXT1/DXT3/DXT5
//efine	format	TCOMPRESS_LA
//nclude "compress_amp.cpp"		// DXT1/DXT3/DXT5
//efine	format	TCOMPRESS_LH
//nclude "compress_amp.cpp"		// DXT1/DXT3/DXT5
#define	format	TCOMPRESS_RGBa
#include "compress_amp.cpp"		// DXT1/DXT3/DXT5
#define	format	TCOMPRESS_RGBA
#include "compress_amp.cpp"		// DXT1/DXT3/DXT5
#define	format	TCOMPRESS_RGBH
#include "compress_amp.cpp"		// DXT1/DXT3/DXT5
#define	format	TCOMPRESS_xyzD
#include "compress_amp.cpp"		// DXT1/DXT3/DXT5
#define	format	TCOMPRESS_XYZD
#include "compress_amp.cpp"		// DXT1/DXT3/DXT5
#define	format	TCOMPRESS_XZYD
#include "compress_amp.cpp"		// DXT1/DXT3/DXT5
#endif

#if	(fiting_max >= 8)
#define	fiting	8
#define	coding	3
//efine	format	TCOMPRESS_La
//nclude "compress_amp.cpp"		// DXT1/DXT3/DXT5
//efine	format	TCOMPRESS_LA
//nclude "compress_amp.cpp"		// DXT1/DXT3/DXT5
//efine	format	TCOMPRESS_LH
//nclude "compress_amp.cpp"		// DXT1/DXT3/DXT5
#define	format	TCOMPRESS_RGBa
#include "compress_amp.cpp"		// DXT1/DXT3/DXT5
#define	format	TCOMPRESS_RGBA
#include "compress_amp.cpp"		// DXT1/DXT3/DXT5
#define	format	TCOMPRESS_RGBH
#include "compress_amp.cpp"		// DXT1/DXT3/DXT5
#define	format	TCOMPRESS_xyzD
#include "compress_amp.cpp"		// DXT1/DXT3/DXT5
#define	format	TCOMPRESS_XYZD
#include "compress_amp.cpp"		// DXT1/DXT3/DXT5
#define	format	TCOMPRESS_XZYD
#include "compress_amp.cpp"		// DXT1/DXT3/DXT5
#elif	(fiting_max >= 1)
#define	fiting	1
#define	coding	3
//efine	format	TCOMPRESS_La
//nclude "compress_amp.cpp"		// DXT1/DXT3/DXT5
//efine	format	TCOMPRESS_LA
//nclude "compress_amp.cpp"		// DXT1/DXT3/DXT5
//efine	format	TCOMPRESS_LH
//nclude "compress_amp.cpp"		// DXT1/DXT3/DXT5
#define	format	TCOMPRESS_RGBa
#include "compress_amp.cpp"		// DXT1/DXT3/DXT5
#define	format	TCOMPRESS_RGBA
#include "compress_amp.cpp"		// DXT1/DXT3/DXT5
#define	format	TCOMPRESS_RGBH
#include "compress_amp.cpp"		// DXT1/DXT3/DXT5
#define	format	TCOMPRESS_xyzD
#include "compress_amp.cpp"		// DXT1/DXT3/DXT5
#define	format	TCOMPRESS_XYZD
#include "compress_amp.cpp"		// DXT1/DXT3/DXT5
#define	format	TCOMPRESS_XZYD
#include "compress_amp.cpp"		// DXT1/DXT3/DXT5
#endif

#undef	fiting
#undef	coding
#undef	format

  template<typename UTYPE, typename type>
  void TextureCompressDXT_BC3(const int format, RESOURCEINFO &texo, RESOURCEINFO &texd, ULONG *texs, ULONG *texr, int level, int l, int blocksize, int flags) {

#if	(fiting_max >= 8)
    if (flags & squish::kColourIterativeClusterFit) {
      switch (format) {
//      case TCOMPRESS_a   : ff(TextureCompressDXT, TCOMPRESS_a   , 5, 0)<UTYPE, type>(texo, texd, texs, texr, level, l, blocksize, flags); break;
//      case TCOMPRESS_A   : ff(TextureCompressDXT, TCOMPRESS_A   , 5, 0)<UTYPE, type>(texo, texd, texs, texr, level, l, blocksize, flags); break;
//      case TCOMPRESS_L   :
////    case TCOMPRESS_RGB :
////    case TCOMPRESS_RGBV: ff(TextureCompressDXT, TCOMPRESS_RGB , 3, 8)<UTYPE, type>(texo, texd, texs, texr, level, l, blocksize, flags); break;
        case TCOMPRESS_La  :
        case TCOMPRESS_RGBa: ff(TextureCompressDXT, TCOMPRESS_RGBa, 3, 8)<UTYPE, type>(texo, texd, texs, texr, level, l, blocksize, flags); break;
        case TCOMPRESS_LA  :
        case TCOMPRESS_RGBA: ff(TextureCompressDXT, TCOMPRESS_RGBA, 3, 8)<UTYPE, type>(texo, texd, texs, texr, level, l, blocksize, flags); break;
        case TCOMPRESS_LH  :
        case TCOMPRESS_RGBH: ff(TextureCompressDXT, TCOMPRESS_RGBH, 3, 8)<UTYPE, type>(texo, texd, texs, texr, level, l, blocksize, flags); break;

//      case TCOMPRESS_xy  : ff(TextureCompressDXT, TCOMPRESS_xy  , 4, 0)<UTYPE, type>(texo, texd, texs, texr, level, l, blocksize, flags); break;
//      case TCOMPRESS_xyZ : ff(TextureCompressDXT, TCOMPRESS_xyZ , 5, 0)<UTYPE, type>(texo, texd, texs, texr, level, l, blocksize, flags); break;
//      case TCOMPRESS_XY  : ff(TextureCompressDXT, TCOMPRESS_XY  , 4, 0)<UTYPE, type>(texo, texd, texs, texr, level, l, blocksize, flags); break;
//      case TCOMPRESS_XYz : ff(TextureCompressDXT, TCOMPRESS_XYz , 4, 0)<UTYPE, type>(texo, texd, texs, texr, level, l, blocksize, flags); break;
////    case TCOMPRESS_xyz :
////    case TCOMPRESS_xyzV: ff(TextureCompressDXT, TCOMPRESS_xyz , 3, 8)<UTYPE, type>(texo, texd, texs, texr, level, l, blocksize, flags); break;
        case TCOMPRESS_xyzD: ff(TextureCompressDXT, TCOMPRESS_xyzD, 3, 8)<UTYPE, type>(texo, texd, texs, texr, level, l, blocksize, flags); break;
////    case TCOMPRESS_XYZ :
////    case TCOMPRESS_XYZV: ff(TextureCompressDXT, TCOMPRESS_XYZ , 3, 8)<UTYPE, type>(texo, texd, texs, texr, level, l, blocksize, flags); break;
        case TCOMPRESS_XYZD: ff(TextureCompressDXT, TCOMPRESS_XYZD, 3, 8)<UTYPE, type>(texo, texd, texs, texr, level, l, blocksize, flags); break;
////    case TCOMPRESS_XZY :
////    case TCOMPRESS_XZYV: ff(TextureCompressDXT, TCOMPRESS_XZY , 3, 8)<UTYPE, type>(texo, texd, texs, texr, level, l, blocksize, flags); break;
        case TCOMPRESS_XZYD: ff(TextureCompressDXT, TCOMPRESS_XZYD, 3, 8)<UTYPE, type>(texo, texd, texs, texr, level, l, blocksize, flags); break;
      }

      return;
    }
#elif	(fiting_max >= 1)
    if (flags & squish::kColourIterativeClusterFit) {
      switch (format) {
//      case TCOMPRESS_a   : ff(TextureCompressDXT, TCOMPRESS_a   , 5, 0)<UTYPE, type>(texo, texd, texs, texr, level, l, blocksize, flags); break;
//      case TCOMPRESS_A   : ff(TextureCompressDXT, TCOMPRESS_A   , 5, 0)<UTYPE, type>(texo, texd, texs, texr, level, l, blocksize, flags); break;
//      case TCOMPRESS_L   :
////    case TCOMPRESS_RGB :
////    case TCOMPRESS_RGBV: ff(TextureCompressDXT, TCOMPRESS_RGB , 3, 1)<UTYPE, type>(texo, texd, texs, texr, level, l, blocksize, flags); break;
        case TCOMPRESS_La  :
        case TCOMPRESS_RGBa: ff(TextureCompressDXT, TCOMPRESS_RGBa, 3, 1)<UTYPE, type>(texo, texd, texs, texr, level, l, blocksize, flags); break;
        case TCOMPRESS_LA  :
        case TCOMPRESS_RGBA: ff(TextureCompressDXT, TCOMPRESS_RGBA, 3, 1)<UTYPE, type>(texo, texd, texs, texr, level, l, blocksize, flags); break;
        case TCOMPRESS_LH  :
        case TCOMPRESS_RGBH: ff(TextureCompressDXT, TCOMPRESS_RGBH, 3, 1)<UTYPE, type>(texo, texd, texs, texr, level, l, blocksize, flags); break;

//      case TCOMPRESS_xy  : ff(TextureCompressDXT, TCOMPRESS_xy  , 4, 0)<UTYPE, type>(texo, texd, texs, texr, level, l, blocksize, flags); break;
//      case TCOMPRESS_xyZ : ff(TextureCompressDXT, TCOMPRESS_xyZ , 5, 0)<UTYPE, type>(texo, texd, texs, texr, level, l, blocksize, flags); break;
//      case TCOMPRESS_XY  : ff(TextureCompressDXT, TCOMPRESS_XY  , 4, 0)<UTYPE, type>(texo, texd, texs, texr, level, l, blocksize, flags); break;
//      case TCOMPRESS_XYz : ff(TextureCompressDXT, TCOMPRESS_XYz , 4, 0)<UTYPE, type>(texo, texd, texs, texr, level, l, blocksize, flags); break;
////    case TCOMPRESS_xyz :
////    case TCOMPRESS_xyzV: ff(TextureCompressDXT, TCOMPRESS_xyz , 3, 1)<UTYPE, type>(texo, texd, texs, texr, level, l, blocksize, flags); break;
        case TCOMPRESS_xyzD: ff(TextureCompressDXT, TCOMPRESS_xyzD, 3, 1)<UTYPE, type>(texo, texd, texs, texr, level, l, blocksize, flags); break;
////    case TCOMPRESS_XYZ :
////    case TCOMPRESS_XYZV: ff(TextureCompressDXT, TCOMPRESS_XYZ , 3, 1)<UTYPE, type>(texo, texd, texs, texr, level, l, blocksize, flags); break;
        case TCOMPRESS_XYZD: ff(TextureCompressDXT, TCOMPRESS_XYZD, 3, 1)<UTYPE, type>(texo, texd, texs, texr, level, l, blocksize, flags); break;
////    case TCOMPRESS_XZY :
////    case TCOMPRESS_XZYV: ff(TextureCompressDXT, TCOMPRESS_XZY , 3, 1)<UTYPE, type>(texo, texd, texs, texr, level, l, blocksize, flags); break;
        case TCOMPRESS_XZYD: ff(TextureCompressDXT, TCOMPRESS_XZYD, 3, 1)<UTYPE, type>(texo, texd, texs, texr, level, l, blocksize, flags); break;
      }

      return;
    }
#endif

#if	(fiting_max >= 0)
    {
      switch (format) {
//      case TCOMPRESS_a   : ff(TextureCompressDXT, TCOMPRESS_a   , 5, 0)<UTYPE, type>(texo, texd, texs, texr, level, l, blocksize, flags); break;
//      case TCOMPRESS_A   : ff(TextureCompressDXT, TCOMPRESS_A   , 5, 0)<UTYPE, type>(texo, texd, texs, texr, level, l, blocksize, flags); break;
////    case TCOMPRESS_L   :
////    case TCOMPRESS_RGB :
////    case TCOMPRESS_RGBV: ff(TextureCompressDXT, TCOMPRESS_RGB , 3, 0)<UTYPE, type>(texo, texd, texs, texr, level, l, blocksize, flags); break;
        case TCOMPRESS_La  :
        case TCOMPRESS_RGBa: ff(TextureCompressDXT, TCOMPRESS_RGBa, 3, 0)<UTYPE, type>(texo, texd, texs, texr, level, l, blocksize, flags); break;
        case TCOMPRESS_LA  :
        case TCOMPRESS_RGBA: ff(TextureCompressDXT, TCOMPRESS_RGBA, 3, 0)<UTYPE, type>(texo, texd, texs, texr, level, l, blocksize, flags); break;
        case TCOMPRESS_LH  :
        case TCOMPRESS_RGBH: ff(TextureCompressDXT, TCOMPRESS_RGBH, 3, 0)<UTYPE, type>(texo, texd, texs, texr, level, l, blocksize, flags); break;

//      case TCOMPRESS_xy  : ff(TextureCompressDXT, TCOMPRESS_xy  , 4, 0)<UTYPE, type>(texo, texd, texs, texr, level, l, blocksize, flags); break;
//      case TCOMPRESS_xyZ : ff(TextureCompressDXT, TCOMPRESS_xyZ , 5, 0)<UTYPE, type>(texo, texd, texs, texr, level, l, blocksize, flags); break;
//      case TCOMPRESS_XY  : ff(TextureCompressDXT, TCOMPRESS_XY  , 4, 0)<UTYPE, type>(texo, texd, texs, texr, level, l, blocksize, flags); break;
//      case TCOMPRESS_XYz : ff(TextureCompressDXT, TCOMPRESS_XYz , 4, 0)<UTYPE, type>(texo, texd, texs, texr, level, l, blocksize, flags); break;
////    case TCOMPRESS_xyz :
////    case TCOMPRESS_xyzV: ff(TextureCompressDXT, TCOMPRESS_xyz , 3, 0)<UTYPE, type>(texo, texd, texs, texr, level, l, blocksize, flags); break;
        case TCOMPRESS_xyzD: ff(TextureCompressDXT, TCOMPRESS_xyzD, 3, 0)<UTYPE, type>(texo, texd, texs, texr, level, l, blocksize, flags); break;
////    case TCOMPRESS_XYZ :
////    case TCOMPRESS_XYZV: ff(TextureCompressDXT, TCOMPRESS_XYZ , 3, 0)<UTYPE, type>(texo, texd, texs, texr, level, l, blocksize, flags); break;
        case TCOMPRESS_XYZD: ff(TextureCompressDXT, TCOMPRESS_XYZD, 3, 0)<UTYPE, type>(texo, texd, texs, texr, level, l, blocksize, flags); break;
////    case TCOMPRESS_XZY :
////    case TCOMPRESS_XZYV: ff(TextureCompressDXT, TCOMPRESS_XZY , 3, 0)<UTYPE, type>(texo, texd, texs, texr, level, l, blocksize, flags); break;
        case TCOMPRESS_XZYD: ff(TextureCompressDXT, TCOMPRESS_XZYD, 3, 0)<UTYPE, type>(texo, texd, texs, texr, level, l, blocksize, flags); break;
      }

      return;
    }
#endif
  }

  void _TextureCompressDXT_BC3(RESOURCEINFO &texo, RESOURCEINFO &texd, ULONG *texs, ULONG *texr, int level, int l, int blocksize, int flags) {
//  TextureCompressDXT_BC3<ULONG, long >(TCOMPRESS_a   , texo, texd, texs, texr, level, l, blocksize, flags);
//  TextureCompressDXT_BC3<ULONG, long >(TCOMPRESS_A   , texo, texd, texs, texr, level, l, blocksize, flags);
    TextureCompressDXT_BC3<ULONG, long >(TCOMPRESS_RGBa, texo, texd, texs, texr, level, l, blocksize, flags);
    TextureCompressDXT_BC3<ULONG, long >(TCOMPRESS_RGBA, texo, texd, texs, texr, level, l, blocksize, flags);
    TextureCompressDXT_BC3<ULONG, long >(TCOMPRESS_RGBH, texo, texd, texs, texr, level, l, blocksize, flags);

//  TextureCompressDXT_BC3<ULONG, float>(TCOMPRESS_a   , texo, texd, texs, texr, level, l, blocksize, flags);
//  TextureCompressDXT_BC3<ULONG, float>(TCOMPRESS_A   , texo, texd, texs, texr, level, l, blocksize, flags);
    TextureCompressDXT_BC3<ULONG, float>(TCOMPRESS_RGBa, texo, texd, texs, texr, level, l, blocksize, flags);
    TextureCompressDXT_BC3<ULONG, float>(TCOMPRESS_RGBA, texo, texd, texs, texr, level, l, blocksize, flags);
    TextureCompressDXT_BC3<ULONG, float>(TCOMPRESS_RGBH, texo, texd, texs, texr, level, l, blocksize, flags);

//  TextureCompressDXT_BC3<ULONG, float>(TCOMPRESS_xy  , texo, texd, texs, texr, level, l, blocksize, flags);
//  TextureCompressDXT_BC3<ULONG, float>(TCOMPRESS_xyZ , texo, texd, texs, texr, level, l, blocksize, flags);
//  TextureCompressDXT_BC3<ULONG, float>(TCOMPRESS_XY  , texo, texd, texs, texr, level, l, blocksize, flags);
//  TextureCompressDXT_BC3<ULONG, float>(TCOMPRESS_XYz , texo, texd, texs, texr, level, l, blocksize, flags);
    TextureCompressDXT_BC3<ULONG, float>(TCOMPRESS_xyzD, texo, texd, texs, texr, level, l, blocksize, flags);
    TextureCompressDXT_BC3<ULONG, float>(TCOMPRESS_XYZD, texo, texd, texs, texr, level, l, blocksize, flags);
    TextureCompressDXT_BC3<ULONG, float>(TCOMPRESS_XZYD, texo, texd, texs, texr, level, l, blocksize, flags);
  }

#endif
