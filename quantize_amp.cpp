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
  void ff(TextureQuantizeRAW,format,A)(RESOURCEINFO &texo, RESOURCEINFO &texd, ULONG *texs, ULONG *texr, int level, int l) {
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

#if	(TCOMPRESS_CHANNELS(format) == 4)
    /* ABGR -> ARGB */ cwidth = (cwidth +  0) >> 0; /* 1x LONG to 1x LONG */
#elif	(TCOMPRESS_CHANNELS(format) == 3)
    /* -BGR -> -RGB */ cwidth = (cwidth +  1) >> 1; /* 1x LONG to 1x SHORT */
#elif	(TCOMPRESS_CHANNELS(format) == 2)
    /* LA-- -> AL-- */ cwidth = (cwidth +  3) >> 2; /* 1x LONG to 1x CHAR */
#elif	(TCOMPRESS_CHANNELS(format) == 1)
    /* A--- -> A--- */ cwidth = (cwidth + 31) >> 5; /* 8x LONG to 1x CHAR */
#else
#error
#endif

    /* ensure tile ability (bit on overhead for non-4 resolutions) */
    owidth  = (owidth  + (TX - 1)) & (~(TX - 1));
    oheight = (oheight + (TY - 1)) & (~(TY - 1));

    assert((owidth  & (TX - 1)) == 0);
    assert((oheight & (TY - 1)) == 0);

#if	defined(SQUASH_USE_AMP) && !defined(SQUASH_USE_AMP_DEBUG)
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
//    tile_static UTYPE bTex[2][TY][TX];
      tile_static int   bTex[2][TY][TX];
      tile_static type  fTex[2][TY][TX][DIM];

//    const int y = elm.global[0] - ly;
//    const int x = elm.global[1] - lx;
      const int y = elm.tile[0] * TY;
      const int x = elm.tile[1] * TX;
      const int ly = elm.local[0];
      const int lx = elm.local[1];
#else
    array_view<const unsigned int, 2> sArr(iheight, iwidth, (const unsigned int *)texs);
    array_view<      unsigned int, 2> dArr(cheight, cwidth, (      unsigned int *)texr, true);

    for (int groupsy = 0; groupsy < (owidth  / TY); groupsy++)
    for (int groupsx = 0; groupsx < (oheight / TX); groupsx++) {
      typedef type accu[DIM];

      /* tile static memory */
//    UTYPE bTex[2][TY][TX];
      int   bTex[2][TY][TX];
      type  fTex[2][TY][TX][DIM];

    for (int tiley = 0; tiley < TY; tiley++)
    for (int tilex = 0; tilex < TX; tilex++)
    {
      const int y = groupsy * TY;
      const int x = groupsx * TX;
      const int ly = tiley;
      const int lx = tilex;
#endif
      /* generate this level's 4x4-block from the original surface */
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

	  Accu(tt, t);	// +=
	}

	/* build average of each channel */
	Norm(fTex[0][ly][lx], tt, av, level, l);
      }

#if	defined(SQUASH_USE_AMP) && !defined(SQUASH_USE_AMP_DEBUG)
      tile_static accu tr; tr[(ly * TX + lx) & 7] = 0;

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
	for (int ly = 0; ly < TY; ly += 1)
	for (int lx = 0; lx < TX; lx += 1) {
	  Look(fTex[0][ly][lx], tr);
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
#endif

      /* generate this level's 4x4-block from the original surface */
      {
	/* build average of each channel an join */
	ULONG t;

	Code(fTex[0][ly][lx], tr, (A > 2 ? 4 : (A > 1 ? 10 : (A > 0 ? 5 : 6)))); t =
	Qunt(fTex[0][ly][lx], tr, (A > 2 ? 4 : (A > 1 ? 10 : (A > 0 ? 5 : 6))));

	/* write the result */

	/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#if	(TCOMPRESS_CHANNELS(format) == 4)
	/* ABGR -> RGBA */
	bTex[0][ly][lx] = t;
	/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#elif	(TCOMPRESS_CHANNELS(format) == 3)
	/* -BGR -> RGB- */
	bTex[0][ly][lx] = t;
	/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#elif	(TCOMPRESS_CHANNELS(format) == 2)
	/* AL-- -> LA-- */
	bTex[0][ly][lx] = t;
#else
#error
#endif
      }

      /* put this level's 4x4-block into the destination surface */
      {
	/* assume seamless tiling: wrap pixels around */
	const int posx = (x + lx) % owidth;
	const int posy = (y + ly) % oheight;

	/* convert unaligned output location to "int"-space output location */
	const int linear = ((posy * owidth) + posx) * 1;
	const int lposx = (linear << 0) % (cwidth << 0);
	const int lposy = (linear << 0) / (cwidth << 0);

	/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#if	(TCOMPRESS_CHANNELS(format) <= 4)
	/* ABGR -> ARGB */
	if (sizeof(UTYPE) == 4) {
	  /* every single thread */
	  {
	    int t0 = bTex[0][ly][lx + 0];

	    /* write combining */
	    unsigned int val = (ULONG)t0;

	    /* write out all of an "int" */
	    dArr(lposy, lposx) = val;
	  }
	}
	/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#elif	(TCOMPRESS_CHANNELS(format) <= 3)
	/* -BGR -> -RGB */
	if (sizeof(UTYPE) == 2) {
	  /* every second thread */
	  if (!(elm.local[1] & 1)) {
	    int t0 = bTex[0][ly][lx + 0];
	    int t1 = bTex[0][ly][lx + 1];

	    /* write combining */
	    unsigned int val = (ULONG)((t1 << 16) + (t0 << 0));

	    /* write out all of an "int" */
	    dArr(lposy, lposx) = val;
	  }
	}
	/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#elif	(TCOMPRESS_CHANNELS(format) <= 2)
	/* --YX -> XY-- */
	/* LA-- -> AL-- */
	if (sizeof(UTYPE) == 1) {
	  /* every fourth thread */
	  if (!(elm.local[1] & 3)) {
	    int t0 = bTex[0][ly][lx + 0];
	    int t1 = bTex[0][ly][lx + 1];
	    int t2 = bTex[0][ly][lx + 2];
	    int t3 = bTex[0][ly][lx + 3];

	    /* write combining */
	    unsigned int val = (ULONG)((t3 << 24) + (t2 << 16) + (t1 << 8) + (t0 << 0));

	    /* write out all of an "int" */
	    dArr(lposy, lposx) = val;
	  }
	}
#else
#error
#endif
      }

//    dTex += 0;
#if	defined(SQUASH_USE_AMP) && !defined(SQUASH_USE_AMP_DEBUG)
    });

    dArr.synchronize();
#else
    }}
#endif
  }
