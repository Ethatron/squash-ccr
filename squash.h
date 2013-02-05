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
extern int manualflags;

/* ------------------------------------------------------------------------------------
 */

#include "config.h"

#if	(_MSC_VER == 1700) && (SQUASH_USE_API >= 110)
#pragma warning (disable : 4005)
#include <d3d11.h>
#include <d3dx11.h>

namespace squash {
  
  extern D3D_DRIVER_TYPE tD3DDevice;
  extern ID3D11Device *pD3DDevice;
  extern ID3D11DeviceContext *pD3DDeviceContext;

} // namespace squash

typedef struct _D3DBASE_DESC {
  UINT Width;
  UINT Height;
  UINT Depth;
  UINT MipLevels;
  UINT Slices;
  
  D3D11_RESOURCE_DIMENSION Type;
  DXGI_FORMAT Format;
  D3D11_USAGE Usage;

  UINT BindFlags;
  UINT CPUAccessFlags;
  UINT MiscFlags;

  DXGI_SAMPLE_DESC SampleDesc;
  
  _D3DBASE_DESC& operator = (const D3D11_TEXTURE1D_DESC &p) {
    MiscFlags = p.MiscFlags; Usage = p.Usage; CPUAccessFlags = p.CPUAccessFlags; BindFlags = p.BindFlags;
    Format = p.Format; Width = p.Width; Height = 1; Slices = 1; Depth = 1;
    SampleDesc.Count = 0; SampleDesc.Quality = 0;
    Type = D3D11_RESOURCE_DIMENSION_TEXTURE1D;

    return *this;
  }
  
  _D3DBASE_DESC& operator = (const D3D11_TEXTURE2D_DESC &p) {
    MiscFlags = p.MiscFlags; Usage = p.Usage; CPUAccessFlags = p.CPUAccessFlags; BindFlags = p.BindFlags;
    Format = p.Format; Width = p.Width; Height = p.Height; Slices = p.ArraySize; Depth = 1;
    SampleDesc = p.SampleDesc;
    Type = D3D11_RESOURCE_DIMENSION_TEXTURE2D;

    return *this;
  }
  
  _D3DBASE_DESC& operator = (const D3D11_TEXTURE3D_DESC &v) {
    MiscFlags = v.MiscFlags; Usage = v.Usage; CPUAccessFlags = v.CPUAccessFlags; BindFlags = v.BindFlags;
    Format = v.Format; Width = v.Width; Height = 1; Slices = 1; Depth = v.Depth;
    SampleDesc.Count = 0; SampleDesc.Quality = 0;
    Type = D3D11_RESOURCE_DIMENSION_TEXTURE3D;

    return *this;
  }

} D3DBASE_DESC;

typedef struct ID3D11Resource *LPDIRECT3DTEXTURE11, *PDIRECT3DTEXTURE11;
#define	LPDIRECT3DBASETEXTURE LPDIRECT3DTEXTURE11
#define	LPDIRECT3DCUBETEXTURE LPDIRECT3DTEXTURE11
#define	LPDIRECT3DVOLTEXTURE LPDIRECT3DTEXTURE11
#define	LPDIRECT3DTEXTURE LPDIRECT3DTEXTURE11
#define	LPD3DBUFFER	  LPD3D10BLOB
#define	RESOURCETYPE	  D3D11_RESOURCE_DIMENSION
#define RESOURCEINFO	  D3DBASE_DESC
#define	TEXMEMORY	  D3D11_MAPPED_SUBRESOURCE
#define	TEXVOLMEMORY	  D3D11_MAPPED_SUBRESOURCE
#define TEXINFO		  D3DX11_IMAGE_INFO
#define FILEFORMAT	  D3DX11_IMAGE_FILE_FORMAT
#define DRIVERTYPE	  D3D_DRIVER_TYPE
#define	D3D_OK		  S_OK
#define DX11
#else
#include <d3d9.h>
#include <d3dx9.h>

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")

namespace squash {

  extern D3DDEVTYPE tD3DDevice;
  extern IDirect3D9 *pD3D;
  extern IDirect3DDevice9 *pD3DDevice;

} // namespace squash

typedef struct _D3DBASE_DESC {
  D3DFORMAT           Format;
  D3DRESOURCETYPE     Type;
  DWORD               Usage;
  D3DPOOL             Pool;

  UINT                Width;
  UINT                Height;
  UINT                Slices;
  UINT                Depth;

  D3DMULTISAMPLE_TYPE MultiSampleType;
  DWORD               MultiSampleQuality;

  _D3DBASE_DESC& operator = (const D3DSURFACE_DESC &p) {
    Format = p.Format; Type = p.Type; Usage = p.Usage; Pool = p.Pool;
    Width = p.Width; Height = p.Height; Slices = 1; Depth = 1;
    MultiSampleType = p.MultiSampleType;
    MultiSampleQuality = p.MultiSampleQuality;

    return *this;
  }
  
  _D3DBASE_DESC& operator = (const D3DVOLUME_DESC &v) {
    Format = v.Format; Type = v.Type; Usage = v.Usage; Pool = v.Pool;
    Width = v.Width; Height = v.Height; Slices = 1; Depth = v.Depth;
    MultiSampleType = D3DMULTISAMPLE_NONE;
    MultiSampleQuality = 0;

    return *this;
  }

} D3DBASE_DESC;

#define	LPDIRECT3DBASETEXTURE LPDIRECT3DBASETEXTURE9
#define	LPDIRECT3DCUBETEXTURE LPDIRECT3DCUBETEXTURE9
#define	LPDIRECT3DVOLTEXTURE LPDIRECT3DVOLUMETEXTURE9
#define	LPDIRECT3DTEXTURE LPDIRECT3DTEXTURE9
#define	LPD3DBUFFER	  LPD3DXBUFFER
#define	RESOURCETYPE	  D3DRESOURCETYPE
#define RESOURCEINFO	  D3DBASE_DESC

#define	TEXMEMORY	  D3DLOCKED_RECT
#define	TEXVOLMEMORY	  D3DLOCKED_BOX
#define TEXINFO		  D3DXIMAGE_INFO
#define FILEFORMAT	  D3DXIMAGE_FILEFORMAT
#define DRIVERTYPE	  D3DDEVTYPE
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
  int  TextureCalcVolumeMip(int w, int h, int d, int minlvl);
  int  TextureCalcVolumeMip(int w, int h, int d, int lw, int lh, int ld);

  bool TextureInfo(const void *inmem, size_t insize, TEXINFO &nfo);
  RESOURCETYPE TextureInfoType(LPDIRECT3DBASETEXTURE tex);
  size_t TextureInfoSize(LPDIRECT3DBASETEXTURE tex);
  bool TextureInfoLevel(LPDIRECT3DBASETEXTURE tex, RESOURCEINFO &nfo, int lvl);

  ULONG *TextureLock(LPDIRECT3DBASETEXTURE tex, int lvl, int slice, ULONG *pitch, bool writable = false);
  void TextureUnlock(LPDIRECT3DBASETEXTURE tex, int lvl, int slice);

  bool TextureFillMip(int w, int h, LPDIRECT3DBASETEXTURE *tex, bool dither, bool gamma, int levels);
  bool TextureDownMip(int w, int h, LPDIRECT3DBASETEXTURE *tex);
  
  HRESULT TextureCreate(RESOURCEINFO &info, LPDIRECT3DBASETEXTURE *tex, int levels);
  bool TextureConvert(RESOURCEINFO &info, LPDIRECT3DBASETEXTURE *tex, bool black);

} // namespace squash

#ifdef DX11
HRESULT D3DX11CreateBaseTextureFromMemory(
      ID3D11Device*             pDevice,
      LPCVOID                   pSrcData,
      SIZE_T                    SrcDataSize,
      D3DX11_IMAGE_LOAD_INFO*   pLoadInfo,    
      ID3DX11ThreadPump*        pPump,    
      ID3D11Resource**          ppTexture,
      HRESULT*                  pHResult);

LPDIRECT3DTEXTURE LoadBC45TextureFromFileInMemory(
      ID3D11Device *pDevice, const void *data, size_t size, bool decompress = false);
#else
HRESULT D3DXCreateBaseTextureFromFileInMemoryEx(
      LPDIRECT3DDEVICE9         pDevice,
      LPCVOID                   pSrcData,
      UINT                      SrcDataSize,
      UINT                      Width,
      UINT                      Height,
      UINT                      Depth,
      UINT                      MipLevels,
      DWORD                     Usage,
      D3DFORMAT                 Format,
      D3DPOOL                   Pool,
      DWORD                     Filter,
      DWORD                     MipFilter,
      D3DCOLOR                  ColorKey,
      D3DXIMAGE_INFO*           pSrcInfo,
      PALETTEENTRY*             pPalette,
      LPDIRECT3DBASETEXTURE9*   ppTexture);

LPDIRECT3DTEXTURE9 LoadBC45TextureFromFileInMemory(
      LPDIRECT3DDEVICE9 pDevice, const void *data, size_t size, bool decompress = false);
HRESULT SaveBC45TextureToFileInMemory(LPD3DBUFFER *oubuf,
      LPDIRECT3DBASETEXTURE9 pSrcTexture);
HRESULT SaveBC45TextureToFile(const char *fp,
      LPDIRECT3DBASETEXTURE9 pSrcTexture);
#endif

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

namespace squash {

  bool TextureQuantizeR10G10B10_A2(LPDIRECT3DBASETEXTURE *base, int minlevel, bool gamma, bool contrast);
  bool TextureQuantizeR10G10B10A2(LPDIRECT3DBASETEXTURE *base, int minlevel, bool gamma, bool contrast);
  bool TextureQuantizeR10G10B10H2(LPDIRECT3DBASETEXTURE *base, int minlevel, bool gamma);
  bool TextureQuantizeR10G10B10V2(LPDIRECT3DBASETEXTURE *base, int minlevel, bool gamma);
  bool TextureQuantizeR5G5B5_A1(LPDIRECT3DBASETEXTURE *base, int minlevel, bool gamma, bool contrast);
  bool TextureQuantizeR5G5B5A1(LPDIRECT3DBASETEXTURE *base, int minlevel, bool gamma, bool contrast);
  bool TextureQuantizeR5G5B5H1(LPDIRECT3DBASETEXTURE *base, int minlevel, bool gamma);
  bool TextureQuantizeR5G5B5V1(LPDIRECT3DBASETEXTURE *base, int minlevel, bool gamma);
  bool TextureQuantizeR4G4B4_A4(LPDIRECT3DBASETEXTURE *base, int minlevel, bool gamma, bool contrast);
  bool TextureQuantizeR4G4B4A4(LPDIRECT3DBASETEXTURE *base, int minlevel, bool gamma, bool contrast);
  bool TextureQuantizeR4G4B4H4(LPDIRECT3DBASETEXTURE *base, int minlevel, bool gamma);
  bool TextureQuantizeR4G4B4V4(LPDIRECT3DBASETEXTURE *base, int minlevel, bool gamma);
  bool TextureQuantizeR5G6B5(LPDIRECT3DBASETEXTURE *base, int minlevel, bool gamma);
  bool TextureQuantizeL4_A4(LPDIRECT3DBASETEXTURE *base, int minlevel, bool gamma, bool contrast);
  bool TextureQuantizeL4A4(LPDIRECT3DBASETEXTURE *base, int minlevel, bool gamma, bool contrast);
  bool TextureQuantizeL4H4(LPDIRECT3DBASETEXTURE *base, int minlevel, bool gamma);

  bool TextureQuantize_X10Y10Z10D2(LPDIRECT3DBASETEXTURE *norm, int minlevel);
  bool TextureQuantizeX10Y10Z10D2(LPDIRECT3DBASETEXTURE *norm, int minlevel);
  bool TextureQuantize_X10Y10Z10V2(LPDIRECT3DBASETEXTURE *norm, int minlevel);
  bool TextureQuantizeX10Y10Z10V2(LPDIRECT3DBASETEXTURE *norm, int minlevel);
  bool TextureQuantize_X4Y4Z4D4(LPDIRECT3DBASETEXTURE *norm, int minlevel);
  bool TextureQuantizeX4Y4Z4D4(LPDIRECT3DBASETEXTURE *norm, int minlevel);
  bool TextureQuantize_X4Y4Z4V4(LPDIRECT3DBASETEXTURE *norm, int minlevel);
  bool TextureQuantizeX4Y4Z4V4(LPDIRECT3DBASETEXTURE *norm, int minlevel);
  bool TextureQuantizeX5Y6Z5(LPDIRECT3DBASETEXTURE *norm, int minlevel);
  bool TextureQuantize_X5Y6Z5(LPDIRECT3DBASETEXTURE *norm, int minlevel);
  bool TextureQuantizeX5Z6Y5(LPDIRECT3DBASETEXTURE *norm, int minlevel);

  bool TextureConvertRGB_A(LPDIRECT3DBASETEXTURE *base, int minlevel, bool gamma, bool contrast);
  bool TextureConvertRGBA(LPDIRECT3DBASETEXTURE *base, int minlevel, bool gamma, bool contrast);
  bool TextureConvertRGBH(LPDIRECT3DBASETEXTURE *base, int minlevel, bool gamma);
  bool TextureConvertRGBV(LPDIRECT3DBASETEXTURE *base, int minlevel, bool gamma);
  bool TextureConvertRGB(LPDIRECT3DBASETEXTURE *base, int minlevel, bool gamma);
  bool TextureConvertL_A(LPDIRECT3DBASETEXTURE *base, int minlevel, bool gamma, bool contrast);
  bool TextureConvertLA(LPDIRECT3DBASETEXTURE *base, int minlevel, bool gamma, bool contrast);
  bool TextureConvertLH(LPDIRECT3DBASETEXTURE *base, int minlevel, bool gamma);
  bool TextureConvertL(LPDIRECT3DBASETEXTURE *lumi, int minlevel, bool gamma);
  bool TextureConvert_A(LPDIRECT3DBASETEXTURE *alpha, int minlevel, bool contrast);
  bool TextureConvertA(LPDIRECT3DBASETEXTURE *alpha, int minlevel, bool contrast);

  bool TextureConvert_XYZV(LPDIRECT3DBASETEXTURE *norm, int minlevel);
  bool TextureConvertXYZV(LPDIRECT3DBASETEXTURE *norm, int minlevel);
  bool TextureConvert_XYZD(LPDIRECT3DBASETEXTURE *norm, int minlevel);
  bool TextureConvertXYZD(LPDIRECT3DBASETEXTURE *norm, int minlevel);
  bool TextureConvert_XY_Z(LPDIRECT3DBASETEXTURE *norm, LPDIRECT3DBASETEXTURE *z, int minlevel);
  bool TextureConvertXY_Z(LPDIRECT3DBASETEXTURE *norm, LPDIRECT3DBASETEXTURE *z, int minlevel);
  bool TextureConvert_XYZ(LPDIRECT3DBASETEXTURE *norm, int minlevel);
  bool TextureConvertXYZ(LPDIRECT3DBASETEXTURE *norm, int minlevel);
  bool TextureConvert_XY(LPDIRECT3DBASETEXTURE *norm, int minlevel);
  bool TextureConvertXY(LPDIRECT3DBASETEXTURE *norm, int minlevel);

  bool TextureCompressRGB_A(LPDIRECT3DBASETEXTURE *base, int minlevel, bool gamma, bool contrast);
  bool TextureCompressRGBA(LPDIRECT3DBASETEXTURE *base, int minlevel, bool gamma, bool contrast);
  bool TextureCompressRGBH(LPDIRECT3DBASETEXTURE *base, int minlevel, bool gamma);
  bool TextureCompressRGB(LPDIRECT3DBASETEXTURE *base, int minlevel, bool gamma);
  bool TextureCompressL_A(LPDIRECT3DBASETEXTURE *base, int minlevel, bool gamma, bool contrast);
  bool TextureCompressLA(LPDIRECT3DBASETEXTURE *base, int minlevel, bool gamma, bool contrast);
  bool TextureCompressLH(LPDIRECT3DBASETEXTURE *base, int minlevel, bool gamma);
  bool TextureCompressL(LPDIRECT3DBASETEXTURE *base, int minlevel, bool gamma);
  bool TextureCompress_A(LPDIRECT3DBASETEXTURE *alpha, int minlevel, bool contrast);
  bool TextureCompressA(LPDIRECT3DBASETEXTURE *alpha, int minlevel, bool contrast);

  bool TextureCompress_XYZD(LPDIRECT3DBASETEXTURE *norm, int minlevel);
  bool TextureCompressXYZD(LPDIRECT3DBASETEXTURE *norm, int minlevel);
  bool TextureCompress_XY_Z(LPDIRECT3DBASETEXTURE *norm, LPDIRECT3DBASETEXTURE *z, int minlevel);
  bool TextureCompressXY_Z(LPDIRECT3DBASETEXTURE *norm, LPDIRECT3DBASETEXTURE *z, int minlevel);
  bool TextureCompress_XYZ(LPDIRECT3DBASETEXTURE *norm, int minlevel);
  bool TextureCompressXYZ(LPDIRECT3DBASETEXTURE *norm, int minlevel);
  bool TextureCompress_XY(LPDIRECT3DBASETEXTURE *norm, int minlevel);
  bool TextureCompressXY(LPDIRECT3DBASETEXTURE *norm, int minlevel);
  bool TextureCompress_Z(LPDIRECT3DBASETEXTURE *norm, int minlevel);
  bool TextureCompressZ(LPDIRECT3DBASETEXTURE *norm, int minlevel);

  bool TextureCompressPM(LPDIRECT3DBASETEXTURE *base, LPDIRECT3DBASETEXTURE *norm, int minlevel, bool gamma);
  bool TextureCompressQDM(LPDIRECT3DBASETEXTURE *base, LPDIRECT3DBASETEXTURE *norm, int minlevel, bool gamma, bool LODed);

} // namespace squash

#endif // !TEXTURE_DDS_H