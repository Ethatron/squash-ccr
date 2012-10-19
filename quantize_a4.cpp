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

#define	SQUASH_USE_PRE
/* only way to do this, <format> from template argument to preprocessor check */
#if	defined(SQUASH_USE_AMP) && defined(SQUASH_USE_PRE)

#define	strprete(i)	#i
#define	stringify(i)	strprete(i)
#define	ff(s,f,A)	ff2(s,f,A)
#define	ff2(s,f,A)	s ## _ ## f ## _ ## A

#define	A	4
#define	format	TCOMPRESS_La			// 44
#include "quantize_amp.cpp"
#define	format	TCOMPRESS_LA			// 44
#include "quantize_amp.cpp"
#define	format	TCOMPRESS_LH			// 44
#include "quantize_amp.cpp"

#define	A	4
#define	format	TCOMPRESS_RGBa			// 1555,4444,201010
#include "quantize_amp.cpp"
#define	format	TCOMPRESS_RGBV			// 1555,4444,201010
#include "quantize_amp.cpp"
#define	format	TCOMPRESS_RGBA			// 1555,4444,201010
#include "quantize_amp.cpp"
#define	format	TCOMPRESS_RGBH			// 1555,4444,201010
#include "quantize_amp.cpp"

#define	A	4
//efine	format	TCOMPRESS_XY			// 44
//nclude "texture-dds_quantize_amp.cpp"
//efine	format	TCOMPRESS_xyZ
//nclude "texture-dds_quantize_amp.cpp"
//efine	format	TCOMPRESS_XYz			// 44
//nclude "texture-dds_quantize_amp.cpp"

#define	A	4
#define	format	TCOMPRESS_xyzV			// 1555,4444,201010
#include "quantize_amp.cpp"
#define	format	TCOMPRESS_xyzD			// 1555,4444,201010
#include "quantize_amp.cpp"
#define	format	TCOMPRESS_XYZV			// 1555,4444,201010
#include "quantize_amp.cpp"
#define	format	TCOMPRESS_XZYV			// 1555,4444,201010
#include "quantize_amp.cpp"
#define	format	TCOMPRESS_XYZD			// 1555,4444,201010
#include "quantize_amp.cpp"
#define	format	TCOMPRESS_XZYD			// 1555,4444,201010
#include "quantize_amp.cpp"

#undef	A
#undef	format

  template<typename UTYPE, typename type>
  void TextureQuantizeRAW_A4(const int format, RESOURCEINFO &texo, RESOURCEINFO &texd, ULONG *texs, ULONG *texr, int level, int l) {
    switch (format) {
      case TCOMPRESS_La  : ff(TextureQuantizeRAW, TCOMPRESS_La  , 4)<UTYPE, type>(texo, texd, texs, texr, level, l); break;
      case TCOMPRESS_LA  : ff(TextureQuantizeRAW, TCOMPRESS_LA  , 4)<UTYPE, type>(texo, texd, texs, texr, level, l); break;
      case TCOMPRESS_LH  : ff(TextureQuantizeRAW, TCOMPRESS_LH  , 4)<UTYPE, type>(texo, texd, texs, texr, level, l); break;
      case TCOMPRESS_RGBa: ff(TextureQuantizeRAW, TCOMPRESS_RGBa, 4)<UTYPE, type>(texo, texd, texs, texr, level, l); break;
      case TCOMPRESS_RGBV: ff(TextureQuantizeRAW, TCOMPRESS_RGBV, 4)<UTYPE, type>(texo, texd, texs, texr, level, l); break;
      case TCOMPRESS_RGBA: ff(TextureQuantizeRAW, TCOMPRESS_RGBA, 4)<UTYPE, type>(texo, texd, texs, texr, level, l); break;
      case TCOMPRESS_RGBH: ff(TextureQuantizeRAW, TCOMPRESS_RGBH, 4)<UTYPE, type>(texo, texd, texs, texr, level, l); break;

//    case TCOMPRESS_xy  : ff(TextureQuantizeRAW, TCOMPRESS_xy  , 4)<UTYPE, type>(texo, texd, texs, texr, level, l); break;
//    case TCOMPRESS_XY  : ff(TextureQuantizeRAW, TCOMPRESS_XY  , 4)<UTYPE, type>(texo, texd, texs, texr, level, l); break;
//    case TCOMPRESS_XYz : ff(TextureQuantizeRAW, TCOMPRESS_XYz , 4)<UTYPE, type>(texo, texd, texs, texr, level, l); break;
      case TCOMPRESS_xyzV: ff(TextureQuantizeRAW, TCOMPRESS_xyzV, 4)<UTYPE, type>(texo, texd, texs, texr, level, l); break;
      case TCOMPRESS_xyzD: ff(TextureQuantizeRAW, TCOMPRESS_xyzD, 4)<UTYPE, type>(texo, texd, texs, texr, level, l); break;
      case TCOMPRESS_XYZV: ff(TextureQuantizeRAW, TCOMPRESS_XYZV, 4)<UTYPE, type>(texo, texd, texs, texr, level, l); break;
      case TCOMPRESS_XZYV: ff(TextureQuantizeRAW, TCOMPRESS_XZYV, 4)<UTYPE, type>(texo, texd, texs, texr, level, l); break;
      case TCOMPRESS_XYZD: ff(TextureQuantizeRAW, TCOMPRESS_XYZD, 4)<UTYPE, type>(texo, texd, texs, texr, level, l); break;
      case TCOMPRESS_XZYD: ff(TextureQuantizeRAW, TCOMPRESS_XZYD, 4)<UTYPE, type>(texo, texd, texs, texr, level, l); break;
    }
  }

  void _TextureQuantizeRAW_A4(RESOURCEINFO &texo, RESOURCEINFO &texd, ULONG *texs, ULONG *texr, int level, int l) {
    TextureQuantizeRAW_A4<UCHAR , long >(TCOMPRESS_La  , texo, texd, texs, texr, level, l);
    TextureQuantizeRAW_A4<UCHAR , long >(TCOMPRESS_LA  , texo, texd, texs, texr, level, l);
    TextureQuantizeRAW_A4<UCHAR , long >(TCOMPRESS_LH  , texo, texd, texs, texr, level, l);

    TextureQuantizeRAW_A4<UCHAR , float>(TCOMPRESS_La  , texo, texd, texs, texr, level, l);
    TextureQuantizeRAW_A4<UCHAR , float>(TCOMPRESS_LA  , texo, texd, texs, texr, level, l);
    TextureQuantizeRAW_A4<UCHAR , float>(TCOMPRESS_LH  , texo, texd, texs, texr, level, l);

    TextureQuantizeRAW_A4<USHORT, long >(TCOMPRESS_RGBa, texo, texd, texs, texr, level, l);
    TextureQuantizeRAW_A4<USHORT, long >(TCOMPRESS_RGBV, texo, texd, texs, texr, level, l);
    TextureQuantizeRAW_A4<USHORT, long >(TCOMPRESS_RGBA, texo, texd, texs, texr, level, l);
    TextureQuantizeRAW_A4<USHORT, long >(TCOMPRESS_RGBH, texo, texd, texs, texr, level, l);

    TextureQuantizeRAW_A4<USHORT, float>(TCOMPRESS_RGBa, texo, texd, texs, texr, level, l);
    TextureQuantizeRAW_A4<USHORT, float>(TCOMPRESS_RGBV, texo, texd, texs, texr, level, l);
    TextureQuantizeRAW_A4<USHORT, float>(TCOMPRESS_RGBA, texo, texd, texs, texr, level, l);
    TextureQuantizeRAW_A4<USHORT, float>(TCOMPRESS_RGBH, texo, texd, texs, texr, level, l);

//  TextureQuantizeRAW_A4<UCHAR , float>(TCOMPRESS_xy  , texo, texd, texs, texr, level, l);
//  TextureQuantizeRAW_A4<UCHAR , float>(TCOMPRESS_XY  , texo, texd, texs, texr, level, l);
//  TextureQuantizeRAW_A4<UCHAR , float>(TCOMPRESS_XYz , texo, texd, texs, texr, level, l);

    TextureQuantizeRAW_A4<USHORT, float>(TCOMPRESS_xyzV, texo, texd, texs, texr, level, l);
    TextureQuantizeRAW_A4<USHORT, float>(TCOMPRESS_xyzD, texo, texd, texs, texr, level, l);
    TextureQuantizeRAW_A4<USHORT, float>(TCOMPRESS_XYZV, texo, texd, texs, texr, level, l);
    TextureQuantizeRAW_A4<USHORT, float>(TCOMPRESS_XZYV, texo, texd, texs, texr, level, l);
    TextureQuantizeRAW_A4<USHORT, float>(TCOMPRESS_XYZD, texo, texd, texs, texr, level, l);
    TextureQuantizeRAW_A4<USHORT, float>(TCOMPRESS_XZYD, texo, texd, texs, texr, level, l);
  }

  /* not possible (blank out via specialization) */
  template<> void TextureQuantizeRAW_A4<UCHAR , longlong>(const int format, RESOURCEINFO &texo, RESOURCEINFO &texd, ULONG *texs, ULONG *texr, int level, int l) {}
  template<> void TextureQuantizeRAW_A4<USHORT, longlong>(const int format, RESOURCEINFO &texo, RESOURCEINFO &texd, ULONG *texs, ULONG *texr, int level, int l) {}

  template<> void TextureQuantizeRAW_A4<ULONG , long    >(const int format, RESOURCEINFO &texo, RESOURCEINFO &texd, ULONG *texs, ULONG *texr, int level, int l) {}
  template<> void TextureQuantizeRAW_A4<ULONG , longlong>(const int format, RESOURCEINFO &texo, RESOURCEINFO &texd, ULONG *texs, ULONG *texr, int level, int l) {}
  template<> void TextureQuantizeRAW_A4<ULONG , float   >(const int format, RESOURCEINFO &texo, RESOURCEINFO &texd, ULONG *texs, ULONG *texr, int level, int l) {}

#endif
