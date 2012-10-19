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

#include "filterdispatch.h"

  template<typename UTYPE, typename type>
  void ff(TextureCompressDXT,format,coding,fiting)(RESOURCEINFO &texo, RESOURCEINFO &texd, ULONG *texs, ULONG *texr, int level, int l, int blocksize, int flags) {
    /* square dimension of this surface-level */
    /* square area of this surface-level */
    const int lv = (1 << l);
    const int av = lv * lv;

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
    cheight = (cheight + 3) / 4;	/* 4x4 LONG ... */
    cwidth  = (cwidth  + 3) / 4;	/* 4x4 LONG ... */
    cwidth *= 2 * blocksize;		/* ... to 2x|4x LONG */

    /* ensure tile ability (bit on overhead for non-4 resolutions) */
    owidth  = (owidth  + (TX - 1)) & (~(TX - 1));
    oheight = (oheight + (TY - 1)) & (~(TY - 1));

    assert((owidth  & (TX - 1)) == 0);
    assert((oheight & (TY - 1)) == 0);

#if	defined(SQUASH_USE_AMP) && !defined(SQUASH_USE_AMP_DEBUG)
    /* constant buffer array */
    Concurrency::array_view<const SingleColourLookup_CCR, 2> lArr(2, 256, (const SingleColourLookup_CCR *)::lookup_34_56_ccr);
    Concurrency::array_view<const   IndexBlockLookup_CCR, 2> yArr(4, 8,   (const   IndexBlockLookup_CCR *)::lookup_c34a57_ccr);

    /* get a two-dimensional extend over the whole output (without re-cast to LONG),
     * then get a tile-extend over that one ()
     */
    Concurrency::extent<2> ee(oheight, owidth);
    Concurrency::tiled_extent<TY, TX> te(ee);

    Concurrency::array_view<const unsigned int, 2> sArr(iheight, iwidth, (const unsigned int *)texs);
    Concurrency::array_view<      unsigned int, 2> dArr(cheight, cwidth, (      unsigned int *)texr);

    Concurrency::parallel_for_each(te /*dArr.extent.tile<TY, TX>(osize)*/, [=](tiled_index<TY, TX> elm) restrict(amp) {
      typedef type accu[DIM];

      /* tile static memory */
//    tile_static UTYPE bTex[2][TY*TX];
      tile_static type  fTex[2][TY*TX][DIM];
      tile_static int   iTex[2][TY*TX][DIM];

      /* generate this level's 4x4-block from the original surface */
//    const int y = elm.global[0] - ly;
//    const int x = elm.global[1] - lx;
      const int y = elm.tile[0] * TY;
      const int x = elm.tile[1] * TX;
      const int ly = elm.local[0];
      const int lx = elm.local[1];
      const int lxy = ly * TX + lx;
#else
    Concurrency::array_view<const SingleColourLookup_CCR, 2> lArr(2, 256, (const SingleColourLookup_CCR *)::lookup_34_56_ccr);

    Concurrency::array_view<const unsigned int, 2> sArr(iheight, iwidth, (const unsigned int *)texs);
    Concurrency::array_view<      unsigned int, 2> dArr(cheight, cwidth, (      unsigned int *)texr, true);

    for (int groupsy = 0; groupsy < (owidth  / TY); groupsy++)
    for (int groupsx = 0; groupsx < (oheight / TX); groupsx++) {
      typedef type accu[DIM];

      /* tile static memory */
//    UTYPE bTex[2][TY*TX];
      type  fTex[2][TY*TX][DIM];
      int   iTex[2][TY*TX][DIM];

      for (int tiley = 0; tiley < TY; tiley++)
      for (int tilex = 0; tilex < TX; tilex++)
      {
	const int y = groupsy * TY;
	const int x = groupsx * TX;
	const int ly = tiley;
	const int lx = tilex;
	const int lxy = ly * TX + lx;
#endif

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

	  Accu(tt, t);	// +=
	}
	}

	/* build average of each channel */
	Norm(fTex[0][lxy], tt, av, level, l);
      }

#if	defined(SQUASH_USE_AMP) && !defined(SQUASH_USE_AMP_DEBUG)
      tile_static accu tr; tr[lxy & 7] = 0;

      tile_static_memory_fence(elm.barrier);
//    elm.barrier.wait_with_tile_static_memory_fence();
#else
      }

      accu tr = {0};
#endif

      /* runs on only 1 thread per tile (reduction) */
#if	defined(SQUASH_USE_AMP) && !defined(SQUASH_USE_AMP_DEBUG)
      if (elm.local == index<2>(0, 0))
#endif
      {
	/* analyze this level's 4x4-block */
	for (int lxy = 0; lxy < TY*TX; lxy += 1) {
	  Look(fTex[0][lxy], tr);
	}
      }

#if	defined(SQUASH_USE_AMP) && !defined(SQUASH_USE_AMP_DEBUG)
      tile_static_memory_fence(elm.barrier);
//    elm.barrier.wait_with_tile_static_memory_fence();
#else
      for (int tiley = 0; tiley < TY; tiley++)
      for (int tilex = 0; tilex < TX; tilex++)
      {
	const int y = groupsy;
	const int x = groupsx;
	const int ly = tiley;
	const int lx = tilex;
	const int lxy = ly * TX + lx;
#endif

      /* generate this level's 4x4-block from the original surface */
      {
	/* build average of each channel an join */
	Code (fTex[0][lxy], tr,
	  (TCOMPRESS_CHANNELS(format) +
	  (TCOMPRESS_GREYS   (format) ? 2 : 0)) == 2 ? 8 :
	  (TCOMPRESS_SWIZZL  (format) ? 6 : 5));
	Range(iTex[0][lxy],
	      fTex[0][lxy]);

#if	(TCOMPRESS_SWIZZL(format))
	/* swizzle ABGR -> AGBR */
        {
	  int swap =        iTex[0][lxy][1];
	  iTex[0][lxy][1] = iTex[0][lxy][2];
	  iTex[0][lxy][2] = swap           ;
	}
#endif

	/* write the result */

	/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#if	((TCOMPRESS_CHANNELS(format) + (TCOMPRESS_GREYS(format) ? 2 : 0)) == 4)
	/* ABGR -> RGBA */
	/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#elif	((TCOMPRESS_CHANNELS(format) + (TCOMPRESS_GREYS(format) ? 2 : 0)) == 3)
	/* -BGR -> RGB- */
	iTex[0][lxy][0] = 0xFF;
	/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#elif	((TCOMPRESS_CHANNELS(format) + (TCOMPRESS_GREYS(format) ? 2 : 0)) == 2)
	/* --YX -> XY-- */
	/* AL-- -> LA-- */
#if	(format == TCOMPRESS_XYz)
	iTex[0][lxy][0] = iTex[0][lxy][2],  // Y
	iTex[1][lxy][0] = iTex[0][lxy][3];  // X
#else
	iTex[0][lxy][0] = iTex[0][lxy][0],  // A
	iTex[1][lxy][0] = iTex[0][lxy][1];  // Z
#endif
	/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#elif	((TCOMPRESS_CHANNELS(format) + (TCOMPRESS_GREYS(format) ? 2 : 0)) == 1)
	  /* -Z-- -> Z--- */
	  /* A--- -> A--- */
	  /* -LLL -> L--- */
#if	(format == TCOMPRESS_a  )
	iTex[0][lxy][0] = iTex[0][lxy][0];  // A
#elif	(format == TCOMPRESS_A  )
	iTex[0][lxy][0] = iTex[0][lxy][0];  // A
#elif	(format == TCOMPRESS_xyZ)
	iTex[0][lxy][0] = iTex[0][lxy][1];  // Z
#else
	iTex[0][lxy][0] = iTex[0][lxy][3];  // X
#endif
#else
#error
#endif
      }

#if	defined(SQUASH_USE_AMP) && !defined(SQUASH_USE_AMP_DEBUG)
      tile_static_memory_fence(elm.barrier);
//    elm.barrier.wait_with_tile_static_memory_fence();

#define local_is(a,b) elm.local == index<2>(a, b)
#else
      }

      for (int tiley = 0; tiley < TY; tiley++)
      for (int tilex = 0; tilex < TX; tilex++)
      {
	const int y = groupsy;
	const int x = groupsx;
	const int ly = tiley;
	const int lx = tilex;
	const int lxy = ly * TX + lx;

#define local_is(a,b) ((ly == a) && (lx == b))
#endif

      /* put this level's 4x4-block into the destination surface */
      {
	/* round down */
	int posx = (x + lx) >> 2;
	int posy = (y + ly) >> 2;

	/* first and second block */
	unsigned int b[2][2];

        /* compress to DXT1/DXT3/DXT5/ATI1/ATI2 */
#define	sflgs	TCOMPRESS_COLOR(format) ? SQUISH_METRIC_PERCEPTUAL : SQUISH_METRIC_UNIFORM,	\
		TCOMPRESS_TRANS(format),							\
		                fiting

	/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#if	((TCOMPRESS_CHANNELS(format) + (TCOMPRESS_GREYS(format) ? 2 : 0)) == 4) ||		\
	((TCOMPRESS_CHANNELS(format) + (TCOMPRESS_GREYS(format) ? 2 : 0)) == 3)
	/* 1x LONG per block for DXT1, 2x for the others */

#if	(coding == 1)
	{ posx <<= 0;
	  squish::CompressColorBtc1(elm.barrier, lxy, iTex[0], 0xFFFF, b[1], sflgs, yArr, lArr);

	  dArr(posy, posx + 0) = b[1][0];
	  dArr(posy, posx + 1) = b[1][1];
	}
#elif	(coding == 2)
	{ posx <<= 1;
	  squish::CompressAlphaBtc2(elm.barrier, lxy, iTex[0], 0xFFFF, b[0]       , yArr      );
	  squish::CompressColorBtc2(elm.barrier, lxy, iTex[0], 0xFFFF, b[1], sflgs, yArr, lArr);

	  dArr(posy, posx + 0) = b[0][0];
	  dArr(posy, posx + 1) = b[0][1];
	  dArr(posy, posx + 2) = b[1][0];
	  dArr(posy, posx + 3) = b[1][1];
	}
#elif	(coding == 3)
	{ posx <<= 1;
	  squish::CompressAlphaBtc3(elm.barrier, lxy, iTex[0], 0xFFFF, b[0]       , yArr      );
	  squish::CompressColorBtc3(elm.barrier, lxy, iTex[0], 0xFFFF, b[1], sflgs, yArr, lArr);

	  dArr(posy, posx + 0) = b[0][0];
	  dArr(posy, posx + 1) = b[0][1];
	  dArr(posy, posx + 2) = b[1][0];
	  dArr(posy, posx + 3) = b[1][1];
	}
#else
#error
#endif
	/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#elif	((TCOMPRESS_CHANNELS(format) + (TCOMPRESS_GREYS(format) ? 2 : 0)) == 2)
	/* 2x LONG for ATI2 */

#if	(coding == 4)
	{ posx <<= 1;
	  squish::CompressAlphaBtc3(elm.barrier, lxy, iTex[0], 0xFFFF, b[0]       , yArr      );
	  squish::CompressAlphaBtc3(elm.barrier, lxy, iTex[1], 0xFFFF, b[1]       , yArr      );

	  dArr(posy, posx + 0) = b[0][0];
	  dArr(posy, posx + 1) = b[0][1];
	  dArr(posy, posx + 2) = b[1][0];
	  dArr(posy, posx + 3) = b[1][1];
	}
#else
#error
#endif
	/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#elif	((TCOMPRESS_CHANNELS(format) + (TCOMPRESS_GREYS(format) ? 2 : 0)) == 1)
	/* 1x LONG for ATI1 */

#if	(coding == 5)
	{ posx <<= 0;
	  squish::CompressAlphaBtc3(elm.barrier, lxy, iTex[0], 0xFFFF, b[0]       , yArr      );

	  dArr(posy, posx + 0) = b[0][0];
	  dArr(posy, posx + 1) = b[0][1];
	}
#else
#error
#endif
#else
#error
#endif

#undef	sflgs

//	elm.barrier.wait();

        /* advance pointer of compressed blocks */
//      wTex += blocksize;
//      dTex += blocksize;
      }

#if	defined(SQUASH_USE_AMP) && !defined(SQUASH_USE_AMP_DEBUG)
//    elm.barrier.wait();

//    dTex += 0;
    });

    dArr.synchronize();
#else
    }}
#endif
  }
