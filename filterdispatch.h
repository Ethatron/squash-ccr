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

#undef	Accu
#undef	Reduce
#undef	Norm
#undef	Look
#undef	Code
#undef	Qunt

#if	!defined(format)
/* nested lambda-templates not possible in VS2010 */
template<const int fmt>
struct f { const int format() const { return fmt; }};
#endif

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */

#if	defined(format)
#if	(TCOMPRESS_TRESH  (format))
#define	Accu(tt, t)	\
  AccuRGBA<ACCUMODE_SCALE >(tt, t, colorgamma, alphacontrast);	// +=
#elif	(TCOMPRESS_TRANS  (format))
#define	Accu(tt, t)	\
  AccuRGBA<ACCUMODE_LINEAR>(tt, t, colorgamma, alphacontrast);	// +=
#elif	(TCOMPRESS_COLOR  (format))
#define	Accu(tt, t)	\
  AccuRGBH<ACCUMODE_LINEAR>(tt, t, colorgamma);	// +=

#elif	(TCOMPRESS_xy   == format )
#define	Accu(tt, t)	\
  AccuXYZD<ACCUMODE_LINEAR>(tt, t);	// +=
#elif	(TCOMPRESS_XY   == format )
#define	Accu(tt, t)	\
  AccuXYZD<ACCUMODE_SCALE >(tt, t);	// +=
#elif	(TCOMPRESS_xyCD == format )
#define	Accu(tt, t)	\
  AccuXYCD<ACCUMODE_LINEAR>(tt, t);	// +=
#elif	(TCOMPRESS_XYCD == format )
#define	Accu(tt, t)	\
  AccuXYCD<ACCUMODE_SCALE >(tt, t);	// +=
#elif	(TCOMPRESS_NINDEP(format))
#define	Accu(tt, t)	\
  AccuXYZD<ACCUMODE_LINEAR>(tt, t);	// +=
#elif	(TCOMPRESS_NORMAL(format))
#define	Accu(tt, t)	\
  AccuXYZD<ACCUMODE_SCALE >(tt, t);	// +=
#else
#error
#endif
#else
#define	Accu(nn, n) {												\
  /**/ if (TCOMPRESS_TRESH  (fmt.format())) AccuRGBA<ACCUMODE_SCALE >(nn, n, colorgamma, alphacontrast);	\
  else if (TCOMPRESS_TRANS  (fmt.format())) AccuRGBA<ACCUMODE_LINEAR>(nn, n, colorgamma, alphacontrast);	\
  else if (TCOMPRESS_COLOR  (fmt.format())) AccuRGBH<ACCUMODE_LINEAR>(nn, n, colorgamma);			\
														\
  else if (TCOMPRESS_xy   == fmt.format() ) AccuXYZD<ACCUMODE_LINEAR>(nn, n);					\
  else if (TCOMPRESS_XY   == fmt.format() ) AccuXYZD<ACCUMODE_SCALE >(nn, n);					\
  else if (TCOMPRESS_xyCD == fmt.format() ) AccuXYCD<ACCUMODE_LINEAR>(nn, n);					\
  else if (TCOMPRESS_XYCD == fmt.format() ) AccuXYCD<ACCUMODE_SCALE >(nn, n);					\
  else if (TCOMPRESS_NINDEP (fmt.format())) AccuXYZD<ACCUMODE_LINEAR>(nn, n);					\
  else if (TCOMPRESS_NORMAL (fmt.format())) AccuXYZD<ACCUMODE_SCALE >(nn, n);					\
}
#endif

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */

#if	defined(format)
#if	(TCOMPRESS_TRESH  (format))
#define	Reduce(nn, nt)	\
  ReduceRGBA<ACCUMODE_SCALE >(nn, nt);	// +=
#elif	(TCOMPRESS_TRANS  (format))
#define	Reduce(nn, nt)	\
  ReduceRGBA<ACCUMODE_LINEAR>(nn, nt);	// +=
#elif	(TCOMPRESS_COLOR  (format))
#define	Reduce(nn, nt)	\
  ReduceRGBH<ACCUMODE_LINEAR>(nn, nt);	// +=

#elif	(TCOMPRESS_xy   == format )
#define	Reduce(nn, nt)	\
  ReduceXYZD<ACCUMODE_LINEAR>(nn, nt);	// +=
#elif	(TCOMPRESS_XY   == format )
#define	Reduce(nn, nt)	\
  ReduceXYZD<ACCUMODE_SCALE >(nn, nt);	// +=
#elif	(TCOMPRESS_xyCD == format )
#define	Reduce(nn, nt)	\
  ReduceXYCD<ACCUMODE_LINEAR>(nn, nt);	// +=
#elif	(TCOMPRESS_XYCD == format )
#define	Reduce(nn, nt)	\
  ReduceXYCD<ACCUMODE_SCALE >(nn, nt);	// +=
#elif	(TCOMPRESS_NINDEP (format))
#define	Reduce(nn, nt)	\
  ReduceXYZD<ACCUMODE_LINEAR>(nn, nt);	// +=
#elif	(TCOMPRESS_NORMAL (format))
#define	Reduce(nn, nt)	\
  ReduceXYZD<ACCUMODE_SCALE >(nn, nt);	// +=
#else
#error
#endif
#else
#define	Reduce(nn, nt) {								\
  /**/ if (TCOMPRESS_TRESH  (fmt.format())) ReduceRGBA<ACCUMODE_SCALE >(nn, nt);	\
  else if (TCOMPRESS_TRANS  (fmt.format())) ReduceRGBA<ACCUMODE_LINEAR>(nn, nt);	\
  else if (TCOMPRESS_COLOR  (fmt.format())) ReduceRGBH<ACCUMODE_LINEAR>(nn, nt);	\
											\
  else if (TCOMPRESS_xy   == fmt.format() ) ReduceXYZD<ACCUMODE_LINEAR>(nn, nt);	\
  else if (TCOMPRESS_XY   == fmt.format() ) ReduceXYZD<ACCUMODE_SCALE >(nn, nt);	\
  else if (TCOMPRESS_xyCD == fmt.format() ) ReduceXYCD<ACCUMODE_LINEAR>(nn, nt);	\
  else if (TCOMPRESS_XYCD == fmt.format() ) ReduceXYCD<ACCUMODE_SCALE >(nn, nt);	\
  else if (TCOMPRESS_NINDEP (fmt.format())) ReduceXYZD<ACCUMODE_LINEAR>(nn, nt);	\
  else if (TCOMPRESS_NORMAL (fmt.format())) ReduceXYZD<ACCUMODE_SCALE >(nn, nt);	\
}
#endif

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */

#if	defined(format)
#if	(TCOMPRESS_TRESH  (format))
#define	Norm(onn, nn, av, level, l)	\
  NormRGBA<ACCUMODE_SCALE >(onn, nn, av, level, l, ALPHAS_SCALEBYLEVEL, colorgammainv, alphacontrastinv);
#elif	(TCOMPRESS_TRANS  (format))
#define	Norm(onn, nn, av, level, l)	\
  NormRGBA<ACCUMODE_LINEAR>(onn, nn, av, level, l, ALPHAS_SCALEBYLEVEL, colorgammainv, alphacontrastinv);
#elif	(TCOMPRESS_COLOR  (format))
#define	Norm(onn, nn, av, level, l)	\
  NormRGBH<ACCUMODE_LINEAR>(onn, nn, av                               , colorgammainv);

#elif	(TCOMPRESS_xy   == format )
#define	Norm(onn, nn, av, level, l)	\
  NormXYZD<ACCUMODE_LINEAR>(onn, nn, av, level, l, NORMALS_SCALEBYLEVEL);
#elif	(TCOMPRESS_XY   == format )
#define	Norm(onn, nn, av, level, l)	\
  NormXYZD<ACCUMODE_SCALE >(onn, nn, av, level, l, NORMALS_SCALEBYLEVEL);
#elif	(TCOMPRESS_xyCD == format )
#define	Norm(onn, nn, av, level, l)	\
  NormXYCD<ACCUMODE_LINEAR>(onn, nn, av, level, l, NORMALS_SCALEBYLEVEL);
#elif	(TCOMPRESS_XYCD == format )
#define	Norm(onn, nn, av, level, l)	\
  NormXYCD<ACCUMODE_SCALE >(onn, nn, av, level, l, NORMALS_SCALEBYLEVEL);
#elif	(TCOMPRESS_NINDEP (format))
#define	Norm(onn, nn, av, level, l)	\
  NormXYZD<ACCUMODE_LINEAR>(onn, nn, av, level, l, NORMALS_SCALEBYLEVEL);
#elif	(TCOMPRESS_NORMAL (format))
#define	Norm(onn, nn, av, level, l)	\
  NormXYZD<ACCUMODE_SCALE >(onn, nn, av, level, l, NORMALS_SCALEBYLEVEL);
#else
#error
#endif
#else
#define	Norm(onn, nn, av, level, l) {														\
  /**/ if (TCOMPRESS_TRESH  (format)) NormRGBA<ACCUMODE_SCALE >(onn, nn, av, level, l, ALPHAS_SCALEBYLEVEL, colorgammainv, alphacontrastinv);	\
  else if (TCOMPRESS_TRANS  (format)) NormRGBA<ACCUMODE_LINEAR>(onn, nn, av, level, l, ALPHAS_SCALEBYLEVEL, colorgammainv, alphacontrastinv);	\
  else if (TCOMPRESS_COLOR  (format)) NormRGBH<ACCUMODE_LINEAR>(onn, nn, av                               , colorgammainv);			\
																		\
  else if (TCOMPRESS_xy   == format ) NormXYZD<ACCUMODE_LINEAR>(onn, nn, av, level, l, NORMALS_SCALEBYLEVEL);					\
  else if (TCOMPRESS_XY   == format ) NormXYZD<ACCUMODE_SCALE >(onn, nn, av, level, l, NORMALS_SCALEBYLEVEL);					\
  else if (TCOMPRESS_xyCD == format ) NormXYCD<ACCUMODE_LINEAR>(onn, nn, av, level, l, NORMALS_SCALEBYLEVEL);					\
  else if (TCOMPRESS_XYCD == format ) NormXYCD<ACCUMODE_SCALE >(onn, nn, av, level, l, NORMALS_SCALEBYLEVEL);					\
  else if (TCOMPRESS_NINDEP (format)) NormXYZD<ACCUMODE_LINEAR>(onn, nn, av, level, l, NORMALS_SCALEBYLEVEL);					\
  else if (TCOMPRESS_NORMAL (format)) NormXYZD<ACCUMODE_SCALE >(onn, nn, av, level, l, NORMALS_SCALEBYLEVEL);					\
}
#endif

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */

#if	defined(format)
#if	(TCOMPRESS_TRANS  (format))
#define	Look(nn, nr)	\
  LookRGBA<TRGTMODE_CODING_RGB    >(nn, nr);
#elif	(TCOMPRESS_COLOR  (format))
#define	Look(nn, nr)	\
  LookRGBH<TRGTMODE_CODING_RGB    >(nn, nr);

#elif	(TCOMPRESS_xy   == format )
#define	Look(nn, nr)	\
  LookXYZD<TRGTMODE_CODING_XY     >(nn, nr);
#elif	(TCOMPRESS_XY   == format )
#define	Look(nn, nr)	\
  LookXYZD<TRGTMODE_CODING_DXDYt  >(nn, nr);
#elif	(TCOMPRESS_xyCD == format )
#define	Look(nn, nr)	\
  LookXYCD<TRGTMODE_CODING_XY     >(nn, nr);
#elif	(TCOMPRESS_XYCD == format )
#define	Look(nn, nr)	\
  LookXYCD<TRGTMODE_CODING_DXDYt  >(nn, nr);
#elif	(TCOMPRESS_NINDEP (format))
#define	Look(nn, nr)	\
  LookXYZD<TRGTMODE_CODING_XYZ    >(nn, nr);
#elif	(TCOMPRESS_NORMAL (format))
#define	Look(nn, nr)	\
  LookXYZD<TRGTMODE_CODING_DXDYDZt>(nn, nr);
#else
#error
#endif
#else
#define	Look(nn, nr) {									\
  /**/ if (TCOMPRESS_TRANS  (format)) LookRGBA<TRGTMODE_CODING_RGB    >(nn, nr);	\
  else if (TCOMPRESS_COLOR  (format)) LookRGBH<TRGTMODE_CODING_RGB    >(nn, nr);	\
											\
  else if (TCOMPRESS_xy   == format ) LookXYZD<TRGTMODE_CODING_XY     >(nn, nr);	\
  else if (TCOMPRESS_XY   == format ) LookXYZD<TRGTMODE_CODING_DXDYt  >(nn, nr);	\
  else if (TCOMPRESS_xyCD == format ) LookXYCD<TRGTMODE_CODING_XY     >(nn, nr);	\
  else if (TCOMPRESS_XYCD == format ) LookXYCD<TRGTMODE_CODING_DXDYt  >(nn, nr);	\
  else if (TCOMPRESS_NINDEP (format)) LookXYZD<TRGTMODE_CODING_XYZ    >(nn, nr);	\
  else if (TCOMPRESS_NORMAL (format)) LookXYZD<TRGTMODE_CODING_DXDYdZt>(nn, nr);	\
}
#endif

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */

#if	defined(format)
#if	(TCOMPRESS_TRANS  (format))
#define	Code(nn, nr, z)	\
  CodeRGBA<TRGTMODE_CODING_RGB       >(nn, nr);
#elif	(TCOMPRESS_COLOR  (format))
#define	Code(nn, nr, z)	\
  CodeRGBH<TRGTMODE_CODING_RGB       >(nn, nr);

#elif	(TCOMPRESS_xy  ==  format )
#define	Code(nn, nr, z)	\
  CodeXYZD<TRGTMODE_CODING_XY     , z>(nn, nr);
#elif	(TCOMPRESS_XY  ==  format )
#define	Code(nn, nr, z)	\
  CodeXYZD<TRGTMODE_CODING_DXDYt  , z>(nn, nr);
#elif	(TCOMPRESS_xyCD == format )
#define	Code(nn, nr, z)	\
  CodeXYCD<TRGTMODE_CODING_XY     , z>(nn, nr);
#elif	(TCOMPRESS_XYCD == format )
#define	Code(nn, nr, z)	\
  CodeXYCD<TRGTMODE_CODING_DXDYt  , z>(nn, nr);
#elif	(TCOMPRESS_NINDEP (format))
#define	Code(nn, nr, z)	\
  CodeXYZD<TRGTMODE_CODING_XYZ    , z>(nn, nr);
#elif	(TCOMPRESS_NORMAL (format))
#define	Code(nn, nr, z)	\
  CodeXYZD<TRGTMODE_CODING_DXDYDZt, z>(nn, nr);
#else
#error
#endif
#else
#define	Code(nn, nr, zprec) {								\
  /**/ if (TCOMPRESS_TRANS  (format)) CodeRGBA<TRGTMODE_CODING_RGB           >(nn, nr);	\
  else if (TCOMPRESS_COLOR  (format)) CodeRGBH<TRGTMODE_CODING_RGB           >(nn, nr);	\
											\
  else if (TCOMPRESS_xy   == format ) CodeXYZD<TRGTMODE_CODING_XY     , zprec>(nn, nr);	\
  else if (TCOMPRESS_XY   == format ) CodeXYZD<TRGTMODE_CODING_DXDYt  , zprec>(nn, nr);	\
  else if (TCOMPRESS_xyCD == format ) CodeXYCD<TRGTMODE_CODING_XY     , zprec>(nn, nr);	\
  else if (TCOMPRESS_XYCD == format ) CodeXYCD<TRGTMODE_CODING_DXDYt  , zprec>(nn, nr);	\
  else if (TCOMPRESS_NINDEP (format)) CodeXYZD<TRGTMODE_CODING_XYZ    , zprec>(nn, nr);	\
  else if (TCOMPRESS_NORMAL (format)) CodeXYZD<TRGTMODE_CODING_DXDYdZt, zprec>(nn, nr);	\
}
#endif

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */

#if	defined(format)
#if	(TCOMPRESS_TRANS  (format))
#define	Join(nn, nr)	      JoinRGBA<TRGTMODE_CODING_RGB    >(nn, nr)
#elif	(TCOMPRESS_COLOR  (format))
#define	Join(nn, nr)	      JoinRGBH<TRGTMODE_CODING_RGB    >(nn, nr)

#elif	(TCOMPRESS_xy   == format )
#define	Join(nn, nr)	      JoinXYZD<TRGTMODE_CODING_XY     >(nn, nr)
#elif	(TCOMPRESS_XY   == format )
#define	Join(nn, nr)	      JoinXYZD<TRGTMODE_CODING_DXDYt  >(nn, nr)
#elif	(TCOMPRESS_xyCD == format )
#define	Join(nn, nr)	      JoinXYCD<TRGTMODE_CODING_XY     >(nn, nr)
#elif	(TCOMPRESS_XYCD == format )
#define	Join(nn, nr)	      JoinXYCD<TRGTMODE_CODING_DXDYt  >(nn, nr)
#elif	(TCOMPRESS_NINDEP (format))
#define	Join(nn, nr)	      JoinXYZD<TRGTMODE_CODING_XYZ    >(nn, nr)
#elif	(TCOMPRESS_NORMAL (format))
#define	Join(nn, nr)	      JoinXYZD<TRGTMODE_CODING_DXDYDZt>(nn, nr)
#else
#error
#endif
#else
#define	Join(nn, nr)								\
  (TCOMPRESS_TRANS  (format) ? JoinRGBA<TRGTMODE_CODING_RGB    >(nn, nr) :	\
  (TCOMPRESS_COLOR  (format) ? JoinRGBH<TRGTMODE_CODING_RGB    >(nn, nr) :	\
										\
  (TCOMPRESS_xy   == format  ? JoinXYZD<TRGTMODE_CODING_XY     >(nn, nr) :	\
  (TCOMPRESS_XY   == format  ? JoinXYZD<TRGTMODE_CODING_DXDYt  >(nn, nr) :	\
  (TCOMPRESS_xyCD == format  ? JoinXYCD<TRGTMODE_CODING_XY     >(nn, nr) :	\
  (TCOMPRESS_XYCD == format  ? JoinXYCD<TRGTMODE_CODING_DXDYt  >(nn, nr) :	\
  (TCOMPRESS_NINDEP (format) ? JoinXYZD<TRGTMODE_CODING_XYZ    >(nn, nr) :	\
  (TCOMPRESS_NORMAL (format) ? JoinXYZD<TRGTMODE_CODING_DXDYdZt>(nn, nr) :	\
  (0)))))))))
#endif

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */

#if	defined(format)
#if	(TCOMPRESS_TRANS  (format))
#define	Range(bo, bi)	\
  RangeRGBA<TRGTMODE_CODING_RGB    >(bo, bi);
#elif	(TCOMPRESS_COLOR  (format))
#define	Range(bo, bi)	\
  RangeRGBH<TRGTMODE_CODING_RGB    >(bo, bi);
#elif	(TCOMPRESS_xy   == format )
#define	Range(bo, bi)	\
  RangeXYZD<TRGTMODE_CODING_XY     >(bo, bi);
#elif	(TCOMPRESS_XY   == format )
#define	Range(bo, bi)	\
  RangeXYZD<TRGTMODE_CODING_DXDYt  >(bo, bi);
#elif	(TCOMPRESS_xyCD == format )
#define	Range(bo, bi)	\
  RangeXYZD<TRGTMODE_CODING_XY     >(bo, bi);
#elif	(TCOMPRESS_XYCD == format )
#define	Range(bo, bi)	\
  RangeXYZD<TRGTMODE_CODING_DXDYt  >(bo, bi);
#elif	(TCOMPRESS_NINDEP (format))
#define	Range(bo, bi)	\
  RangeXYZD<TRGTMODE_CODING_XYZ    >(bo, bi);
#elif	(TCOMPRESS_NORMAL (format))
#define	Range(bo, bi)	\
  RangeXYZD<TRGTMODE_CODING_DXDYDZt>(bo, bi);
#else
#error
#endif
#else
#define	Range(bo, bi)								\
  (TCOMPRESS_TRANS  (format) ? RangeRGBA<TRGTMODE_CODING_RGB    >(bo, bi) :	\
  (TCOMPRESS_COLOR  (format) ? RangeRGBH<TRGTMODE_CODING_RGB    >(bo, bi) :	\
										\
  (TCOMPRESS_xy   == format  ? RangeXYZD<TRGTMODE_CODING_XY     >(bo, bi) :	\
  (TCOMPRESS_XY   == format  ? RangeXYZD<TRGTMODE_CODING_DXDYt  >(bo, bi) :	\
  (TCOMPRESS_xyCD == format  ? RangeXYCD<TRGTMODE_CODING_XY     >(bo, bi) :	\
  (TCOMPRESS_XYCD == format  ? RangeXYCD<TRGTMODE_CODING_DXDYt  >(bo, bi) :	\
  (TCOMPRESS_NINDEP (format) ? RangeXYZD<TRGTMODE_CODING_XYZ    >(bo, bi) :	\
  (TCOMPRESS_NORMAL (format) ? RangeXYZD<TRGTMODE_CODING_DXDYdZt>(bo, bi) :	\
  (0)))))))))
#endif

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */

#if	defined(format)
#if	defined(A)

#if	(TCOMPRESS_CHANNELS(format) == 4)
#if	(A > 2)
#if	(TCOMPRESS_TRANS (format))
#define	Qunt(nn, nr, A)	\
  QuntRGBA<TRGTMODE_CODING_RGB    , 4,  4,  4,  4>(nn, nr);
#elif	(TCOMPRESS_COLOR (format))
#define	Qunt(nn, nr, A)	\
  QuntRGBH<TRGTMODE_CODING_RGB    , 4,  4,  4,  4>(nn, nr);
#elif	(TCOMPRESS_NINDEP(format))
#define	Qunt(nn, nr, A)	\
  QuntXYZD<TRGTMODE_CODING_XYZ    , 4,  4,  4,  4>(nn, nr);
#elif	(TCOMPRESS_NORMAL(format))
#define	Qunt(nn, nr, A)	\
  QuntXYZD<TRGTMODE_CODING_DXDYDZt, 4,  4,  4,  4>(nn, nr);
#else
#error
#endif
#elif	(A > 1)
#if	(TCOMPRESS_TRANS (format))
#define	Qunt(nn, nr, A)	\
  QuntRGBA<TRGTMODE_CODING_RGB    , 2, 10, 10, 10>(nn, nr);
#elif	(TCOMPRESS_COLOR (format))
#define	Qunt(nn, nr, A)	\
  QuntRGBH<TRGTMODE_CODING_RGB    , 2, 10, 10, 10>(nn, nr);
#elif	(TCOMPRESS_NINDEP(format))
#define	Qunt(nn, nr, A)	\
  QuntXYZD<TRGTMODE_CODING_XYZ    , 2, 10, 10, 10>(nn, nr);
#elif	(TCOMPRESS_NORMAL(format))
#define	Qunt(nn, nr, A)	\
  QuntXYZD<TRGTMODE_CODING_DXDYDZt, 2, 10, 10, 10>(nn, nr);
#else
#error
#endif
#else
#if	(TCOMPRESS_COLOR (format))
#define	Qunt(nn, nr, A)	\
  QuntRGBH<TRGTMODE_CODING_RGB    , 1,  5,  5,  5>(nn, nr);
#elif	(TCOMPRESS_NINDEP(format))
#define	Qunt(nn, nr, A)	\
  QuntXYZD<TRGTMODE_CODING_XYZ    , 1,  5,  5,  5>(nn, nr);
#elif	(TCOMPRESS_NORMAL(format))
#define	Qunt(nn, nr, A)	\
  QuntXYZD<TRGTMODE_CODING_DXDYDZt, 1,  5,  5,  5>(nn, nr);
#else
#error
#endif
#endif

#elif	(TCOMPRESS_CHANNELS(format) == 3)
#if	(TCOMPRESS_COLOR (format))
#define	Qunt(nn, nr, A)	\
  QuntRGBH<TRGTMODE_CODING_RGB    , 0,  5,  6,  5>(nn, nr);
#elif	(TCOMPRESS_NINDEP(format))
#define	Qunt(nn, nr, A)	\
  QuntXYZD<TRGTMODE_CODING_XYZ    , 0,  5,  6,  5>(nn, nr);
#elif	(TCOMPRESS_NORMAL(format))
#define	Qunt(nn, nr, A)	\
  QuntXYZD<TRGTMODE_CODING_DXDYDZt, 0,  5,  6,  5>(nn, nr);
#else
#error
#endif
#elif	(TCOMPRESS_CHANNELS(format) == 2)
#if	(TCOMPRESS_TRANS (format))
#define	Qunt(nn, nr, A)	\
  QuntRGBA<TRGTMODE_CODING_RGB    , 4,  0,  4,  0>(nn, nr);
#elif	(TCOMPRESS_COLOR (format))
#define	Qunt(nn, nr, A)	\
  QuntRGBH<TRGTMODE_CODING_RGB    , 4,  0,  4,  0>(nn, nr);
#elif	(TCOMPRESS_NINDEP(format))
#define	Qunt(nn, nr, A)	\
  QuntXYZD<TRGTMODE_CODING_XYZ    , 4,  0,  4,  0>(nn, nr);
#elif	(TCOMPRESS_NORMAL(format))
#define	Qunt(nn, nr, A)	\
  QuntXYZD<TRGTMODE_CODING_DXDYDZt, 4,  0,  4,  0>(nn, nr);
#else
#error
#endif
#elif	(TCOMPRESS_CHANNELS(format) == 1)
#endif
#endif
#else
#define	Qunt(nn, nr, zprec)									\
  (TCOMPRESS_CHANNELS(format) == 4 ?								\
    (A > 2 ?											\
      (TCOMPRESS_TRANS (format) ? QuntRGBA<TRGTMODE_CODING_RGB    , 4,  4,  4,  4>(nn, nr) :	\
      (TCOMPRESS_COLOR (format) ? QuntRGBH<TRGTMODE_CODING_RGB    , 4,  4,  4,  4>(nn, nr) :	\
      (TCOMPRESS_NINDEP(format) ? QuntXYZD<TRGTMODE_CODING_XYZ    , 4,  4,  4,  4>(nn, nr) :	\
      (TCOMPRESS_NORMAL(format) ? QuntXYZD<TRGTMODE_CODING_DXDYDZt, 4,  4,  4,  4>(nn, nr) : 0	\
      )))) :											\
    (A > 1 ?											\
      (TCOMPRESS_TRANS (format) ? QuntRGBA<TRGTMODE_CODING_RGB    , 2, 10, 10, 10>(nn, nr) :	\
      (TCOMPRESS_COLOR (format) ? QuntRGBH<TRGTMODE_CODING_RGB    , 2, 10, 10, 10>(nn, nr) :	\
      (TCOMPRESS_NINDEP(format) ? QuntXYZD<TRGTMODE_CODING_XYZ    , 2, 10, 10, 10>(nn, nr) :	\
      (TCOMPRESS_NORMAL(format) ? QuntXYZD<TRGTMODE_CODING_DXDYDZt, 2, 10, 10, 10>(nn, nr) : 0	\
      )))) :											\
    (												\
      (TCOMPRESS_TRANS (format) ? QuntRGBA<TRGTMODE_CODING_RGB    , 1,  5,  5,  5>(nn, nr) :	\
      (TCOMPRESS_COLOR (format) ? QuntRGBH<TRGTMODE_CODING_RGB    , 1,  5,  5,  5>(nn, nr) :	\
      (TCOMPRESS_NINDEP(format) ? QuntXYZD<TRGTMODE_CODING_XYZ    , 1,  5,  5,  5>(nn, nr) :	\
      (TCOMPRESS_NORMAL(format) ? QuntXYZD<TRGTMODE_CODING_DXDYDZt, 1,  5,  5,  5>(nn, nr) : 0	\
      ))))											\
    ))) :											\
  (TCOMPRESS_CHANNELS(format) == 3 ?								\
    (												\
      (TCOMPRESS_COLOR (format) ? QuntRGBA<TRGTMODE_CODING_RGB    , 0,  5,  6,  5>(nn, nr) :	\
      (TCOMPRESS_COLOR (format) ? QuntRGBH<TRGTMODE_CODING_RGB    , 0,  5,  6,  5>(nn, nr) :	\
      (TCOMPRESS_NINDEP(format) ? QuntXYZD<TRGTMODE_CODING_XYZ    , 0,  5,  6,  5>(nn, nr) :	\
      (TCOMPRESS_NORMAL(format) ? QuntXYZD<TRGTMODE_CODING_DXDYDZt, 0,  5,  6,  5>(nn, nr) : 0	\
      ))))											\
    ) :											\
  (TCOMPRESS_CHANNELS(format) == 2 ?								\
    (												\
      (TCOMPRESS_COLOR (format) ? QuntRGBA<TRGTMODE_CODING_RGB    , 4,  0,  4,  0>(nn, nr) :	\
      (TCOMPRESS_COLOR (format) ? QuntRGBH<TRGTMODE_CODING_RGB    , 4,  0,  4,  0>(nn, nr) :	\
      (TCOMPRESS_NINDEP(format) ? QuntXYZD<TRGTMODE_CODING_XYZ    , 4,  0,  4,  0>(nn, nr) :	\
      (TCOMPRESS_NORMAL(format) ? QuntXYZD<TRGTMODE_CODING_DXDYDZt, 4,  0,  4,  0>(nn, nr) : 0	\
      ))))											\
    ) :												\
  (0)												\
  )))
#endif
