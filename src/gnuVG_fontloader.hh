/*
 * OpenVG FreeType glyph loader
 * Copyright 2014 by Anton Persson
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

#ifndef __FONT_LOADER
#define __FONT_LOADER

#include <VG/openvg.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_TYPES_H
#include FT_OUTLINE_H
#include FT_RENDER_H

/* virtual DPI used internally */
#define GNUVG_FONT_DPI			10000
/* font size, in points, used with FreeType */
#define GNUVG_FONT_POINTS		12
/* A point is 1 / 72th of an inch */
#define GNUVG_FONT_POINTS_PER_INCH	72
/* Use this to rescale when rendering */
#define GNUVG_FONT_PIXELSIZE (GNUVG_FONT_POINTS * GNUVG_FONT_DPI)

#ifdef __cplusplus
extern "C" {
#endif

	int fontLoader_setup();
	VGFont fontLoader_load_font(VGFont font, FT_Face* m_face, const char *font_path, VGboolean vertical);
	VGPath fontLoader_load_single_glyph(const char *font_path, VGint glyph_unicode);



#ifdef __cplusplus
}
#endif

#endif
