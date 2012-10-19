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

#ifndef SQUASH_H
#define SQUASH_H

/* ############################################################################################## */

extern float colorgamma;
extern float colorgammainv;
extern float alphacontrast;
extern float alphacontrastinv;
extern int normalsteepness;
extern float alphaopaqueness;
extern int ignoreborder;
extern bool ignorewhitealpha;

/* ------------------------------------------------------------------------------------
 */

#if	(_MSC_VER == 1700) && 0
#pragma warning (disable : 4005)
#include <d3d11.h>
#include <d3dx11.h>

namespace squash {

  extern ID3D11Device *pD3DDevice;
  extern ID3D11DeviceContext *pD3DDeviceContext;

} // namespace squash

typedef struct ID3D11Resource *LPDIRECT3DTEXTURE11, *PDIRECT3DTEXTURE11;
#define	LPDIRECT3DCUBETEXTURE LPDIRECT3DTEXTURE11
#define	LPDIRECT3DTEXTURE LPDIRECT3DTEXTURE11
#define	LPD3DBUFFER	  LPD3D10BLOB
#define	RESOURCETYPE	  D3D11_RESOURCE_DIMENSION
#define RESOURCEINFO	  D3D11_TEXTURE2D_DESC
#define	TEXMEMORY	  D3D11_MAPPED_SUBRESOURCE
#define TEXINFO		  D3DX11_IMAGE_INFO
#define FILEFORMAT	  D3DX11_IMAGE_FILE_FORMAT
#define	D3D_OK		  S_OK
#define DX11
#else
#include <d3d9.h>
#include <d3dx9.h>

namespace squash {

  extern IDirect3D9 *pD3D;
  extern IDirect3DDevice9 *pD3DDevice;

} // namespace squash

#define	LPDIRECT3DCUBETEXTURE LPDIRECT3DCUBETEXTURE9
#define	LPDIRECT3DTEXTURE LPDIRECT3DTEXTURE9
#define	LPD3DBUFFER	  LPD3DXBUFFER
#define	RESOURCETYPE	  D3DRESOURCETYPE
#define RESOURCEINFO	  D3DSURFACE_DESC
#define	TEXMEMORY	  D3DLOCKED_RECT
#define TEXINFO		  D3DXIMAGE_INFO
#define FILEFORMAT	  D3DXIMAGE_FILEFORMAT
#define DX9
#endif

#include "format.h"

namespace squash {

  const char *findType(TEXINFO &nfo);
  const char *findFileformat(FILEFORMAT fmt);

} // namespace squash

/* ############################################################################################## */

namespace squash {

  bool TextureInit();
  void TextureCleanup();
  int  TextureCalcMip(int w, int h, int minlvl);
  int  TextureCalcMip(int w, int h, int lw, int lh);
  bool TextureInfo(void *inmem, UINT insize, TEXINFO &nfo);

  ULONG *TextureLock(LPDIRECT3DTEXTURE tex, int lvl, ULONG *pitch, bool writable = false);
  void TextureUnlock(LPDIRECT3DTEXTURE tex, int lvl);
  bool TextureInfoLevel(LPDIRECT3DTEXTURE tex, RESOURCEINFO &nfo, int lvl);

  ULONG *TextureLock(LPDIRECT3DCUBETEXTURE tex, int fce, int lvl, ULONG *pitch, bool writable);
  void TextureUnlock(LPDIRECT3DCUBETEXTURE tex, int fce, int lvl);
  bool TextureInfoLevel(LPDIRECT3DCUBETEXTURE tex, RESOURCEINFO &nfo, int fce, int lvl);

  bool TextureFillMip(int w, int h, LPDIRECT3DTEXTURE *tex, bool dither, bool gamma, int levels);
  bool TextureDownMip(int w, int h, LPDIRECT3DTEXTURE *tex);

  bool TextureConvert(RESOURCEINFO &info, LPDIRECT3DTEXTURE *tex, bool black);
  
} // namespace squash

#ifndef DX11
LPDIRECT3DTEXTURE9 LoadBC45TextureFromFileInMemory(LPDIRECT3DDEVICE9 pDevice, const void *data, int size);
HRESULT SaveBC45TextureToFileInMemory(LPD3DBUFFER *oubuf, LPDIRECT3DBASETEXTURE9 pSrcTexture);
HRESULT SaveBC45TextureToFile(const char *fp, LPDIRECT3DBASETEXTURE9 pSrcTexture);
#endif

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

namespace squash {

  bool TextureQuantizeR10G10B10_A2(LPDIRECT3DTEXTURE *base, int minlevel, bool gamma, bool contrast);
  bool TextureQuantizeR10G10B10A2(LPDIRECT3DTEXTURE *base, int minlevel, bool gamma, bool contrast);
  bool TextureQuantizeR10G10B10H2(LPDIRECT3DTEXTURE *base, int minlevel, bool gamma);
  bool TextureQuantizeR10G10B10V2(LPDIRECT3DTEXTURE *base, int minlevel, bool gamma);
  bool TextureQuantizeR5G5B5_A1(LPDIRECT3DTEXTURE *base, int minlevel, bool gamma, bool contrast);
  bool TextureQuantizeR5G5B5A1(LPDIRECT3DTEXTURE *base, int minlevel, bool gamma, bool contrast);
  bool TextureQuantizeR5G5B5H1(LPDIRECT3DTEXTURE *base, int minlevel, bool gamma);
  bool TextureQuantizeR5G5B5V1(LPDIRECT3DTEXTURE *base, int minlevel, bool gamma);
  bool TextureQuantizeR4G4B4_A4(LPDIRECT3DTEXTURE *base, int minlevel, bool gamma, bool contrast);
  bool TextureQuantizeR4G4B4A4(LPDIRECT3DTEXTURE *base, int minlevel, bool gamma, bool contrast);
  bool TextureQuantizeR4G4B4H4(LPDIRECT3DTEXTURE *base, int minlevel, bool gamma);
  bool TextureQuantizeR4G4B4V4(LPDIRECT3DTEXTURE *base, int minlevel, bool gamma);
  bool TextureQuantizeR5G6B5(LPDIRECT3DTEXTURE *base, int minlevel, bool gamma);
  bool TextureQuantizeL4_A4(LPDIRECT3DTEXTURE *base, int minlevel, bool gamma, bool contrast);
  bool TextureQuantizeL4A4(LPDIRECT3DTEXTURE *base, int minlevel, bool gamma, bool contrast);
  bool TextureQuantizeL4H4(LPDIRECT3DTEXTURE *base, int minlevel, bool gamma);

  bool TextureQuantize_X10Y10Z10D2(LPDIRECT3DTEXTURE *norm, int minlevel);
  bool TextureQuantizeX10Y10Z10D2(LPDIRECT3DTEXTURE *norm, int minlevel);
  bool TextureQuantize_X10Y10Z10V2(LPDIRECT3DTEXTURE *norm, int minlevel);
  bool TextureQuantizeX10Y10Z10V2(LPDIRECT3DTEXTURE *norm, int minlevel);
  bool TextureQuantize_X4Y4Z4D4(LPDIRECT3DTEXTURE *norm, int minlevel);
  bool TextureQuantizeX4Y4Z4D4(LPDIRECT3DTEXTURE *norm, int minlevel);
  bool TextureQuantize_X4Y4Z4V4(LPDIRECT3DTEXTURE *norm, int minlevel);
  bool TextureQuantizeX4Y4Z4V4(LPDIRECT3DTEXTURE *norm, int minlevel);
  bool TextureQuantizeX5Y6Z5(LPDIRECT3DTEXTURE *norm, int minlevel);
  bool TextureQuantize_X5Y6Z5(LPDIRECT3DTEXTURE *norm, int minlevel);
  bool TextureQuantizeX5Z6Y5(LPDIRECT3DTEXTURE *norm, int minlevel);

  bool TextureConvertRGB_A(LPDIRECT3DTEXTURE *base, int minlevel, bool gamma, bool contrast);
  bool TextureConvertRGBA(LPDIRECT3DTEXTURE *base, int minlevel, bool gamma, bool contrast);
  bool TextureConvertRGBH(LPDIRECT3DTEXTURE *base, int minlevel, bool gamma);
  bool TextureConvertRGBV(LPDIRECT3DTEXTURE *base, int minlevel, bool gamma);
  bool TextureConvertRGB(LPDIRECT3DTEXTURE *base, int minlevel, bool gamma);
  bool TextureConvertL_A(LPDIRECT3DTEXTURE *base, int minlevel, bool gamma, bool contrast);
  bool TextureConvertLA(LPDIRECT3DTEXTURE *base, int minlevel, bool gamma, bool contrast);
  bool TextureConvertLH(LPDIRECT3DTEXTURE *base, int minlevel, bool gamma);
  bool TextureConvertL(LPDIRECT3DTEXTURE *lumi, int minlevel, bool gamma);
  bool TextureConvert_A(LPDIRECT3DTEXTURE *alpha, int minlevel, bool contrast);
  bool TextureConvertA(LPDIRECT3DTEXTURE *alpha, int minlevel, bool contrast);

  bool TextureConvert_XYZV(LPDIRECT3DTEXTURE *norm, int minlevel);
  bool TextureConvertXYZV(LPDIRECT3DTEXTURE *norm, int minlevel);
  bool TextureConvert_XYZD(LPDIRECT3DTEXTURE *norm, int minlevel);
  bool TextureConvertXYZD(LPDIRECT3DTEXTURE *norm, int minlevel);
  bool TextureConvertXY_Z(LPDIRECT3DTEXTURE *norm, LPDIRECT3DTEXTURE *z, int minlevel);
  bool TextureConvert_XYZ(LPDIRECT3DTEXTURE *norm, int minlevel);
  bool TextureConvertXYZ(LPDIRECT3DTEXTURE *norm, int minlevel);
  bool TextureConvertXY(LPDIRECT3DTEXTURE *norm, int minlevel);

  bool TextureCompressRGB_A(LPDIRECT3DTEXTURE *base, int minlevel, bool gamma, bool contrast);
  bool TextureCompressRGBA(LPDIRECT3DTEXTURE *base, int minlevel, bool gamma, bool contrast);
  bool TextureCompressRGBH(LPDIRECT3DTEXTURE *base, int minlevel, bool gamma);
  bool TextureCompressRGB(LPDIRECT3DTEXTURE *base, int minlevel, bool gamma);
  bool TextureCompressL_A(LPDIRECT3DTEXTURE *base, int minlevel, bool gamma, bool contrast);
  bool TextureCompressLA(LPDIRECT3DTEXTURE *base, int minlevel, bool gamma, bool contrast);
  bool TextureCompressLH(LPDIRECT3DTEXTURE *base, int minlevel, bool gamma);
  bool TextureCompressL(LPDIRECT3DTEXTURE *base, int minlevel, bool gamma);
  bool TextureCompress_A(LPDIRECT3DTEXTURE *alpha, int minlevel, bool contrast);
  bool TextureCompressA(LPDIRECT3DTEXTURE *alpha, int minlevel, bool contrast);

  bool TextureCompress_XYZD(LPDIRECT3DTEXTURE *norm, int minlevel);
  bool TextureCompressXYZD(LPDIRECT3DTEXTURE *norm, int minlevel);
  bool TextureCompressXY_Z(LPDIRECT3DTEXTURE *norm, LPDIRECT3DTEXTURE *z, int minlevel);
  bool TextureCompress_XYZ(LPDIRECT3DTEXTURE *norm, int minlevel);
  bool TextureCompressXYZ(LPDIRECT3DTEXTURE *norm, int minlevel);
  bool TextureCompress_XY(LPDIRECT3DTEXTURE *norm, int minlevel);
  bool TextureCompressXY(LPDIRECT3DTEXTURE *norm, int minlevel);
  bool TextureCompress_Z(LPDIRECT3DTEXTURE *norm, int minlevel);

  bool TextureCompressPM(LPDIRECT3DTEXTURE *base, LPDIRECT3DTEXTURE *norm, int minlevel, bool gamma);
  bool TextureCompressQDM(LPDIRECT3DTEXTURE *base, LPDIRECT3DTEXTURE *norm, int minlevel, bool gamma, bool LODed);
  
} // namespace squash

#endif // !TEXTURE_DDS_H