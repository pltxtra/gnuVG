/*
 * gnuVG - a free Vector Graphics library
 * Copyright (C) 2014 by Anton Persson
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as
 *  published by the Free Software Foundation, either version 3 of
 *  the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef _GVGEXTENSIONS_H
#define _GVGEXTENSIONS_H

#ifdef __cplusplus
extern "C" {
#endif

	VG_API_CALL VGHandle VG_API_ENTRY gnuvgCreateContext() VG_API_EXIT;
	VG_API_CALL void VG_API_ENTRY gnuvgDestroyContext(VGHandle context) VG_API_EXIT;
	VG_API_CALL void VG_API_ENTRY gnuvgUseContext(VGHandle context) VG_API_EXIT;
	VG_API_CALL void VG_API_ENTRY gnuvgResize(VGint pixel_width, VGint pixel_height) VG_API_EXIT;

	// pass NULL or VG_INVALID_HANDLE to reset to on-screen rendering
	VG_API_CALL void VG_API_ENTRY gnuvgRenderToImage(VGImage image) VG_API_EXIT;
	VG_API_CALL VGImage VG_API_ENTRY gnuvgGetRenderTarget(void) VG_API_EXIT;


	#define	gnuVG_NORMAL               0
	#define	gnuVG_ITALIC               1
	#define	gnuVG_BOLD                 2
	#define	gnuVG_BOLD_ITALIC          3 /* you can use gnuVG_ITALIC | gnuVG_BOLD */
	typedef int gnuVGFontStyle;

	typedef enum {
		gnuVG_ANCHOR_START               = 0,
		gnuVG_ANCHOR_MIDDLE              = 1,
		gnuVG_ANCHOR_END                 = 2,
	} gnuVGTextAnchor;

	VG_API_CALL VGFont VG_API_ENTRY gnuvgLoadFont(const char* family,
						      gnuVGFontStyle fontStyle);
	VG_API_CALL void VG_API_ENTRY gnuvgRenderText(
		VGFont font, VGfloat size, gnuVGTextAnchor anchor,
		const char* utf8, VGfloat x_anchor, VGfloat y_anchor);

	/* reset bounding box calculation */
	VG_API_CALL void VG_API_ENTRY gnuvgResetBoundingBox();

	/* Return bounding box covering all paths rendered since last reset.
	 * Coordinates are returned in an array:
	 * sp_ep[] = {top_left_x, top_left_y, bottom_right_x, bottom_right_y }
	 */
	VG_API_CALL void VG_API_ENTRY gnuvgGetBoundingBox(VGfloat *sp_ep);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* _GVGEXTENSIONS_H */
