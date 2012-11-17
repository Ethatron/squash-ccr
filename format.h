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

#ifndef SQUASH_FORMAT_H
#define SQUASH_FORMAT_H

#include "config.h"

/* ############################################################################################## */

#if	(_MSC_VER == 1700) && (SQUASH_USE_API >= 110)
#define DX11
#endif

#define D3DFMT_ATI1	(D3DFORMAT)MAKEFOURCC('A','T','I','1')
#define D3DFMT_ATI2	(D3DFORMAT)MAKEFOURCC('A','T','I','2')

#define D3DFMT_INTZ	(D3DFORMAT)MAKEFOURCC('I','N','T','Z')
#define D3DFMT_DF24	(D3DFORMAT)MAKEFOURCC('D','F','2','4')
#define D3DFMT_DF16	(D3DFORMAT)MAKEFOURCC('D','F','1','6')
#define D3DFMT_RAWZ	(D3DFORMAT)MAKEFOURCC('R','A','W','Z')

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
#ifdef DX11
#pragma warning (disable : 4005)
#include  <d3d11.h>

#define TEXFORMAT_CAST	DXGI_FORMAT
#define TEXFMT_GREY(f)	(false)
#define TEXFMT_BTC(f)	(((f >= DXGI_FORMAT_BC1_TYPELESS) && (f <= DXGI_FORMAT_BC5_SNORM)) ||	\
			 ((f >= DXGI_FORMAT_BC6H_TYPELESS) && (f <= DXGI_FORMAT_BC7_UNORM_SRGB)))
/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
#else
#include  <d3d9.h>

#define TEXFORMAT_CAST	D3DFORMAT
#define TEXFMT_GREY(f)	((f == D3DFMT_L8) || (f == D3DFMT_A8L8) || (f == D3DFMT_A4L4) || (f == D3DFMT_L16))
#define TEXFMT_BTC(f)	((f == D3DFMT_DXT1) || (f == D3DFMT_DXT3) || (f == D3DFMT_DXT5))
#endif

/* ############################################################################################## */

struct DDS_PIXELFORMAT {
  DWORD dwSize;
  DWORD dwFlags;
  DWORD dwFourCC;
  DWORD dwRGBBitCount;
  DWORD dwRBitMask;
  DWORD dwGBitMask;
  DWORD dwBBitMask;
  DWORD dwABitMask;
};

typedef struct {
  DWORD dwMagicNumber;
#define DDS_MAGIC	MAKEFOURCC('D','D','S',' ')	// 0x20534444
#define DDS_DX10	MAKEFOURCC('D','X','1','0')
#define DDS_DX11	MAKEFOURCC('D','X','1','1')

  DWORD dwSize;
  DWORD dwHeaderFlags;
  DWORD dwHeight;
  DWORD dwWidth;
  DWORD dwPitchOrLinearSize;
  DWORD dwDepth; // only if DDS_HEADER_FLAGS_VOLUME is set in dwHeaderFlags
  DWORD dwMipMapCount;
  DWORD dwReserved1[11];
  DDS_PIXELFORMAT ddspf;
  DWORD dwSurfaceFlags;
  DWORD dwCubemapFlags;
  DWORD dwReserved2[3];
} DDS_HEADER;

#ifdef	DX11
typedef struct {
  DXGI_FORMAT              dxgiFormat;
  D3D10_RESOURCE_DIMENSION resourceDimension;
  UINT                     dxCubemapFlags;
  UINT                     dxArraySize;
  UINT                     dxReserved;
} DDS_HEADER10;
#endif

/* ############################################################################################## */

namespace squash {

typedef enum {
  TEXFMT_UNKNOWN              ,

  TEXFMT_R1                   ,
  TEXFMT_R8                   ,
  TEXFMT_G8R8                 ,
  TEXFMT_R8G8B8               ,
  TEXFMT_A8R8G8B8             ,
  TEXFMT_X8R8G8B8             ,
  TEXFMT_R5G6B5               ,
  TEXFMT_X1R5G5B5             ,
  TEXFMT_A1R5G5B5             ,
  TEXFMT_A4R4G4B4             ,
  TEXFMT_R3G3B2               ,
  TEXFMT_A8R3G3B2             ,
  TEXFMT_X4R4G4B4             ,
  TEXFMT_A8B8G8R8             ,
  TEXFMT_X8B8G8R8             ,

  TEXFMT_A8R8G8B8srgb         ,
  TEXFMT_X8R8G8B8srgb         ,
  TEXFMT_A8B8G8R8srgb         ,
  TEXFMT_X8B8G8R8srgb         ,

  TEXFMT_A2B10G10R10          ,
  TEXFMT_A2R10G10B10          ,
  TEXFMT_A2B10G10R10_XR_BIAS  ,
  TEXFMT_R16                  ,
  TEXFMT_G16R16               ,
  TEXFMT_A16B16G16R16         ,
  TEXFMT_R32                  ,
  TEXFMT_G32R32               ,
  TEXFMT_B32G32R32            ,
  TEXFMT_A32B32G32R32         ,
  TEXFMT_A1                   ,
  TEXFMT_A8                   ,
  TEXFMT_P8                   ,
  TEXFMT_A8P8                 ,
  TEXFMT_L8                   ,
  TEXFMT_L16                  ,
  TEXFMT_A4L4                 ,
  TEXFMT_A8L8                 ,

  TEXFMT_R9G9B9E5             ,

  TEXFMT_U8                   ,
  TEXFMT_V8U8                 ,
  TEXFMT_CxV8U8               ,
  TEXFMT_L6V5U5               ,
  TEXFMT_X8L8V8U8             ,
  TEXFMT_Q8W8V8U8             ,
  TEXFMT_A2W10V10U10          ,
  TEXFMT_U16                  ,
  TEXFMT_V16U16               ,
  TEXFMT_Q16W16V16U16         ,

  TEXFMT_R11G11B10F           ,
  TEXFMT_R16F                 ,
  TEXFMT_G16R16F              ,
  TEXFMT_A16B16G16R16F        ,
  TEXFMT_R32F                 ,
  TEXFMT_G32R32F              ,
  TEXFMT_B32G32R32F           ,
  TEXFMT_A32B32G32R32F        ,

  TEXFMT_R32G8X24             ,
  TEXFMT_R32X8X24             ,
  TEXFMT_X32G8X24             ,
  TEXFMT_R24G8                ,
  TEXFMT_R24X8                ,
  TEXFMT_X24G8                ,

  TEXFMT_UYVY                 ,
  TEXFMT_YUY2                 ,
  TEXFMT_R8G8_B8G8            ,
  TEXFMT_G8R8_G8B8            ,

  TEXFMT_DXT1                 ,
  TEXFMT_DXT2                 ,
  TEXFMT_DXT3                 ,
  TEXFMT_DXT4                 ,
  TEXFMT_DXT5                 ,
  TEXFMT_DXTM                 ,
  TEXFMT_ATI1                 ,
  TEXFMT_ATI2                 ,
  TEXFMT_DXTH                 ,

  TEXFMT_DXT1srgb             ,
  TEXFMT_DXT2srgb             ,
  TEXFMT_DXT3srgb             ,
  TEXFMT_DXT4srgb             ,
  TEXFMT_DXT5srgb             ,
  TEXFMT_DXTMsrgb             ,
  TEXFMT_ATI1s                ,
  TEXFMT_ATI2s                ,
  TEXFMT_DXTHs                ,

  TEXFMT_S8_LOCKABLE          ,
  TEXFMT_D15S1                ,
  TEXFMT_D16                  ,
  TEXFMT_D16_LOCKABLE         ,
  TEXFMT_D24S8                ,
  TEXFMT_D24X8                ,
  TEXFMT_D24X4S4              ,
  TEXFMT_D24FS8               ,
  TEXFMT_D32                  ,
  TEXFMT_D32_LOCKABLE         ,
  TEXFMT_D32S8                ,
  TEXFMT_D32F_LOCKABLE        ,
  TEXFMT_INTZ                 ,
  TEXFMT_DF24                 ,
  TEXFMT_DF16                 ,
  TEXFMT_RAWZ                 ,

  TEXFMT_VERTEXDATA           ,
  TEXFMT_INDEX16              ,
  TEXFMT_INDEX32              ,
  TEXFMT_MULTI2_ARGB8         ,

  TEXFMT_BINARYBUFFER         ,

  // aliases
  TEXFMT_BTC1 = TEXFMT_DXT1   ,
  TEXFMT_BTC2 = TEXFMT_DXT3   ,
  TEXFMT_BTC3 = TEXFMT_DXT5   ,
  TEXFMT_BTC4 = TEXFMT_ATI1   ,
  TEXFMT_BTC5 = TEXFMT_ATI2   ,
  TEXFMT_BTC6 = TEXFMT_DXTH   ,
  TEXFMT_BTC7 = TEXFMT_DXTM   ,

  TEXFMT_BTC1srgb = TEXFMT_DXT1srgb,
  TEXFMT_BTC2srgb = TEXFMT_DXT3srgb,
  TEXFMT_BTC3srgb = TEXFMT_DXT5srgb,
  TEXFMT_BTC7srgb = TEXFMT_DXTMsrgb,
} NONFMT;

class TEXFORMAT {
public:
  NONFMT TEXFMT;

  /* default constructor */
  TEXFORMAT() { TEXFMT = TEXFMT_UNKNOWN; }
  TEXFORMAT(const NONFMT &dxf) { TEXFMT = dxf; }
#ifdef DX11
  TEXFORMAT(const DXGI_FORMAT &dx11f) { *this = dx11f; }
#else
  TEXFORMAT(const D3DFORMAT &dx9f) { *this = dx9f; }
#endif

  /* type-casting to native types */
  operator NONFMT() const { return TEXFMT; }
#ifdef DX11
  operator DXGI_FORMAT() const;
#else
  operator D3DFORMAT() const;
#endif

  /* assignment from native types */
  TEXFORMAT &operator = (const NONFMT &dxf) { TEXFMT = dxf; return *this; }
#ifdef DX11
  TEXFORMAT &operator = (const DXGI_FORMAT &dx11f);
#else
  TEXFORMAT &operator = (const D3DFORMAT &dx9f);
#endif

  /* copy operator */
  TEXFORMAT &operator = (const TEXFORMAT &txf) { TEXFMT = txf.TEXFMT; }

  /* comparison */
  friend bool operator == (const TEXFORMAT &a, const TEXFORMAT &b) { return a.TEXFMT == b.TEXFMT; }
  friend bool operator != (const TEXFORMAT &a, const TEXFORMAT &b) { return a.TEXFMT != b.TEXFMT; }
  friend bool operator == (const TEXFORMAT &a, const NONFMT &b) { return a.TEXFMT == b; }
  friend bool operator != (const TEXFORMAT &a, const NONFMT &b) { return a.TEXFMT != b; }

#ifdef DX11
  friend bool operator <= (const TEXFORMAT &a, const DXGI_FORMAT &b) { DXGI_FORMAT c = a; return c <= b; }
  friend bool operator >= (const TEXFORMAT &a, const DXGI_FORMAT &b) { DXGI_FORMAT c = a; return c >= b; }
#else
  friend bool operator == (const TEXFORMAT &a, const D3DFORMAT &b) { TEXFORMAT c = b; return a.TEXFMT == c.TEXFMT; }
  friend bool operator != (const TEXFORMAT &a, const D3DFORMAT &b) { TEXFORMAT c = b; return a.TEXFMT != c.TEXFMT; }
#endif

  static bool Available(const NONFMT &neutral) {
    TEXFORMAT chk = neutral;
    TEXFORMAT_CAST native = chk;
    TEXFORMAT bck = native;

    return bck != TEXFMT_UNKNOWN;
  }
};

const char *findFormat(TEXFORMAT fmt);
short findFormatDepth(TEXFORMAT fmt);
char findFormatChannels(TEXFORMAT fmt);
short findFormatSize(TEXFORMAT fmt);
bool findAlpha(TEXFORMAT fmt);
const char *findFormatRatio(TEXFORMAT fmt, TEXFORMAT before);

} // namespace squash

#endif