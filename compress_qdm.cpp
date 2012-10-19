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
//#include "squish.h"

/* tile-dimensions (free to choose) */
#define	TX	4
#define	TY	4

/* ------------------------------------------------------------------------------------
 */

#if 0
template<typename UTYPE, typename type, const bool LODed>
static bool TextureCompressQDM(LPDIRECT3DTEXTURE *base, LPDIRECT3DTEXTURE *norm, int minlevel) {
  LPDIRECT3DTEXTURE baset;
  LPDIRECT3DTEXTURE normt;
  RESOURCEINFO based, baseo;
  RESOURCEINFO normd, normo;

  TextureInfoLevel(*base, baseo, 0);
  TextureInfoLevel(*norm, normo, 0);

#if 0
  /* Converts a height map into a normal map. The (x,y,z)
   * components of each normal are mapped to the (r,g,b)
   * channels of the output texture.
   */
  HRESULT D3DXComputeNormalMap(
    __out  LPDIRECT3DTEXTURE pTexture,
    __in   LPDIRECT3DTEXTURE pSrcTexture,
    __in   const PALETTEENTRY *pSrcPalette,
    __in   DWORD Flags,
    __in   DWORD Channel,
    __in   FLOAT Amplitude
    );
#endif

  /* they have to have the same dimension */
  if ((baseo.Width  != normo.Width ) ||
      (baseo.Height != normo.Height))
    return false;

  /* convert to ARGB8 (TODO: support at least the 16bit formats as well) */
  if ((baseo.Format != TEXFMT_A8B8G8R8) && !TextureConvert(baseo, base, false))
    return false;
  if ((normo.Format != TEXFMT_A8B8G8R8) && !TextureConvert(normo, norm, true))
    return false;

  /* create the textures */
  int levels = TextureCalcMip(baseo.Width, baseo.Height, int minlevel);
  int flags = squish::kColourIterativeClusterFit | squish::kBct3;

#ifdef DX11
  ULONG *bases;
  ULONG *norms;

  DWORD basel = 1;
  DWORD norml = 1;
  DWORD level = max(basel, norml);
#else
  /* create the textures */
  pD3DDevice->CreateTexture(baseo.Width, baseo.Height, levels, 0, D3DFMT_DXT5, D3DPOOL_SYSTEMMEM, &baset, NULL);
  pD3DDevice->CreateTexture(normo.Width, normo.Height, levels, 0, D3DFMT_DXT5, D3DPOOL_SYSTEMMEM, &normt, NULL);

  /* damit */
  if (!baset || !normt) {
    if (baset) baset->Release();
    if (normt) normt->Release();

    return false;
  }

  ULONG bPch, *bases = TextureLock(*base, &bPch, 0);
  ULONG nPch, *norms = TextureLock(*norm, &nPch, 0);

  DWORD basel = baset->GetLevelCount();
  DWORD norml = normt->GetLevelCount();
  DWORD level = max(basel, norml);
#endif

  for (unsigned int l = 0; l < level; l++) {
    /* square dimension of this surface-level */
    /* square area of this surface-level */
    int lv = (1 << l);
    int av = lv * lv;

    TextureInfoLevel(baset, based, l);
    TextureInfoLevel(normt, normd, l);

    ULONG *baser = TextureLock(baset, l, true);
    ULONG *normr = TextureLock(normt, l, true);

    ULONG *sBase = (ULONG *)bases;
    ULONG *sNorm = (ULONG *)norms;
    ULONG *dBase = (ULONG *)baser;
    ULONG *dNorm = (ULONG *)normr;

    /* loop over 4x4-blocks of this level (DXT5) */
    for (unsigned int y = 0; y < based.Height; y += TY) {
      if (!(y & 0x3F)) {
	logrf("line processed %d/%d of level %d/%d\r", y, based.Height, l, level);

//	PollProgress();
      }

    for (unsigned int x = 0; x < based.Width; x += TX) {
      UTYPE bBase[2][TY][TX];
      ULONG bNorm[2][TY][TX];
      type  fBase[2][TY][TX][DIM];
      float fNorm[2][TY][TX][DIM];

      /* generate this level's 4x4-block from the original surface */
      for (int ly = 0; ly < TY; ly += 1)
      for (int lx = 0; lx < TX; lx += 1) {
	type  bs[DIM] = {0}; int yl = ((y + ly) << l);
	long  ns[DIM] = {0}; int xl = ((x + lx) << l);
	float nn[DIM] = {0.0f};

	/* access all pixels this level's 4x4-block represents in
	 * the full dimension original surface (high quality mip-mapping)
	 */
	for (int oy = 0; oy < lv; oy += 1)
	for (int ox = 0; ox < lv; ox += 1) {
	  /* assume seamless tiling: wrap pixels around */
	  int posx = (xl + ox) % baseo.Width;
	  int posy = (yl + oy) % baseo.Height;

	  ULONG &b = sBase[(posy * sPch) + posx];
	  ULONG &n = sNorm[(posy * nPch) + posx];

	  /* transfer heightmap into the normal-map (overwrite) */
	  if (LODed)
	    n = (n & 0x00FFFFFF) | (b & 0xFF000000);

	  AccuRGBM<ACCUMODE_LINEAR>(bs, b, level, l, colorgamma);	// += and max
#if	defined(NORMALS_INTEGER)
	  AccuXYZD<ACCUMODE_SCALE >(ns, n, level, l, NORMALS_SCALEBYLEVEL);	// +=
#else
	  AccuXYZD<ACCUMODE_SCALE >(nn, n, level, l, NORMALS_SCALEBYLEVEL);	// +=
#endif
	}

	/* build average of each channel */
	NormRGBM<TRGTMODE_CODING_RGB                         >(fBase[0][ly][lx], bs, av, colorgammainv);
#if	defined(NORMALS_INTEGER)
	NormXYZD<TRGTMODE_CODING_DXDYdZt | TRGTNORM_CUBESPACE>(fNorm[0][ly][lx], ns, av);
#else
	NormXYZD<TRGTMODE_CODING_DXDYdZt | TRGTNORM_CUBESPACE>(fNorm[0][ly][lx], nn, av);
#endif
      }

      type  br[DIM] = {0};
      long  nr[DIM] = {0};
      float rn[DIM] = {0.0f};

      /* analyze this level's 4x4-block */
      for (int ly = 0; ly < TY; ly += 1)
      for (int lx = 0; lx < TX; lx += 1) {
	LookRGBH<TRGTMODE_CODING_RGB                         >(fBase[0][ly][lx], br);
#if	defined(NORMALS_INTEGER)
	LookXYZD<TRGTMODE_CODING_DXDYdZt | TRGTNORM_CUBESPACE>(fNorm[0][ly][lx], nr);
#else
	LookXYZD<TRGTMODE_CODING_DXDYdZt | TRGTNORM_CUBESPACE>(fNorm[0][ly][lx], rn);
#endif
      }

      /* generate this level's 4x4-block from the original surface */
      for (int ly = 0; ly < TY; ly += 1)
      for (int lx = 0; lx < TX; lx += 1) {
	/* build average of each channel an join */
	UTYPE b;
	ULONG n;

	CodeRGBH<TRGTMODE_CODING_RGB                                                           >(fBase[0][ly][lx], br);
#if	defined(NORMALS_INTEGER)
	CodeXYZD<TRGTMODE_CODING_DXDYdZt | TRGTNORM_CUBESPACE, TCOMPRESS_SWIZZL(format) ? 6 : 5>(fNorm[0][ly][lx], nr);
#else
	CodeXYZD<TRGTMODE_CODING_DXDYdZt | TRGTNORM_CUBESPACE, TCOMPRESS_SWIZZL(format) ? 6 : 5>(fNorm[0][ly][lx], rn);
#endif

	b = JoinRGBH<TRGTMODE_CODING_RGB                         >(fBase[0][ly][lx], br);
#if	defined(NORMALS_INTEGER)
	n = JoinXYZD<TRGTMODE_CODING_DXDYdZt | TRGTNORM_CUBESPACE>(fNorm[0][ly][lx], nr);
#else
	n = JoinXYZD<TRGTMODE_CODING_DXDYdZt | TRGTNORM_CUBESPACE>(fNorm[0][ly][lx], rn);
#endif

	/* write the result ABGR */
	bBase[0][ly][lx] = b;
	bNorm[0][ly][lx] = n;
      }

      /* compress to DXT5 */
#if 0
      stb_compress_dxt_block((unsigned char *)dBase, (unsigned char *)bBase[0], true, STB_DXT_DITHER | STB_DXT_HIGHQUAL);
      stb_compress_dxt_block((unsigned char *)dNorm, (unsigned char *)bNorm[0], true, STB_DXT_NORMAL | STB_DXT_HIGHQUAL);
#else
      squish::Compress((unsigned char *)bBase[0], dBase, flags | squish::kColourMetricPerceptual);
      squish::Compress((unsigned char *)bNorm[0], dNorm, flags | squish::kColourMetricUniform);
#endif

      /* advance pointer of compressed blocks */
      dBase += (128 / 32);
      dNorm += (128 / 32);

#if 0
      for (int ly = 0; ly < TY; ly += 1)
      for (int lx = 0; lx < TX; lx += 1) {
	dBase[((y + ly) * bPch) + (x + lx)] = bBase[0][ly][lx];
	dNorm[((y + ly) * nPch) + (x + lx)] = bNorm[0][ly][lx];
      }
#endif
    }
    }

    TextureUnlock(baset, l);
    TextureUnlock(normt, l);
  }

  TextureUnlock((*base), 0);
  TextureUnlock((*norm), 0);

  (*base)->Release();
  (*norm)->Release();

  (*base) = baset;
  (*norm) = normt;

  return true;
}
#endif

#undef	TX
#undef	TY

/* ####################################################################################
 */

bool TextureCompressQDM(LPDIRECT3DTEXTURE *base, LPDIRECT3DTEXTURE *norm, int minlevel, bool gamma, bool LODed) {
  bool res = true;

#if 0
  if (gamma) if (LODed) res = res && TextureCompressQDM<ULONG, float, true >(base, norm, minlevel);
             else       res = res && TextureCompressQDM<ULONG, float, false>(base, norm, minlevel);
  else       if (LODed) res = res && TextureCompressQDM<ULONG, long , true >(base, norm, minlevel);
             else       res = res && TextureCompressQDM<ULONG, long , false>(base, norm, minlevel);
#endif

  return res;
}