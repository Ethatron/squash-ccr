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

///fine	A	0
///fine	format	TCOMPRESS_L
///clude "texture-dds_quantize_amp.cpp"

#define	A	0
#define	format	TCOMPRESS_RGB			// 565
#include "quantize_amp.cpp"

#define	A	0
#define	format	TCOMPRESS_xyz			// 565
#include "quantize_amp.cpp"
#define	format	TCOMPRESS_XYZ			// 565
#include "quantize_amp.cpp"
#define	format	TCOMPRESS_XZY			// 565
#include "quantize_amp.cpp"

#undef	A
#undef	format

  template<typename UTYPE, typename type>
  void TextureQuantizeRAW_A0(const int format, RESOURCEINFO &texo, RESOURCEINFO &texd, ULONG *texs, ULONG *texr, int level, int l) {
    switch (format) {
      case TCOMPRESS_RGB : ff(TextureQuantizeRAW, TCOMPRESS_RGB , 0)<UTYPE, type>(texo, texd, texs, texr, level, l); break;

      case TCOMPRESS_xyz : ff(TextureQuantizeRAW, TCOMPRESS_xyz , 0)<UTYPE, type>(texo, texd, texs, texr, level, l); break;
      case TCOMPRESS_XYZ : ff(TextureQuantizeRAW, TCOMPRESS_XYZ , 0)<UTYPE, type>(texo, texd, texs, texr, level, l); break;
      case TCOMPRESS_XZY : ff(TextureQuantizeRAW, TCOMPRESS_XZY , 0)<UTYPE, type>(texo, texd, texs, texr, level, l); break;
    }
  }

  void _TextureQuantizeRAW_A0(RESOURCEINFO &texo, RESOURCEINFO &texd, ULONG *texs, ULONG *texr, int level, int l) {
    TextureQuantizeRAW_A0<USHORT, long >(TCOMPRESS_RGB, texo, texd, texs, texr, level, l);
    TextureQuantizeRAW_A0<USHORT, float>(TCOMPRESS_RGB, texo, texd, texs, texr, level, l);

    TextureQuantizeRAW_A0<USHORT, float>(TCOMPRESS_xyz, texo, texd, texs, texr, level, l);
    TextureQuantizeRAW_A0<USHORT, float>(TCOMPRESS_XYZ, texo, texd, texs, texr, level, l);
    TextureQuantizeRAW_A0<USHORT, float>(TCOMPRESS_XZY, texo, texd, texs, texr, level, l);
  }

  /* not possible (blank out via specialization) */
  template<> void TextureQuantizeRAW_A0<UCHAR, long    >(const int format, RESOURCEINFO &texo, RESOURCEINFO &texd, ULONG *texs, ULONG *texr, int level, int l) {}
  template<> void TextureQuantizeRAW_A0<UCHAR, longlong>(const int format, RESOURCEINFO &texo, RESOURCEINFO &texd, ULONG *texs, ULONG *texr, int level, int l) {}
  template<> void TextureQuantizeRAW_A0<UCHAR, float   >(const int format, RESOURCEINFO &texo, RESOURCEINFO &texd, ULONG *texs, ULONG *texr, int level, int l) {}

  template<> void TextureQuantizeRAW_A0<ULONG, long    >(const int format, RESOURCEINFO &texo, RESOURCEINFO &texd, ULONG *texs, ULONG *texr, int level, int l) {}
  template<> void TextureQuantizeRAW_A0<ULONG, longlong>(const int format, RESOURCEINFO &texo, RESOURCEINFO &texd, ULONG *texs, ULONG *texr, int level, int l) {}
  template<> void TextureQuantizeRAW_A0<ULONG, float   >(const int format, RESOURCEINFO &texo, RESOURCEINFO &texd, ULONG *texs, ULONG *texr, int level, int l) {}

#endif
