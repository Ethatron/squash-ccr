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

#define	HX	16
#define	HY	16

/* ------------------------------------------------------------------------------------
 */

/* only way to do this, <format> from template argument to preprocessor check */
#if	defined(SQUASH_USE_AMP) && defined(SQUASH_USE_PRE)

namespace squash {

#define	strprete(i)	#i
#define	stringify(i)	strprete(i)
#define	ff(s,f)		ff2(s,f)
#define	ff2(s,f)	s ## _ ## f

#define	format	TCOMPRESS_L
#include "convert_amp.cpp"
#define	format	TCOMPRESS_a
#include "convert_amp.cpp"
#define	format	TCOMPRESS_A
#include "convert_amp.cpp"
#define	format	TCOMPRESS_La
#include "convert_amp.cpp"
#define	format	TCOMPRESS_LA
#include "convert_amp.cpp"
#define	format	TCOMPRESS_LH
#include "convert_amp.cpp"
#define	format	TCOMPRESS_RGB
#include "convert_amp.cpp"
#define	format	TCOMPRESS_RGBa
#include "convert_amp.cpp"
#define	format	TCOMPRESS_RGBV
#include "convert_amp.cpp"
#define	format	TCOMPRESS_RGBA
#include "convert_amp.cpp"
#define	format	TCOMPRESS_RGBH
#include "convert_amp.cpp"

#define	format	TCOMPRESS_xy
#include "convert_amp.cpp"
#define	format	TCOMPRESS_XY
#include "convert_amp.cpp"
#define	format	TCOMPRESS_xyZ
#include "convert_amp.cpp"
#define	format	TCOMPRESS_XYz
#include "convert_amp.cpp"
#define	format	TCOMPRESS_xyz
#include "convert_amp.cpp"
#define	format	TCOMPRESS_xyzV
#include "convert_amp.cpp"
#define	format	TCOMPRESS_xyzD
#include "convert_amp.cpp"
#define	format	TCOMPRESS_XYZ
#include "convert_amp.cpp"
#define	format	TCOMPRESS_XZY
#include "convert_amp.cpp"
#define	format	TCOMPRESS_XYZV
#include "convert_amp.cpp"
#define	format	TCOMPRESS_XZYV
#include "convert_amp.cpp"
#define	format	TCOMPRESS_XYZD
#include "convert_amp.cpp"
#define	format	TCOMPRESS_XZYD
#include "convert_amp.cpp"

#undef	format

} // namespace squash

#endif

/* ------------------------------------------------------------------------------------
 */

namespace squash {

struct histogram {
  unsigned long histo[4][256];
  unsigned long histn[4];
  bool grey, blank;
  unsigned int mask4;
  unsigned int mask5;
  unsigned int mask6;
};

void HistogramConvertRAW(struct histogram *h, RESOURCEINFO &texo, ULONG sPch, ULONG *sTex) {
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

#if	defined(SQUASH_USE_AMP) && !defined(SQUASH_USE_AMP_DEBUG)
    int bools[2] = {0};
    int masks[3] = {0};

    int cwidth  = texo.Width;
    int cheight = texo.Height;

    /* ensure tile ability (bit on overhead for non-4 resolutions) */
    cwidth  = (cwidth  + (HX - 1)) & (~(HX - 1));
    cheight = (cheight + (HY - 1)) & (~(HY - 1));

    assert((cwidth  & (HX - 1)) == 0);
    assert((cheight & (HY - 1)) == 0);

    Concurrency::extent<2> ee(cheight, cwidth);
    Concurrency::tiled_extent<HY, HX> te(ee);

    Concurrency::array_view<const unsigned int, 2> sArr(texo.Height, texo.Width, (const unsigned int *)sTex);
    Concurrency::array_view<  struct histogram, 1> dHst(1                      , (  struct histogram *)h);
    Concurrency::array_view<      unsigned int, 1> dBol(2                      , (      unsigned int *)&bools);
    Concurrency::array_view<      unsigned int, 1> dMsk(3                      , (      unsigned int *)&masks);

    Concurrency::parallel_for_each(te /*dArr.extent.tile<TY, TX>(osize)*/, [=](tiled_index<HY, HX> elm) restrict(amp) {
      tile_static int _bools[2];
      tile_static int _masks[3];
      tile_static int histo[4][256];

      const int y = elm.global[0];
      const int x = elm.global[1];
      const int ly = elm.local[0];
      const int lx = elm.local[1];
      const int ty = elm.tile[0];
      const int tx = elm.tile[1];
      const int t = (ly *          HX) + lx;
      const int o = (ty * cwidth / HX) + tx;

      /* reduce the number of worst-case concurrent write stalls
       * by making each consecusive thread write in a different order-pattern:
       *
       * CLOCK0: thread 0: write a,  thread 1: write r,  thread 2: write g,  thread 3: write b, ...
       * CLOCK1: thread 0: write r,  thread 1: write g,  thread 2: write b,  thread 3: write a, ...
       * CLOCK2: thread 0: write g,  thread 1: write b,  thread 2: write a,  thread 3: write r, ...
       * CLOCK3: thread 0: write b,  thread 1: write a,  thread 2: write r,  thread 3: write g, ...
       * ...
       *
       * may be slower in best-case, because the variables need now be indexable
       */
//#define	ROTATED_ATOMIC

      /* prolog */
      histo[0][t] = 0;
      histo[1][t] = 0;
      histo[2][t] = 0;
      histo[3][t] = 0;
      if (t <= 2)
	_masks[t] = 0;
      if (t <= 1)
	_bools[t] = 1;

      /* main part */
      elm.barrier.wait_with_tile_static_memory_fence();

      if ((y >= top) && (y < bot) &&
      	  (x >= lft) && (x < rgt)) {
#ifdef	ROTATED_ATOMIC
	ULONG t = sArr(y, x);
	ULONG c[4];
	ULONG m[3];

	c[0] = (t >> 24) & 0xFF; /*a*/
	c[1] = (t >> 16) & 0xFF; /*r*/
	c[2] = (t >>  8) & 0xFF; /*g*/
	c[3] = (t >>  0) & 0xFF; /*b*/

	m[0] = ((c[0] ^ (c[0] >> 4)) << 24) |
	       ((c[1] ^ (c[1] >> 4)) << 16) |
	       ((c[2] ^ (c[2] >> 4)) <<  8) |
	       ((c[3] ^ (c[3] >> 4)) <<  0);
	m[1] = ((c[0] ^ (c[0] >> 5)) << 24) |
	       ((c[1] ^ (c[1] >> 5)) << 16) |
	       ((c[2] ^ (c[2] >> 5)) <<  8) |
	       ((c[3] ^ (c[3] >> 5)) <<  0);
	m[2] = ((c[0] ^ (c[0] >> 5)) << 24) |
	       ((c[1] ^ (c[1] >> 5)) << 16) |
	       ((c[2] ^ (c[2] >> 6)) <<  8) |
	       ((c[3] ^ (c[3] >> 5)) <<  0);

	/* read-modify-write is not thread-safe */
	atomic_fetch_inc(&histo[(t + 0) & 3][c[(t + 0) & 3]]);
	atomic_fetch_inc(&histo[(t + 1) & 3][c[(t + 1) & 3]]);
	atomic_fetch_inc(&histo[(t + 2) & 3][c[(t + 2) & 3]]);
	atomic_fetch_inc(&histo[(t + 3) & 3][c[(t + 3) & 3]]);

	/* read-modify-write is not thread-safe */
	atomic_fetch_or(&_masks[(t + 0) % 3], m[(t + 0) % 3]);
	atomic_fetch_or(&_masks[(t + 1) % 3], m[(t + 1) % 3]);
	atomic_fetch_or(&_masks[(t + 2) % 3], m[(t + 2) % 3]);

	/* write-through is thread-safe */
	if ((c[1] != c[2]) || (c[2] != c[3]))
	  _bools[0] = 0;
	if ((c[1] !=   0 ) || (c[2] !=   0 ) || (c[3] != 0))
	  _bools[1] = 0;
#else
	ULONG t = sArr(y, x);
	ULONG a = (t >> 24) & 0xFF; /*a*/
	ULONG r = (t >> 16) & 0xFF; /*r*/
	ULONG g = (t >>  8) & 0xFF; /*g*/
	ULONG b = (t >>  0) & 0xFF; /*b*/
	ULONG q = ((a ^ (a >> 4)) << 24) |
		  ((r ^ (r >> 4)) << 16) |
		  ((g ^ (g >> 4)) <<  8) |
		  ((b ^ (b >> 4)) <<  0);
	ULONG p = ((a ^ (a >> 5)) << 24) |
		  ((r ^ (r >> 5)) << 16) |
		  ((g ^ (g >> 5)) <<  8) |
		  ((b ^ (b >> 5)) <<  0);
	ULONG o = ((a ^ (a >> 5)) << 24) |
		  ((r ^ (r >> 5)) << 16) |
		  ((g ^ (g >> 6)) <<  8) |
		  ((b ^ (b >> 5)) <<  0);

	/* read-modify-write is not thread-safe */
	atomic_fetch_inc(&histo[0][a]);
	atomic_fetch_inc(&histo[1][r]);
	atomic_fetch_inc(&histo[2][g]);
	atomic_fetch_inc(&histo[3][b]);

	/* read-modify-write is not thread-safe */
	atomic_fetch_or(&_masks[0], q);
	atomic_fetch_or(&_masks[1], p);
	atomic_fetch_or(&_masks[2], o);

	/* write-through is thread-safe */
	if ((r != g) || (g != b))
	  atomic_fetch_and(&_bools[0], 0);
	if ((r != 0) || (g != 0) || (b != 0))
	  atomic_fetch_and(&_bools[1], 0);
#endif
      }

      /* epilog */
      elm.barrier.wait_with_tile_static_memory_fence();

      /* read-modify-write is not thread-safe */
#ifdef	ROTATED_ATOMIC
      atomic_fetch_add(h(0).histo[(o + 0) & 3][t], histo[(o + 0) & 3][t]);
      atomic_fetch_add(h(0).histo[(o + 1) & 3][t], histo[(o + 1) & 3][t]);
      atomic_fetch_add(h(0).histo[(o + 2) & 3][t], histo[(o + 2) & 3][t]);
      atomic_fetch_add(h(0).histo[(o + 3) & 3][t], histo[(o + 3) & 3][t]);

      switch (t) {
	case 0:
	case 1:
	case 2: atomic_fetch_or (&dMsk[(o + t) % 3], _masks[(o + t) % 3]); break;
	case 3:
	case 4: atomic_fetch_and(&dBol[(o + t) & 1], _bools[(o + t) & 1]); break;
      }
#else
      atomic_fetch_add(h(0).histo[0][t], histo[0][t]);
      atomic_fetch_add(h(0).histo[1][t], histo[1][t]);
      atomic_fetch_add(h(0).histo[2][t], histo[2][t]);
      atomic_fetch_add(h(0).histo[3][t], histo[3][t]);

      if (t <= 2)
	atomic_fetch_or (&dMsk[t], _masks[lx]);
      if (t <= 1)
	atomic_fetch_and(&dBol[t], _bools[lx]);
#endif

//    elm.barrier.wait_with_global_memory_fence();
    });

    dHst.synchronize();

    h->mask4 = masks[0];
    h->mask5 = masks[1];
    h->mask6 = masks[2];

    h->grey  = !!bools[0];
    h->blank = !!bools[1];
#elif	defined(SQUASH_USE_CCR)
    volatile unsigned __int64 mask4 = 0;
    volatile unsigned __int64 mask5 = 0;
    volatile unsigned __int64 mask6 = 0;
//  combinable<unsigned> _mask4(0);
//  combinable<unsigned> _mask5(0);
//  combinable<unsigned> _mask6(0);

    parallel_for_fixed(top, bot, [=,&mask4,&mask5,&mask6](int y) {
      unsigned long _mask4 = 0;
      unsigned long _mask5 = 0;
      unsigned long _mask6 = 0;

      for (int x = lft; x < rgt; x += 1) {
	ULONG t = sTex[y * sPch + x];
	ULONG a = (t >> 24) & 0xFF; /*a*/
	ULONG r = (t >> 16) & 0xFF; /*a*/
	ULONG g = (t >>  8) & 0xFF; /*a*/
	ULONG b = (t >>  0) & 0xFF; /*a*/
	ULONG q = ((a ^ (a >> 4)) << 24) |
		  ((r ^ (r >> 4)) << 16) |
		  ((g ^ (g >> 4)) <<  8) |
		  ((b ^ (b >> 4)) <<  0);
	ULONG p = ((a ^ (a >> 5)) << 24) |
		  ((r ^ (r >> 5)) << 16) |
		  ((g ^ (g >> 5)) <<  8) |
		  ((b ^ (b >> 5)) <<  0);
	ULONG o = ((a ^ (a >> 5)) << 24) |
		  ((r ^ (r >> 5)) << 16) |
		  ((g ^ (g >> 6)) <<  8) |
		  ((b ^ (b >> 5)) <<  0);

	/* read-modify-write is not thread-safe */
	InterlockedIncrement(&h->histo[0][a]);
	InterlockedIncrement(&h->histo[1][r]);
	InterlockedIncrement(&h->histo[2][g]);
	InterlockedIncrement(&h->histo[3][b]);

	/* read-modify-write is not thread-safe */
	_mask4 |= q;
	_mask5 |= p;
	_mask6 |= o;

//	_mask4.local() |= q;
//	_mask5.local() |= p;
//	_mask6.local() |= o;

	/* write-through is thread-safe */
	if ((r != g) || (g != b))
	  h->grey = false;
	if ((r != 0) || (g != 0) || (b != 0))
	  h->blank = false;
      }

      /* read-modify-write is not thread-safe */
      InterlockedOr(&mask4, _mask4);
      InterlockedOr(&mask5, _mask5);
      InterlockedOr(&mask6, _mask6);
    });

//  unsigned mask4 = _mask4.combine(logical_or<unsigned>());
//  unsigned mask5 = _mask5.combine(logical_or<unsigned>());
//  unsigned mask6 = _mask6.combine(logical_or<unsigned>());

    h->mask4 = (unsigned int)mask4;
    h->mask5 = (unsigned int)mask5;
    h->mask6 = (unsigned int)mask6;
#else
#pragma omp parallel for schedule(dynamic, 4)			\
			 shared(sTex, h)
    for (int y = top; y < bot; y += 1) {
    for (int x = lft; x < rgt; x += 1) {
      ULONG t = sTex[(y * texo.Width) + x];
      ULONG a = (t >> 24) & 0xFF; /*a*/
      ULONG r = (t >> 16) & 0xFF; /*a*/
      ULONG g = (t >>  8) & 0xFF; /*a*/
      ULONG b = (t >>  0) & 0xFF; /*a*/
      ULONG q = ((a ^ (a >> 4)) << 24) |
		((r ^ (r >> 4)) << 16) |
		((g ^ (g >> 4)) <<  8) |
		((b ^ (b >> 4)) <<  0);
      ULONG p = ((a ^ (a >> 5)) << 24) |
		((r ^ (r >> 5)) << 16) |
		((g ^ (g >> 5)) <<  8) |
		((b ^ (b >> 5)) <<  0);
      ULONG o = ((a ^ (a >> 5)) << 24) |
		((r ^ (r >> 5)) << 16) |
		((g ^ (g >> 6)) <<  8) |
		((b ^ (b >> 5)) <<  0);

      /* read-modify-write is not thread-safe */
#pragma omp atomic
      h->histo[0][a]++;
#pragma omp atomic
      h->histo[1][r]++;
#pragma omp atomic
      h->histo[2][g]++;
#pragma omp atomic
      h->histo[3][b]++;

      /* read-modify-write is not thread-safe */
#pragma omp atomic
      h->mask4 |= q;
#pragma omp atomic
      h->mask5 |= p;
#pragma omp atomic
      h->mask6 |= o;

      /* write-through is thread-safe */
      if ((r != g) || (g != b))
	h->grey = false;
      if ((r != 0) || (g != 0) || (b != 0))
	h->blank = false;
    }
    }
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
void TextureConvertRAW(struct spill *smem, RESOURCEINFO &texo, RESOURCEINFO &texd, ULONG sPch, ULONG dPch, ULONG *texs, ULONG *texr, int levels, int l) {
//  logrf("level %2d/%2d\r", l + 1, level);

    /* loop over 4x4-blocks of this level (RAW) */
#if	defined(SQUASH_USE_AMP)
#if	defined(SQUASH_USE_PRE)
    switch (format) {
      case TCOMPRESS_L   : ff(TextureConvertRAW, TCOMPRESS_L   )<UTYPE, type>(texo, texd, texs, texr, levels, l, lv, av); break;
      case TCOMPRESS_a   : ff(TextureConvertRAW, TCOMPRESS_a   )<UTYPE, type>(texo, texd, texs, texr, levels, l, lv, av); break;
      case TCOMPRESS_A   : ff(TextureConvertRAW, TCOMPRESS_A   )<UTYPE, type>(texo, texd, texs, texr, levels, l, lv, av); break;
      case TCOMPRESS_La  : ff(TextureConvertRAW, TCOMPRESS_La  )<UTYPE, type>(texo, texd, texs, texr, levels, l, lv, av); break;
      case TCOMPRESS_LA  : ff(TextureConvertRAW, TCOMPRESS_LA  )<UTYPE, type>(texo, texd, texs, texr, levels, l, lv, av); break;
      case TCOMPRESS_LH  : ff(TextureConvertRAW, TCOMPRESS_LH  )<UTYPE, type>(texo, texd, texs, texr, levels, l, lv, av); break;
      case TCOMPRESS_RGB : ff(TextureConvertRAW, TCOMPRESS_RGB )<UTYPE, type>(texo, texd, texs, texr, levels, l, lv, av); break;
      case TCOMPRESS_RGBa: ff(TextureConvertRAW, TCOMPRESS_RGBa)<UTYPE, type>(texo, texd, texs, texr, levels, l, lv, av); break;
      case TCOMPRESS_RGBV: ff(TextureConvertRAW, TCOMPRESS_RGBV)<UTYPE, type>(texo, texd, texs, texr, levels, l, lv, av); break;
      case TCOMPRESS_RGBA: ff(TextureConvertRAW, TCOMPRESS_RGBA)<UTYPE, type>(texo, texd, texs, texr, levels, l, lv, av); break;
      case TCOMPRESS_RGBH: ff(TextureConvertRAW, TCOMPRESS_RGBH)<UTYPE, type>(texo, texd, texs, texr, levels, l, lv, av); break;

      case TCOMPRESS_xy  : ff(TextureConvertRAW, TCOMPRESS_xy  )<UTYPE, type>(texo, texd, texs, texr, levels, l, lv, av); break;
      case TCOMPRESS_XY  : ff(TextureConvertRAW, TCOMPRESS_XY  )<UTYPE, type>(texo, texd, texs, texr, levels, l, lv, av); break;
      case TCOMPRESS_xyZ : ff(TextureConvertRAW, TCOMPRESS_xyZ )<UTYPE, type>(texo, texd, texs, texr, levels, l, lv, av); break;
      case TCOMPRESS_XYz : ff(TextureConvertRAW, TCOMPRESS_XYz )<UTYPE, type>(texo, texd, texs, texr, levels, l, lv, av); break;
      case TCOMPRESS_xyz : ff(TextureConvertRAW, TCOMPRESS_xyz )<UTYPE, type>(texo, texd, texs, texr, levels, l, lv, av); break;
      case TCOMPRESS_xyzV: ff(TextureConvertRAW, TCOMPRESS_xyzV)<UTYPE, type>(texo, texd, texs, texr, levels, l, lv, av); break;
      case TCOMPRESS_xyzD: ff(TextureConvertRAW, TCOMPRESS_xyzD)<UTYPE, type>(texo, texd, texs, texr, levels, l, lv, av); break;
      case TCOMPRESS_xyCD: ff(TextureConvertRAW, TCOMPRESS_xyCD)<UTYPE, type>(texo, texd, texs, texr, levels, l, lv, av); break;
      case TCOMPRESS_XYZ : ff(TextureConvertRAW, TCOMPRESS_XYZ )<UTYPE, type>(texo, texd, texs, texr, levels, l, lv, av); break;
      case TCOMPRESS_XZY : ff(TextureConvertRAW, TCOMPRESS_XZY )<UTYPE, type>(texo, texd, texs, texr, levels, l, lv, av); break;
      case TCOMPRESS_XYZV: ff(TextureConvertRAW, TCOMPRESS_XYZV)<UTYPE, type>(texo, texd, texs, texr, levels, l, lv, av); break;
      case TCOMPRESS_XZYV: ff(TextureConvertRAW, TCOMPRESS_XZYV)<UTYPE, type>(texo, texd, texs, texr, levels, l, lv, av); break;
      case TCOMPRESS_XYZD: ff(TextureConvertRAW, TCOMPRESS_XYZD)<UTYPE, type>(texo, texd, texs, texr, levels, l, lv, av); break;
      case TCOMPRESS_XZYD: ff(TextureConvertRAW, TCOMPRESS_XZYD)<UTYPE, type>(texo, texd, texs, texr, levels, l, lv, av); break;
   }
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
      /* ABGR -> ARGB */ case 4: cwidth = (cwidth * 4 + 3) / 4; break; /* 1x LONG to 1x LONG */
      /* -BGR -> -RGB */ case 3: cwidth = (cwidth * 3 + 3) / 4; break; /* 1x LONG to 1x SHORT */
      /* LA-- -> AL-- */ case 2: cwidth = (cwidth * 2 + 3) / 4; break; /* 1x LONG to 1x CHAR */
      /* A--- -> A--- */ case 1: cwidth = (cwidth * 1 + 3) / 4; break; /* 8x LONG to 1x CHAR */
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
      tile_static UTYPE bTex[2][TY][TX];
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

	Code<type, format, 8>(fTex[0][ly][lx], tr); t =
	Join<type, format   >(fTex[0][ly][lx], tr);

	/* write the result ABGR, BGR */
        switch (TCOMPRESS_CHANNELS(format)) {
          /* ABGR -> RGBA */
          case 4: bTex[0][ly][lx] = (t) | 0x00000000; break;
          /* -BGR -> RGB- */
	  case 3: bTex[0][ly][lx] = (t) | 0xFF000000; break;
	  /* --YX -> XY-- (signed) */
	  /* AL-- -> LA-- */
          case 2: /**/ if (format == TCOMPRESS_La ) bTex[0][ly][lx] = (t <<  8) & 0xFF000000,
						    bTex[1][ly][lx] = (t <<  0) & 0xFF000000;
		  else if (format == TCOMPRESS_LA ) bTex[0][ly][lx] = (t <<  8) & 0xFF000000,
						    bTex[1][ly][lx] = (t <<  0) & 0xFF000000;
		  else if (format == TCOMPRESS_LH ) bTex[0][ly][lx] = (t <<  8) & 0xFF000000,
						    bTex[1][ly][lx] = (t <<  0) & 0xFF000000;
          	  else if (format == TCOMPRESS_XYz) bTex[0][ly][lx] = (t << 24) & 0xFF000000, bTex[0][ly][lx] -= 0x80000000,
						    bTex[1][ly][lx] = (t << 16) & 0xFF000000, bTex[1][ly][lx] -= 0x80000000;
		  else if (format == TCOMPRESS_XY ) bTex[0][ly][lx] = (t << 24) & 0xFF000000, bTex[0][ly][lx] -= 0x80000000,
						    bTex[1][ly][lx] = (t << 16) & 0xFF000000, bTex[1][ly][lx] -= 0x80000000;
          	  else if (format == TCOMPRESS_xy ) bTex[0][ly][lx] = (t << 24) & 0xFF000000, bTex[0][ly][lx] -= 0x80000000,
						    bTex[1][ly][lx] = (t << 16) & 0xFF000000, bTex[1][ly][lx] -= 0x80000000;
          	  else                              bTex[0][ly][lx] = (t << 16) & 0xFF000000,
						    bTex[1][ly][lx] = (t << 24) & 0xFF000000;
		  break;
          /* -Z-- -> Z--- */
	  /* A--- -> A--- */
	  /* -LLL -> L--- */
          case 1: /**/ if (format == TCOMPRESS_a  ) bTex[0][ly][lx] = (t <<  0) & 0xFF000000;
		  else if (format == TCOMPRESS_A  ) bTex[0][ly][lx] = (t <<  0) & 0xFF000000;
		  else if (format == TCOMPRESS_xyZ) bTex[0][ly][lx] = (t <<  8) & 0xFF000000;
          	  else                              bTex[0][ly][lx] = (t << 24) & 0xFF000000;
          	  break;
        }
      }

      /* put this level's 4x4-block into the destination surface */
      {
	/* assume seamless tiling: wrap pixels around */
	const int posx = (x + lx) % owidth;
	const int posy = (y + ly) % oheight;

	ULONG t0, t1, t2;
	switch (TCOMPRESS_CHANNELS(format)) {
	  /* ABGR -> ARGB */
	  case 4:
	    t0 = bTex[0][ly][lx];

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

	      /* convert unaligned output location to "int"-space output location */
	      const int linear = ((posy * owidth) + posx) * 1;
	      const int lposx = (linear << 0) % (cwidth << 0);
	      const int lposy = (linear << 0) / (cwidth << 0);

	      /* write out all of an "int" */
	      dArr(lposy, lposx) = val;
	    }
	    break;
	  /* -BGR -> -RGB */
	  case 3:
	    t0 = bTex[0][ly][lx];

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
		    atomic_fetch_xor(&dArr(lposy + 1, 0     ), dArr(lposy + 1, 0     ) & (0xFF << 0));
		    atomic_fetch_xor(&dArr(lposy + 1, 0     ),                    (val & 0xFF) << 0 );
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
		    atomic_fetch_xor(&dArr(lposy + 1, 0     ), dArr(lposy + 1, 0     ) & (0xFFFF << 0));
		    atomic_fetch_xor(&dArr(lposy + 1, 0     ),                    (val & 0xFFFF) << 0 );
		  }
		  break;
	      }
	    }
	    break;
	  /* XY-- -> YX-- (signed) */
	  /* LA-- -> AL-- */
	  case 2:
	    t1 = bTex[0][ly][lx];
	    t2 = bTex[1][ly][lx];

	    {
	      unsigned int val = (USHORT)(
	        ((t1 >> 24) & 0x00FF) |
	        ((t2 >> 16) & 0xFF00)
	      );

	      /* convert unaligned output location to "int"-space output location */
	      const int linear = ((posy * owidth) + posx) * 1;
	      const int lposx = (linear << 1) % (cwidth << 1);
	      const int lposy = (linear << 1) / (cwidth << 1);

	      /* write out half of an "int" */
	      atomic_fetch_xor(&dArr(lposy, lposx >> 1), dArr(lposy, lposx >> 1) & (0xFFFF << ((lposx & 0x1) << 4)));
	      atomic_fetch_xor(&dArr(lposy, lposx >> 1),                    (val & 0xFFFF) << ((lposx & 0x1) << 4) );
	    }
	    break;
	  /* Z--- */
	  /* A--- */
	  /* L--- */
	  case 1:
	    t0 = bTex[0][ly][lx];

	    {
	      unsigned int val = (UCHAR)((t0 >> 24) & 0xFF);

	      /* convert unaligned output location to "int"-space output location */
	      const int linear = ((posy * owidth) + posx) * 1;
	      const int lposx = (linear << 2) % (cwidth << 2);
	      const int lposy = (linear << 2) / (cwidth << 2);

	      /* write out quarter of an "int" */
	      atomic_fetch_xor(&dArr(lposy, lposx >> 2), dArr(lposy, lposx >> 2) & (0xFF << ((lposx & 0x3) << 3)));
	      atomic_fetch_xor(&dArr(lposy, lposx >> 2),                    (val & 0xFF) << ((lposx & 0x3) << 3) );
	    }
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
        smem[l].splh =		           texd.Height                                       ;
        smem[l].splw =			                 texd.Width                          ;
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

	Code(fTex[0][ly][lx], tr, 8); t =
	Join(fTex[0][ly][lx], tr);

	/* write the result ABGR, BGR */
        switch (TCOMPRESS_CHANNELS(format)) {
          /* ABGR -> RGBA */
          case 4: bTex[0][ly][lx] = (t) | 0x00000000; break;
          /* -BGR -> RGB- */
	  case 3: bTex[0][ly][lx] = (t) | 0xFF000000; break;
	  /* --YX -> XY-- (signed) */
	  /* AL-- -> LA-- */
          case 2: /**/ if (format == TCOMPRESS_La ) bTex[0][ly][lx] = (t <<  8) & 0xFF000000,
						    bTex[1][ly][lx] = (t <<  0) & 0xFF000000;
		  else if (format == TCOMPRESS_LA ) bTex[0][ly][lx] = (t <<  8) & 0xFF000000,
						    bTex[1][ly][lx] = (t <<  0) & 0xFF000000;
		  else if (format == TCOMPRESS_LH ) bTex[0][ly][lx] = (t <<  8) & 0xFF000000,
						    bTex[1][ly][lx] = (t <<  0) & 0xFF000000;
          	  else if (format == TCOMPRESS_XYz) bTex[0][ly][lx] = (t << 24) & 0xFF000000, bTex[0][ly][lx] -= 0x80000000,
						    bTex[1][ly][lx] = (t << 16) & 0xFF000000, bTex[1][ly][lx] -= 0x80000000;
		  else if (format == TCOMPRESS_XY ) bTex[0][ly][lx] = (t << 24) & 0xFF000000, bTex[0][ly][lx] -= 0x80000000,
						    bTex[1][ly][lx] = (t << 16) & 0xFF000000, bTex[1][ly][lx] -= 0x80000000;
          	  else if (format == TCOMPRESS_xy ) bTex[0][ly][lx] = (t << 24) & 0xFF000000, bTex[0][ly][lx] -= 0x80000000,
						    bTex[1][ly][lx] = (t << 16) & 0xFF000000, bTex[1][ly][lx] -= 0x80000000;
          	  else                              bTex[0][ly][lx] = (t <<  8) & 0xFF000000,
						    bTex[1][ly][lx] = (t <<  0) & 0xFF000000;
		  break;
          /* -Z-- -> Z--- */
	  /* A--- -> A--- */
	  /* -LLL -> L--- */
          case 1: /**/ if (format == TCOMPRESS_a  ) bTex[0][ly][lx] = (t <<  0) & 0xFF000000;
		  else if (format == TCOMPRESS_A  ) bTex[0][ly][lx] = (t <<  0) & 0xFF000000;
		  else if (format == TCOMPRESS_xyZ) bTex[0][ly][lx] = (t <<  8) & 0xFF000000;
          	  else                              bTex[0][ly][lx] = (t << 24) & 0xFF000000;
          	  break;
        }
      }

      /* put this level's 4x4-block into the destination surface */
      for (int ly = 0; ly < TY; ly += 1)
      for (int lx = 0; lx < TX; lx += 1) {
	/* assume seamless tiling: wrap pixels around */
	const int posx = (x + lx) % texd.Width;
	const int posy = (y + ly) % texd.Height;

	ULONG t0, t1, t2;
	switch (TCOMPRESS_CHANNELS(format)) {
	  /* ABGR -> ARGB */
	  case 4:
	    t0 = bTex[0][ly][lx];

	    /* swizzle ARGB -> ARBG */
//	    if (TCOMPRESS_SWIZZL(format))
//	      t0 = //t;
//		  (((t0 >> 24) & 0xFF) << 24 /*a*/)
//	        + (((t0 >> 16) & 0xFF) <<  8 /*r*/)
//	        + (((t0 >>  8) & 0xFF) <<  0 /*g*/)
//	        + (((t0 >>  0) & 0xFF) << 16 /*b*/);
//	    else
	      t0 = //t;
		  (((t0 >> 24) & 0xFF) << 24 /*a*/)
	        + (((t0 >> 16) & 0xFF) <<  0 /*r*/)
	        + (((t0 >>  8) & 0xFF) <<  8 /*g*/)
	        + (((t0 >>  0) & 0xFF) << 16 /*b*/);
	    /* bwap+ror */

	    (ULONG &)(wTex[(posy * dPch) + (posx * 4) + 0]) = t0;
	    break;
	  /* -BGR -> -RGB */
	  case 3:
	    t0 = bTex[0][ly][lx];

	    /* swizzle RGB -> RBG */
	    if (TCOMPRESS_SWIZZL(format)) {
	      wTex[(posy * dPch) + (posx * 3) + 0] = (t0 >> 16) & 0xFF;
	      wTex[(posy * dPch) + (posx * 3) + 2] = (t0 >>  8) & 0xFF;
	      wTex[(posy * dPch) + (posx * 3) + 1] = (t0 >>  0) & 0xFF;
	    }
	    else {
	      wTex[(posy * dPch) + (posx * 3) + 0] = (t0 >> 16) & 0xFF;
	      wTex[(posy * dPch) + (posx * 3) + 1] = (t0 >>  8) & 0xFF;
	      wTex[(posy * dPch) + (posx * 3) + 2] = (t0 >>  0) & 0xFF;
	    }
	    break;
	  /* XY-- -> YX-- (signed) */
	  /* LA-- -> AL-- */
	  case 2:
	    t1 = bTex[0][ly][lx];
	    t2 = bTex[1][ly][lx];

	    {
	      wTex[(posy * dPch) + (posx * 2) + 0] = (t1 >> 24) & 0xFF;
	      wTex[(posy * dPch) + (posx * 2) + 1] = (t2 >> 24) & 0xFF;
	    }
	    break;
	  /* Z--- */
	  /* A--- */
	  /* L--- */
	  case 1:
	    t0 = bTex[0][ly][lx];

	    {
	      wTex[(posy * dPch) + (posx * 1) + 0] = (t0 >> 24) & 0xFF;
	    }
	    break;
	}
      }

      dTex += 0;
    }
#if	defined(SQUASH_USE_CCR)
    });
#else
    }
#endif
#endif

//  logrf("level %2d/%2d\r", l + 1, level);
}

template<typename UTYPE, typename type, const int format>
bool TextureConvertRAW(int minlevel, LPDIRECT3DBASETEXTURE *tex, bool optimize) {
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
    h.grey = h.blank = true;

    ULONG sPch, *sTex =
    TextureLock((*tex), 0, -1, &sPch); sPch >>= 2;
    TextureMatte<format>(texo, sPch, sTex);
    HistogramConvertRAW(&h, texo, sPch, sTex);
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

    if (h.blank) {
      /* check if Alpha is 1bit */
      if (TEXFORMAT::Available(TEXFMT_A1) && MinimalAlpha(format, black, white, blawh))
	return TextureConvert<type>(minlevel, tex, !TCOMPRESS_NORMAL(format), TEXFMT_A1);

      if (TEXFORMAT::Available(TEXFMT_A8) && TCOMPRESS_TRESH(format) && (format != TCOMPRESS_a))
	return TextureConvertRAW<UTYPE, type, TCOMPRESS_a >(minlevel, tex, false);
      if (TEXFORMAT::Available(TEXFMT_A8) && TCOMPRESS_TRANS(format) && (format != TCOMPRESS_A))
	return TextureConvertRAW<UTYPE, type, TCOMPRESS_A >(minlevel, tex, false);
    }
    else {
      bool fallthrough = false;

      /* alpha will be stripped */
      if (CollapseAlpha(format, black, white))
	if (ExistAlpha(format, origFormat))
	  addnote(" Automatic dropped alpha-channel.\n");

#ifndef DX11
      /* it will become grey */
      if (h.grey)
	if (!TEXFMT_GREY(origFormat))
	  addnote(" Automatic greyscale conversion.\n");
#endif

      /* minimal grey case (1:3 or 1:4) */
      if (h.grey) {
	if (CollapseAlpha(format, black, white)) {
	  if (TEXFORMAT::Available(TEXFMT_L8) && (format != TCOMPRESS_L))
	    return TextureConvertRAW<UTYPE, type, TCOMPRESS_L>(minlevel, tex, false);
	}

	if (!(h.mask4 & 0x0F0F0F0F)) {
	  /* check if Alpha is killable */
	    if (TEXFORMAT::Available(TEXFMT_A4L4) && TCOMPRESS_TRESH(format))
	      return TextureQuantizeRAW<USHORT, type, TCOMPRESS_La, 4>(minlevel, tex, false);
	  else
	    if (TEXFORMAT::Available(TEXFMT_A4L4) && TCOMPRESS_TRANS(format))
	      return TextureQuantizeRAW<USHORT, type, TCOMPRESS_LA, 4>(minlevel, tex, false);
	  else
	    if (TEXFORMAT::Available(TEXFMT_A4L4)                           )
	      return TextureQuantizeRAW<USHORT, type, TCOMPRESS_LH, 4>(minlevel, tex, false);
	}

	{
	  /* check if Alpha is killable */
	    if (TEXFORMAT::Available(TEXFMT_A8L8) && TCOMPRESS_TRESH(format) && (format != TCOMPRESS_La))
	      return TextureConvertRAW<UTYPE, type, TCOMPRESS_La>(minlevel, tex, false);
	  else
	    if (TEXFORMAT::Available(TEXFMT_A8L8) && TCOMPRESS_TRANS(format) && (format != TCOMPRESS_LA))
	      return TextureConvertRAW<UTYPE, type, TCOMPRESS_LA>(minlevel, tex, false);
	  else
	    if (TEXFORMAT::Available(TEXFMT_A8L8) &&                            (format != TCOMPRESS_LH))
	      return TextureConvertRAW<UTYPE, type, TCOMPRESS_LH>(minlevel, tex, false);
	}

	fallthrough = true;
      }

      /* 4bit candidate (captures grey as well for 2:3 or 1:2) */
      if (!(h.mask4 & 0x0F0F0F0F)) {
	if (!fallthrough)
	  if (!h.grey) addnote(" Quantized 444 color image detected.\n");
	  else         addnote(" Quantized 4 greyscale image detected.\n");

	/* check if Alpha is killable */
	if (CollapseAlpha(format, black, white)) {
	  if (TEXFORMAT::Available(TEXFMT_X4R4G4B4) && TCOMPRESS_COLOR(format) /*&& TCOMPRESS_SIDES(format)*/)
	    return TextureQuantizeRAW<UTYPE, type, TCOMPRESS_RGBV, 4>(minlevel, tex, false);
	  if (TEXFORMAT::Available(TEXFMT_X4R4G4B4) && ((format == TCOMPRESS_xyzD)))
	    return TextureQuantizeRAW<UTYPE, type, TCOMPRESS_xyzV, 4>(minlevel, tex, false);
	  if (TEXFORMAT::Available(TEXFMT_X4R4G4B4) && ((format == TCOMPRESS_XYZD)))
	    return TextureQuantizeRAW<UTYPE, type, TCOMPRESS_XYZV, 4>(minlevel, tex, false);
	  if (TEXFORMAT::Available(TEXFMT_X4R4G4B4) && ((format == TCOMPRESS_XZYD)))
	    return TextureQuantizeRAW<UTYPE, type, TCOMPRESS_XZYV, 4>(minlevel, tex, false);

	  /* falls through if format isn't supported, but fe. TEXFMT_A4R4G4B4 is */
	}

	/* check if Alpha is 4bit */
	{
	  if (TEXFORMAT::Available(TEXFMT_A4R4G4B4) && ((format == TCOMPRESS_RGBa) || (format == TCOMPRESS_La)))
	    return TextureQuantizeRAW<UTYPE, type, TCOMPRESS_RGBa, 4>(minlevel, tex, false);
	  if (TEXFORMAT::Available(TEXFMT_A4R4G4B4) && ((format == TCOMPRESS_RGBA) || (format == TCOMPRESS_LA)))
	    return TextureQuantizeRAW<UTYPE, type, TCOMPRESS_RGBA, 4>(minlevel, tex, false);
	  if (TEXFORMAT::Available(TEXFMT_A4R4G4B4) && ((format == TCOMPRESS_RGBH) || (format == TCOMPRESS_LH)))
	    return TextureQuantizeRAW<UTYPE, type, TCOMPRESS_RGBH, 4>(minlevel, tex, false);
	  if (TEXFORMAT::Available(TEXFMT_A4R4G4B4) && ((format == TCOMPRESS_xyzD)))
	    return TextureQuantizeRAW<UTYPE, type, TCOMPRESS_xyzD, 4>(minlevel, tex, false);
	  if (TEXFORMAT::Available(TEXFMT_A4R4G4B4) && ((format == TCOMPRESS_XYZD)))
	    return TextureQuantizeRAW<UTYPE, type, TCOMPRESS_XYZD, 4>(minlevel, tex, false);
	  if (TEXFORMAT::Available(TEXFMT_A4R4G4B4) && ((format == TCOMPRESS_XZYD)))
	    return TextureQuantizeRAW<UTYPE, type, TCOMPRESS_XZYD, 4>(minlevel, tex, false);
	}

	fallthrough = true;
      }

      /* 5bit candidate (captures grey as well for 2:3 or 1:2) */
      if (!(h.mask5 & 0x00070707)) {
	if (!fallthrough)
	  if (!h.grey) addnote(" Quantized 555 color image detected.\n");
	  else         addnote(" Quantized 5 greyscale image detected.\n");

	/* check if Alpha is killable */
	if (CollapseAlpha(format, black, white)) {
	  if (TEXFORMAT::Available(TEXFMT_X1R5G5B5) && TCOMPRESS_COLOR(format) /*&& TCOMPRESS_SIDES(format)*/)
	    return TextureQuantizeRAW<UTYPE, type, TCOMPRESS_RGBV, 1>(minlevel, tex, false);
	  if (TEXFORMAT::Available(TEXFMT_X1R5G5B5) && ((format == TCOMPRESS_xyzD)))
	    return TextureQuantizeRAW<UTYPE, type, TCOMPRESS_xyzV, 1>(minlevel, tex, false);
	  if (TEXFORMAT::Available(TEXFMT_X1R5G5B5) && ((format == TCOMPRESS_XYZD)))
	    return TextureQuantizeRAW<UTYPE, type, TCOMPRESS_XYZV, 1>(minlevel, tex, false);
	  if (TEXFORMAT::Available(TEXFMT_X1R5G5B5) && ((format == TCOMPRESS_XZYD)))
	    return TextureQuantizeRAW<UTYPE, type, TCOMPRESS_XZYV, 1>(minlevel, tex, false);

	  /* falls through if format isn't supported, but fe. TEXFMT_A1R5G5B5 is */
	}

	/* check if Alpha is 1bit */
	if (MinimalAlpha(format, black, white, blawh)) {
	  if (TEXFORMAT::Available(TEXFMT_A1R5G5B5) && ((format == TCOMPRESS_RGBa) || (format == TCOMPRESS_La)))
	    return TextureQuantizeRAW<UTYPE, type, TCOMPRESS_RGBa, 1>(minlevel, tex, false);
	  if (TEXFORMAT::Available(TEXFMT_A1R5G5B5) && ((format == TCOMPRESS_RGBA) || (format == TCOMPRESS_LA)))
	    return TextureQuantizeRAW<UTYPE, type, TCOMPRESS_RGBA, 1>(minlevel, tex, false);
	  if (TEXFORMAT::Available(TEXFMT_A1R5G5B5) && ((format == TCOMPRESS_RGBH) || (format == TCOMPRESS_LH)))
	    return TextureQuantizeRAW<UTYPE, type, TCOMPRESS_RGBH, 1>(minlevel, tex, false);
	  if (TEXFORMAT::Available(TEXFMT_A1R5G5B5) && ((format == TCOMPRESS_xyzD)))
	    return TextureQuantizeRAW<UTYPE, type, TCOMPRESS_xyzD, 1>(minlevel, tex, false);
	  if (TEXFORMAT::Available(TEXFMT_A1R5G5B5) && ((format == TCOMPRESS_XYZD)))
	    return TextureQuantizeRAW<UTYPE, type, TCOMPRESS_XYZD, 1>(minlevel, tex, false);
	  if (TEXFORMAT::Available(TEXFMT_A1R5G5B5) && ((format == TCOMPRESS_XZYD)))
	    return TextureQuantizeRAW<UTYPE, type, TCOMPRESS_XZYD, 1>(minlevel, tex, false);
	}

	fallthrough = true;
      }

      /* 6bit candidate (captures grey as well for 2:3 or 1:2) */
      if (!(h.mask5 & 0x00070307)) {
	if (!fallthrough)
	  if (!h.grey) addnote(" Quantized 565 color image detected.\n");
	  else         addnote(" Quantized 6 greyscale image detected.\n");

	/* check if Alpha is killable */
	if (CollapseAlpha(format, black, white)) {
	  if (TEXFORMAT::Available(TEXFMT_R5G6B5) && TCOMPRESS_COLOR(format) && TCOMPRESS_SIDES(format))
	    return TextureQuantizeRAW<UTYPE, type, TCOMPRESS_RGB, 0>(minlevel, tex, false);
	  if (TEXFORMAT::Available(TEXFMT_R5G6B5) && ((format == TCOMPRESS_xyzD)))
	    return TextureQuantizeRAW<UTYPE, type, TCOMPRESS_xyz, 0>(minlevel, tex, false);
	  if (TEXFORMAT::Available(TEXFMT_R5G6B5) && ((format == TCOMPRESS_XYZD)))
	    return TextureQuantizeRAW<UTYPE, type, TCOMPRESS_XYZ, 0>(minlevel, tex, false);
	  if (TEXFORMAT::Available(TEXFMT_R5G6B5) && ((format == TCOMPRESS_XZYD)))
	    return TextureQuantizeRAW<UTYPE, type, TCOMPRESS_XZY, 0>(minlevel, tex, false);
	}

	fallthrough = true;
      }

      /* two colors in alpha */
      if (h.histn[0] <= 2) {
	/* check if Alpha is killable */
	if (CollapseAlpha(format, black, white)) {
	  /* if it wasn't transparent it must be uncorrelated! */
	  if (TEXFORMAT::Available(TEXFMT_A8L8  ) && TCOMPRESS_GREYS(format) && TCOMPRESS_SIDES(format))
	    return TextureConvertRAW<UTYPE, type, TCOMPRESS_L  >(minlevel, tex, false);
	  if (TEXFORMAT::Available(TEXFMT_R8G8B8) && TCOMPRESS_COLOR(format) && TCOMPRESS_SIDES(format))
	    return TextureConvertRAW<UTYPE, type, TCOMPRESS_RGB>(minlevel, tex, false);

	  if (TEXFORMAT::Available(TEXFMT_R8G8B8) && ((format == TCOMPRESS_xyzD)))
	    return TextureConvertRAW<UTYPE, type, TCOMPRESS_xyz>(minlevel, tex, false);
	  if (TEXFORMAT::Available(TEXFMT_R8G8B8) && ((format == TCOMPRESS_XYZD)))
	    return TextureConvertRAW<UTYPE, type, TCOMPRESS_XYZ>(minlevel, tex, false);
	  if (TEXFORMAT::Available(TEXFMT_R8G8B8) && ((format == TCOMPRESS_XZYD)))
	    return TextureConvertRAW<UTYPE, type, TCOMPRESS_XZY>(minlevel, tex, false);

	  fallthrough = true;
	}

	/* TODO: if all black and trans convert to 4x4 black L8 */
      }
    }

    /* TODO: compare RMS of DXT3 vs. DXT5 and choose */
    if (verbose && ExistAlpha(format, origFormat)) {
      /**/ if (blawh)
	addnote(" Alpha-channel is black'n'white.\n");
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

#ifdef DX11
  ID3D11Texture2D *rtex; D3D11_TEXTURE2D_DESC cr;
  memset(&cr, 0, sizeof(cr));

  cr.Width  = texo.Width;
  cr.Height = texo.Height;
  cr.MipLevels = levels;
  cr.ArraySize = texo.Slices;

  TEXFORMAT tf = TEXFMT_UNKNOWN;
  switch (TCOMPRESS_CHANNELS(format)) {
    case 4: if (format == TCOMPRESS_RGBa) tf = TEXFMT_A8B8G8R8;
	    if (format == TCOMPRESS_RGBA) tf = TEXFMT_A8B8G8R8;
	    if (format == TCOMPRESS_RGBH) tf = TEXFMT_A8B8G8R8;
	    if (format == TCOMPRESS_RGBV) tf = TEXFMT_X8B8G8R8;
	    if (format == TCOMPRESS_xyzD) tf = TEXFMT_A8B8G8R8;
	    if (format == TCOMPRESS_XYZD) tf = TEXFMT_A8B8G8R8;
	    if (format == TCOMPRESS_xyzV) tf = TEXFMT_X8B8G8R8;
	    if (format == TCOMPRESS_XYZV) tf = TEXFMT_X8B8G8R8; break;
    case 3: if (format == TCOMPRESS_RGB ) tf = TEXFMT_R8G8B8  ;
	    if (format == TCOMPRESS_xyz ) tf = TEXFMT_R8G8B8  ;
	    if (format == TCOMPRESS_XYZ ) tf = TEXFMT_R8G8B8  ;
	    if (format == TCOMPRESS_XZY ) tf = TEXFMT_R8G8B8  ; break;
    case 2: if (format == TCOMPRESS_La  ) tf = TEXFMT_A8L8    ;
	    if (format == TCOMPRESS_LA  ) tf = TEXFMT_A8L8    ;
	    if (format == TCOMPRESS_LH  ) tf = TEXFMT_A8L8    ;
	    if (format == TCOMPRESS_XYz ) tf = TEXFMT_V8U8    ; /* TEXFMT_CxV8U8 */
	    if (format == TCOMPRESS_xy  ) tf = TEXFMT_V8U8    ;
	    if (format == TCOMPRESS_XY  ) tf = TEXFMT_V8U8    ; break;
    case 1: if (format == TCOMPRESS_a   ) tf = TEXFMT_A8      ;
	    if (format == TCOMPRESS_A   ) tf = TEXFMT_A8      ;
	    if (format == TCOMPRESS_L   ) tf = TEXFMT_L8      ;
	    if (format == TCOMPRESS_xyZ ) tf = TEXFMT_L8      ; break;
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
  D3DFORMAT cnvFormat = D3DFMT_UNKNOWN;
  int cnvWidth  = texo.Width;
  int cnvHeight = texo.Height;

  switch (TCOMPRESS_CHANNELS(format)) {
    case 4: if (format == TCOMPRESS_RGBa) cnvFormat = D3DFMT_A8R8G8B8;
	    if (format == TCOMPRESS_RGBA) cnvFormat = D3DFMT_A8R8G8B8;
	    if (format == TCOMPRESS_RGBH) cnvFormat = D3DFMT_A8R8G8B8;
	    if (format == TCOMPRESS_RGBV) cnvFormat = D3DFMT_X8R8G8B8;
	    if (format == TCOMPRESS_xyzD) cnvFormat = D3DFMT_A8R8G8B8;
	    if (format == TCOMPRESS_XYZD) cnvFormat = D3DFMT_A8R8G8B8;
	    if (format == TCOMPRESS_xyzV) cnvFormat = D3DFMT_X8R8G8B8;
	    if (format == TCOMPRESS_XYZV) cnvFormat = D3DFMT_X8R8G8B8; break;
    case 3: if (format == TCOMPRESS_RGB ) cnvFormat = D3DFMT_R8G8B8  ;
	    if (format == TCOMPRESS_xyz ) cnvFormat = D3DFMT_R8G8B8  ;
	    if (format == TCOMPRESS_XYZ ) cnvFormat = D3DFMT_R8G8B8  ;
	    if (format == TCOMPRESS_XZY ) cnvFormat = D3DFMT_R8G8B8  ; break;
    case 2: if (format == TCOMPRESS_La  ) cnvFormat = D3DFMT_A8L8    ;
	    if (format == TCOMPRESS_LA  ) cnvFormat = D3DFMT_A8L8    ;
	    if (format == TCOMPRESS_LH  ) cnvFormat = D3DFMT_A8L8    ;
	    if (format == TCOMPRESS_XYz ) cnvFormat = D3DFMT_V8U8    ; // TEXFMT_CxV8U8
	    if (format == TCOMPRESS_xy  ) cnvFormat = D3DFMT_V8U8    ;
	    if (format == TCOMPRESS_XY  ) cnvFormat = D3DFMT_V8U8    ; break;
    case 1: if (format == TCOMPRESS_a   ) cnvFormat = D3DFMT_A8      ;
	    if (format == TCOMPRESS_A   ) cnvFormat = D3DFMT_A8      ;
	    if (format == TCOMPRESS_L   ) cnvFormat = D3DFMT_L8      ;
	    if (format == TCOMPRESS_xyZ ) cnvFormat = D3DFMT_L8      ; break;
  }

  pD3DDevice->CreateTexture(cnvWidth, cnvHeight, levels, 0, cnvFormat, D3DPOOL_SYSTEMMEM, (IDirect3DTexture9 **)&text, NULL);
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

    TextureConvertRAW<UTYPE, type, format>(smem, texo, texd, sPch, dPch, texs, texr, levels, l);

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

#undef	HX
#undef	HY

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */

namespace squash {

#pragma	warning (disable : 4100)

/* we don't process normal-maps as integers */
template<> bool TextureConvertRAW<ULONG, long, TCOMPRESS_xyz >(int minlevel, LPDIRECT3DBASETEXTURE *tex, bool optimize) { return false; }
template<> bool TextureConvertRAW<ULONG, long, TCOMPRESS_XYZ >(int minlevel, LPDIRECT3DBASETEXTURE *tex, bool optimize) { return false; }
template<> bool TextureConvertRAW<ULONG, long, TCOMPRESS_XZY >(int minlevel, LPDIRECT3DBASETEXTURE *tex, bool optimize) { return false; }

template<> bool TextureConvertRAW<ULONG, long, TCOMPRESS_xyzD>(int minlevel, LPDIRECT3DBASETEXTURE *tex, bool optimize) { return false; }
template<> bool TextureConvertRAW<ULONG, long, TCOMPRESS_XYZD>(int minlevel, LPDIRECT3DBASETEXTURE *tex, bool optimize) { return false; }
template<> bool TextureConvertRAW<ULONG, long, TCOMPRESS_XZYD>(int minlevel, LPDIRECT3DBASETEXTURE *tex, bool optimize) { return false; }

template<> bool TextureConvertRAW<ULONG, long, TCOMPRESS_xyzV>(int minlevel, LPDIRECT3DBASETEXTURE *tex, bool optimize) { return false; }
template<> bool TextureConvertRAW<ULONG, long, TCOMPRESS_XYZV>(int minlevel, LPDIRECT3DBASETEXTURE *tex, bool optimize) { return false; }
template<> bool TextureConvertRAW<ULONG, long, TCOMPRESS_XZYV>(int minlevel, LPDIRECT3DBASETEXTURE *tex, bool optimize) { return false; }

#pragma	warning (default : 4100)

} // namespace squash

/* ####################################################################################
 */

namespace squash {

template<typename type>
static bool TextureConvert(int minlevel, LPDIRECT3DBASETEXTURE *tex, bool dither, TEXFORMAT target);

template<>
static bool TextureConvert<long>(int minlevel, LPDIRECT3DBASETEXTURE *tex, bool dither, TEXFORMAT target) {
  return TextureConvert(minlevel, tex, dither, false, target);
};

template<>
static bool TextureConvert<float>(int minlevel, LPDIRECT3DBASETEXTURE *tex, bool dither, TEXFORMAT target) {
  return TextureConvert(minlevel, tex, dither, true, target);
};

} // namespace squash

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */

namespace squash {

bool TextureConvertRGBV(LPDIRECT3DBASETEXTURE *base, int minlevel, bool gamma) {
  bool res = true;

  if (gamma) res = res && TextureConvertRAW<ULONG, float, TCOMPRESS_RGBV>(minlevel, base);
  else       res = res && TextureConvertRAW<ULONG, long , TCOMPRESS_RGBV>(minlevel, base);

  return res;
}

bool TextureConvertRGBA(LPDIRECT3DBASETEXTURE *base, int minlevel, bool gamma, bool contrast) {
  bool res = true;

  if   (contrast) res = res && TextureConvertRAW<ULONG, float, TCOMPRESS_RGBA>(minlevel, base);
  else if (gamma) res = res && TextureConvertRAW<ULONG, float, TCOMPRESS_RGBA>(minlevel, base);
  else            res = res && TextureConvertRAW<ULONG, long , TCOMPRESS_RGBA>(minlevel, base);

  return res;
}

bool TextureConvertRGB_A(LPDIRECT3DBASETEXTURE *base, int minlevel, bool gamma, bool contrast) {
  bool res = true;

  if   (contrast) res = res && TextureConvertRAW<ULONG, float, TCOMPRESS_RGBa>(minlevel, base);
  else if (gamma) res = res && TextureConvertRAW<ULONG, float, TCOMPRESS_RGBa>(minlevel, base);
  else            res = res && TextureConvertRAW<ULONG, long , TCOMPRESS_RGBa>(minlevel, base);

  return res;
}

bool TextureConvertRGBH(LPDIRECT3DBASETEXTURE *base, int minlevel, bool gamma) {
  bool res = true;

  if (gamma) res = res && TextureConvertRAW<ULONG, float, TCOMPRESS_RGBH>(minlevel, base);
  else       res = res && TextureConvertRAW<ULONG, long , TCOMPRESS_RGBH>(minlevel, base);

  return res;
}

bool TextureConvertRGB(LPDIRECT3DBASETEXTURE *base, int minlevel, bool gamma) {
  bool res = true;

  if (gamma) res = res && TextureConvertRAW<ULONG, float, TCOMPRESS_RGB>(minlevel, base);
  else       res = res && TextureConvertRAW<ULONG, long , TCOMPRESS_RGB>(minlevel, base);

  return res;
}

bool TextureConvertLA(LPDIRECT3DBASETEXTURE *base, int minlevel, bool gamma, bool contrast) {
  bool res = true;

  if   (contrast) res = res && TextureConvertRAW<ULONG, float, TCOMPRESS_LA>(minlevel, base);
  else if (gamma) res = res && TextureConvertRAW<ULONG, float, TCOMPRESS_LA>(minlevel, base);
  else            res = res && TextureConvertRAW<ULONG, long , TCOMPRESS_LA>(minlevel, base);

  return res;
}

bool TextureConvertL_A(LPDIRECT3DBASETEXTURE *base, int minlevel, bool gamma, bool contrast) {
  bool res = true;

  if   (contrast) res = res && TextureConvertRAW<ULONG, float, TCOMPRESS_La>(minlevel, base);
  else if (gamma) res = res && TextureConvertRAW<ULONG, float, TCOMPRESS_La>(minlevel, base);
  else            res = res && TextureConvertRAW<ULONG, long , TCOMPRESS_La>(minlevel, base);

  return res;
}

bool TextureConvertLH(LPDIRECT3DBASETEXTURE *base, int minlevel, bool gamma) {
  bool res = true;

  if (gamma) res = res && TextureConvertRAW<ULONG, float, TCOMPRESS_LH>(minlevel, base);
  else       res = res && TextureConvertRAW<ULONG, long , TCOMPRESS_LH>(minlevel, base);

  return res;
}

bool TextureConvertL(LPDIRECT3DBASETEXTURE *base, int minlevel, bool gamma) {
  bool res = true;

  if (gamma) res = res && TextureConvertRAW<ULONG, float, TCOMPRESS_L>(minlevel, base);
  else       res = res && TextureConvertRAW<ULONG, long , TCOMPRESS_L>(minlevel, base);

  return res;
}

bool TextureConvertA(LPDIRECT3DBASETEXTURE *alpha, int minlevel, bool contrast) {
  bool res = true;

  if (contrast) res = res && TextureConvertRAW<ULONG, float, TCOMPRESS_A>(minlevel, alpha);
  else          res = res && TextureConvertRAW<ULONG, long , TCOMPRESS_A>(minlevel, alpha);

  return res;
}

bool TextureConvert_A(LPDIRECT3DBASETEXTURE *alpha, int minlevel, bool contrast) {
  bool res = true;

  if (contrast) res = res && TextureConvertRAW<ULONG, float, TCOMPRESS_a>(minlevel, alpha);
  else          res = res && TextureConvertRAW<ULONG, long , TCOMPRESS_a>(minlevel, alpha);

  return res;
}

bool TextureConvertL(LPDIRECT3DBASETEXTURE *lumi, int minlevel) {
  bool res = true;

  res = res && TextureConvertRAW<ULONG, long , TCOMPRESS_L>(minlevel, lumi);

  return res;
}

bool TextureConvertXYZV(LPDIRECT3DBASETEXTURE *norm, int minlevel) {
  bool res = true;

  res = res && TextureConvertRAW<ULONG, float, TCOMPRESS_XYZV>(minlevel, norm);

  return res;
}

bool TextureConvert_XYZV(LPDIRECT3DBASETEXTURE *norm, int minlevel) {
  bool res = true;

  res = res && TextureConvertRAW<ULONG, float, TCOMPRESS_xyzV>(minlevel, norm);

  return res;
}

bool TextureConvertXYZD(LPDIRECT3DBASETEXTURE *norm, int minlevel) {
  bool res = true;

  res = res && TextureConvertRAW<ULONG, float, TCOMPRESS_XYZD>(minlevel, norm);

  return res;
}

bool TextureConvert_XYZD(LPDIRECT3DBASETEXTURE *norm, int minlevel) {
  bool res = true;

  res = res && TextureConvertRAW<ULONG, float, TCOMPRESS_xyzD>(minlevel, norm);

  return res;
}

bool TextureConvert_XY_Z(LPDIRECT3DBASETEXTURE *norm, LPDIRECT3DBASETEXTURE *z, int minlevel) {
  bool res = true;

  /* TODO: not really fast */
  if (z && norm && (*z == *norm)) (*z)->AddRef();

  if (norm) res = res && TextureConvertRAW<ULONG, float, TCOMPRESS_XYz>(minlevel, norm);
  if (z   ) res = res && TextureConvertRAW<ULONG, float, TCOMPRESS_xyZ>(minlevel, z   );

  return res;
}

bool TextureConvertXY_Z(LPDIRECT3DBASETEXTURE *norm, LPDIRECT3DBASETEXTURE *z, int minlevel) {
  bool res = true;

  /* TODO: not really fast */
  if (z && norm && (*z == *norm)) (*z)->AddRef();

  if (norm) res = res && TextureConvertRAW<ULONG, float, TCOMPRESS_XYZ>(minlevel, norm);
  if (z   ) res = res && TextureConvertRAW<ULONG, float, TCOMPRESS_XYZ>(minlevel, z   );

  return res;
}

bool TextureConvertXYZ(LPDIRECT3DBASETEXTURE *norm, int minlevel) {
  bool res = true;

  res = res && TextureConvertRAW<ULONG, float, TCOMPRESS_XYZ>(minlevel, norm);

  return res;
}

bool TextureConvertXZY(LPDIRECT3DBASETEXTURE *norm, int minlevel) {
  bool res = true;

  res = res && TextureConvertRAW<ULONG, float, TCOMPRESS_XZY>(minlevel, norm);

  return res;
}

bool TextureConvert_XYZ(LPDIRECT3DBASETEXTURE *norm, int minlevel) {
  bool res = true;

  res = res && TextureConvertRAW<ULONG, float, TCOMPRESS_xyz>(minlevel, norm);

  return res;
}

bool TextureConvertXY(LPDIRECT3DBASETEXTURE *norm, int minlevel) {
  bool res = true;

  res = res && TextureConvertRAW<ULONG, float, TCOMPRESS_XY>(minlevel, norm);

  return res;
}

bool TextureConvert_XY(LPDIRECT3DBASETEXTURE *norm, int minlevel) {
  bool res = true;

  res = res && TextureConvertRAW<ULONG, float, TCOMPRESS_xy>(minlevel, norm);

  return res;
}

bool TextureConvertPM(LPDIRECT3DBASETEXTURE *base, LPDIRECT3DBASETEXTURE *norm, int minlevel) {
  bool res = true;

  res = res && TextureConvertRAW<ULONG, long , TCOMPRESS_RGBH>(minlevel, base);
  res = res && TextureConvertRAW<ULONG, float, TCOMPRESS_XYZD>(minlevel, norm);

  return res;
}

} // namespace squash
