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

/* tile-dimensions (tied to 4x4 tile per block size) */
#define	TX	4
#define	TY	4

/* ------------------------------------------------------------------------------------
 */
#if	defined(SQUASH_USE_AMP)
#define	SQUISH_USE_AMP
#endif
#if	defined(SQUASH_USE_AMP_DEBUG)
#define	SQUASH_USE_AMP_DEBUG
#endif
#if	defined(SQUASH_USE_PRE)
#define	SQUISH_USE_PRE
#endif
#if	defined(SQUASH_USE_CCR)
#define	SQUISH_USE_CCR
#endif
#if	defined(SQUASH_USE_SSE)
#define	SQUISH_USE_SSE	SQUASH_USE_SSE
#endif
#if	defined(SQUASH_USE_XSSE)
#define	SQUISH_USE_XSSE	SQUASH_USE_XSSE
#endif

/* only way to do this, <format> from template argument to preprocessor check */
#if	defined(SQUASH_USE_AMP) && defined(SQUASH_USE_PRE)

#define	ONLY_HEADERS
#include "squish.inl"
#define	ONLY_ARRAY
#include "squish/coloursinglelookup_ccr.inl"
#include "squish/degeneracy_ccr.inl"

namespace squash {

  template<typename UTYPE, typename type>
  void TextureCompressDXT_BC1 (const int format, RESOURCEINFO &texo, RESOURCEINFO &texd, ULONG *texs, ULONG *texr, int level, int l, int blocksize, int flags);
  template<typename UTYPE, typename type>
  void TextureCompressDXT_BC2 (const int format, RESOURCEINFO &texo, RESOURCEINFO &texd, ULONG *texs, ULONG *texr, int level, int l, int blocksize, int flags);
  template<typename UTYPE, typename type>
  void TextureCompressDXT_BC3 (const int format, RESOURCEINFO &texo, RESOURCEINFO &texd, ULONG *texs, ULONG *texr, int level, int l, int blocksize, int flags);
  template<typename UTYPE, typename type>
  void TextureCompressDXT_BC3 (const int format, RESOURCEINFO &texo, RESOURCEINFO &texd, ULONG *texs, ULONG *texr, int level, int l, int blocksize, int flags);
  template<typename UTYPE, typename type>
  void TextureCompressDXT_BC45(const int format, RESOURCEINFO &texo, RESOURCEINFO &texd, ULONG *texs, ULONG *texr, int level, int l, int blocksize, int flags);

} // namespace squash

#else
#include "..\squish\squish.inl"
#endif

/* ------------------------------------------------------------------------------------
 */

namespace squash {

struct histogram {
  unsigned long histo[4][256];
  unsigned long histn[4];
  bool grey, blank, black;
};

void HistogramCompressDXT(struct histogram *h, RESOURCEINFO &texo, ULONG sPch, ULONG *sTex) {
    /* exclude the border-region from checking */
    int lft = 0, rgt = (int)texo.Width;
    int top = 0, bot = (int)texo.Height;
    if (ignoreborder) {
      /* 64² -> 1px, 128² -> 2px etc. */
      int borderx = min(ignoreborder, (rgt * min(ignoreborder, 2)) / 128);
      int bordery = min(ignoreborder, (bot * min(ignoreborder, 2)) / 128);

      lft += borderx, rgt -= borderx;
      top += bordery, bot -= bordery;
    }

#ifdef	SQUASH_USE_CCR
    parallel_for_fixed(top, bot, [=](int y) {
      for (int x = lft; x < rgt; x += 1) {
	ULONG t = sTex[y * sPch + x];
	ULONG a = (t >> 24) & 0xFF; /*a*/
	ULONG r = (t >> 16) & 0xFF; /*a*/
	ULONG g = (t >>  8) & 0xFF; /*a*/
	ULONG b = (t >>  0) & 0xFF; /*a*/

	/* read-modify-write is not thread-safe */
	InterlockedIncrement(&h->histo[0][a]);
	InterlockedIncrement(&h->histo[1][r]);
	InterlockedIncrement(&h->histo[2][g]);
	InterlockedIncrement(&h->histo[3][b]);

	/* write-through is thread-safe */
	if ((r != g) || (g != b))
	  h->grey = false;
	if ((r != 0) || (g != 0) || (b != 0)) {
	  h->blank = false;
	  if (a == 0)
	    h->black = false;
	}
      }
    });
#else
#pragma omp parallel for schedule(dynamic, 4)		\
			 shared(sTex, h)
    for (int y = top; y < bot; y += 1) {
    for (int x = lft; x < rgt; x += 1) {
      ULONG t = sTex[y * sPch + x];
      ULONG a = (t >> 24) & 0xFF; /*a*/
      ULONG r = (t >> 16) & 0xFF; /*a*/
      ULONG g = (t >>  8) & 0xFF; /*a*/
      ULONG b = (t >>  0) & 0xFF; /*a*/

#pragma omp atomic
      h->histo[0][a]++;
#pragma omp atomic
      h->histo[1][r]++;
#pragma omp atomic
      h->histo[2][g]++;
#pragma omp atomic
      h->histo[3][b]++;

      if ((r != g) || (g != b))
	h->grey = false;
      if ((r != 0) || (g != 0) || (b != 0)) {
	h->blank = false;
	if (a == 0)
	  h->black = false;
      }
    }}
#endif

}

#if	defined(SQUASH_INTERMEDIATES)
struct spill {
  ULONG  splh;
  ULONG  splw;
  ULONG *splc;
};
#endif

template<typename UTYPE, typename type, const int format>
void TextureCompressDXT(struct spill *smem, RESOURCEINFO &texo, RESOURCEINFO &texd, ULONG sPch, ULONG dPch, ULONG *texs, ULONG *texr, int levels, int l, int blocksize, int flags) {
//  logrf("level %2d/%2d\r", l + 1, level);

    /* loop over 4x4-blocks of this level (DXT5) */
#if	defined(SQUASH_USE_AMP)
#if	defined(SQUASH_USE_PRE)
//  flags = squish::FixFlags(flags);

    /**/ if ((flags & squish::kBtc1))
      TextureCompressDXT_BC1 <UTYPE, type>(format, texo, texd, texs, texr, levels, l, lv, av, blocksize, flags);
    else if ((flags & squish::kBtc2))
      TextureCompressDXT_BC2 <UTYPE, type>(format, texo, texd, texs, texr, levels, l, lv, av, blocksize, flags);
    else if ((flags & squish::kBtc3) && (TCOMPRESS_CHANNELS(format) >= 3))
      TextureCompressDXT_BC3 <UTYPE, type>(format, texo, texd, texs, texr, levels, l, lv, av, blocksize, flags);
    else if ((flags & squish::kBtc3) && (TCOMPRESS_CHANNELS(format) <= 2))
      TextureCompressDXT_BC45<UTYPE, type>(format, texo, texd, texs, texr, levels, l, lv, av, blocksize, flags);
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
    cheight = cheight / 4;		/* 4x LONG to 1x|2x LONG */
    cwidth  = cwidth / (blocksize / 4);	/* 4x LONG to 1x|2x LONG */

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
      tile_static UTYPE bTex[2][TY][TX];
      tile_static type  fTex[2][TY][TX][DIM];
      tile_static int   iTex[2][TY][TX][DIM];

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

	accu tt = {0};

	/* access all pixels this level's 4x4-block represents in
	 * the full dimension original surface (high quality mip-mapping)
	 */
	for (int oy = 0; oy < lv; oy += 1) {
	for (int ox = 0; ox < lv; ox += 1) {
	  /* assume seamless tiling: wrap pixels around */
	  const int posx = (xl + ox) % iwidth;
	  const int posy = (yl + oy) % iheight;

	  const ULONG &t = sArr(posy, posx);

	  Accu<type, format>(tt, t);
	}
	}

	/* build average of each channel */
	Norm<type, format>(fTex[0][ly][lx], tt, av, level, l);
      }

      elm.barrier.wait_with_tile_static_memory_fence();

      accu tr = {0};

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
	Code <type, format, (TCOMPRESS_CHANNELS(format) +
			    (TCOMPRESS_GREYS   (format) ? 2 : 0)) == 2 ? 8 :
			    (TCOMPRESS_SWIZZL  (format) ? 6 : 5)>(
			                     fTex[0][ly][lx], tr);
	Range<type, format>(iTex[0][ly][lx], fTex[0][ly][lx]);

	/* swizzle ABGR -> AGBR */
        if (TCOMPRESS_SWIZZL(format)) {
	  int swap =           iTex[0][ly][lx][1];
	  iTex[0][ly][lx][1] = iTex[0][ly][lx][2];
	  iTex[0][ly][lx][2] = swap              ;
	}

	/* write the result ABGR, BGR */
	switch (TCOMPRESS_CHANNELS(format) + (TCOMPRESS_GREYS(format) ? 2 : 0)) {
	  /* ABGR -> RGBA */
	  case 4: break;
	  /* -BGR -> RGB- */
	  case 3:                                   iTex[0][ly][lx][0] = 0xFF;
		  break;
	  /* --YX -> XY-- */
	  /* AL-- -> LA-- */
	  case 2: /**/ if (format == TCOMPRESS_XYz) iTex[0][ly][lx][0] = 0xFF;
		  else if (format == TCOMPRESS_xy ) iTex[0][ly][lx][0] = iTex[0][ly][lx][2],  // Y
						    iTex[1][ly][lx][0] = iTex[0][ly][lx][3];  // X
		  else if (format == TCOMPRESS_XY ) iTex[0][ly][lx][0] = iTex[0][ly][lx][2],  // Y
						    iTex[1][ly][lx][0] = iTex[0][ly][lx][3];  // X
		  else                              iTex[0][ly][lx][0] = iTex[0][ly][lx][0],  // A
						    iTex[1][ly][lx][0] = iTex[0][ly][lx][1];  // Z
		  break;
	  /* -Z-- -> Z--- */
	  /* A--- -> A--- */
	  /* -LLL -> L--- */
	  case 1: /**/ if (format == TCOMPRESS_a  ) iTex[0][ly][lx][0] = iTex[0][ly][lx][0];  // A
		  else if (format == TCOMPRESS_A  ) iTex[0][ly][lx][0] = iTex[0][ly][lx][0];  // A
		  else if (format == TCOMPRESS_xyZ) iTex[0][ly][lx][0] = iTex[0][ly][lx][1];  // Z
//		  else if (format == TCOMPRESS_z  ) iTex[0][ly][lx][0] = iTex[0][ly][lx][1];  // Z
//		  else if (format == TCOMPRESS_Z  ) iTex[0][ly][lx][0] = iTex[0][ly][lx][1];  // Z
		  else                              iTex[0][ly][lx][0] = iTex[0][ly][lx][3];  // X
		  break;
	}
      }

      elm.barrier.wait_with_tile_static_memory_fence();

      /* put this level's 4x4-block into the destination surface */
      {
	/* compile-time constant */
	const int channels = TCOMPRESS_CHANNELS(format) + (TCOMPRESS_GREYS(format) ? 2 : 0);

	/* round down */
	int posx = (x + lx) >> 2;
	int posy = (y + ly) >> 2;

	/* first and second block */
	unsigned int b0, b1, b2, b3;

        /* compress to DXT1/DXT3/DXT5/ATI1/ATI2 */
        switch (channels) {
	  case 4:
	  case 3:
	    /* 1x LONG per block for DXT1, 2x for the others */
	    /**/ if (flags & squish::kBtc1)
	    { posx <<= 0;
	      if (elm.local == index<2>(0, 2)) { squish::CompressColorBtc (iTex[0],    0xFFFF, b2, b3, flags); dArr(posy, posx + 0) = b2;
													       dArr(posy, posx + 1) = b3; }
	    }
	    else if (flags & squish::kBtc2)
	    { posx <<= 1;
	      if (elm.local == index<2>(0, 0)) { squish::CompressAlphaBtc2(iTex[0], 0, 0xFFFF, b0           ); dArr(posy, posx + 0) = b0; }
	      if (elm.local == index<2>(0, 1)) { squish::CompressAlphaBtc2(iTex[0], 4, 0xFFFF, b1           ); dArr(posy, posx + 1) = b1; }
	      if (elm.local == index<2>(0, 2)) { squish::CompressColorBtc (iTex[0],    0xFFFF, b2, b3, flags); dArr(posy, posx + 2) = b2;
													       dArr(posy, posx + 3) = b3; }
	    }
	    else if (flags & squish::kBtc3)
	    { posx <<= 1;
	      if (elm.local == index<2>(0, 0)) { squish::CompressAlphaBtc3(iTex[0],    0xFFFF, b0, b1       ); dArr(posy, posx + 0) = b0;
													       dArr(posy, posx + 1) = b1; }
	      if (elm.local == index<2>(0, 2)) { squish::CompressColorBtc (iTex[0],    0xFFFF, b2, b3, flags); dArr(posy, posx + 2) = b2;
													       dArr(posy, posx + 3) = b3; }
	    }
	    break;
	  case 2:
	    /* 2x LONG for ATI2 */
	    if (format == TCOMPRESS_XYz)
	    { posx <<= 1;
	      if (elm.local == index<2>(0, 0)) { squish::CompressNormalBtc5(iTex[0],   0xFFFF, b0, b1, b2, b3); dArr(posy, posx + 0) = b0;
													        dArr(posy, posx + 1) = b1; }
	      if (elm.local == index<2>(0, 1)) { squish::CompressNormalBtc5(iTex[0],   0xFFFF, b0, b1, b2, b3); dArr(posy, posx + 2) = b2;
													        dArr(posy, posx + 3) = b3; }
	    }
	    else
	    { posx <<= 1;
	      if (elm.local == index<2>(0, 0)) { squish::CompressAlphaBtc3(iTex[0],    0xFFFF, b0, b1       ); dArr(posy, posx + 0) = b0;
													       dArr(posy, posx + 1) = b1; }
	      if (elm.local == index<2>(0, 1)) { squish::CompressAlphaBtc3(iTex[0],    0xFFFF, b2, b3       ); dArr(posy, posx + 2) = b2;
													       dArr(posy, posx + 3) = b3; }
	    }
	    break;
	  case 1:
	    /* 1x LONG for ATI1 */
	    { posx <<= 0;
	      if (elm.local == index<2>(0, 0)) { squish::CompressAlphaBtc3(iTex[0],    0xFFFF, b0, b1       ); dArr(posy, posx + 0) = b0;
													       dArr(posy, posx + 1) = b1; }
	    }
	    break;
        }

//	elm.barrier.wait();

//      /* advance pointer of compressed blocks */
//      wTex += blocksize;
//      dTex += blocksize;
      }

//    elm.barrier.wait();

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
			 firstprivate(dTex, blocksize)
    for (int y = 0; y < (int)texd.Height; y += TY) {
#endif
      if (!(y & 0x3F)) {
//	logrf("level %2d/%2d: line %4d/%4d processed        \r", l + 1, levels, y, texd.Height);

	/* problematic, the throw() inside may not block all threads ... */
//	PollProgress();
      }

      /* calculate pointer of compressed blocks */
      ULONG *wTex = (ULONG *)texr;
//    wTex += ((y + TY - 1) / TY) * ((texo.Width + TX - 1) / TX) * blocksize;
      wTex += ((y + TY - 1) / TY) * dPch;
//    assert(wTex == dTex);

//#pragma omp parallel if((int)texd.Height < (lv >> 1)) shared(sTex) firstprivate(wTex)
    for (int x = 0; x < (int)texd.Width; x += TX) {
      /* nested lambda-templates not possible in VS2010 */
      static const f<format> fmt;
      typedef type a16 accu[DIM];

      a16 UTYPE bTex[1][TY][TX];
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

	  for (int oy = 0; oy < (1 << 1); oy += 1)
	  for (int ox = 0; ox < (1 << 1); ox += 1) {
	    /* assume seamless tiling: wrap pixels around */
	    const int posx = (xl + ox) % wSpill;
	    const int posy = (yl + oy) % hSpill;

	    /* TODO: scale-by-level types */
	    accu &tt = (accu &)dSpill[((posy * wSpill) + posx) * DIM];

	    Reduce(ts, tt);	// +=
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
	  Spill(st, ts);
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

	Code(fTex[0][ly][lx], tr, (TCOMPRESS_CHANNELS(format) +
				  (TCOMPRESS_GREYS   (format) ? 2 : 0)) == 2 ? 8 :
				  (TCOMPRESS_SWIZZL  (format) ? 6 : 5)); t =
	Join(fTex[0][ly][lx], tr);

	/* swizzle ABGR -> AGBR */
        if (TCOMPRESS_SWIZZL(format))
//	  t = (t & 0xFFFF0000) + ((t >> 8) & 0x00FF) + ((t & 0x00FF) << 8);
	  t = (t & 0xFF0000FF) + ((t >> 8) & 0xFF00) + ((t & 0xFF00) << 8);

	/* write the result ABGR, BGR */
        switch (TCOMPRESS_CHANNELS(format) + (TCOMPRESS_GREYS(format) ? 2 : 0)) {
          /* ABGR -> RGBA */
          case 4:                                   bTex[0][ly][lx] = (t) | 0x00000000; break;
          /* -BGR -> RGB- */
	  case 3:                                   bTex[0][ly][lx] = (t) | 0xFF000000; break;
	  /* --YX -> XYXY */
	  /* AL-- -> LALA */
          case 2: /**/ if (format == TCOMPRESS_XYz) bTex[0][ly][lx] = ((t & 0xFFFF0000) + _byteswap_ushort((unsigned short)t)) | 0xFF000000;
		  else if (format == TCOMPRESS_xy ) bTex[0][ly][lx] = _byteswap_ushort((unsigned short)t) + (_byteswap_ushort((unsigned short)t) << 16);
		  else if (format == TCOMPRESS_XY ) bTex[0][ly][lx] = _byteswap_ushort((unsigned short)t) + (_byteswap_ushort((unsigned short)t) << 16);
		  else                              bTex[0][ly][lx] = ((t & 0xFFFF0000) + (t >> 16));
		  break;
          /* -Z-- -> Z--- */
	  /* A--- -> A--- */
	  /* -LLL -> L--- */
          case 1: /**/ if (format == TCOMPRESS_a  ) bTex[0][ly][lx] = (t <<  0) & 0xFF000000;
		  else if (format == TCOMPRESS_A  ) bTex[0][ly][lx] = (t <<  0) & 0xFF000000;
		  else if (format == TCOMPRESS_xyZ) bTex[0][ly][lx] = (t <<  8) & 0xFF000000;
//        	  else if (format == TCOMPRESS_z  ) bTex[0][ly][lx] = (t <<  8) & 0xFF000000;
//        	  else if (format == TCOMPRESS_Z  ) bTex[0][ly][lx] = (t <<  8) & 0xFF000000;
          	  else                              bTex[0][ly][lx] = (t << 24) & 0xFF000000;
          	  break;
        }
      }

      /* compress to DXT1/DXT3/DXT5/ATI1/ATI2 */
      squish::Compress((const unsigned char *)bTex[0], wTex + 0, flags);

      /* advance pointer of compressed blocks */
      wTex += blocksize;
      dTex += blocksize;
    }
#if	defined(SQUASH_USE_AMP) || defined(SQUASH_USE_CCR)
    });
#else
    }
#endif
#endif

}

template<typename UTYPE, typename type, const int format>
bool TextureCompressDXT(int minlevel, LPDIRECT3DBASETEXTURE *tex, bool optimize) {
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
    struct histogram h;
    memset((void *)&h, 0, sizeof(h));
    h.grey = h.blank = h.black = true;

    ULONG sPch, *sTex =
    TextureLock((*tex), 0, -1, &sPch); sPch >>= 2;
    TextureMatte<format>(texo, sPch, sTex);
    HistogramCompressDXT(&h, texo, sPch, sTex);
    TextureUnlock((*tex), 0, -1);

    for (unsigned int c = 0; c < 256; c += 1) {
      if (h.histo[0][c]) h.histn[0]++;
      if (h.histo[1][c]) h.histn[1]++;
      if (h.histo[2][c]) h.histn[2]++;
      if (h.histo[3][c]) h.histn[3]++;
    }

    if ((h.histn[0] == 1) &&
	(h.histn[1] == 1) &&
	(h.histn[2] == 1) &&
	(h.histn[3] == 1)) {
      addnote(" Planar image detected, collapsing to size 1x1.\n");

      return TextureCollapse(tex, TEXFMT_A8R8G8B8, TCOMPRESS_SWIZZL(format));
    }

    bool white = ((h.histn[0] == 1) && h.histo[0][0xFF]                    );
    bool black = ((h.histn[0] == 1) &&                     h.histo[0][0x00]);
    bool blawh = ((h.histn[0] == 2) && h.histo[0][0xFF] && h.histo[0][0x00]);

    /* if it wasn't transparent it must be uncorrelated! */
    if (h.grey) {
      if ((TCOMPRESS_CHANNELS(format) + (TCOMPRESS_GREYS(format) ? 2 : 0)) > 2)
	addnote(" Automatic greyscale conversion.\n");

      /* give the same 1:4 compression at no loss (DXT1 is 1:6) */
      /* check if Alpha is killable */
      if (CollapseAlpha(format, black, white) && (origFormat != TEXFMT_DXT1) && (origFormat != TEXFMT_DXTM)) {
	if (ExistAlpha(format, origFormat))
	  addnote(" Automatic dropped alpha-channel.\n");

	/* these are already destroyed, no point in elevation */
	if (TEXFORMAT::Available(TEXFMT_L8) && !TEXFMT_BTC(origFormat))
	  return TextureConvertRAW<UTYPE, type, TCOMPRESS_L >(minlevel, tex, false);
	else
	  origFormat = TEXFMT_DXT1;
      }
      /* check if Color is killable */
      else if (h.blank) {
	if (TCOMPRESS_TRANS(format))
	  addnote(" Automatic dropped color-channels.\n");

	/* may even go down to A1 */
	if (TEXFORMAT::Available(TEXFMT_A8) && TCOMPRESS_TRESH(format))
	  return TextureConvertRAW<UTYPE, type, TCOMPRESS_a >(minlevel, tex, true);
	if (TEXFORMAT::Available(TEXFMT_A8) && TCOMPRESS_TRANS(format))
	  return TextureConvertRAW<UTYPE, type, TCOMPRESS_A >(minlevel, tex, true);
      }
      /* gives only 1:2 or 1:4 compression (DXT1 is 1:6, other 1:4) */
      else if (!TEXFMT_BTC(origFormat)) {
	/* may even go down to A4L4 */
	if (TEXFORMAT::Available(TEXFMT_A8L8) && TCOMPRESS_TRESH(format))
	  return TextureConvertRAW<UTYPE, type, TCOMPRESS_La>(minlevel, tex, true);
	if (TEXFORMAT::Available(TEXFMT_A8L8) && TCOMPRESS_TRANS(format))
	  return TextureConvertRAW<UTYPE, type, TCOMPRESS_LA>(minlevel, tex, true);
	if (TEXFORMAT::Available(TEXFMT_A8L8))
	  return TextureConvertRAW<UTYPE, type, TCOMPRESS_LH>(minlevel, tex, true);
      }
    }

    /* two colors in alpha (if colours appear in black alpha areas don't do DXT1) */
    if ((h.histn[0] <= 2)) {
      /* check if Alpha is killable */
      if (CollapseAlpha(format, black, white) && (origFormat != TEXFMT_DXT1)) {
	if (ExistAlpha(format, origFormat))
	  addnote(" Automatic dropped alpha-channel.\n");

	/* "drop" the alpha-channel, 1bit alpha */
	if (TEXFORMAT::Available(TEXFMT_DXT1) && TCOMPRESS_COLOR(format) /*&& TCOMPRESS_SIDES(format)*/)
	  return TextureCompressDXT<UTYPE, type, TCOMPRESS_RGB>(minlevel, tex, false);
	if (TEXFORMAT::Available(TEXFMT_DXT1) && (format == TCOMPRESS_xyzD))
	  return TextureCompressDXT<UTYPE, type, TCOMPRESS_xyz>(minlevel, tex, false);
	if (TEXFORMAT::Available(TEXFMT_DXT1) && (format == TCOMPRESS_XYZD))
	  return TextureCompressDXT<UTYPE, type, TCOMPRESS_XYZ>(minlevel, tex, false);
	if (TEXFORMAT::Available(TEXFMT_DXT1) && (format == TCOMPRESS_XZYD))
	  return TextureCompressDXT<UTYPE, type, TCOMPRESS_XZY>(minlevel, tex, false);
      }
      /* black and white?, destroy color */
      else if (MinimalAlpha(format, black, white, blawh)) {
	/* if it wasn't transparent it must be uncorrelated! */
	if (TCOMPRESS_TRANS(format) && h.black)
	  origFormat = TEXFMT_DXT1;
      }
    }

    /* sixteen colors in alpha */
    if (!white && (h.histn[0] >= 1) && (h.histn[0] <= 16)) {
      unsigned long histc = 0;
      if (h.histo[0][0x00]) histc++; if (h.histo[0][0x11]) histc++;
      if (h.histo[0][0x22]) histc++; if (h.histo[0][0x33]) histc++;
      if (h.histo[0][0x44]) histc++; if (h.histo[0][0x55]) histc++;
      if (h.histo[0][0x66]) histc++; if (h.histo[0][0x77]) histc++;
      if (h.histo[0][0x88]) histc++; if (h.histo[0][0x99]) histc++;
      if (h.histo[0][0xAA]) histc++; if (h.histo[0][0xBB]) histc++;
      if (h.histo[0][0xCC]) histc++; if (h.histo[0][0xDD]) histc++;
      if (h.histo[0][0xEE]) histc++; if (h.histo[0][0xFF]) histc++;

      /* quantized only? should be extreme rare ... */
      if (histc == h.histn[0]) {
	if (!white && !blawh) {
	  if ((origFormat != TEXFMT_DXT3) &&
	      (origFormat != TEXFMT_A4R4G4B4) &&
	      (origFormat != TEXFMT_A4L4))
	    addnote(" Quantized alpha-channel detected.\n");

	  if (h.grey && (h.histn[1] >= 1) && (h.histn[1] <= 16)) {
	    unsigned long histc = 0;
	    if (h.histo[1][0x00]) histc++; if (h.histo[1][0x11]) histc++;
	    if (h.histo[1][0x22]) histc++; if (h.histo[1][0x33]) histc++;
	    if (h.histo[1][0x44]) histc++; if (h.histo[1][0x55]) histc++;
	    if (h.histo[1][0x66]) histc++; if (h.histo[1][0x77]) histc++;
	    if (h.histo[1][0x88]) histc++; if (h.histo[1][0x99]) histc++;
	    if (h.histo[1][0xAA]) histc++; if (h.histo[1][0xBB]) histc++;
	    if (h.histo[1][0xCC]) histc++; if (h.histo[1][0xDD]) histc++;
	    if (h.histo[1][0xEE]) histc++; if (h.histo[1][0xFF]) histc++;

	    /* quantized only? should be extreme rare ... */
	    if (histc == h.histn[1]) {
	      if (origFormat != TEXFMT_A4L4)
		addnote(" Quantized grey-channel detected.\n");

	      /* may even go down to A4L4 */
		if (TEXFORMAT::Available(TEXFMT_A8L8) && TCOMPRESS_TRESH(format))
		  return TextureConvertRAW<UTYPE, type, TCOMPRESS_La>(minlevel, tex, true);
	      else
		if (TEXFORMAT::Available(TEXFMT_A8L8) && TCOMPRESS_TRANS(format))
		  return TextureConvertRAW<UTYPE, type, TCOMPRESS_LA>(minlevel, tex, true);
	      else
		if (TEXFORMAT::Available(TEXFMT_A8L8))
		  return TextureConvertRAW<UTYPE, type, TCOMPRESS_LH>(minlevel, tex, true);
	    }
	  }

	  /* go DXT3 (this can even happen if DXT5 was used
	   * with black and white end-points)
	   */
	  origFormat = TEXFMT_DXT3;
	}
      }
    }

    /* TODO: compare RMS of DXT3 vs. DXT5 and choose */
    if (verbose && ExistAlpha(format, origFormat)) {
      /**/ if (blawh && h.black)
	addnote(" Alpha-channel is black'n'white, and all black alphas also have a black color.\n");
      else if (blawh)
	addnote(" Alpha-channel is black'n'white, and black alphas have all kind of colors.\n");
      else if (black)
	addnote(" Alpha-channel is just black.\n");
      else if (white)
	addnote(" Alpha-channel is just white.\n");
      else if (h.histo[0][0xFF] || h.histo[0][0x00])
	addnote(" Alpha-channel has %d distinct value(s), including %s.\n", h.histn[0], h.histo[0][0xFF] && h.histo[0][0x00] ? "black and white" : (h.histo[0][0x00] ? "black" : "white"));
      else
	addnote(" Alpha-channel has %d distinct value(s), without black or white.\n", h.histn[0]);
    }
  }

  /* create the textures */
  const int levels = TextureCalcMip(texo.Width, texo.Height, minlevel);
  
  int squish__kBtcD = squish::kBtc3;
  int flags;

  /* smart max. quality settings */
  if (manualflags == -1) {
    flags = 0;

    if (optimizequick)
      flags = squish::kColourRangeFit;
    else
      flags = squish::kColourIterativeClusterFit | squish::kAlphaIterativeFit;
  }
  /* enforce desired format */
  else {
    flags = manualflags;

    if (flags & squish::kBtc5) origFormat = TEXFMT_ATI2;
    if (flags & squish::kBtc4) origFormat = TEXFMT_ATI1;
    if (flags & squish::kBtc3) origFormat = TEXFMT_DXT5;
    if (flags & squish::kBtc2) origFormat = TEXFMT_DXT3;
    if (flags & squish::kBtc1) origFormat = TEXFMT_DXT1;
  }

#ifdef DX11
  ID3D11Texture2D *rtex; D3D11_TEXTURE2D_DESC cr;
  memset(&cr, 0, sizeof(cr));

  cr.Width  = (texo.Width  + (TX - 1)) & (~(TX - 1));
  cr.Height = (texo.Height + (TY - 1)) & (~(TY - 1));
  cr.MipLevels = levels;
  cr.ArraySize = texo.Slices;

  TEXFORMAT tf = TEXFMT_UNKNOWN;
  switch (TCOMPRESS_CHANNELS(format) + (TCOMPRESS_GREYS(format) ? 2 : 0)) {
    case 4: if (origFormat == TEXFMT_DXT5)
	    flags |= squish::kBtc3, tf = TEXFMT_DXT5; else
	    if (origFormat == TEXFMT_DXT4)
	    flags |= squish::kBtc3, tf = TEXFMT_DXT4; else
	    if (origFormat == TEXFMT_DXT3)
	    flags |= squish::kBtc2, tf = TEXFMT_DXT3; else
	    if (origFormat == TEXFMT_DXT2)
	    flags |= squish::kBtc2, tf = TEXFMT_DXT2; else
	    if (origFormat == TEXFMT_DXT1)
	    flags |= squish::kBtc1, tf = TEXFMT_DXT1; else

	    flags |= squish__kBtcD, tf = TEXFMT_DXT5; break;
    case 3: flags |= squish::kBtc1; tf = TEXFMT_DXT1; break;
    case 2: flags |= squish::kBtc5; tf = TEXFMT_ATI2; break;
    case 1: flags |= squish::kBtc4; tf = TEXFMT_ATI1; break;
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
  D3DFORMAT dxtFormat = D3DFMT_UNKNOWN;
  int dxtWidth  = (texo.Width  + (TX - 1)) & (~(TX - 1));
  int dxtHeight = (texo.Height + (TY - 1)) & (~(TY - 1));

  HRESULT res;
  switch (TCOMPRESS_CHANNELS(format) + (TCOMPRESS_GREYS(format) ? 2 : 0)) {
    case 4: if (origFormat == D3DFMT_DXT5)
	    flags |= squish::kBtc3, dxtFormat = D3DFMT_DXT5; else
	    if (origFormat == D3DFMT_DXT4)
	    flags |= squish::kBtc3, dxtFormat = D3DFMT_DXT4; else
	    if (origFormat == D3DFMT_DXT3)
	    flags |= squish::kBtc2, dxtFormat = D3DFMT_DXT3; else
	    if (origFormat == D3DFMT_DXT2)
	    flags |= squish::kBtc2, dxtFormat = D3DFMT_DXT2; else
	    if (origFormat == D3DFMT_DXT1)
	    flags |= squish::kBtc1, dxtFormat = D3DFMT_DXT1; else

	    flags |= squish__kBtcD, dxtFormat = D3DFMT_DXT5; break;
    case 3: flags |= squish::kBtc1; dxtFormat = D3DFMT_DXT1; break;
//  case 2: flags |= squish::kBtc5; dxtFormat = D3DFMT_ATI2; break;
//  case 1: flags |= squish::kBtc4; dxtFormat = D3DFMT_ATI1; break;
    case 2: flags |= squish::kBtc5; dxtFormat = D3DFMT_DXT3; break;
    case 1: flags |= squish::kBtc4; dxtFormat = D3DFMT_DXT1; break;
  }

  res = pD3DDevice->CreateTexture(dxtWidth, dxtHeight, levels, 0, dxtFormat, D3DPOOL_SYSTEMMEM, (IDirect3DTexture9 **)&text, NULL);

  /* retry */
  if (!text) {
    switch (TCOMPRESS_CHANNELS(format) + (TCOMPRESS_GREYS(format) ? 2 : 0)) {
      /* ATI1/2 without hardware, mem-layout (alignment etc.) is the same as DXT1/3 */
      case 2: flags |= squish::kBtc5; dxtFormat = D3DFMT_DXT3; break;
      case 1: flags |= squish::kBtc4; dxtFormat = D3DFMT_DXT1; break;
    }

    res = pD3DDevice->CreateTexture(dxtWidth, dxtHeight, levels, 0, dxtFormat, D3DPOOL_SYSTEMMEM, (IDirect3DTexture9 **)&text, NULL);
  }
#endif

  /* damit */
  if (!text)
    return false;

  /* smart max. quality settings */
  if (manualflags == -1) {
    /**/ if (TCOMPRESS_TRANS(format))
      flags |= squish::kWeightColourByAlpha;
    /**/ if (TCOMPRESS_GREYS(format))
      flags |= squish::kColourMetricUniform;
    else if (TCOMPRESS_COLOR(format))
      flags |= squish::kColourMetricPerceptual;
    else if (TCOMPRESS_NORMAL(format))
      flags |= squish::kColourMetricUniform;
  }

  /* calculate pointer of compressed blocks */
  int blocksize = 0;
  switch (TCOMPRESS_CHANNELS(format) + (TCOMPRESS_GREYS(format) ? 2 : 0)) {
    case 4: if (!(flags & squish::kBtc1)) {
	    blocksize = ((8+8) / 4); break; }/* 4x4x4 -> 16bytes */
    case 3: blocksize = (( 8 ) / 4); break;  /* 4x4x3 ->  8bytes */
    case 2: blocksize = ((4+4) / 2); break;  /* 4x4x2 -> 16bytes */
    case 1: blocksize = (( 4 ) / 2); break;  /* 4x4x1 ->  8bytes */
  }

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
    TextureLock(text, l, -1, &dPch, true); dPch >>= 2;

    TextureCompressDXT<UTYPE, type, format>(smem, texo, texd, sPch, dPch, texs, texr, levels, l, blocksize, flags);

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

/* we don't process normal-maps as integers */
template<> bool TextureCompressDXT<ULONG, long, TCOMPRESS_xyz >(int minlevel, LPDIRECT3DBASETEXTURE *tex, bool optimize) { return false; }
template<> bool TextureCompressDXT<ULONG, long, TCOMPRESS_XYZ >(int minlevel, LPDIRECT3DBASETEXTURE *tex, bool optimize) { return false; }
template<> bool TextureCompressDXT<ULONG, long, TCOMPRESS_XZY >(int minlevel, LPDIRECT3DBASETEXTURE *tex, bool optimize) { return false; }

template<> bool TextureCompressDXT<ULONG, long, TCOMPRESS_xyzD>(int minlevel, LPDIRECT3DBASETEXTURE *tex, bool optimize) { return false; }
template<> bool TextureCompressDXT<ULONG, long, TCOMPRESS_XYZD>(int minlevel, LPDIRECT3DBASETEXTURE *tex, bool optimize) { return false; }
template<> bool TextureCompressDXT<ULONG, long, TCOMPRESS_XZYD>(int minlevel, LPDIRECT3DBASETEXTURE *tex, bool optimize) { return false; }

template<> bool TextureCompressDXT<ULONG, long, TCOMPRESS_xyzV>(int minlevel, LPDIRECT3DBASETEXTURE *tex, bool optimize) { return false; }
template<> bool TextureCompressDXT<ULONG, long, TCOMPRESS_XYZV>(int minlevel, LPDIRECT3DBASETEXTURE *tex, bool optimize) { return false; }
template<> bool TextureCompressDXT<ULONG, long, TCOMPRESS_XZYV>(int minlevel, LPDIRECT3DBASETEXTURE *tex, bool optimize) { return false; }

} // namespace squash

/* ####################################################################################
 */

namespace squash {

bool TextureCompressRGBA(LPDIRECT3DBASETEXTURE *base, int minlevel, bool gamma, bool contrast) {
  bool res = true;

  if   (contrast) res = res && TextureCompressDXT<ULONG, float, TCOMPRESS_RGBA>(minlevel, base, manualflags == -1);
  else if (gamma) res = res && TextureCompressDXT<ULONG, float, TCOMPRESS_RGBA>(minlevel, base, manualflags == -1);
  else            res = res && TextureCompressDXT<ULONG, long , TCOMPRESS_RGBA>(minlevel, base, manualflags == -1);

  return res;
}

bool TextureCompressRGB_A(LPDIRECT3DBASETEXTURE *base, int minlevel, bool gamma, bool contrast) {
  bool res = true;

  if   (contrast) res = res && TextureCompressDXT<ULONG, float, TCOMPRESS_RGBa>(minlevel, base, manualflags == -1);
  else if (gamma) res = res && TextureCompressDXT<ULONG, float, TCOMPRESS_RGBa>(minlevel, base, manualflags == -1);
  else            res = res && TextureCompressDXT<ULONG, long , TCOMPRESS_RGBa>(minlevel, base, manualflags == -1);

  return res;
}

bool TextureCompressRGBH(LPDIRECT3DBASETEXTURE *base, int minlevel, bool gamma) {
  bool res = true;

  if (gamma) res = res && TextureCompressDXT<ULONG, float, TCOMPRESS_RGBH>(minlevel, base, manualflags == -1);
  else       res = res && TextureCompressDXT<ULONG, long , TCOMPRESS_RGBH>(minlevel, base, manualflags == -1);

  return res;
}

bool TextureCompressRGB(LPDIRECT3DBASETEXTURE *base, int minlevel, bool gamma) {
  bool res = true;

  if (gamma) res = res && TextureCompressDXT<ULONG, float, TCOMPRESS_RGB>(minlevel, base, manualflags == -1);
  else       res = res && TextureCompressDXT<ULONG, long , TCOMPRESS_RGB>(minlevel, base, manualflags == -1);

  return res;
}

bool TextureCompressLA(LPDIRECT3DBASETEXTURE *base, int minlevel, bool gamma, bool contrast) {
  bool res = true;

  if   (contrast) res = res && TextureCompressDXT<ULONG, float, TCOMPRESS_LA>(minlevel, base, manualflags == -1);
  else if (gamma) res = res && TextureCompressDXT<ULONG, float, TCOMPRESS_LA>(minlevel, base, manualflags == -1);
  else            res = res && TextureCompressDXT<ULONG, long , TCOMPRESS_LA>(minlevel, base, manualflags == -1);

  return res;
}

bool TextureCompressL_A(LPDIRECT3DBASETEXTURE *base, int minlevel, bool gamma, bool contrast) {
  bool res = true;

  if   (contrast) res = res && TextureCompressDXT<ULONG, float, TCOMPRESS_La>(minlevel, base, manualflags == -1);
  else if (gamma) res = res && TextureCompressDXT<ULONG, float, TCOMPRESS_La>(minlevel, base, manualflags == -1);
  else            res = res && TextureCompressDXT<ULONG, long , TCOMPRESS_La>(minlevel, base, manualflags == -1);

  return res;
}

bool TextureCompressLH(LPDIRECT3DBASETEXTURE *base, int minlevel, bool gamma) {
  bool res = true;

  if (gamma) res = res && TextureCompressDXT<ULONG, float, TCOMPRESS_LH>(minlevel, base, manualflags == -1);
  else       res = res && TextureCompressDXT<ULONG, long , TCOMPRESS_LH>(minlevel, base, manualflags == -1);

  return res;
}

bool TextureCompressL(LPDIRECT3DBASETEXTURE *base, int minlevel, bool gamma) {
  bool res = true;

  if (gamma) res = res && TextureCompressDXT<ULONG, float, TCOMPRESS_L>(minlevel, base, manualflags == -1);
  else       res = res && TextureCompressDXT<ULONG, long , TCOMPRESS_L>(minlevel, base, manualflags == -1);

  return res;
}

bool TextureCompressA(LPDIRECT3DBASETEXTURE *base, int minlevel, bool contrast) {
  bool res = true;

  if (contrast) res = res && TextureCompressDXT<ULONG, float, TCOMPRESS_A>(minlevel, base, manualflags == -1);
  else          res = res && TextureCompressDXT<ULONG, long , TCOMPRESS_A>(minlevel, base, manualflags == -1);

  return res;
}

bool TextureCompress_A(LPDIRECT3DBASETEXTURE *base, int minlevel, bool contrast) {
  bool res = true;

  if (contrast) res = res && TextureCompressDXT<ULONG, float, TCOMPRESS_a>(minlevel, base, manualflags == -1);
  else          res = res && TextureCompressDXT<ULONG, long , TCOMPRESS_a>(minlevel, base, manualflags == -1);

  return res;
}

bool TextureCompressXYZD(LPDIRECT3DBASETEXTURE *norm, int minlevel) {
  bool res = true;

  res = res && TextureCompressDXT<ULONG, float, TCOMPRESS_XYZD>(minlevel, norm, manualflags == -1);

  return res;
}

bool TextureCompress_XYZD(LPDIRECT3DBASETEXTURE *norm, int minlevel) {
  bool res = true;

  res = res && TextureCompressDXT<ULONG, float, TCOMPRESS_xyzD>(minlevel, norm, manualflags == -1);

  return res;
}

bool TextureCompress_XY_Z(LPDIRECT3DBASETEXTURE *norm, LPDIRECT3DBASETEXTURE *z, int minlevel) {
  bool res = true;

  /* TODO: not really fast */
  if (z && norm && (*z == *norm)) (*z)->AddRef();

  if (norm) res = res && TextureCompressDXT<ULONG, float, TCOMPRESS_XYz>(minlevel, norm, manualflags == -1);
  if (z   ) res = res && TextureCompressDXT<ULONG, float, TCOMPRESS_xyZ>(minlevel, z   , manualflags == -1);

  return res;
}

bool TextureCompressXY_Z(LPDIRECT3DBASETEXTURE *norm, LPDIRECT3DBASETEXTURE *z, int minlevel) {
  bool res = true;

  /* TODO: not really fast */
  if (z && norm && (*z == *norm)) (*z)->AddRef();

  if (norm) res = res && TextureCompressDXT<ULONG, float, TCOMPRESS_XYZ>(minlevel, norm, manualflags == -1);
  if (z   ) res = res && TextureCompressDXT<ULONG, float, TCOMPRESS_XYZ>(minlevel, z   , manualflags == -1);

  return res;
}

bool TextureCompressXYZ(LPDIRECT3DBASETEXTURE *norm, int minlevel) {
  bool res = true;

  res = res && TextureCompressDXT<ULONG, float, TCOMPRESS_XYZ>(minlevel, norm, manualflags == -1);

  return res;
}

bool TextureCompressXZY(LPDIRECT3DBASETEXTURE *norm, int minlevel) {
  bool res = true;

  res = res && TextureCompressDXT<ULONG, float, TCOMPRESS_XZY>(minlevel, norm, manualflags == -1);

  return res;
}

bool TextureCompress_XYZ(LPDIRECT3DBASETEXTURE *norm, int minlevel) {
  bool res = true;

  res = res && TextureCompressDXT<ULONG, float, TCOMPRESS_xyz>(minlevel, norm, manualflags == -1);

  return res;
}

bool TextureCompressXY(LPDIRECT3DBASETEXTURE *norm, int minlevel) {
  bool res = true;

  res = res && TextureCompressDXT<ULONG, float, TCOMPRESS_XY>(minlevel, norm, manualflags == -1);

  return res;
}

bool TextureCompress_XY(LPDIRECT3DBASETEXTURE *norm, int minlevel) {
  bool res = true;

  res = res && TextureCompressDXT<ULONG, float, TCOMPRESS_xy>(minlevel, norm, manualflags == -1);

  return res;
}

bool TextureCompressZ(LPDIRECT3DBASETEXTURE *norm, int minlevel) {
  bool res = true;

//res = res && TextureCompressDXT<ULONG, float, TCOMPRESS_Z  >(minlevel, norm, manualflags == -1);
  res = res && TextureCompressDXT<ULONG, float, TCOMPRESS_XYZ>(minlevel, norm, manualflags == -1);

  return res;
}

bool TextureCompress_Z(LPDIRECT3DBASETEXTURE *norm, int minlevel) {
  bool res = true;

  res = res && TextureCompressDXT<ULONG, float, TCOMPRESS_xyZ>(minlevel, norm, manualflags == -1);

  return res;
}

bool TextureCompressPM(LPDIRECT3DBASETEXTURE *base, LPDIRECT3DBASETEXTURE *norm, int minlevel) {
  bool res = true;

  res = res && TextureCompressDXT<ULONG, long , TCOMPRESS_RGBH>(minlevel, base, manualflags == -1);
  res = res && TextureCompressDXT<ULONG, float, TCOMPRESS_XYZD>(minlevel, norm, manualflags == -1);

  return res;
}

} // namespace squash
