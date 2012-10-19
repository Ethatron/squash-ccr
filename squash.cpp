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

#include <assert.h>

#include "../../globals.h"
#include "squash.h"

/* ####################################################################################
 */

float colorgamma = 1.0f / 1.0f;
float colorgammainv = 1.0f / 1.0f;
float alphacontrast = 1.0f / 1.0f;
float alphacontrastinv = 1.0f / 1.0f;
int normalsteepness = 8;
float alphaopaqueness = 0.0f;
int ignoreborder = 0;
bool ignorewhitealpha = false;

/* ------------------------------------------------------------------------------------
 */

namespace squash {

const char *findType(TEXINFO &nfo) {
#ifdef	DX11
  switch (nfo.ResourceDimension) {
    case D3D11_RESOURCE_DIMENSION_BUFFER: return "Buffer";
    case D3D11_RESOURCE_DIMENSION_TEXTURE1D: return "Texture Strip";
    case D3D11_RESOURCE_DIMENSION_TEXTURE2D: return (nfo.ArraySize == 6 ? "Cubic texture" : "Planar texture");
    case D3D11_RESOURCE_DIMENSION_TEXTURE3D: return "Volumetric texture";
  }
#else
  switch (nfo.ResourceType) {
    case D3DRTYPE_SURFACE: return "Surface";
    case D3DRTYPE_VOLUME: return "Volume";
    case D3DRTYPE_TEXTURE: return "Planar texture";
    case D3DRTYPE_VOLUMETEXTURE: return "Volumetric texture";
    case D3DRTYPE_CUBETEXTURE: return "Cubic texture";
    case D3DRTYPE_VERTEXBUFFER: return "Vertexbuffer";
    case D3DRTYPE_INDEXBUFFER: return "Indexbuffer";
  }
#endif

  return "Unknown";
}

const char *findFileformat(FILEFORMAT fmt) {
#ifdef	DX11
  switch (fmt) {
    case D3DX11_IFF_BMP: return "Windows Bitmap";
    case D3DX11_IFF_JPG: return "JPEG";
    case D3DX11_IFF_PNG: return "PNG";
    case D3DX11_IFF_DDS: return "DirectX texture";
    case D3DX11_IFF_TIFF: return "TIFF";
    case D3DX11_IFF_GIF: return "GIF";
    case D3DX11_IFF_WMP: return "Windows HDR";
  }
#else
  switch (fmt) {
    case D3DXIFF_BMP: return "Windows Bitmap";
    case D3DXIFF_JPG: return "JPEG";
    case D3DXIFF_TGA: return "Targa";
    case D3DXIFF_PNG: return "PNG";
    case D3DXIFF_DDS: return "DirectX texture";
    case D3DXIFF_PPM: return "NetPBM PPM";
    case D3DXIFF_DIB: return "DIB";
    case D3DXIFF_HDR: return "Radiance HDR";
    case D3DXIFF_PFM: return "NetPBM PFM";
  }
#endif

  return "Unknown";
}

} // namespace squash

/* ####################################################################################
 */

namespace squash {

#ifdef	DX11
#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"d3dx11.lib")

ID3D11Device *pD3DDevice = NULL;
ID3D11DeviceContext *pD3DDeviceContext = NULL;
IDXGISwapChain *pD3DSwapChain = NULL;

bool TextureInit() {
  D3D_FEATURE_LEVEL flevels[2] = { D3D_FEATURE_LEVEL_11_0 }, flevel;
//DXGI_SWAP_CHAIN_DESC swapChainDesc;

  if (pD3DDevice && pD3DDeviceContext)
    return true;

  /* can you believe A8B8G8R8 -> A8R8G8B8 needs hardware? */
  HRESULT res;
  if ((res = D3D11CreateDevice(
	NULL,
	D3D_DRIVER_TYPE_HARDWARE,
	NULL,
	D3D11_CREATE_DEVICE_SINGLETHREADED,
	flevels,
	1,
	D3D11_SDK_VERSION,
	&pD3DDevice,
	&flevel,
	&pD3DDeviceContext)) != D3D_OK) {
    if (pD3DDeviceContext)
      pD3DDeviceContext->Release();
    if (pD3DSwapChain)
      pD3DSwapChain->Release();
    if (pD3DDevice)
      pD3DDevice->Release();

    pD3DDeviceContext = NULL;
    pD3DSwapChain = NULL;
    pD3DDevice = NULL;

    return false;
  }

#if 0
  ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

  swapChainDesc.BufferCount = 1;
  swapChainDesc.BufferDesc.Width = 1;
  swapChainDesc.BufferDesc.Height = 1;
  swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
  swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
  swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
  swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
  swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  swapChainDesc.Flags = 0;
  swapChainDesc.OutputWindow = NULL;
  swapChainDesc.SampleDesc.Count = 1;
  swapChainDesc.SampleDesc.Quality = 0;
  swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
  swapChainDesc.Windowed = TRUE;

  if ((res = D3D11CreateDeviceAndSwapChain(
  	NULL,
  	D3D_DRIVER_TYPE_HARDWARE,
  	NULL,
  	D3D11_CREATE_DEVICE_SINGLETHREADED,
	flevels,
	1,
	D3D11_SDK_VERSION,
	&swapChainDesc,
	&pD3DSwapChain,
	&pD3DDevice,
	&flevel,
	&pD3DDeviceContext)) != D3D_OK) {
    if (pD3DDeviceContext)
      pD3DDeviceContext->Release();
    if (pD3DSwapChain)
      pD3DSwapChain->Release();
    if (pD3DDevice)
      pD3DDevice->Release();

    pD3DDeviceContext = NULL;
    pD3DSwapChain = NULL;
    pD3DDevice = NULL;

    return false;
  }
#endif

  return true;
}

void TextureCleanup() {
  if (pD3DDeviceContext)
    pD3DDeviceContext->Release(),
    pD3DDeviceContext = NULL;
  if (pD3DSwapChain)
    pD3DSwapChain->Release(),
    pD3DSwapChain = NULL;
  if (pD3DDevice)
    pD3DDevice->Release(),
    pD3DDevice = NULL;
}
#else
IDirect3D9 *pD3D = NULL;
IDirect3DDevice9 *pD3DDevice = NULL;

bool TextureInit() {
  D3DPRESENT_PARAMETERS Parameters;
  D3DDISPLAYMODE Mode;

  if (pD3D && pD3DDevice)
    return true;
  if (!pD3D && !(pD3D = Direct3DCreate9(D3D_SDK_VERSION)))
    return false;

  pD3D->GetAdapterDisplayMode(0, &Mode);

  memset(&Parameters, 0, sizeof(Parameters));

  Parameters.BackBufferWidth  = 1;
  Parameters.BackBufferHeight = 1;
  Parameters.BackBufferFormat = Mode.Format;
  Parameters.BackBufferCount  = 1;
  Parameters.SwapEffect       = D3DSWAPEFFECT_COPY;
  Parameters.Windowed         = TRUE;

  /* can you believe A8B8G8R8 -> A8R8G8B8 needs hardware? */
  HRESULT res;
  if ((res = pD3D->CreateDevice(
    D3DADAPTER_DEFAULT,
    D3DDEVTYPE_NULLREF,
//  D3DDEVTYPE_REF,
//  D3DDEVTYPE_SW,
//  D3DDEVTYPE_HAL,
    GetConsoleWindow(),
    D3DCREATE_MULTITHREADED |
    D3DCREATE_SOFTWARE_VERTEXPROCESSING,
//  D3DCREATE_HARDWARE_VERTEXPROCESSING,
    &Parameters,
    &pD3DDevice
  )) != D3D_OK) {
    pD3D->Release();
    pD3D = NULL;
    pD3DDevice = NULL;

    return false;
  }

  return true;
}

void TextureCleanup() {
  if (pD3DDevice)
    pD3DDevice->Release();
    pD3DDevice = NULL;
  if (pD3D)
    pD3D->Release();
    pD3D = NULL;
}
#endif

} // namespace squash

/* ####################################################################################
 */

namespace squash {
  
#define	MIPMAP_ROUND		0	// round down, throw away 1 line for non-pow2
#define	MIPMAP_MINIMUM		1	// 4
#define	MIPMAP_CHECK(w, h)	(!leavemiplevels	\
  ? (ww > MIPMAP_MINIMUM) && (hh > MIPMAP_MINIMUM)	\
  : (ww > MIPMAP_MINIMUM) || (hh > MIPMAP_MINIMUM))

int TextureCalcMip(int w, int h, int minlvl) {
  int levels = 1;
  
  /* the lowest mip-level contains a row or a column of 4x4 blocks
   * we won't generate mip-levels for mips smaller than the BTC-area
   */
  int ww = w;
  int hh = h;
  while (MIPMAP_CHECK(ww, hh)) {
    ww = (ww + MIPMAP_ROUND) >> 1;
    hh = (hh + MIPMAP_ROUND) >> 1;

    levels++;
  }

  if (minlvl < 0)
    minlvl = -minlvl, levels = min(minlvl, levels);
  assert(levels >= 0);

  return levels;
}

int TextureCalcMip(int w, int h, int lw, int lh) {
  int baselvl = 0;

  int ww = w;
  int hh = h;
  while ((ww > lw) || (hh > lh)) {
    ww = (ww + MIPMAP_ROUND) >> 1;
    hh = (hh + MIPMAP_ROUND) >> 1;

    baselvl++;
  }

  return baselvl;
}

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

bool TextureInfo(void *inmem, UINT insize, TEXINFO &nfo) {
#ifdef DX11
  return (D3DX11GetImageInfoFromMemory(inmem, insize, NULL, &nfo, NULL) == S_OK);
#else
  /* ATI1/2 without hardware, mem-layout (alignment etc.) is the same as DXT1/3 */
  if ((((DDS_HEADER *)inmem)->ddspf.dwFourCC == D3DFMT_ATI1) ||
      (((DDS_HEADER *)inmem)->ddspf.dwFourCC == D3DFMT_ATI2)) {

    nfo.Width     = ((DDS_HEADER *)inmem)->dwWidth;
    nfo.Height    = ((DDS_HEADER *)inmem)->dwHeight;
    nfo.Depth     = ((DDS_HEADER *)inmem)->dwDepth;
    nfo.MipLevels = ((DDS_HEADER *)inmem)->dwMipMapCount;

    nfo.Format = (D3DFORMAT)((DDS_HEADER *)inmem)->ddspf.dwFourCC;
    nfo.ImageFileFormat = D3DXIFF_DDS;
    nfo.ResourceType = D3DRTYPE_TEXTURE;

    return true;
  }

  return (D3DXGetImageInfoFromFileInMemory(inmem, insize, &nfo) == D3D_OK);
#endif
}

bool TextureInfoLevel(LPDIRECT3DTEXTURE tex, RESOURCEINFO &nfo, int lvl) {
#ifdef DX11
  ID3D11Texture2D *plane = (ID3D11Texture2D *)tex;
  plane->GetDesc(&nfo);

  for (int l = 0; l < lvl; l++) {
    nfo.Width  = (nfo.Width  + MIPMAP_ROUND) >> 1;
    nfo.Height = (nfo.Height + MIPMAP_ROUND) >> 1;
  }

  return true;
#else
  return (tex->GetLevelDesc(lvl, &nfo) == D3D_OK);
#endif
}

bool TextureInfoLevel(LPDIRECT3DCUBETEXTURE tex, RESOURCEINFO &nfo, int fce, int lvl) {
#ifdef DX11
  ID3D11Texture2D *plane = (ID3D11Texture2D *)tex;
  plane->GetDesc(&nfo);

  for (int l = 0; l < lvl; l++) {
    nfo.Width  = (nfo.Width  + MIPMAP_ROUND) >> 1;
    nfo.Height = (nfo.Height + MIPMAP_ROUND) >> 1;
  }

  return true;
#else
  return (tex->GetLevelDesc(lvl, &nfo) == D3D_OK);
#endif
}

ULONG *TextureLock(LPDIRECT3DTEXTURE tex, int lvl, ULONG *pitch, bool writable) {
  TEXMEMORY texs;

#ifdef DX11
  pD3DDeviceContext->Map(tex, lvl, (writable ? D3D11_MAP_READ_WRITE : D3D11_MAP_READ), 0, &texs);
  ULONG *sTex = (ULONG *)texs.pData;
  if (pitch) *pitch = texs.RowPitch;
#else
  tex->LockRect(lvl, &texs, NULL, 0);
  ULONG *sTex = (ULONG *)texs.pBits;
  if (pitch) *pitch = texs.Pitch;
#endif

  return sTex;
}

void TextureUnlock(LPDIRECT3DTEXTURE tex, int lvl) {
#ifdef DX11
  pD3DDeviceContext->Unmap(tex, lvl);
#else
  tex->UnlockRect(lvl);
#endif
}

ULONG *TextureLock(LPDIRECT3DCUBETEXTURE tex, int fce, int lvl, ULONG *pitch, bool writable) {
  TEXMEMORY texs;

#ifdef DX11
  UINT sub = D3D11CalcSubresource(lvl, fce, ?);
  pD3DDeviceContext->Map(tex, sub, (writable ? D3D11_MAP_READ_WRITE : D3D11_MAP_READ), 0, &texs);
  ULONG *sTex = (ULONG *)texs.pData;
  if (pitch) *pitch = texs.RowPitch;
#else
  tex->LockRect((D3DCUBEMAP_FACES)fce, lvl, &texs, NULL, 0);
  ULONG *sTex = (ULONG *)texs.pBits;
  if (pitch) *pitch = texs.Pitch;
#endif

  return sTex;
}

void TextureUnlock(LPDIRECT3DCUBETEXTURE tex, int fce, int lvl) {
#ifdef DX11
  UINT sub = D3D11CalcSubresource(lvl, fce, ?);
  pD3DDeviceContext->Unmap(tex, lvl);
#else
  tex->UnlockRect((D3DCUBEMAP_FACES)fce, lvl);
#endif
}

} // namespace squash

/* ####################################################################################
 */

namespace squash {

bool TextureConvert(RESOURCEINFO &info, LPDIRECT3DTEXTURE *tex, bool black) {
  LPDIRECT3DTEXTURE replct;
  HRESULT res;

#ifdef DX11
  ID3D11Texture2D *rtex; RESOURCEINFO cr;
  memset(&cr, 0, sizeof(cr));

  cr.Width  = info.Width;
  cr.Height = info.Height;
  cr.MipLevels = 1;
  cr.ArraySize = info.ArraySize;

  cr.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  cr.Usage = D3D11_USAGE_STAGING;
  cr.BindFlags = 0;
  cr.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
  cr.SampleDesc.Count = 1;

  res = pD3DDevice->CreateTexture2D(&cr, NULL, &rtex);
  if (res != S_OK)
    return false;

  D3DX11_TEXTURE_LOAD_INFO load;
  memset(&load, 0, sizeof(load));

  load.SrcFirstMip = 0;
  load.DstFirstMip = 0;
  load.NumMips = 1;
  load.NumElements = D3DX11_DEFAULT;
  load.Filter = D3DX11_FILTER_NONE;
  load.MipFilter = D3DX11_FILTER_NONE;

  res = D3DX11LoadTextureFromTexture(pD3DDeviceContext, *tex, &load, rtex); replct = rtex;
#else
//pD3DDevice->CreateTexture(info.Width, info.Height, 0, 0, D3DFMT_A8B8G8R8, D3DPOOL_SYSTEMMEM, &replct, NULL);
//pD3DDevice->CreateTexture(info.Width, info.Height, 0, 0, D3DFMT_A8B8G8R8, D3DPOOL_DEFAULT, &replct, NULL);
//pD3DDevice->CreateTexture(info.Width, info.Height, 1, 0, D3DFMT_A8B8G8R8, D3DPOOL_DEFAULT, &replct, NULL);
//pD3DDevice->CreateTexture(info.Width, info.Height, 1, D3DUSAGE_AUTOGENMIPMAP, D3DFMT_A8B8G8R8, D3DPOOL_DEFAULT, &replct, NULL);
//pD3DDevice->CreateTexture(info.Width, info.Height, 1, D3DUSAGE_AUTOGENMIPMAP, D3DFMT_A8B8G8R8, D3DPOOL_DEFAULT, &replct, NULL);
//pD3DDevice->CreateTexture(info.Width, info.Height, 1, D3DUSAGE_AUTOGENMIPMAP, D3DFMT_A8B8G8R8, D3DPOOL_SYSTEMMEM, &replct, NULL);
//pD3DDevice->CreateTexture(info.Width, info.Height, 0, 0, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &replct, NULL);
//pD3DDevice->CreateTexture(info.Width, info.Height, 0, 0, D3DFMT_A8B8G8R8, D3DPOOL_DEFAULT, &replct, NULL);
  res = pD3DDevice->CreateTexture(info.Width, info.Height, 0, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &replct, NULL);
  if (res != D3D_OK)
    return false;

  LPDIRECT3DSURFACE9 stex, srep;

  (*tex)->GetSurfaceLevel(0, &stex);
  replct->GetSurfaceLevel(0, &srep);

  res = D3DXLoadSurfaceFromSurface(srep, NULL, NULL, stex, NULL, NULL, D3DX_FILTER_NONE, 0);

  /* this is not right unfortunately L8 becomes L8A8 */
#if 0
  /* put a custom default alpha-value */
  if (!findAlpha(info.Format)) {
    D3DLOCKED_RECT texs;
    unsigned char a = (black ? 0x00000000 : 0xFF000000);

    replct->LockRect(0, &texs, NULL, 0);
    ULONG *sTex = (ULONG *)texs.pBits;

    for (int y = 0; y < (int)info.Height; y += 1) {
    for (int x = 0; x < (int)info.Width ; x += 1) {
      ULONG t = sTex[(y * info.Width) + x];
      sTex[(y * info.Width) + x] = (t & 0x00FFFFFF) | a;
    }
    }

    replct->UnlockRect(0);
  }
#endif

  stex->Release();
  srep->Release();
#endif

  if (res == D3D_OK) {
    (*tex)->Release();
    (*tex) = replct;

    return true;
  }
  else {
    replct->Release();
    replct = (*tex);

    return false;
  }
}

bool TextureFillMip(int w, int h, LPDIRECT3DTEXTURE *tex, bool dither, bool gamma, int levels) {
  LPDIRECT3DTEXTURE replct;
  RESOURCEINFO texo;
  HRESULT res;

  TextureInfoLevel((*tex), texo, 0);

#ifdef DX11
  ID3D11Texture2D *rtex; RESOURCEINFO cr;
  memset(&cr, 0, sizeof(cr));

  cr.Width  = texo.Width;
  cr.Height = texo.Height;
  cr.MipLevels = levels;
  cr.ArraySize = texo.ArraySize;

  cr.Format = texo.Format;
  cr.Usage = D3D11_USAGE_STAGING;
  cr.BindFlags = 0;
  cr.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
  cr.SampleDesc.Count = 1;

  res = pD3DDevice->CreateTexture2D(&cr, NULL, &rtex); replct = rtex;
  if (res != S_OK)
    return false;

  UINT filter = D3DX11_FILTER_BOX | (dither ? D3DX11_FILTER_DITHER : 0) | (gamma ? D3DX11_FILTER_SRGB : 0);
  D3DX11_TEXTURE_LOAD_INFO load;
  memset(&load, 0, sizeof(load));

  load.SrcFirstMip = 0;
  load.DstFirstMip = 0;
  load.NumMips = 1;
  load.NumElements = D3DX11_DEFAULT;
  load.Filter = filter;
  load.MipFilter = filter;

  res = D3DX11LoadTextureFromTexture(pD3DDeviceContext, (*tex), &load, rtex); replct = rtex;
#else
  if ((res = pD3DDevice->CreateTexture(texo.Width, texo.Height, levels, 0, texo.Format, D3DPOOL_MANAGED, &replct, NULL)) != D3D_OK)
    return false;

  DWORD filter = D3DX_FILTER_BOX | (dither ? D3DX_FILTER_DITHER : 0) | (gamma ? D3DX_FILTER_SRGB : 0);
  LPDIRECT3DSURFACE9 stex, srep;

  (*tex)->GetSurfaceLevel(0, &stex);
  replct->GetSurfaceLevel(0, &srep);

  res = D3DXLoadSurfaceFromSurface(srep, NULL, NULL, stex, NULL, NULL, filter, 0);

  stex->Release();
  srep->Release();
#endif

  if (res == D3D_OK) {
    (*tex)->Release();
    (*tex) = replct;

#ifdef DX11
    if (D3DX11FilterTexture(pD3DDeviceContext, (*tex), 1, filter) == S_OK)
      return true;
#else
    if (D3DXFilterTexture((*tex), NULL, 0, filter) == D3D_OK)
      return true;
#endif

    return false;
  }
  else {
    replct->Release();
    replct = (*tex);

    return false;
  }
}

bool TextureDownMip(int w, int h, LPDIRECT3DTEXTURE *tex) {
  LPDIRECT3DTEXTURE replct;
  RESOURCEINFO texo;
  HRESULT res;

  /* get original size */
  TextureInfoLevel(*tex, texo, 0);
  
  /* calculate how much levels to strip */
  int levels  = TextureCalcMip(w, h, 0);
  int baselvl = TextureCalcMip(texo.Width, texo.Height, w, h);
  int trnslvl = 0;
  
  /* create a new textures with the size of the selected mip */
  TextureInfoLevel(*tex, texo, baselvl);

  /* nothing to do */
  if (baselvl == trnslvl)
    return true;
  
#ifdef DX11
  ID3D11Texture2D *rtex; RESOURCEINFO cr;
  memset(&cr, 0, sizeof(cr));

  cr.Width  = texo.Width;
  cr.Height = texo.Height;
  cr.MipLevels = 1;
  cr.ArraySize = texo.ArraySize;

  cr.Format = texo.Format;
  cr.Usage = D3D11_USAGE_STAGING;
  cr.BindFlags = 0;
  cr.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
  cr.SampleDesc.Count = 1;

  res = pD3DDevice->CreateTexture2D(&cr, NULL, &rtex); replct = rtex;
  if (res != S_OK)
    return false;

  D3DX11_TEXTURE_LOAD_INFO load;
  memset(&load, 0, sizeof(load));

  load.SrcFirstMip = 0;
  load.DstFirstMip = 0;
  load.NumMips = 1;
  load.NumElements = D3DX11_DEFAULT;
  load.Filter = D3DX11_FILTER_NONE;
  load.MipFilter = D3DX11_FILTER_NONE;

  while (trnslvl < levels) {
    load.SrcFirstMip = baselvl;
    load.DstFirstMip = trnslvl;

    if ((res = D3DX11LoadTextureFromTexture(pD3DDeviceContext, *tex, &load, rtex)) != S_OK)
      break;

    baselvl++;
    trnslvl++;
  }
#else
  if ((res = D3DXCreateTexture(
    pD3DDevice,
    texo.Width, texo.Height, 0,
    0, texo.Format, D3DPOOL_SYSTEMMEM, &replct
  )) != D3D_OK)
    return false;

  while (trnslvl < levels) {
    LPDIRECT3DSURFACE9 stex, srep;

    if ((res = (*tex)->GetSurfaceLevel(baselvl, &stex)) != D3D_OK)
      break;
    if ((res = replct->GetSurfaceLevel(trnslvl, &srep)) != D3D_OK)
      break;

    if ((res = D3DXLoadSurfaceFromSurface(srep, NULL, NULL, stex, NULL, NULL, D3DX_FILTER_NONE, 0)) != D3D_OK)
      break;

    stex->Release();
    srep->Release();

    baselvl++;
    trnslvl++;
  }
#endif

  if (res == D3D_OK) {
    (*tex)->Release();
    (*tex) = replct;

    return true;
  }
  else {
    replct->Release();
    replct = (*tex);

    return false;
  }
}

bool TextureConvert(int minlevel, LPDIRECT3DTEXTURE *tex, bool dither, bool gamma, TEXFORMAT target) {
  LPDIRECT3DTEXTURE replct;
  RESOURCEINFO texo;
  HRESULT res;

  TextureInfoLevel(*tex, texo, 0);
  
  /* create the textures */
  int levels = TextureCalcMip(texo.Width, texo.Height, minlevel);

#ifdef DX11
  ID3D11Texture2D *rtex; RESOURCEINFO cr;
  memset(&cr, 0, sizeof(cr));

  cr.Width  = texo.Width;
  cr.Height = texo.Height;
  cr.MipLevels = levels;
  cr.ArraySize = texo.ArraySize;

  cr.Format = target;
  cr.Usage = D3D11_USAGE_STAGING;
  cr.BindFlags = 0;
  cr.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
  cr.SampleDesc.Count = 1;

  res = pD3DDevice->CreateTexture2D(&cr, NULL, &rtex);
  if (res != S_OK)
    return false;

  UINT filter = D3DX11_FILTER_BOX | (dither ? D3DX11_FILTER_DITHER : 0) | (gamma ? D3DX11_FILTER_SRGB : 0);
  D3DX11_TEXTURE_LOAD_INFO load;
  memset(&load, 0, sizeof(load));

  load.SrcFirstMip = 0;
  load.DstFirstMip = 0;
  load.NumMips = 1;
  load.NumElements = D3DX11_DEFAULT;
  load.Filter = filter;
  load.MipFilter = filter;

  res = D3DX11LoadTextureFromTexture(pD3DDeviceContext, *tex, &load, rtex); replct = rtex;
#else
  res = pD3DDevice->CreateTexture(texo.Width, texo.Height, levels, 0, target, D3DPOOL_MANAGED, &replct, NULL);
  if (res != D3D_OK)
    return false;

  DWORD filter = D3DX_FILTER_BOX | (dither ? D3DX_FILTER_DITHER : 0) | (gamma ? D3DX_FILTER_SRGB : 0);
  LPDIRECT3DSURFACE9 stex, srep;

  (*tex)->GetSurfaceLevel(0, &stex);
  replct->GetSurfaceLevel(0, &srep);

  res = D3DXLoadSurfaceFromSurface(srep, NULL, NULL, stex, NULL, NULL, filter, 0);

  stex->Release();
  srep->Release();
#endif

  if (res == D3D_OK) {
    (*tex)->Release();
    (*tex) = replct;

#ifdef DX11
    if (D3DX11FilterTexture(pD3DDeviceContext, (*tex), 1, filter) == S_OK)
      return true;
#else
    if (D3DXFilterTexture((*tex), NULL, 0, filter) == D3D_OK)
      return true;
#endif

    return false;
  }
  else {
    replct->Release();
    replct = (*tex);

    return false;
  }
}

bool TextureCollapse(LPDIRECT3DTEXTURE *tex, TEXFORMAT target, bool swizzle) {
  LPDIRECT3DTEXTURE replct;
  HRESULT res;

#ifdef DX11
  ID3D11Texture2D *rtex; RESOURCEINFO cr;
  memset(&cr, 0, sizeof(cr));

  cr.Width  = 1;
  cr.Height = 1;
  cr.MipLevels = 1;
  cr.ArraySize = texo.ArraySize;

  cr.Format = target;
  cr.Usage = D3D11_USAGE_STAGING;
  cr.BindFlags = 0;
  cr.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
  cr.SampleDesc.Count = 1;

  res = pD3DDevice->CreateTexture2D(&cr, NULL, &rtex);
  if (res != S_OK)
    return false;

  UINT filter = D3DX11_FILTER_BOX | D3DX11_FILTER_DITHER;
  D3DX11_TEXTURE_LOAD_INFO load;
  memset(&load, 0, sizeof(load));

  D3D11_BOX oo = {0,0,0,1,1,0};
  load.pSrcBox = &oo;
  load.pDstBox = &oo;
  load.SrcFirstMip = 0;
  load.DstFirstMip = 0;
  load.NumMips = 1;
  load.NumElements = D3DX11_DEFAULT;
  load.Filter = filter;
  load.MipFilter = filter;

  res = D3DX11LoadTextureFromTexture(pD3DDeviceContext, *tex, &load, rtex); replct = rtex;
#else
  res = pD3DDevice->CreateTexture(1, 1, 0, 0, target, D3DPOOL_MANAGED, &replct, NULL);
  if (res != D3D_OK)
    return false;

  DWORD filter = D3DX_FILTER_BOX | D3DX_FILTER_DITHER;
  LPDIRECT3DSURFACE9 stex, srep;

  (*tex)->GetSurfaceLevel(0, &stex);
  replct->GetSurfaceLevel(0, &srep);

  RECT oo = {0,0,1,1};
  res = D3DXLoadSurfaceFromSurface(srep, NULL, &oo, stex, NULL, &oo, filter, 0);

  stex->Release();
  srep->Release();
#endif

  if (res == D3D_OK) {
    (*tex)->Release();
    (*tex) = replct;

    /* put a custom default alpha-value */
    if (swizzle) {
      ULONG sPch, *sTex = TextureLock(replct, 0, &sPch, true);

      /* swizzle ARGB -> ARBG */
      ULONG t = sTex[0];

      sTex[0] = (t & 0xFFFF0000) | ((t >> 8) & 0x00FF) | ((t & 0x00FF) << 8);
//    sTex[0] = (t & 0xFF0000FF) | ((t >> 8) & 0xFF00) | ((t & 0xFF00) << 8);

      TextureUnlock(replct, 0);
    }

    return true;
  }
  else {
    replct->Release();
    replct = (*tex);

    return false;
  }
}

} // namespace squash

// *********************************************************************************************************

#ifndef DX11
static const DWORD DDSD_CAPS = 0x00000001U;
static const DWORD DDSD_PIXELFORMAT = 0x00001000U;
static const DWORD DDSD_WIDTH = 0x00000004U;
static const DWORD DDSD_HEIGHT = 0x00000002U;
static const DWORD DDSD_PITCH = 0x00000008U;
static const DWORD DDSD_MIPMAPCOUNT = 0x00020000U;
static const DWORD DDSD_LINEARSIZE = 0x00080000U;
static const DWORD DDSD_DEPTH = 0x00800000U;

static const DWORD DDSCAPS_COMPLEX = 0x00000008U;
static const DWORD DDSCAPS_TEXTURE = 0x00001000U;
static const DWORD DDSCAPS_MIPMAP = 0x00400000U;
static const DWORD DDSCAPS2_VOLUME = 0x00200000U;
static const DWORD DDSCAPS2_CUBEMAP = 0x00000200U;

static const DWORD DDSCAPS2_CUBEMAP_POSITIVEX = 0x00000400U;
static const DWORD DDSCAPS2_CUBEMAP_NEGATIVEX = 0x00000800U;
static const DWORD DDSCAPS2_CUBEMAP_POSITIVEY = 0x00001000U;
static const DWORD DDSCAPS2_CUBEMAP_NEGATIVEY = 0x00002000U;
static const DWORD DDSCAPS2_CUBEMAP_POSITIVEZ = 0x00004000U;
static const DWORD DDSCAPS2_CUBEMAP_NEGATIVEZ = 0x00008000U;
static const DWORD DDSCAPS2_CUBEMAP_ALL_FACES = 0x0000FC00U;

static const DWORD DDPF_ALPHAPIXELS = 0x00000001U;
static const DWORD DDPF_ALPHA = 0x00000002U;
static const DWORD DDPF_FOURCC = 0x00000004U;
static const DWORD DDPF_RGB = 0x00000040U;
static const DWORD DDPF_PALETTEINDEXED1 = 0x00000800U;
static const DWORD DDPF_PALETTEINDEXED2 = 0x00001000U;
static const DWORD DDPF_PALETTEINDEXED4 = 0x00000008U;
static const DWORD DDPF_PALETTEINDEXED8 = 0x00000020U;
static const DWORD DDPF_LUMINANCE = 0x00020000U;
static const DWORD DDPF_ALPHAPREMULT = 0x00008000U;
static const DWORD DDPF_NORMAL = 0x80000000U;	// @@ Custom nv flag.

static UINT BitsPerPixel(DWORD FourCC) {
  switch(FourCC) {
    case D3DFMT_ATI1:
    case D3DFMT_ATI2:
      return 8;

    default:
      assert( FALSE ); // unhandled format
      return 0;
  }
}

static void GetSurfaceInfo(UINT width, UINT height, DWORD FourCC, UINT *pNumBytes, UINT *pRowBytes, UINT *pNumRows) {
  UINT numBytes = 0;
  UINT rowBytes = 0;
  UINT numRows = 0;

  bool bc = true;
  int bcnumBytesPerBlock = 16;
  switch (FourCC) {
    case D3DFMT_ATI1:
      bcnumBytesPerBlock = 8;
      break;
    case D3DFMT_ATI2:
      bcnumBytesPerBlock = 16;
      break;
    default:
      bc = false;
      break;
  }

  if (bc) {
    int numBlocksWide = 0;
    if (width > 0)
      numBlocksWide = max(1, width / 4);
    int numBlocksHigh = 0;
    if (height > 0)
      numBlocksHigh = max(1, height / 4);

    rowBytes = numBlocksWide * bcnumBytesPerBlock;
    numRows = numBlocksHigh;
  }
  else {
    UINT bpp = BitsPerPixel(FourCC);
    rowBytes = (width * bpp + 7) / 8; // round up to nearest byte
    numRows = height;
  }

  numBytes = rowBytes * numRows;
  if (pNumBytes != NULL)
    *pNumBytes = numBytes;
  if (pRowBytes != NULL)
    *pRowBytes = rowBytes;
  if (pNumRows != NULL)
    *pNumRows = numRows;
}

LPDIRECT3DTEXTURE9 LoadBC45TextureFromFileInMemory(LPDIRECT3DDEVICE9 pDevice, const void *data, int size) {
  if ((((DDS_HEADER *)data)->ddspf.dwFourCC != D3DFMT_ATI1) &&
      (((DDS_HEADER *)data)->ddspf.dwFourCC != D3DFMT_ATI2))
    return NULL;

#if 0
  // Check if ATI1N is supported
  hr = lastOBGEDirect3D9->CheckDeviceFormat(AdapterOrdinal, DeviceType, AdapterFormat,
    0, D3DRTYPE_TEXTURE, D3DFMT_ATI1);
  BOOL bATI1NSupported = (hr == D3D_OK);
  • To check support for ATI2N:
  // Check if ATI2N is supported
  HRESULT hr;
  hr = lastOBGEDirect3D9->CheckDeviceFormat(AdapterOrdinal, DeviceType, AdapterFormat,
    0, D3DRTYPE_TEXTURE, D3DFMT_ATI2);
  BOOL bATI2NSupported = (hr == D3D_OK);
#endif

  LPDIRECT3DTEXTURE9 GPUTexture = NULL;
  HRESULT res;

  if (FAILED(res = pDevice->CreateTexture(
    ((DDS_HEADER *)data)->dwWidth,
    ((DDS_HEADER *)data)->dwHeight,
    ((DDS_HEADER *)data)->dwMipMapCount,
    0, /*(D3DFORMAT)
    ((DDS_HEADER *)data)->ddspf.dwFourCC*/
    D3DFMT_A8R8G8B8,
    D3DPOOL_MANAGED,
    &GPUTexture,
    NULL)))
    return NULL;

  // Lock, fill, unlock
  D3DSURFACE_DESC desc;
  D3DLOCKED_RECT LockedRect;
  UINT RowBytes, NumRows;
  BYTE *pSrcBits = (BYTE *)data + sizeof(DDS_HEADER);

  UINT iWidth = ((DDS_HEADER *)data)->dwWidth;
  UINT iHeight = ((DDS_HEADER *)data)->dwHeight;
  UINT iMipCount = ((DDS_HEADER *)data)->dwMipMapCount;
  DWORD iFourCC = ((DDS_HEADER *)data)->ddspf.dwFourCC;
  if (0 == iMipCount)
    iMipCount = 1;

  for (UINT i = 0; i < iMipCount; i++) {
    GetSurfaceInfo(iWidth, iHeight, iFourCC, NULL, &RowBytes, &NumRows);

    GPUTexture->GetLevelDesc(i, &desc);
    if (SUCCEEDED(GPUTexture->LockRect(i, &LockedRect, NULL, 0))) {
      BYTE *pDestBits = (BYTE *)LockedRect.pBits;

#if 1
//    int size = (iWidth * iHeight * (iFourCC == D3DFMT_ATI2 ? 2 : 1)) >> 1;
//    CopyMemory(pDestBits, pSrcBits, size);

      for (UINT y = 0; y < iHeight; y += 4U)
      for (UINT x = 0; x < iWidth ; x += 4U)
      for (UINT z = 0; z < (iFourCC == D3DFMT_ATI2 ? 2U : 1U); z += 1U) {
        // get the two alpha values
        unsigned char const* bytes = reinterpret_cast< unsigned char const* >( pSrcBits );
        int alpha0 = bytes[0];
        int alpha1 = bytes[1];

        // compare the values to build the codebook
        unsigned char codes[8];
        codes[0] = ( unsigned char )alpha0;
        codes[1] = ( unsigned char )alpha1;
        if( alpha0 <= alpha1 )
        {
          // use 5-alpha codebook
          for( int i = 1; i < 5; ++i )
            codes[1 + i] = ( unsigned char )( ( ( 5 - i )*alpha0 + i*alpha1 )/5 );
          codes[6] = 0;
          codes[7] = 255;
        }
        else
        {
          // use 7-alpha codebook
          for( int i = 1; i < 7; ++i )
            codes[1 + i] = ( unsigned char )( ( ( 7 - i )*alpha0 + i*alpha1 )/7 );
        }

        // decode the indices
        unsigned char indices[16];
        unsigned char const* src = bytes + 2;
        unsigned char* dest = indices;
        for( int i = 0; i < 2; ++i )
        {
          // grab 3 bytes
          int value = 0;
          for( int j = 0; j < 3; ++j )
          {
            int byte = *src++;
            value |= ( byte << 8*j );
          }

          // unpack 8 3-bit values from it
          for( int j = 0; j < 8; ++j )
          {
            int index = ( value >> 3*j ) & 0x7;
            *dest++ = ( unsigned char )index;
          }
        }

        // write out the indexed codebook values
        for( UINT i = 0; i < 16U; ++i ) {
          UINT yy = y + (i >> 2);
          UINT xx = x + (i  & 3);

          if (yy < desc.Height)
          if (xx < desc.Width ) {
//          if (iFourCC == D3DFMT_ATI2) {
              pDestBits[((yy * desc.Width) + xx) * 4 +   z + 1      ] = codes[indices[i]];
              pDestBits[((yy * desc.Width) + xx) * 4 + ((z + 3) & 3)] = 0xFF;
//          }
          }
        }

	pSrcBits += 8;
	RowBytes -= 8;
      }

//    pDestBits += size;
//    pSrcBits += size;
#else
      // Copy stride line by line
      for (UINT h = 0; h < NumRows; h++) {
	CopyMemory(pDestBits, pSrcBits, RowBytes);

	pDestBits += LockedRect.Pitch;
	pSrcBits += RowBytes;
      }
#endif

      GPUTexture->UnlockRect(i);
    }

    iWidth  = (iWidth  + 1) >> 1;
    iHeight = (iHeight + 1) >> 1;
  }

  return GPUTexture;
}

HRESULT SaveBC45TextureToFileInMemory(LPD3DBUFFER *oubuf, LPDIRECT3DBASETEXTURE9 pSrcTexture) {
  D3DSURFACE_DESC desc; int levels =
  ((IDirect3DTexture9 *)pSrcTexture)->GetLevelCount();
  ((IDirect3DTexture9 *)pSrcTexture)->GetLevelDesc(0, &desc);

  if (desc.Format == D3DFMT_DXT1)
    desc.Format = D3DFMT_ATI1;
  if (desc.Format == D3DFMT_DXT3)
    desc.Format = D3DFMT_ATI2;

  DDS_HEADER head; memset(&head, 0, sizeof(head));

  head.dwMagicNumber = MAKEFOURCC('D', 'D', 'S', ' ');
  head.dwSize = 0x7C;
  head.dwHeaderFlags =
    DDSD_CAPS |
    DDSD_WIDTH |
    DDSD_HEIGHT |
    DDSD_MIPMAPCOUNT |
//  DDSD_LINEARSIZE |
    DDSD_PIXELFORMAT;
  head.dwWidth = desc.Width;
  head.dwHeight = desc.Height;
  head.dwMipMapCount = levels;
//head.dwPitchOrLinearSize = (desc.Width * desc.Height * (desc.Format == D3DFMT_ATI2 ? 2 : 1)) >> 1;
  head.ddspf.dwFourCC = desc.Format;
  head.ddspf.dwFlags = DDPF_FOURCC;
  head.dwSurfaceFlags = (levels > 1 ? DDSCAPS_MIPMAP : 0) | DDSCAPS_TEXTURE | DDSCAPS_COMPLEX;
// (desc.Format == D3DFMT_ATI2 ? DDPF_NORMAL : 0);

  // Lock, fill, unlock
  D3DLOCKED_RECT LockedRect;
  UINT RowBytes, NumRows;

  UINT iWidth = head.dwWidth;
  UINT iHeight = head.dwHeight;
  UINT iMipCount = head.dwMipMapCount;
  DWORD iFourCC = head.ddspf.dwFourCC;
  if (0 == iMipCount)
    iMipCount = 1;

  int blobsize = sizeof(head);
  for (UINT i = 0; i < iMipCount; i++) {
    GetSurfaceInfo(iWidth, iHeight, iFourCC, NULL, &RowBytes, &NumRows);

    blobsize += RowBytes * NumRows;

    iWidth  = (iWidth  + 1) >> 1;
    iHeight = (iHeight + 1) >> 1;
  }

  if (D3DXCreateBuffer(blobsize, oubuf) != D3D_OK)
    return S_FALSE;
  BYTE *pDstBits = (BYTE *)(*oubuf)->GetBufferPointer();

  iWidth = head.dwWidth;
  iHeight = head.dwHeight;

  /* copy head */
  memcpy(pDstBits, &head, sizeof(head));
  pDstBits += sizeof(head);

  for (UINT i = 0; i < iMipCount; i++) {
    GetSurfaceInfo(iWidth, iHeight, iFourCC, NULL, &RowBytes, &NumRows);

    if (SUCCEEDED(((IDirect3DTexture9 *)pSrcTexture)->LockRect(i, &LockedRect, NULL, 0))) {
      BYTE *pSrcBits = (BYTE *)LockedRect.pBits;

      // Write stride line by line
      for (UINT h = 0; h < NumRows; h++) {
        memcpy(pDstBits, pSrcBits, RowBytes);

	pDstBits += RowBytes;
	pSrcBits += RowBytes;
      }

      ((IDirect3DTexture9 *)pSrcTexture)->UnlockRect(i);
    }

    iWidth  = (iWidth  + 1) >> 1;
    iHeight = (iHeight + 1) >> 1;
  }

  return D3D_OK;
}

HRESULT SaveBC45TextureToFile(const char *fp, LPDIRECT3DBASETEXTURE9 pSrcTexture) {
  FILE *f;
  if (fopen_s(&f, fp, "wb"))
    return S_FALSE;

  D3DSURFACE_DESC desc; int levels =
  ((IDirect3DTexture9 *)pSrcTexture)->GetLevelCount();
  ((IDirect3DTexture9 *)pSrcTexture)->GetLevelDesc(0, &desc);

  if (desc.Format == D3DFMT_DXT1)
    desc.Format = D3DFMT_ATI1;
  if (desc.Format == D3DFMT_DXT3)
    desc.Format = D3DFMT_ATI2;

  DDS_HEADER head; memset(&head, 0, sizeof(head));

  head.dwMagicNumber = MAKEFOURCC('D', 'D', 'S', ' ');
  head.dwSize = 0x7C;
  head.dwHeaderFlags =
    DDSD_CAPS |
    DDSD_WIDTH |
    DDSD_HEIGHT |
    DDSD_MIPMAPCOUNT |
//  DDSD_LINEARSIZE |
    DDSD_PIXELFORMAT;
  head.dwWidth = desc.Width;
  head.dwHeight = desc.Height;
  head.dwMipMapCount = levels;
//head.dwPitchOrLinearSize = (desc.Width * desc.Height * (desc.Format == D3DFMT_ATI2 ? 2 : 1)) >> 1;
  head.ddspf.dwFourCC = desc.Format;
  head.ddspf.dwFlags = DDPF_FOURCC;
  head.dwSurfaceFlags = (levels > 1 ? DDSCAPS_MIPMAP : 0) | DDSCAPS_TEXTURE | DDSCAPS_COMPLEX;
// (desc.Format == D3DFMT_ATI2 ? DDPF_NORMAL : 0);

  fwrite(&head, 1, sizeof(head), f);

  // Lock, fill, unlock
  D3DLOCKED_RECT LockedRect;
  UINT RowBytes, NumRows;

  UINT iWidth = head.dwWidth;
  UINT iHeight = head.dwHeight;
  UINT iMipCount = head.dwMipMapCount;
  DWORD iFourCC = head.ddspf.dwFourCC;
  if (0 == iMipCount)
    iMipCount = 1;

  for (UINT i = 0; i < iMipCount; i++) {
    GetSurfaceInfo(iWidth, iHeight, iFourCC, NULL, &RowBytes, &NumRows);

    if (SUCCEEDED(((IDirect3DTexture9 *)pSrcTexture)->LockRect(i, &LockedRect, NULL, 0))) {
      BYTE *pSrcBits = (BYTE *)LockedRect.pBits;

      // Write stride line by line
      for (UINT h = 0; h < NumRows; h++) {
	fwrite(pSrcBits, 1, RowBytes, f);

	pSrcBits += RowBytes;
      }

      ((IDirect3DTexture9 *)pSrcTexture)->UnlockRect(i);
    }

    iWidth  = (iWidth  + 1) >> 1;
    iHeight = (iHeight + 1) >> 1;
  }

  fclose(f);

  return D3D_OK;
}

#endif
