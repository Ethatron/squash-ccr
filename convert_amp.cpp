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
  void ff(TextureConvertRAW,format)(RESOURCEINFO &texo, RESOURCEINFO &texd, ULONG *texs, ULONG *texr, int level, int l) {
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
    /* ABGR -> ARGB */ cwidth = (cwidth * 4 + 3) / 4; /* 1x LONG to 1x LONG */
#elif	(TCOMPRESS_CHANNELS(format) == 3)
    /* -BGR -> -RGB */ cwidth = (cwidth * 3 + 3) / 4; /* 1x LONG to 1x SHORT */
#elif	(TCOMPRESS_CHANNELS(format) == 2)
    /* LA-- -> AL-- */ cwidth = (cwidth * 2 + 3) / 4; /* 1x LONG to 1x CHAR */
#elif	(TCOMPRESS_CHANNELS(format) == 1)
    /* A--- -> A--- */ cwidth = (cwidth * 1 + 3) / 4; /* 8x LONG to 1x CHAR */
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
      tile_static UTYPE bTex[2][TY][TX];
      tile_static type  fTex[2][TY][TX][DIM];

      /* generate this level's 4x4-block from the original surface */
//    const int y = elm.global[0] - ly;
//    const int x = elm.global[1] - lx;
      const int y = elm.tile[0] * TY;
      const int x = elm.tile[1] * TX;
      const int ly = elm.local[0];
      const int lx = elm.local[1];
#else
    Concurrency::array_view<const unsigned int, 2> sArr(iheight, iwidth, (const unsigned int *)texs);
    Concurrency::array_view<      unsigned int, 2> dArr(cheight, cwidth, (      unsigned int *)texr, true);

    for (int groupsy = 0; groupsy < (owidth  / TY); groupsy++)
    for (int groupsx = 0; groupsx < (oheight / TX); groupsx++) {
      typedef type accu[DIM];

      /* tile static memory */
      UTYPE bTex[2][TY][TX];
      type  fTex[2][TY][TX][DIM];

    for (int tiley = 0; tiley < TY; tiley++)
    for (int tilex = 0; tilex < TX; tilex++)
    {
      const int y = groupsy * TY;
      const int x = groupsx * TX;
      const int ly = tiley;
      const int lx = tilex;
#endif

      {
	const int yl = ((y + ly) << l);
	const int xl = ((x + lx) << l);

	accu tt = {0};

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

#define local_mask(a) !(elm.local[1] & a)
#else
      for (int tiley = 0; tiley < TY; tiley++)
      for (int tilex = 0; tilex < TX; tilex++)
      {
	const int y = groupsy;
	const int x = groupsx;
	const int ly = tiley;
	const int lx = tilex;

#define local_mask(a) !(lx & a)
#endif

      /* generate this level's 4x4-block from the original surface */
      {
	/* build average of each channel an join */
	ULONG t;

	Code(fTex[0][ly][lx], tr, 8); t =
	Join(fTex[0][ly][lx], tr   );

	/* write the result */

	/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#if	(TCOMPRESS_CHANNELS(format) == 4)
        /* ABGR -> RGBA */
        bTex[0][ly][lx] = (t <<  0) | 0x00000000;
	/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#elif	(TCOMPRESS_CHANNELS(format) == 3)
        /* -BGR -> RGB- */
	bTex[0][ly][lx] = (t <<  0) | 0xFF000000;
	/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#elif	(TCOMPRESS_CHANNELS(format) == 2)
	/* --YX -> XY-- (signed) */
	/* AL-- -> LA-- */
#if	(format == TCOMPRESS_La )
        bTex[0][ly][lx] = (t <<  8) & 0xFF000000,
	bTex[1][ly][lx] = (t <<  0) & 0xFF000000;
#elif	(format == TCOMPRESS_LA )
	bTex[0][ly][lx] = (t <<  8) & 0xFF000000,
	bTex[1][ly][lx] = (t <<  0) & 0xFF000000;
#elif	(format == TCOMPRESS_LH )
	bTex[0][ly][lx] = (t <<  8) & 0xFF000000,
	bTex[1][ly][lx] = (t <<  0) & 0xFF000000;
#elif	(format == TCOMPRESS_XYz)
        bTex[0][ly][lx] = (t << 16) & 0xFF000000 - 0x80,
	bTex[1][ly][lx] = (t << 24) & 0xFF000000 - 0x80;
#elif	(format == TCOMPRESS_XY )
	bTex[0][ly][lx] = (t << 16) & 0xFF000000 - 0x80,
	bTex[1][ly][lx] = (t << 24) & 0xFF000000 - 0x80;
#elif	(format == TCOMPRESS_xy )
	bTex[0][ly][lx] = (t << 16) & 0xFF000000 - 0x80,
	bTex[1][ly][lx] = (t << 24) & 0xFF000000 - 0x80;
#else
        bTex[0][ly][lx] = (t << 16) & 0xFF000000,
	bTex[1][ly][lx] = (t << 24) & 0xFF000000;
#endif
	/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#elif	(TCOMPRESS_CHANNELS(format) == 1)
        /* -Z-- -> Z--- */
	/* A--- -> A--- */
	/* -LLL -> L--- */
#if	(format == TCOMPRESS_a  )
        bTex[0][ly][lx] = (t <<  0) & 0xFF000000;
#elif	(format == TCOMPRESS_a  )
	bTex[0][ly][lx] = (t <<  0) & 0xFF000000;
#elif	(format == TCOMPRESS_xyZ)
	bTex[0][ly][lx] = (t <<  8) & 0xFF000000;
#else
        bTex[0][ly][lx] = (t << 24) & 0xFF000000;
#endif
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
#if	(TCOMPRESS_CHANNELS(format) == 4)
	  /* ABGR -> ARGB */
	  int   t0 = bTex[0][ly][lx];

	  /* swizzle ARGB -> ARBG */
//	    if (TCOMPRESS_SWIZZL(format))
//	      t0 = //t;
//		  (((t0 >> 24) & 0xFF) << 24 /*h*/)
//	        + (((t0 >> 16) & 0xFF) <<  8 /*r*/)
//	        + (((t0 >>  8) & 0xFF) <<  0 /*g*/)
//	        + (((t0 >>  0) & 0xFF) << 16 /*b*/);
//	    else
	    t0 = //t;
		(((t0 >> 24) & 0xFF) << 24 /*h*/)
	      + (((t0 >> 16) & 0xFF) <<  0 /*r*/)
	      + (((t0 >>  8) & 0xFF) <<  8 /*g*/)
	      + (((t0 >>  0) & 0xFF) << 16 /*b*/);
	  /* bwap+ror */

	  {
	    unsigned int val = (UTYPE)t0;

	    /* write out all of an "int" */
	    dArr(lposy, lposx) = val;
	  }
	/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#elif	(TCOMPRESS_CHANNELS(format) == 3)
	  /* -BGR -> -RGB */
	  int t0 = bTex[0][ly][lx];

	  unsigned int val;

	  /* swizzle RGB -> RBG */
	  if (TCOMPRESS_SWIZZL(format)) {
	    val = (
	      (((t0 >> 16) & 0xFF) <<  0) +
	      (((t0 >>  0) & 0xFF) <<  8) +
	      (((t0 >>  8) & 0xFF) << 16)
	    );
	  }
	  else {
	    val = (
	      (((t0 >> 16) & 0xFF) <<  0) +
	      (((t0 >>  8) & 0xFF) <<  8) +
	      (((t0 >>  0) & 0xFF) << 16)
	    );
	  }

	  {
	    /* convert unaligned output location to "int"-space output location */
	    const int linear = ((posy * owidth) + posx) * 1;
	    const int lposx = (linear * 3) % (cwidth * 4);
	    const int lposy = (linear * 3) / (cwidth * 4);

#if	defined(SQUASH_USE_AMP) && defined(SQUASH_USE_AMP_DEBUG)
#define atomic_fetch_xor(a,b) (*a)^=(b)
#endif

	    switch (lposx & 3) {
	      case 0:
		/* write out three-quarter of an "int" */
		atomic_fetch_xor(&dArr(lposy, lposx >> 2), dArr(lposy, lposx >> 2) & (0xFFFFFF << 0));
		atomic_fetch_xor(&dArr(lposy, lposx >> 2),                    (val & 0xFFFFFF) << 0 );
		break;
	      case 1:
		/* write out three-quarter of an "int" */
		atomic_fetch_xor(&dArr(lposy, lposx >> 2), dArr(lposy, lposx >> 2) & (0xFFFFFF << 8));
		atomic_fetch_xor(&dArr(lposy, lposx >> 2),                    (val & 0xFFFFFF) << 8 );
		break;
	      case 2:
		/* write out three-quarter of an "int" */
		  atomic_fetch_xor(&dArr(lposy, lposx >> 2), dArr(lposy, lposx >> 2) & (0xFFFF << 16));
		  atomic_fetch_xor(&dArr(lposy, lposx >> 2),                    (val & 0xFFFF) << 16 );

		/* maybe striding end of row */
		if (((lposx >> 2) + 1) < owidth) {
		  atomic_fetch_xor(&dArr(lposy, lposx >> 2), dArr(lposy, lposx >> 2) & (0xFF << 0));
		  atomic_fetch_xor(&dArr(lposy, lposx >> 2),                    (val & 0xFF) << 0 );
		}
		else {
		  atomic_fetch_xor(&dArr(lposy + 1, 0), dArr(lposy + 1, 0) & (0xFF << 0));
		  atomic_fetch_xor(&dArr(lposy + 1, 0),               (val & 0xFF) << 0 );
		}
		break;
	      case 3:
		/* write out three-quarter of an "int" */
		  atomic_fetch_xor(&dArr(lposy, lposx >> 2), dArr(lposy, lposx >> 2) & (0xFF << 24));
		  atomic_fetch_xor(&dArr(lposy, lposx >> 2),                    (val & 0xFF) << 24 );

		/* maybe striding end of row */
		if (((lposx >> 2) + 1) < owidth) {
		  atomic_fetch_xor(&dArr(lposy, lposx >> 2), dArr(lposy, lposx >> 2) & (0xFFFF << 0));
		  atomic_fetch_xor(&dArr(lposy, lposx >> 2),                    (val >> 8) & 0xFFFF );
		}
		else {
		  atomic_fetch_xor(&dArr(lposy + 1, 0), dArr(lposy + 1, 0) & (0xFFFF << 0));
		  atomic_fetch_xor(&dArr(lposy + 1, 0),               (val & 0xFFFF) << 0 );
		}
		break;
	    }
	  }
	/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#elif	(TCOMPRESS_CHANNELS(format) == 2)
	  /* XY-- -> YX-- (signed) */
	  /* LA-- -> AL-- */

	  /* every second thread */
	  if (local_mask(1)) {
	    int t0 = bTex[0][ly][lx + 0];
	    int t1 = bTex[1][ly][lx + 0];
	    int t2 = bTex[0][ly][lx + 1];
	    int t3 = bTex[1][ly][lx + 1];

	    /* write combining */
	    unsigned int val = (ULONG)((t3 << 24) + (t2 << 16) + (t1 << 8) + (t0 << 0));

	    /* write out all of an "int" */
	    dArr(lposy, lposx) = val;
	  }
	/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#elif	(TCOMPRESS_CHANNELS(format) == 1)
	  /* Z--- */
	  /* A--- */
	  /* L--- */

	  /* every fourth thread */
	  if (local_mask(3)) {
	    int t0 = bTex[0][ly][lx + 0];
	    int t1 = bTex[0][ly][lx + 1];
	    int t2 = bTex[0][ly][lx + 2];
	    int t3 = bTex[0][ly][lx + 3];

	    /* write combining */
	    unsigned int val = (ULONG)((t3 << 24) + (t2 << 16) + (t1 << 8) + (t0 << 0));

	    /* write out all of an "int" */
	    dArr(lposy, lposx) = val;
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
