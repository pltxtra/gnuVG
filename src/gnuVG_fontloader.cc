/*
 * OpenVG FreeType glyph loader
 * Copyright 2016 by Anton Persson
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


#include "gnuVG_fontloader.hh"

//#define __DO_GNUVG_DEBUG
#include "gnuVG_debug.hh"

#ifdef __cplusplus
extern "C" {
#endif

	static FT_Library m_library;

	int fontLoader_setup() {
		FT_Error error = FT_Err_Ok;
		m_library = 0;

		// For simplicity, the error handling is very rudimentary.
		error = FT_Init_FreeType(&m_library);
		return error ? -1 : 0;
	}

	static int fontLoader_Outline_MoveToFunc(const FT_Vector*  to,
						 void*             user ) {
		VGPath *p = ((VGPath *)user);
		VGubyte seg = VG_MOVE_TO_ABS;
		VGfloat data[] = {
			((float)(to->x)),
			((float)(to->y))
		};
		GNUVG_DEBUG("M%f,%f ", data[0], data[1]);
		vgAppendPathData(*p, 1, &seg, data);
		return 0;
	}

	static int fontLoader_Outline_LineToFunc(const FT_Vector*  to,
						 void*             user ) {
		VGPath *p = ((VGPath *)user);
		VGubyte seg = VG_LINE_TO_ABS;
		VGfloat data[] = {
			((float)(to->x)),
			((float)(to->y))
		};
		GNUVG_DEBUG("L%f,%f ", data[0], data[1]);
		vgAppendPathData(*p, 1, &seg, data);
		return 0;
	}

	static int fontLoader_Outline_ConicToFunc(const FT_Vector*  control,
						  const FT_Vector*  to,
						  void*             user ) {
		VGPath *p = ((VGPath *)user);
		VGubyte seg = VG_QUAD_TO_ABS;
		VGfloat data[] = {
			((float)(control->x)),
			((float)(control->y)),
			((float)(to->x)),
			((float)(to->y))
		};
		GNUVG_DEBUG("Q%f,%f %f,%f ", data[0], data[1], data[2], data[3]);
		vgAppendPathData(*p, 1, &seg, data);
		return 0;
	}

	static int fontLoader_Outline_CubicToFunc(const FT_Vector*  control1,
						  const FT_Vector*  control2,
						  const FT_Vector*  to,
						  void*             user ) {
		VGPath *p = ((VGPath *)user);
		VGubyte seg = VG_CUBIC_TO_ABS;
		VGfloat data[] = {
			((float)(control1->x)),
			((float)(control1->y)),
			((float)(control2->x)),
			((float)(control2->y)),
			((float)(to->x)),
			((float)(to->y))
		};
		GNUVG_DEBUG("C%f,%f %f,%f %f,%f ", data[0], data[1], data[2], data[3], data[4], data[5]);
		vgAppendPathData(*p, 1, &seg, data);
		return 0;
	}

	static FT_Outline_Funcs fot_funcs = {
		.move_to = fontLoader_Outline_MoveToFunc,
		.line_to = fontLoader_Outline_LineToFunc,
		.conic_to = fontLoader_Outline_ConicToFunc,
		.cubic_to = fontLoader_Outline_CubicToFunc,
		.shift = 0,
		.delta = 0
	};

	static VGPath create_path(FT_Face m_face) {
		FT_Outline* outline = &m_face->glyph->outline;

		VGPath result = vgCreatePath(VG_PATH_FORMAT_STANDARD, VG_PATH_DATATYPE_F,
					     1,0,0,0, VG_PATH_CAPABILITY_ALL);

		GNUVG_DEBUG("d=\"");
		if(result != VG_INVALID_HANDLE) {
			(void)FT_Outline_Decompose(outline,
						   &fot_funcs,
						   &result);
		}
		GNUVG_DEBUG("\"\n");

		return result;
	}

	VGPath fontLoader_load_single_glyph(const char *font_path, VGint glyph_unicode) {
		FT_Error error = FT_Err_Ok;
		FT_Face m_face = 0;
		VGPath path = VG_INVALID_HANDLE;

		error = FT_New_Face(m_library,
				    font_path,
				    0,
				    &m_face);
		if (!error) {
			error = FT_Set_Char_Size(m_face,
						 0,
						 /* FreeType parameter is in 1/64th of points */
						 GNUVG_FONT_POINTS * 64,
						 GNUVG_FONT_DPI,
						 GNUVG_FONT_DPI);

			if (!error) {
				FT_ULong  charcode;
				FT_UInt   gindex;

				charcode = FT_Get_First_Char(m_face, &gindex );
				while (gindex != 0) {
					if(charcode == (FT_ULong)glyph_unicode) {
						error = FT_Load_Glyph(m_face,
								      gindex,
								      FT_LOAD_DEFAULT);
						if(!error) {
							path = create_path(m_face);
						}

						return path;
					}

					charcode = FT_Get_Next_Char(m_face, charcode, &gindex );
				}
			}
		}
		return path;
	}

	static void load_glyph(VGFont vg_font, FT_Face m_face, FT_ULong charcode,
			       FT_UInt gindex, VGboolean vertical) {
		FT_Error error = FT_Err_Ok;
		VGfloat origin[] = {0.0f, 0.0f};
		VGfloat escape[] = {0.0f, 0.0f};

		error = FT_Load_Glyph(m_face,
				      gindex,
				      FT_LOAD_DEFAULT);
		if(!error) {
			auto path = create_path(m_face);
			if(path != VG_INVALID_HANDLE) {
				if(vertical) {
					escape[0] = 0.0f;
					escape[1] = m_face->glyph->metrics.vertAdvance;
				} else {
					escape[0] = m_face->glyph->metrics.horiAdvance;
					escape[1] = 0.0f;
				}

				vgSetGlyphToPath(vg_font, charcode, path, VG_FALSE, origin, escape);
				vgDestroyPath(path); /* we can destroy our handle - it is safely stored
						      * in the VGFont object already.
						      */
			}
		}
	}

	VGFont fontLoader_load_font(
		VGFont vg_font,
		FT_Face* m_face,
		const char *font_path,
		VGboolean vertical) {
		FT_Error error = FT_Err_Ok;

		if(vg_font) {
			error = FT_New_Face(m_library,
					    font_path,
					    0,
					    m_face);
			if (!error) {
				error = FT_Set_Char_Size(*m_face,
							 0,
							 /* FreeType parameter is in 1/64th of points */
							 GNUVG_FONT_POINTS * 64,
							 GNUVG_FONT_DPI,
							 GNUVG_FONT_DPI);

				if (!error) {
					FT_ULong  charcode;
					FT_UInt   gindex;

					charcode = FT_Get_First_Char(*m_face, &gindex );
					while ( gindex != 0 )
					{
						if(charcode < 128)
							load_glyph(vg_font, *m_face, charcode, gindex, vertical);
						charcode = FT_Get_Next_Char(*m_face, charcode, &gindex );
					}
					return vg_font;
				}
				FT_Done_Face(*m_face);
			}
		}
		return VG_INVALID_HANDLE;
	}


#ifdef __cplusplus
}
#endif
