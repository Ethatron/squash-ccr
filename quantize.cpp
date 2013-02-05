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
#include "filterdispatch.h"

/* tile-dimensions (free to choose) */
#define	TX	4
#define	TY	4

/* ------------------------------------------------------------------------------------
 */

/* only way to do this, <format> from template argument to preprocessor check */
#if	defined(SQUASH_USE_AMP) && defined(SQUASH_USE_PRE)

namespace squash {

  template<typename UTYPE, typename type>
  void TextureQuantizeRAW_A0(const int format, RESOURCEINFO &texo, RESOURCEINFO &texd, ULONG *texs, ULONG *texr, int levels, int l, int lv, int av);
  template<typename UTYPE, typename type>
  void TextureQuantizeRAW_A1(const int format, RESOURCEINFO &texo, RESOURCEINFO &texd, ULONG *texs, ULONG *texr, int levels, int l, int lv, int av);
  template<typename UTYPE, typename type>
  void TextureQuantizeRAW_A2(const int format, RESOURCEINFO &texo, RESOURCEINFO &texd, ULONG *texs, ULONG *texr, int levels, int l, int lv, int av);
  template<typename UTYPE, typename type>
  void TextureQuantizeRAW_A4(const int format, RESOURCEINFO &texo, RESOURCEINFO &texd, ULONG *texs, ULONG *texr, int levels, int l, int lv, int av);

} // namespace squash

#endif

/* ------------------------------------------------------------------------------------
 */

namespace squash {

#if	defined(SQUASH_INTERMEDIATES)
struct spill {
  ULONG  splh;
  ULONG  splw;
  ULONG *splc;
};
#endif

template<typename UTYPE, typename type, const int format, const int A>
void TextureQuantizeRAW(struct spill *smem, RESOURCEINFO &texo, RESOURCEINFO &texd, ULONG sPch, ULONG dPch, ULONG *texs, ULONG *texr, int levels, int l) {
//  logrf("level %2d/%2d\r", l + 1, level);

    /* loop over 4x4-blocks of this level (RAW) */
#if	defined(SQUASH_USE_AMP)
#if	defined(SQUASH_USE_PRE)
    /* Visual Studio 2011 bugs out */
    /**/ if (A > 2) TextureQuantizeRAW_A4<UTYPE, type>(format, texo, texd, texs, texr, levels, l, lv, av);
    else if (A > 1) TextureQuantizeRAW_A2<UTYPE, type>(format, texo, texd, texs, texr, levels, l, lv, av);
    else if (A > 0) TextureQuantizeRAW_A1<UTYPE, type>(format, texo, texd, texs, texr, levels, l, lv, av);
    else            TextureQuantizeRAW_A0<UTYPE, type>(format, texo, texd, texs, texr, levels, l, lv, av);
#else
    /* ------------------------------------------------------------------------------------------------------- */
    const int NORMALS_SCALEBYLEVEL = ::NORMALS_SCALEBYLEVEL;
    const int  ALPHAS_SCALEBYLEVEL =  ::ALPHAS_SCALEBYLEVEL;
    const float colorgamma       = ::colorgamma;
    const float alphacontrast    = ::alphacontrast;
    const float colorgammainv    = ::colorgammainv;
    const float alphacontrastinv = ::alphacontrastinv;

    int iwidth  = texo.Width;
    int iheight = texo.Height;
    int owidth  = texd.Width;
    int oheight = texd.Height;
    int cwidth  = owidth;
    int cheight = oheight;

    /* get the data back to the CPU */
    switch (TCOMPRESS_CHANNELS(format)) {
      /* ABGR -> ARGB */ case 4: cwidth = (cwidth +  0) >> 0; break; /* 1x LONG to 1x LONG */
      /* -BGR -> -RGB */ case 3: cwidth = (cwidth +  1) >> 1; break; /* 1x LONG to 1x SHORT */
      /* LA-- -> AL-- */ case 2: cwidth = (cwidth +  3) >> 2; break; /* 1x LONG to 1x CHAR */
      /* A--- -> A--- */ case 1: cwidth = (cwidth + 31) >> 5; break; /* 8x LONG to 1x CHAR */
    }

    /* ensure tile ability (bit on overhead for non-4 resolutions) */
    owidth  = (owidth  + (TX - 1)) & (~(TX - 1));
    oheight = (oheight + (TY - 1)) & (~(TY - 1));

    assert((owidth  & (TX - 1)) == 0);
    assert((oheight & (TY - 1)) == 0);

    /* get a two-dimensional extend over the whole output (without re-cast to LONG),
     * then get a tile-extend over that one ()
     */
    Concurrency::extent<2> ee(oheight, owidth);
    Concurrency::tiled_extent<TY, TX> te(ee);

    Concurrency::array_view<const unsigned int, 2> sArr(iheight, iwidth, (const unsigned int *)texs);
    Concurrency::array_view<      unsigned int, 2> dArr(cheight, cwidth, (      unsigned int *)texr, true);

    Concurrency::parallel_for_each(te /*dArr.extent.tile<TY, TX>(osize)*/, [=](tiled_index<TY, TX> elm) restrict(amp) {
      typedef type accu[DIM];

      /* tile static memory */
//    tile_static UTYPE bTex[2][TY][TX];
      tile_static int   bTex[2][TY][TX];
      tile_static type  fTex[2][TY][TX][DIM];

      /* generate this level's 4x4-block from the original surface */
//    const int y = elm.global[0] - ly;
//    const int x = elm.global[1] - lx;
      const int y = elm.tile[0] * TY;
      const int x = elm.tile[1] * TX;
      const int ly = elm.local[0];
      const int lx = elm.local[1];

      {
	const int yl = ((y + ly) << l);
	const int xl = ((x + lx) << l);

	accu tt; tt[0] = tt[1] = tt[2] = tt[3] = tt[4] = tt[5] = tt[6] = tt[7] = 0;

	/* access all pixels this level's 4x4-block represents in
	 * the full dimension original surface (high quality mip-mapping)
	 */
	for (int oy = 0; oy < lv; oy += 1)
	for (int ox = 0; ox < lv; ox += 1) {
	  /* assume seamless tiling: wrap pixels around */
	  const int posx = (xl + ox) % iwidth;
	  const int posy = (yl + oy) % iheight;

	  const ULONG &t = sArr(posy, posx);

	  Accu<type, format>(tt, t);
	}

	/* build average of each channel */
	Norm<type, format>(fTex[0][ly][lx], tt, av, level, l);
      }

      tile_static accu tr; tr[(ly * TX + lx) & 7] = 0;

      elm.barrier.wait_with_tile_static_memory_fence();

      /* runs on only 1 thread per tile (reduction) */
      if (elm.local == index<2>(0, 0)) {
	/* analyze this level's 4x4-block */
	for (int ly = 0; ly < TY; ly += 1)
	for (int lx = 0; lx < TX; lx += 1) {
	  Look<type, format>(fTex[0][ly][lx], tr);
	}
      }

      elm.barrier.wait_with_tile_static_memory_fence();

      /* generate this level's 4x4-block from the original surface */
      {
	/* build average of each channel an join */
	ULONG t;

	Code<type, format, (A > 2 ? 4 : (A > 1 ? 10 : (A > 0 ? 5 : 6)))>(fTex[0][ly][lx], tr); t =
	Qunt<type, format, (A > 2 ? 4 : (A > 1 ? 10 : (A > 0 ? 5 : 6)))>(fTex[0][ly][lx], tr);

	/* write the result ABGR, BGR */
	switch (TCOMPRESS_CHANNELS(format)) {
	  /* ABGR -> RGBA */
	  case 4:
	    bTex[0][ly][lx] = ((UTYPE)t) + 0x0000;
	    break;
	    /* -BGR -> RGB- */
	  case 3:
	    bTex[0][ly][lx] = ((UTYPE)t) + 0x0000;
	    break;
	    /* AL-- -> LA-- */
	  case 2:
	    bTex[0][ly][lx] = ((UTYPE)t) + 0x0000;
	    break;
	  case 1:
	    break;
	}
      }

      /* put this level's 4x4-block into the destination surface */
      {
	/* assume seamless tiling: wrap pixels around */
	const int posx = (x + lx) % owidth;
	const int posy = (y + ly) % oheight;

//	UTYPE t = bTex[0][ly][lx];
	int   t = bTex[0][ly][lx];
	switch (TCOMPRESS_CHANNELS(format)) {
	  /* ABGR -> ARGB */
	  case 4:
	    if (sizeof(UTYPE) == 4) {
	      unsigned int val = (UTYPE)t;

	      /* convert unaligned output location to "int"-space output location */
	      const int linear = ((posy * owidth) + posx) * 1;
	      const int lposx = (linear << 0) % (cwidth << 0);
	      const int lposy = (linear << 0) / (cwidth << 0);

	      /* write out all of an "int" */
	      dArr(lposy, lposx) = val;
	      break;
	    }
	  /* -BGR -> -RGB */
	  case 3:
	    if (sizeof(UTYPE) == 2) {
	      unsigned int val = (USHORT)t;

	      /* convert unaligned output location to "int"-space output location */
	      const int linear = ((posy * owidth) + posx) * 1;
	      const int lposx = (linear << 1) % (cwidth << 1);
	      const int lposy = (linear << 1) / (cwidth << 1);

	      /* write out half of an "int" */
	      atomic_fetch_xor(&dArr(lposy, lposx >> 1), dArr(lposy, lposx >> 1) & (0xFFFF << ((lposx & 0x1) << 4)));
	      atomic_fetch_xor(&dArr(lposy, lposx >> 1),                    (val & 0xFFFF) << ((lposx & 0x1) << 4) );
	      break;
	    }
	  /* LA-- -> AL-- */
	  case 2:
	    if (sizeof(UTYPE) == 1) {
	      unsigned int val = (UCHAR)t;

	      /* convert unaligned output location to "int"-space output location */
	      const int linear = ((posy * owidth) + posx) * 1;
	      const int lposx = (linear << 2) % (cwidth << 2);
	      const int lposy = (linear << 2) / (cwidth << 2);

	      /* write out quarter of an "int" */
	      atomic_fetch_xor(&dArr(lposy, lposx >> 2), dArr(lposy, lposx >> 2) & (0xFF << ((lposx & 0x3) << 3)));
	      atomic_fetch_xor(&dArr(lposy, lposx >> 2),                    (val & 0xFF) << ((lposx & 0x3) << 3) );
	      break;
	    }
	  case 1:
	    break;
	}
      }

//    dTex += 0;
    });

    dArr.synchronize();
#endif
#else
    /* ------------------------------------------------------------------------------------------------------- */
    /* square dimension of this surface-level */
    /* square area of this surface-level */
    const int lv = (1 << l);
    const int av = lv * lv;

#if	defined(SQUASH_INTERMEDIATES)
    /* starting from this mip-level we spill intermediate results into memory
     * for the next time
     */
    if (l >= SQUASH_INTERMEDIATES) {
      if (!smem[l].splc) {
        smem[l].splh =		                texd.Height                                       ;
        smem[l].splw =			                      texd.Width                          ;
        smem[l].splc = (ULONG *)_aligned_malloc(texd.Height * texd.Width * DIM * sizeof(type), 16);
      }
    }
#endif

#if	defined(SQUASH_USE_CCR)
    ULONG *_sTex = (ULONG *)texs;
    UCHAR *_dTex = (UCHAR *)texr;

    parallel_for(0, (int)(texd.Height), TY, [=](int y) {
      ULONG *sTex = _sTex;
      UCHAR *dTex = _dTex;
#else
    ULONG *sTex = (ULONG *)texs;
    UCHAR *dTex = (UCHAR *)texr;

#pragma omp parallel for if((int)texd.Height >= (lv >> 1))	\
			 schedule(dynamic, 1)			\
			 shared(texo, texd, texr)		\
			 firstprivate(dTex)
    for (int y = 0; y < (int)texd.Height; y += TY) {
#endif
      if (!(y & 0x3F)) {
//	logrf("level %2d/%2d: line %4d/%4d processed        \r", l + 1, levels, y, texd.Height);

	/* problematic, the throw() inside may not block all threads ... */
//	PollProgress();
      }

      /* calculate pointer of compressed blocks */
      UCHAR *wTex = (UCHAR *)texr;
      assert(wTex == dTex);

    for (int x = 0; x < (int)texd.Width; x += TX) {
      /* nested lambda-templates not possible in VS2010 */
      static const f<format> fmt;
      typedef type a16 accu[DIM];

      a16 UTYPE bTex[2][TY][TX];
      a16 type  fTex[2][TY][TX][DIM];

#if	defined(SQUASH_INTERMEDIATES)
      /* starting from this mip-level we spill intermediate results into memory
       * for the next time
       */
      if (l > SQUASH_INTERMEDIATES) {
        ULONG hSpill =         smem[l - 1].splh;
        ULONG wSpill =         smem[l - 1].splw;
	type *dSpill = (type *)smem[l - 1].splc;

        /* generate this level's 4x4-block from the previous' surface spilled data */
        for (int ly = 0; ly < TY; ly += 1)
        for (int lx = 0; lx < TX; lx += 1) {
	  const int yl = ((y + ly) << 1);
	  const int xl = ((x + lx) << 1);

	  accu ts = {0};

	  for (int oy = 0; oy < (1 << 1); oy += 1) {
	  for (int ox = 0; ox < (1 << 1); ox += 1) {
	    /* assume seamless tiling: wrap pixels around */
	    const int posx = (xl + ox) % wSpill;
	    const int posy = (yl + oy) % hSpill;

	    /* TODO: scale-by-level types */
	    accu &tt = (accu &)dSpill[((posy * wSpill + posx)) * DIM];

	    Reduce(ts, tt);	// +=
	  }
	  }

	  /* starting from this mip-level we spill intermediate results into memory
	   * for the next time
	   */
	  if (l >= SQUASH_INTERMEDIATES) {
	    /* assume seamless tiling: wrap pixels around */
	    const int posx = (x + lx) % smem[l].splw;
	    const int posy = (y + ly) % smem[l].splh;

	    accu *tSpill = (accu *)smem[l].splc;
	    accu *st = tSpill + posy * smem[l].splw + posx;

	    /* copy via assignment (optimizable) */
	    Spill(st, ts);
	  }

	  /* build average of each channel */
	  Norm(fTex[0][ly][lx], ts, av, levels, l);
        }
      }
      else
#endif

      /* generate this level's 4x4-block from the original surface */
      for (int ly = 0; ly < TY; ly += 1)
      for (int lx = 0; lx < TX; lx += 1) {
	const int yl = ((y + ly) << l);
	const int xl = ((x + lx) << l);

	/* access all pixels this level's 4x4-block represents in
	 * the full dimension original surface (high quality mip-mapping)
	 */
#if	defined(SQUASH_USE_CCR)
	accu ts = {0};
	critical_section cs;

	parallel_for_fixed(0, (int)(lv), [=,&ts,&cs](int oy) {
	accu tt = {0};
#else
	/* OpenMP can't use [] in the reduction clause, provide
	 * compatible anonymous fake union for individuals
	 */
	accu ts = {0},
	     tt = {0};

	/* global constant execution-time per iteration (guaranteed) */
#pragma omp parallel for if((int)texd.Height < (lv >> 1))	\
			 schedule(static, 1) ordered		\
			 shared(sTex) firstprivate(tt)
	for (int oy = 0; oy < lv; oy += 1) {
#endif
	for (int ox = 0; ox < lv; ox += 1) {
	  /* assume seamless tiling: wrap pixels around */
	  const int posx = (xl + ox) % texo.Width;
	  const int posy = (yl + oy) % texo.Height;

	  const ULONG &t = sTex[(posy * sPch) + posx];

	  Accu(tt, t);
	}

#if	defined(SQUASH_USE_CCR)
	cs.lock(); {
#else
#pragma omp ordered
	{
#endif

	  /* reduce may be called multiple times, thus clear the accumulation vars */
	  Reduce(ts, tt);	// +=
	  Clear (    tt);	// =0

#if	defined(SQUASH_USE_CCR)
	} cs.unlock();
        });
#else
	}
	}
#endif

#if	defined(SQUASH_INTERMEDIATES)
	/* starting from this mip-level we spill intermediate results into memory
	 * for the next time
	 */
	if (l >= SQUASH_INTERMEDIATES) {
	  /* assume seamless tiling: wrap pixels around */
	  const int posx = (x + lx) % smem[l].splw;
	  const int posy = (y + ly) % smem[l].splh;

	  accu *tSpill = (accu *)smem[l].splc;
	  accu *st = tSpill + posy * smem[l].splw + posx;

	  /* copy via assignment (optimizable) */
	  Spill<type>(st, ts);
	}
#endif

	/* build average of each channel */
	Norm(fTex[0][ly][lx], ts, av, levels, l);
      }

      accu tr = {0};

      /* analyze this level's 4x4-block */
      for (int ly = 0; ly < TY; ly += 1)
      for (int lx = 0; lx < TX; lx += 1) {
	Look(fTex[0][ly][lx], tr);
      }

      /* generate this level's 4x4-block from the original surface */
      for (int ly = 0; ly < TY; ly += 1)
      for (int lx = 0; lx < TX; lx += 1) {
	/* build average of each channel an join */
	ULONG t;

	Code(fTex[0][ly][lx], tr, (A > 2 ? 4 : (A > 1 ? 10 : (A > 0 ? 5 : 6)))); t =
	Qunt(fTex[0][ly][lx], tr, (A > 2 ? 4 : (A > 1 ? 10 : (A > 0 ? 5 : 6))));

	/* write the result ABGR, BGR */
        switch (TCOMPRESS_CHANNELS(format)) {
          /* ABGR -> RGBA */
          case 4:
            bTex[0][ly][lx] = ((UTYPE)t) + 0x0000;
            break;
          /* -BGR -> RGB- */
	  case 3:
            bTex[0][ly][lx] = ((UTYPE)t) + 0x0000;
            break;
	  /* AL-- -> LA-- */
          case 2:
            bTex[0][ly][lx] = ((UTYPE)t) + 0x0000;
            break;
          case 1:
            break;
        }
      }

      /* put this level's 4x4-block into the destination surface */
      for (int ly = 0; ly < TY; ly += 1)
      for (int lx = 0; lx < TX; lx += 1) {
	/* assume seamless tiling: wrap pixels around */
	const int posx = (x + lx) % texd.Width;
	const int posy = (y + ly) % texd.Height;

	UTYPE t = bTex[0][ly][lx];
	switch (TCOMPRESS_CHANNELS(format)) {
	  /* ABGR -> ARGB */
	  case 4:
	    (UTYPE &)(wTex[(posy * dPch) + (posx * sizeof(UTYPE ))]) = (UTYPE)t;
	    break;
	  /* -BGR -> -RGB */
	  case 3:
	    (UTYPE &)(wTex[(posy * dPch) + (posx * sizeof(USHORT))]) = (USHORT)t;
	    break;
	  /* LA-- -> AL-- */
	  case 2:
	    (UCHAR &)(wTex[(posy * dPch) + (posx * sizeof(UCHAR ))]) = (UCHAR)t;
	    break;
	  case 1:
	    break;
	}
      }

      dTex += 0;
#if	defined(SQUASH_USE_CCR)
    }
    });
#else
    }
    }
#endif
#endif
}

template<typename UTYPE, typename type, const int format, const int A>
bool TextureQuantizeRAW(int minlevel, LPDIRECT3DBASETEXTURE *tex, bool optimize) {
  LPDIRECT3DBASETEXTURE text = NULL;
  RESOURCEINFO texo;

  TextureInfoLevel(*tex, texo, 0);

#if 0
  /* Converts a height map into a normal map. The (x,y,z)
   * components of each normal are mapped to the (r,g,t)
   * channels of the output texture.
   */
  HRESULT D3DXComputeNormalMap(
    __out  LPDIRECT3DBASETEXTURE pTexture,
    __in   LPDIRECT3DBASETEXTURE pSrcTexture,
    __in   const PALETTEENTRY *pSrcPalette,
    __in   DWORD Flags,
    __in   DWORD Channel,
    __in   FLOAT Amplitude
    );
#endif

  /* convert to ARGB8 (TODO: support at least the 16bit formats as well) */
  TEXFORMAT origFormat = texo.Format; texo.Format = (TEXFORMAT)TEXFMT_A8R8G8B8;
  if ((origFormat != TEXFMT_A8R8G8B8) && !TextureConvert(texo, tex, TCOMPRESS_NORMAL(format)))
    return false;

  /* make a histogram of the alpha-channel */
  if (optimize) {
    ULONG sPch, *sTex =
    TextureLock((*tex), 0, -1, &sPch); sPch >>= 2;
    TextureMatte<format>(texo, sPch, sTex);
    TextureUnlock((*tex), 0, -1);
  }

  /* create the textures */
  const int levels = TextureCalcMip(texo.Width, texo.Height, minlevel);

#ifdef DX11
  ID3D11Texture2D *rtex; D3D11_TEXTURE2D_DESC cr;
  memset(&cr, 0, sizeof(cr));

  cr.Width  = texo.Width;
  cr.Height = texo.Height;
  cr.MipLevels = levels;
  cr.ArraySize = texo.Slices;

  TEXFORMAT tf = TEXFMT_UNKNOWN;
  switch (TCOMPRESS_CHANNELS(format)) {
    case 4: if (format == TCOMPRESS_RGBa) tf = (A > 2 ? TEXFMT_A4R4G4B4 : (A > 1 ? TEXFMT_A2R10G10B10 : TEXFMT_A1R5G5B5));
	    if (format == TCOMPRESS_RGBV) tf = (A > 2 ? TEXFMT_X4R4G4B4 : (A > 1 ? TEXFMT_A2R10G10B10 : TEXFMT_X1R5G5B5));
	    if (format == TCOMPRESS_RGBA) tf = (A > 2 ? TEXFMT_A4R4G4B4 : (A > 1 ? TEXFMT_A2R10G10B10 : TEXFMT_A1R5G5B5));
	    if (format == TCOMPRESS_RGBH) tf = (A > 2 ? TEXFMT_A4R4G4B4 : (A > 1 ? TEXFMT_A2R10G10B10 : TEXFMT_A1R5G5B5));
	    if (format == TCOMPRESS_xyzV) tf = (A > 2 ? TEXFMT_X4R4G4B4 : (A > 1 ? TEXFMT_A2B10G10R10 : TEXFMT_X1R5G5B5));
	    if (format == TCOMPRESS_XYZV) tf = (A > 2 ? TEXFMT_X4R4G4B4 : (A > 1 ? TEXFMT_A2B10G10R10 : TEXFMT_X1R5G5B5));
	    if (format == TCOMPRESS_XZYV) tf = (A > 2 ? TEXFMT_X4R4G4B4 : (A > 1 ? TEXFMT_A2R10G10B10 : TEXFMT_X1R5G5B5));
	    if (format == TCOMPRESS_xyzD) tf = (A > 2 ? TEXFMT_A4R4G4B4 : (A > 1 ? TEXFMT_A2R10G10B10 : TEXFMT_A1R5G5B5));
	    if (format == TCOMPRESS_XYZD) tf = (A > 2 ? TEXFMT_A4R4G4B4 : (A > 1 ? TEXFMT_A2R10G10B10 : TEXFMT_A1R5G5B5));
	    if (format == TCOMPRESS_XZYD) tf = (A > 2 ? TEXFMT_A4R4G4B4 : (A > 1 ? TEXFMT_A2R10G10B10 : TEXFMT_A1R5G5B5)); break;
    case 3: if (format == TCOMPRESS_RGB ) tf =                                                          TEXFMT_R5G6B5    ;
	    if (format == TCOMPRESS_xyz ) tf =                                                          TEXFMT_R5G6B5    ;
	    if (format == TCOMPRESS_XYZ ) tf =                                                          TEXFMT_R5G6B5    ;
	    if (format == TCOMPRESS_XZY ) tf =                                                          TEXFMT_R5G6B5    ; break;
    case 2: if (format == TCOMPRESS_La  ) tf =          TEXFMT_A4L4                                                      ;
	    if (format == TCOMPRESS_LA  ) tf =          TEXFMT_A4L4                                                      ;
	    if (format == TCOMPRESS_LH  ) tf =          TEXFMT_A4L4                                                      ; break;
    case 1: abort(); break;
  }

  if ((tf == TEXFMT_UNKNOWN) || !TEXFORMAT::Available(tf))
    return false;

  cr.Format = tf;
  cr.Usage = D3D11_USAGE_STAGING;
  cr.BindFlags = 0;
  cr.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
  cr.SampleDesc.Count = 1;

  HRESULT res;
  if ((res = pD3DDevice->CreateTexture2D(&cr, NULL, &rtex)) == S_OK)
    text = rtex;
#else
  D3DFORMAT qntFormat = D3DFMT_UNKNOWN;
  int qntWidth  = texo.Width;
  int qntHeight = texo.Height;

  switch (TCOMPRESS_CHANNELS(format)) {
    case 4: if (format == TCOMPRESS_RGBa) qntFormat = (A > 2 ? D3DFMT_A4R4G4B4 : (A > 1 ? D3DFMT_A2R10G10B10 : D3DFMT_A1R5G5B5));
	    if (format == TCOMPRESS_RGBV) qntFormat = (A > 2 ? D3DFMT_X4R4G4B4 : (A > 1 ? D3DFMT_A2R10G10B10 : D3DFMT_X1R5G5B5));
	    if (format == TCOMPRESS_RGBA) qntFormat = (A > 2 ? D3DFMT_A4R4G4B4 : (A > 1 ? D3DFMT_A2R10G10B10 : D3DFMT_A1R5G5B5));
	    if (format == TCOMPRESS_RGBH) qntFormat = (A > 2 ? D3DFMT_A4R4G4B4 : (A > 1 ? D3DFMT_A2R10G10B10 : D3DFMT_A1R5G5B5));
	    if (format == TCOMPRESS_xyzV) qntFormat = (A > 2 ? D3DFMT_X4R4G4B4 : (A > 1 ? D3DFMT_A2R10G10B10 : D3DFMT_X1R5G5B5));
	    if (format == TCOMPRESS_XYZV) qntFormat = (A > 2 ? D3DFMT_X4R4G4B4 : (A > 1 ? D3DFMT_A2R10G10B10 : D3DFMT_X1R5G5B5));
	    if (format == TCOMPRESS_XZYV) qntFormat = (A > 2 ? D3DFMT_X4R4G4B4 : (A > 1 ? D3DFMT_A2R10G10B10 : D3DFMT_X1R5G5B5));
	    if (format == TCOMPRESS_xyzD) qntFormat = (A > 2 ? D3DFMT_A4R4G4B4 : (A > 1 ? D3DFMT_A2R10G10B10 : D3DFMT_A1R5G5B5));
	    if (format == TCOMPRESS_XYZD) qntFormat = (A > 2 ? D3DFMT_A4R4G4B4 : (A > 1 ? D3DFMT_A2R10G10B10 : D3DFMT_A1R5G5B5));
	    if (format == TCOMPRESS_XZYD) qntFormat = (A > 2 ? D3DFMT_A4R4G4B4 : (A > 1 ? D3DFMT_A2R10G10B10 : D3DFMT_A1R5G5B5)); break;
    case 3: if (format == TCOMPRESS_RGB ) qntFormat =                                                          D3DFMT_R5G6B5    ;
	    if (format == TCOMPRESS_xyz ) qntFormat =                                                          D3DFMT_R5G6B5    ;
	    if (format == TCOMPRESS_XYZ ) qntFormat =                                                          D3DFMT_R5G6B5    ;
	    if (format == TCOMPRESS_XZY ) qntFormat =                                                          D3DFMT_R5G6B5    ; break;
    case 2: if (format == TCOMPRESS_La  ) qntFormat =          D3DFMT_A4L4                                                      ;
	    if (format == TCOMPRESS_LA  ) qntFormat =          D3DFMT_A4L4                                                      ;
	    if (format == TCOMPRESS_LH  ) qntFormat =          D3DFMT_A4L4                                                      ; break;
    case 1: break;
  }

  pD3DDevice->CreateTexture(qntWidth, qntHeight, levels, 0, qntFormat, D3DPOOL_SYSTEMMEM, (IDirect3DTexture9 **)&text, NULL);
#endif

  /* damit */
  if (!text)
    return false;

  ULONG sPch, *texs = TextureLock((*tex), 0, -1, &sPch); sPch >>= 2;

#if	defined(SQUASH_INTERMEDIATES)
  struct spill smem[32] = {{0}};
#else
  void *smem = NULL;
#endif

  /* create work-pool here, and let OMP react dynamically to the demand */
  omp_set_nested(3);
//#pragma omp parallel for if(level > 6) schedule(dynamic, 1) num_threads(2)
  for (int l = 0; l < levels; l++) {
    RESOURCEINFO texd;

    TextureInfoLevel(text, texd, l); ULONG dPch, *texr =
    TextureLock(text, l, -1, &dPch, true);

    TextureQuantizeRAW<UTYPE, type, format, A>(smem, texo, texd, sPch, dPch, texs, texr, levels, l);

    TextureUnlock(text, l, -1);
  }

//logrf("                                                      \r");

#if	defined(SQUASH_INTERMEDIATES)
  for (int l = 0; l < levels; l++)
    if (smem[l].splc) _aligned_free(smem[l].splc);
#endif

  TextureUnlock((*tex), 0, -1);
  (*tex)->Release();
  (*tex) = text;

  return true;
}

} // namespace squash

#undef	TX
#undef	TY

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */

namespace squash {

#pragma	warning (disable : 4100)

/* there exist only a <USHORT, long> template */
template<> bool TextureQuantizeRAW<ULONG, long, TCOMPRESS_RGB , 0>(int minlevel, LPDIRECT3DBASETEXTURE *tex, bool optimize) { return false; }
template<> bool TextureQuantizeRAW<ULONG, long, TCOMPRESS_RGB , 1>(int minlevel, LPDIRECT3DBASETEXTURE *tex, bool optimize) { return false; }
template<> bool TextureQuantizeRAW<ULONG, long, TCOMPRESS_RGB , 4>(int minlevel, LPDIRECT3DBASETEXTURE *tex, bool optimize) { return false; }

/* we don't process normal-maps as integers */
template<> bool TextureQuantizeRAW<ULONG, long, TCOMPRESS_xyz , 0>(int minlevel, LPDIRECT3DBASETEXTURE *tex, bool optimize) { return false; }
template<> bool TextureQuantizeRAW<ULONG, long, TCOMPRESS_XYZ , 0>(int minlevel, LPDIRECT3DBASETEXTURE *tex, bool optimize) { return false; }
template<> bool TextureQuantizeRAW<ULONG, long, TCOMPRESS_XZY , 0>(int minlevel, LPDIRECT3DBASETEXTURE *tex, bool optimize) { return false; }

template<> bool TextureQuantizeRAW<ULONG, long, TCOMPRESS_xyz , 1>(int minlevel, LPDIRECT3DBASETEXTURE *tex, bool optimize) { return false; }
template<> bool TextureQuantizeRAW<ULONG, long, TCOMPRESS_XYZ , 1>(int minlevel, LPDIRECT3DBASETEXTURE *tex, bool optimize) { return false; }
template<> bool TextureQuantizeRAW<ULONG, long, TCOMPRESS_XZY , 1>(int minlevel, LPDIRECT3DBASETEXTURE *tex, bool optimize) { return false; }

template<> bool TextureQuantizeRAW<ULONG, long, TCOMPRESS_xyz , 4>(int minlevel, LPDIRECT3DBASETEXTURE *tex, bool optimize) { return false; }
template<> bool TextureQuantizeRAW<ULONG, long, TCOMPRESS_XYZ , 4>(int minlevel, LPDIRECT3DBASETEXTURE *tex, bool optimize) { return false; }
template<> bool TextureQuantizeRAW<ULONG, long, TCOMPRESS_XZY , 4>(int minlevel, LPDIRECT3DBASETEXTURE *tex, bool optimize) { return false; }

/* we don't process normal-maps as integers */
template<> bool TextureQuantizeRAW<ULONG, long, TCOMPRESS_xyzD, 0>(int minlevel, LPDIRECT3DBASETEXTURE *tex, bool optimize) { return false; }
template<> bool TextureQuantizeRAW<ULONG, long, TCOMPRESS_XYZD, 0>(int minlevel, LPDIRECT3DBASETEXTURE *tex, bool optimize) { return false; }
template<> bool TextureQuantizeRAW<ULONG, long, TCOMPRESS_XZYD, 0>(int minlevel, LPDIRECT3DBASETEXTURE *tex, bool optimize) { return false; }

template<> bool TextureQuantizeRAW<ULONG, long, TCOMPRESS_xyzD, 1>(int minlevel, LPDIRECT3DBASETEXTURE *tex, bool optimize) { return false; }
template<> bool TextureQuantizeRAW<ULONG, long, TCOMPRESS_XYZD, 1>(int minlevel, LPDIRECT3DBASETEXTURE *tex, bool optimize) { return false; }
template<> bool TextureQuantizeRAW<ULONG, long, TCOMPRESS_XZYD, 1>(int minlevel, LPDIRECT3DBASETEXTURE *tex, bool optimize) { return false; }

template<> bool TextureQuantizeRAW<ULONG, long, TCOMPRESS_xyzD, 4>(int minlevel, LPDIRECT3DBASETEXTURE *tex, bool optimize) { return false; }
template<> bool TextureQuantizeRAW<ULONG, long, TCOMPRESS_XYZD, 4>(int minlevel, LPDIRECT3DBASETEXTURE *tex, bool optimize) { return false; }
template<> bool TextureQuantizeRAW<ULONG, long, TCOMPRESS_XZYD, 4>(int minlevel, LPDIRECT3DBASETEXTURE *tex, bool optimize) { return false; }

/* we don't process normal-maps as integers */
template<> bool TextureQuantizeRAW<ULONG, long, TCOMPRESS_xyzV, 0>(int minlevel, LPDIRECT3DBASETEXTURE *tex, bool optimize) { return false; }
template<> bool TextureQuantizeRAW<ULONG, long, TCOMPRESS_XYZV, 0>(int minlevel, LPDIRECT3DBASETEXTURE *tex, bool optimize) { return false; }
template<> bool TextureQuantizeRAW<ULONG, long, TCOMPRESS_XZYV, 0>(int minlevel, LPDIRECT3DBASETEXTURE *tex, bool optimize) { return false; }

template<> bool TextureQuantizeRAW<ULONG, long, TCOMPRESS_xyzV, 1>(int minlevel, LPDIRECT3DBASETEXTURE *tex, bool optimize) { return false; }
template<> bool TextureQuantizeRAW<ULONG, long, TCOMPRESS_XYZV, 1>(int minlevel, LPDIRECT3DBASETEXTURE *tex, bool optimize) { return false; }
template<> bool TextureQuantizeRAW<ULONG, long, TCOMPRESS_XZYV, 1>(int minlevel, LPDIRECT3DBASETEXTURE *tex, bool optimize) { return false; }

template<> bool TextureQuantizeRAW<ULONG, long, TCOMPRESS_xyzV, 4>(int minlevel, LPDIRECT3DBASETEXTURE *tex, bool optimize) { return false; }
template<> bool TextureQuantizeRAW<ULONG, long, TCOMPRESS_XYZV, 4>(int minlevel, LPDIRECT3DBASETEXTURE *tex, bool optimize) { return false; }
template<> bool TextureQuantizeRAW<ULONG, long, TCOMPRESS_XZYV, 4>(int minlevel, LPDIRECT3DBASETEXTURE *tex, bool optimize) { return false; }

#pragma	warning (default : 4100)

} // namespace squash

/* ####################################################################################
 */

namespace squash {

bool TextureQuantizeR10G10B10H2(LPDIRECT3DBASETEXTURE *base, int minlevel, bool gamma) {
  bool res = true;

  if (gamma) res = res && TextureQuantizeRAW<ULONG, float   , TCOMPRESS_RGBH, 2>(minlevel, base);
  else       res = res && TextureQuantizeRAW<ULONG, longlong, TCOMPRESS_RGBH, 2>(minlevel, base);

  return res;
}

bool TextureQuantizeR10G10B10V2(LPDIRECT3DBASETEXTURE *base, int minlevel, bool gamma) {
  bool res = true;

  if (gamma) res = res && TextureQuantizeRAW<ULONG, float   , TCOMPRESS_RGBV, 2>(minlevel, base);
  else       res = res && TextureQuantizeRAW<ULONG, longlong, TCOMPRESS_RGBV, 2>(minlevel, base);

  return res;
}

bool TextureQuantizeR10G10B10A2(LPDIRECT3DBASETEXTURE *base, int minlevel, bool gamma, bool contrast) {
  bool res = true;

  if   (contrast) res = res && TextureQuantizeRAW<ULONG, float   , TCOMPRESS_RGBA, 2>(minlevel, base);
  else if (gamma) res = res && TextureQuantizeRAW<ULONG, float   , TCOMPRESS_RGBA, 2>(minlevel, base);
  else            res = res && TextureQuantizeRAW<ULONG, longlong, TCOMPRESS_RGBA, 2>(minlevel, base);

  return res;
}

bool TextureQuantizeR10G10B10_A2(LPDIRECT3DBASETEXTURE *base, int minlevel, bool gamma, bool contrast) {
  bool res = true;

  if   (contrast) res = res && TextureQuantizeRAW<ULONG, float   , TCOMPRESS_RGBa, 2>(minlevel, base);
  else if (gamma) res = res && TextureQuantizeRAW<ULONG, float   , TCOMPRESS_RGBa, 2>(minlevel, base);
  else            res = res && TextureQuantizeRAW<ULONG, longlong, TCOMPRESS_RGBa, 2>(minlevel, base);

  return res;
}

bool TextureQuantizeR5G5B5V1(LPDIRECT3DBASETEXTURE *base, int minlevel, bool gamma) {
  bool res = true;

  if (gamma) res = res && TextureQuantizeRAW<USHORT, float, TCOMPRESS_RGBV, 1>(minlevel, base);
  else       res = res && TextureQuantizeRAW<USHORT, long , TCOMPRESS_RGBV, 1>(minlevel, base);

  return res;
}

bool TextureQuantizeR5G5B5A1(LPDIRECT3DBASETEXTURE *base, int minlevel, bool gamma, bool contrast) {
  bool res = true;

  if   (contrast) res = res && TextureQuantizeRAW<USHORT, float, TCOMPRESS_RGBA, 1>(minlevel, base);
  else if (gamma) res = res && TextureQuantizeRAW<USHORT, float, TCOMPRESS_RGBA, 1>(minlevel, base);
  else            res = res && TextureQuantizeRAW<USHORT, long , TCOMPRESS_RGBA, 1>(minlevel, base);

  return res;
}

bool TextureQuantizeR5G5B5_A1(LPDIRECT3DBASETEXTURE *base, int minlevel, bool gamma, bool contrast) {
  bool res = true;

  if   (contrast) res = res && TextureQuantizeRAW<USHORT, float, TCOMPRESS_RGBa, 1>(minlevel, base);
  else if (gamma) res = res && TextureQuantizeRAW<USHORT, float, TCOMPRESS_RGBa, 1>(minlevel, base);
  else            res = res && TextureQuantizeRAW<USHORT, long , TCOMPRESS_RGBa, 1>(minlevel, base);

  return res;
}

bool TextureQuantizeR5G5B5H1(LPDIRECT3DBASETEXTURE *base, int minlevel, bool gamma) {
  bool res = true;

  if (gamma) res = res && TextureQuantizeRAW<USHORT, float, TCOMPRESS_RGBH, 1>(minlevel, base);
  else       res = res && TextureQuantizeRAW<USHORT, long , TCOMPRESS_RGBH, 1>(minlevel, base);

  return res;
}

bool TextureQuantizeR4G4B4V4(LPDIRECT3DBASETEXTURE *base, int minlevel, bool gamma) {
  bool res = true;

  if (gamma) res = res && TextureQuantizeRAW<USHORT, float, TCOMPRESS_RGBV, 4>(minlevel, base);
  else       res = res && TextureQuantizeRAW<USHORT, long , TCOMPRESS_RGBV, 4>(minlevel, base);

  return res;
}

bool TextureQuantizeR4G4B4A4(LPDIRECT3DBASETEXTURE *base, int minlevel, bool gamma, bool contrast) {
  bool res = true;

  if   (contrast) res = res && TextureQuantizeRAW<USHORT, float, TCOMPRESS_RGBA, 4>(minlevel, base);
  else if (gamma) res = res && TextureQuantizeRAW<USHORT, float, TCOMPRESS_RGBA, 4>(minlevel, base);
  else            res = res && TextureQuantizeRAW<USHORT, long , TCOMPRESS_RGBA, 4>(minlevel, base);

  return res;
}

bool TextureQuantizeR4G4B4_A4(LPDIRECT3DBASETEXTURE *base, int minlevel, bool gamma, bool contrast) {
  bool res = true;

  if   (contrast) res = res && TextureQuantizeRAW<USHORT, float, TCOMPRESS_RGBa, 4>(minlevel, base);
  else if (gamma) res = res && TextureQuantizeRAW<USHORT, float, TCOMPRESS_RGBa, 4>(minlevel, base);
  else            res = res && TextureQuantizeRAW<USHORT, long , TCOMPRESS_RGBa, 4>(minlevel, base);

  return res;
}

bool TextureQuantizeR4G4B4H4(LPDIRECT3DBASETEXTURE *base, int minlevel, bool gamma) {
  bool res = true;

  if (gamma) res = res && TextureQuantizeRAW<USHORT, float, TCOMPRESS_RGBH, 4>(minlevel, base);
  else       res = res && TextureQuantizeRAW<USHORT, long , TCOMPRESS_RGBH, 4>(minlevel, base);

  return res;
}

bool TextureQuantizeR5G6B5(LPDIRECT3DBASETEXTURE *base, int minlevel, bool gamma) {
  bool res = true;

  if (gamma) res = res && TextureQuantizeRAW<USHORT, float, TCOMPRESS_RGB, 0>(minlevel, base);
  else       res = res && TextureQuantizeRAW<USHORT, long , TCOMPRESS_RGB, 0>(minlevel, base);

  return res;
}

bool TextureQuantizeL4A4(LPDIRECT3DBASETEXTURE *base, int minlevel, bool gamma, bool contrast) {
  bool res = true;

  if   (contrast) res = res && TextureQuantizeRAW<UCHAR, float, TCOMPRESS_LA, 4>(minlevel, base);
  else if (gamma) res = res && TextureQuantizeRAW<UCHAR, float, TCOMPRESS_LA, 4>(minlevel, base);
  else            res = res && TextureQuantizeRAW<UCHAR, long , TCOMPRESS_LA, 4>(minlevel, base);

  return res;
}

bool TextureQuantizeL4_A4(LPDIRECT3DBASETEXTURE *base, int minlevel, bool gamma, bool contrast) {
  bool res = true;

  if   (contrast) res = res && TextureQuantizeRAW<UCHAR, float, TCOMPRESS_La, 4>(minlevel, base);
  else if (gamma) res = res && TextureQuantizeRAW<UCHAR, float, TCOMPRESS_La, 4>(minlevel, base);
  else            res = res && TextureQuantizeRAW<UCHAR, long , TCOMPRESS_La, 4>(minlevel, base);

  return res;
}

bool TextureQuantizeL4H4(LPDIRECT3DBASETEXTURE *base, int minlevel, bool gamma) {
  bool res = true;

  if (gamma) res = res && TextureQuantizeRAW<UCHAR, float, TCOMPRESS_LH, 4>(minlevel, base);
  else       res = res && TextureQuantizeRAW<UCHAR, long , TCOMPRESS_LH, 4>(minlevel, base);

  return res;
}

bool TextureQuantizeX10Y10Z10V2(LPDIRECT3DBASETEXTURE *norm, int minlevel) {
  bool res = true;

  res = res && TextureQuantizeRAW<ULONG, float, TCOMPRESS_XYZV, 2>(minlevel, norm);

  return res;
}

bool TextureQuantize_X10Y10Z10V2(LPDIRECT3DBASETEXTURE *norm, int minlevel) {
  bool res = true;

  res = res && TextureQuantizeRAW<ULONG, float, TCOMPRESS_xyzV, 2>(minlevel, norm);

  return res;
}

bool TextureQuantizeX10Y10Z10D2(LPDIRECT3DBASETEXTURE *norm, int minlevel) {
  bool res = true;

  res = res && TextureQuantizeRAW<ULONG, float, TCOMPRESS_XYZD, 2>(minlevel, norm);

  return res;
}

bool TextureQuantize_X10Y10Z10D2(LPDIRECT3DBASETEXTURE *norm, int minlevel) {
  bool res = true;

  res = res && TextureQuantizeRAW<ULONG, float, TCOMPRESS_xyzD, 2>(minlevel, norm);

  return res;
}

bool TextureQuantizeX4Y4Z4D4(LPDIRECT3DBASETEXTURE *norm, int minlevel) {
  bool res = true;

  res = res && TextureQuantizeRAW<USHORT, float, TCOMPRESS_XYZD, 4>(minlevel, norm);

  return res;
}

bool TextureQuantize_X4Y4Z4D4(LPDIRECT3DBASETEXTURE *norm, int minlevel) {
  bool res = true;

  res = res && TextureQuantizeRAW<USHORT, float, TCOMPRESS_xyzD, 4>(minlevel, norm);

  return res;
}

bool TextureQuantizeX4Y4Z4V4(LPDIRECT3DBASETEXTURE *norm, int minlevel) {
  bool res = true;

  res = res && TextureQuantizeRAW<USHORT, float, TCOMPRESS_XYZV, 4>(minlevel, norm);

  return res;
}

bool TextureQuantize_X4Y4Z4V4(LPDIRECT3DBASETEXTURE *norm, int minlevel) {
  bool res = true;

  res = res && TextureQuantizeRAW<USHORT, float, TCOMPRESS_xyzV, 4>(minlevel, norm);

  return res;
}

bool TextureQuantizeX5Y6Z5(LPDIRECT3DBASETEXTURE *norm, int minlevel) {
  bool res = true;

  res = res && TextureQuantizeRAW<USHORT, float, TCOMPRESS_XYZ, 0>(minlevel, norm);

  return res;
}

bool TextureQuantizeX5Z6Y5(LPDIRECT3DBASETEXTURE *norm, int minlevel) {
  bool res = true;

  res = res && TextureQuantizeRAW<USHORT, float, TCOMPRESS_XZY, 0>(minlevel, norm);

  return res;
}

bool TextureQuantize_X5Y6Z5(LPDIRECT3DBASETEXTURE *norm, int minlevel) {
  bool res = true;

  res = res && TextureQuantizeRAW<USHORT, float, TCOMPRESS_xyz, 0>(minlevel, norm);

  return res;
}

} // namespace squash

/* ------------------------------------------------------------------------------------
 */

namespace squash {

void _TextureQuantize(int minlevel, LPDIRECT3DBASETEXTURE *norm) {
  TextureQuantizeRAW<USHORT, long , TCOMPRESS_La  , 4>(minlevel, norm);
  TextureQuantizeRAW<USHORT, long , TCOMPRESS_LA  , 4>(minlevel, norm);
  TextureQuantizeRAW<USHORT, long , TCOMPRESS_LH  , 4>(minlevel, norm);

  TextureQuantizeRAW<ULONG , long , TCOMPRESS_RGB , 0>(minlevel, norm);
  TextureQuantizeRAW<ULONG , long , TCOMPRESS_xyz , 0>(minlevel, norm);	// because of templating
  TextureQuantizeRAW<ULONG , long , TCOMPRESS_XYZ , 0>(minlevel, norm);	// because of templating
  TextureQuantizeRAW<ULONG , long , TCOMPRESS_XZY , 0>(minlevel, norm);	// because of templating

  TextureQuantizeRAW<USHORT, float, TCOMPRESS_La  , 4>(minlevel, norm);
  TextureQuantizeRAW<USHORT, float, TCOMPRESS_LA  , 4>(minlevel, norm);
  TextureQuantizeRAW<USHORT, float, TCOMPRESS_LH  , 4>(minlevel, norm);

  TextureQuantizeRAW<ULONG , float, TCOMPRESS_RGB , 0>(minlevel, norm);
  TextureQuantizeRAW<ULONG , float, TCOMPRESS_xyz , 0>(minlevel, norm);
  TextureQuantizeRAW<ULONG , float, TCOMPRESS_XYZ , 0>(minlevel, norm);
  TextureQuantizeRAW<ULONG , float, TCOMPRESS_XZY , 0>(minlevel, norm);

#if	1
  TextureQuantizeRAW<ULONG , long , TCOMPRESS_RGBa, 1>(minlevel, norm);
  TextureQuantizeRAW<ULONG , long , TCOMPRESS_RGBA, 1>(minlevel, norm);
  TextureQuantizeRAW<ULONG , long , TCOMPRESS_RGBH, 1>(minlevel, norm);

  TextureQuantizeRAW<ULONG , long , TCOMPRESS_RGBV, 1>(minlevel, norm);

  TextureQuantizeRAW<ULONG , float, TCOMPRESS_RGBa, 1>(minlevel, norm);
  TextureQuantizeRAW<ULONG , float, TCOMPRESS_RGBA, 1>(minlevel, norm);
  TextureQuantizeRAW<ULONG , float, TCOMPRESS_RGBH, 1>(minlevel, norm);
  TextureQuantizeRAW<ULONG , float, TCOMPRESS_xyzD, 1>(minlevel, norm);
  TextureQuantizeRAW<ULONG , float, TCOMPRESS_XYZD, 1>(minlevel, norm);
  TextureQuantizeRAW<ULONG , float, TCOMPRESS_XZYD, 1>(minlevel, norm);

  TextureQuantizeRAW<ULONG , float, TCOMPRESS_RGBV, 1>(minlevel, norm);
  TextureQuantizeRAW<ULONG , float, TCOMPRESS_xyzV, 1>(minlevel, norm);
  TextureQuantizeRAW<ULONG , float, TCOMPRESS_XYZV, 1>(minlevel, norm);
  TextureQuantizeRAW<ULONG , float, TCOMPRESS_XZYV, 1>(minlevel, norm);

  TextureQuantizeRAW<ULONG , long , TCOMPRESS_RGBa, 4>(minlevel, norm);
  TextureQuantizeRAW<ULONG , long , TCOMPRESS_RGBA, 4>(minlevel, norm);
  TextureQuantizeRAW<ULONG , long , TCOMPRESS_RGBH, 4>(minlevel, norm);

  TextureQuantizeRAW<ULONG , long , TCOMPRESS_RGBV, 4>(minlevel, norm);

  TextureQuantizeRAW<ULONG , float, TCOMPRESS_RGBa, 4>(minlevel, norm);
  TextureQuantizeRAW<ULONG , float, TCOMPRESS_RGBA, 4>(minlevel, norm);
  TextureQuantizeRAW<ULONG , float, TCOMPRESS_RGBH, 4>(minlevel, norm);
  TextureQuantizeRAW<ULONG , float, TCOMPRESS_xyzD, 4>(minlevel, norm);
  TextureQuantizeRAW<ULONG , float, TCOMPRESS_XYZD, 4>(minlevel, norm);
  TextureQuantizeRAW<ULONG , float, TCOMPRESS_XZYD, 4>(minlevel, norm);

  TextureQuantizeRAW<ULONG , float, TCOMPRESS_RGBV, 4>(minlevel, norm);
  TextureQuantizeRAW<ULONG , float, TCOMPRESS_xyzV, 4>(minlevel, norm);
  TextureQuantizeRAW<ULONG , float, TCOMPRESS_XYZV, 4>(minlevel, norm);
  TextureQuantizeRAW<ULONG , float, TCOMPRESS_XZYV, 4>(minlevel, norm);
#endif
}

} // namespace squash
