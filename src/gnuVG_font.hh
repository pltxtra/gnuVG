/*
 * gnuVG - a free Vector Graphics library
 * Copyright (C) 2016 by Anton Persson
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

#ifndef __GNUVG_FONT_HH
#define __GNUVG_FONT_HH

#include "gnuVG_context.hh"
#include "gnuVG_object.hh"
#include "gnuVG_image.hh"
#include "gnuVG_path.hh"

#include <libtess2.h>
#include <vector>
#include <string>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_TYPES_H
#include FT_OUTLINE_H
#include FT_RENDER_H

namespace gnuVG {
	class Font : public Object {
	private:
		FT_Face freetype_face = 0; /* 0 indicates no FT_Face equivalent */
		bool freetype_face_set = false;

		class Glyph {
		public:
			bool isHinted = false;
			Path* path = VG_INVALID_HANDLE;
			Image* image = VG_INVALID_HANDLE;
			VGfloat origin[2], escapement[2];

			Glyph();
			~Glyph();

			void clear();
			void set_path(Path* _path,
				      VGboolean _isHinted,
				      const VGfloat _origin[2], const VGfloat _escapement[2]);
			void set_image(Image* _image,
				       const VGfloat _origin[2], const VGfloat _escapement[2]);
		};

		std::vector<Glyph*> glyphs;
		std::vector<VGfloat> adjustments_x;
		std::vector<VGuint> glyph_indices;
		void secure_max_glyphIndex(VGuint glyphIndex);

	public:
		Font(VGint glyphCapacityHint);
		virtual ~Font();

		/* gnuVG extensions API */
		bool gnuVG_load_font(const char* family, gnuVGFontStyle fontStyle);
		void gnuVG_render_text(
			VGfloat size, gnuVGTextAnchor anchor,
			const char* utf8, VGfloat x_anchor, VGfloat y_anchor);

		/* OpenVG equivalent API */
		virtual void vgSetParameterf(VGint paramType, VGfloat value);
		virtual void vgSetParameteri(VGint paramType, VGint value);
		virtual void vgSetParameterfv(VGint paramType, VGint count, const VGfloat *values);
		virtual void vgSetParameteriv(VGint paramType, VGint count, const VGint *values);

		virtual VGfloat vgGetParameterf(VGint paramType);
		virtual VGint vgGetParameteri(VGint paramType);

		virtual VGint vgGetParameterVectorSize(VGint paramType);

		virtual void vgGetParameterfv(VGint paramType, VGint count, VGfloat *values);
		virtual void vgGetParameteriv(VGint paramType, VGint count, VGint *values);

		void vgSetGlyphToPath(VGuint glyphIndex,
				      Path* path,
				      VGboolean isHinted,
				      const VGfloat glyphOrigin[2],
				      const VGfloat escapement[2]);
		void vgSetGlyphToImage(VGuint glyphIndex,
				       Image* image,
				       const VGfloat glyphOrigin[2],
				       const VGfloat escapement[2]);
		void vgClearGlyph(VGuint glyphIndex);
		void vgDrawGlyph(VGuint glyphIndex,
				 VGbitfield paintModes,
				 VGboolean allowAutoHinting);
		void vgDrawGlyphs(VGint glyphCount,
				  const VGuint *glyphIndices,
				  const VGfloat *adjustments_x,
				  const VGfloat *adjustments_y,
				  VGbitfield paintModes,
				  VGboolean allowAutoHinting);

	};
}

#endif
