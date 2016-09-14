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

#include "gnuVG_font.hh"
#include "gnuVG_fontloader.hh"

//#define __DO_GNUVG_DEBUG
#include "gnuVG_debug.hh"

#define ENABLE_GNUVG_PROFILER
#include <VG/gnuVG_profiler.hh>

namespace gnuVG {

	void Font::Glyph::set_path(Path* _path,
				   VGboolean _isHinted,
				   const VGfloat _origin[2], const VGfloat _escapement[2]) {
		clear();
		Object::reference(_path);
		path = _path;
		isHinted = _isHinted;
		for(int k = 0; k < 2; ++k) {
			origin[k] = _origin[k];
			escapement[k] = _escapement[k];
		}
	}

	void Font::Glyph::set_image(Image* _image,
				    const VGfloat _origin[2], const VGfloat _escapement[2]) {
		clear();
		Object::reference(_image);
		image = _image;

		for(int k = 0; k < 2; ++k) {
			origin[k] = _origin[k];
			escapement[k] = _escapement[k];
		}
	}

	void Font::Glyph::clear() {
		if(path != VG_INVALID_HANDLE)
			Object::dereference(path);
		if(image != VG_INVALID_HANDLE)
			Object::dereference(image);
		path = VG_INVALID_HANDLE;
		image = VG_INVALID_HANDLE;
		origin[0] = 0.0f;
		origin[1] = 0.0f;
		escapement[0] = 0.0f;
		escapement[1] = 0.0f;
	}

	Font::Glyph::Glyph() {
		clear();
	}

	Font::Glyph::~Glyph() {
		clear();
	}

	void Font::secure_max_glyphIndex(VGuint glyphIndex) {
		if(glyphIndex >= glyphs.size()) {
			int max_k = glyphIndex - glyphs.size() + 1;
			for(int k = 0; k < max_k; ++k)
				glyphs.push_back(new Glyph());
		}
	}

	Font::Font(VGint glyphCapacityHint) {}
	Font::~Font() {
		if(freetype_face_set)
			FT_Done_Face(freetype_face);
		for(auto glyph : glyphs)
			delete glyph;
	}

	/* gnuVG extensions API */
	bool Font::gnuVG_load_font(const char* family, gnuVGFontStyle fontStyle) {
		std::string path = "/system/fonts/";
		path += family;
		path += "-";
		switch(fontStyle) {
		case gnuVG_NORMAL:
			path += "Regular.ttf";
			break;
		case gnuVG_ITALIC:
			path += "Italic.ttf";
			break;
		case gnuVG_BOLD:
			path += "Bold.ttf";
			break;
		case gnuVG_BOLD_ITALIC:
			path += "BoldItalic.ttf";
			break;
		}

		if(fontLoader_load_font((VGFont)this, &freetype_face, path.c_str(), VG_FALSE) != (VGFont)this)
			return false;
		freetype_face_set = true;
		return true;
	}

	// XXX this only handles ISO-8859-1 currently
	void Font::gnuVG_render_text(
		VGfloat size, gnuVGTextAnchor anchor,
		const char* utf8, VGfloat x_anchor, VGfloat y_anchor) {

		if(!freetype_face_set) return;

		auto num_chars = strlen(utf8);
		adjustments_x.clear();
		glyph_indices.clear();
		VGfloat conversion_factor = size / GNUVG_FONT_PIXELSIZE;
		VGfloat text_width = 0.0f;
		/* calculate adjustment vector - kerning */
		bool has_kerning = FT_HAS_KERNING(freetype_face);
		FT_UInt previous = 0;

		for(size_t n = 0; n < num_chars; ++n) {
			auto c = utf8[n];
			/* convert character code to glyph index */
			auto glyph_index = FT_Get_Char_Index(freetype_face, c);

			auto glyph = glyphs[c];
			if(glyph) {
				ADD_GNUVG_PROFILER_PROBE(process_glyph);
				glyph_indices.push_back(c);
				/* retrieve kerning distance and move pen position */
				if(has_kerning && previous && glyph_index) {
					FT_Vector  delta;
					FT_Get_Kerning(freetype_face, previous, glyph_index,
						       FT_KERNING_DEFAULT, &delta);
					adjustments_x.push_back((VGfloat)(delta.x >> 6));
					text_width += adjustments_x[n] * conversion_factor;
				}

				text_width += glyph->escapement[0] * conversion_factor;

				previous = glyph_index;
			}
		}

		switch(anchor) {
		case gnuVG_ANCHOR_START:
			/* do nothing */;
			break;
		case gnuVG_ANCHOR_MIDDLE:
			x_anchor -= text_width / 2.0f;
			break;
		case gnuVG_ANCHOR_END:
			x_anchor -= text_width;
			break;
		}

		VGfloat gorigin[] = {0.0, 0.0};
		vgSetfv(VG_GLYPH_ORIGIN, 2, gorigin);

		VGfloat mtrx[9];

		VGint old_matrixmode;
		old_matrixmode = vgGeti(VG_MATRIX_MODE);
		vgSeti(VG_MATRIX_MODE, VG_MATRIX_GLYPH_USER_TO_SURFACE);

		vgGetMatrix(mtrx);
		vgTranslate(x_anchor, y_anchor);
		vgScale(conversion_factor, -conversion_factor);

		VGfloat* adj_x = adjustments_x.size() == 0 ? nullptr : adjustments_x.data();

		vgDrawGlyphs(num_chars,
			     glyph_indices.data(),
			     adj_x,
			     nullptr,
			     VG_FILL_PATH,
			     VG_FALSE);

		vgLoadMatrix(mtrx);

		vgSeti(VG_MATRIX_MODE, old_matrixmode);
	}

	/* inherited virtual interface */
	void Font::vgSetParameterf(VGint paramType, VGfloat value) {
		Context::get_current()->set_error(VG_ILLEGAL_ARGUMENT_ERROR);
	}

	void Font::vgSetParameteri(VGint paramType, VGint value) {
		Context::get_current()->set_error(VG_ILLEGAL_ARGUMENT_ERROR);
	}

	void Font::vgSetParameterfv(VGint paramType, VGint count, const VGfloat *values) {
		Context::get_current()->set_error(VG_ILLEGAL_ARGUMENT_ERROR);
	}

	void Font::vgSetParameteriv(VGint paramType, VGint count, const VGint *values) {
		Context::get_current()->set_error(VG_ILLEGAL_ARGUMENT_ERROR);
	}

	VGfloat Font::vgGetParameterf(VGint paramType) {
		Context::get_current()->set_error(VG_ILLEGAL_ARGUMENT_ERROR);
		return 0.0f;
	}

	VGint Font::vgGetParameteri(VGint paramType) {
		if(paramType == VG_FONT_NUM_GLYPHS)
			return (VGint)glyphs.size();

		Context::get_current()->set_error(VG_ILLEGAL_ARGUMENT_ERROR);
		return 0;
	}


	VGint Font::vgGetParameterVectorSize(VGint paramType) {
		if(paramType == VG_FONT_NUM_GLYPHS)
			return 1;
		Context::get_current()->set_error(VG_ILLEGAL_ARGUMENT_ERROR);
		return 0;
	}


	void Font::vgGetParameterfv(VGint paramType, VGint count, VGfloat *values) {
		if(count == 1)
			values[0] = vgGetParameterf(paramType);
		else
			Context::get_current()->set_error(VG_ILLEGAL_ARGUMENT_ERROR);
	}

	void Font::vgGetParameteriv(VGint paramType, VGint count, VGint *values) {
		if(count == 1)
			values[0] = vgGetParameteri(paramType);
		else
			Context::get_current()->set_error(VG_ILLEGAL_ARGUMENT_ERROR);
	}


	void Font::vgSetGlyphToPath(VGuint glyphIndex,
				    Path* path,
				    VGboolean isHinted,
				    const VGfloat glyphOrigin[2],
				    const VGfloat escapement[2]) {
		secure_max_glyphIndex(glyphIndex);
		glyphs[glyphIndex]->set_path(path, isHinted, glyphOrigin, escapement);
	}

	void Font::vgSetGlyphToImage(VGuint glyphIndex,
				     Image* image,
				     const VGfloat glyphOrigin[2],
				     const VGfloat escapement[2]) {
		secure_max_glyphIndex(glyphIndex);
		glyphs[glyphIndex]->set_image(image, glyphOrigin, escapement);
	}

	void Font::vgClearGlyph(VGuint glyphIndex) {
		if(glyphIndex >= glyphs.size())
			return;
		glyphs[glyphIndex]->clear();
	}

	void Font::vgDrawGlyph(VGuint glyphIndex,
			       VGbitfield paintModes,
			       VGboolean allowAutoHinting) {
		if(!(paintModes & (VG_FILL_PATH | VG_STROKE_PATH)))
			return;
		if(glyphIndex >= glyphs.size())
			return;
		auto ctx = Context::get_current();
		if(ctx == nullptr)
			return;

		ctx->select_conversion_matrix(Context::GNUVG_MATRIX_GLYPH_USER_TO_SURFACE);

		auto glyph = glyphs.data()[glyphIndex];
		Context::get_current()->use_glyph_origin_as_pre_translation(glyph->origin);
		if(glyph->path != VG_INVALID_HANDLE)
			glyph->path->vgDrawPath(paintModes);
		Context::get_current()->adjust_glyph_origin(glyph->escapement);
	}

	void Font::vgDrawGlyphs(VGint glyphCount,
				const VGuint *glyphIndices,
				const VGfloat *adjustments_x,
				const VGfloat *adjustments_y,
				VGbitfield paintModes,
				VGboolean allowAutoHinting) {
		if(!(paintModes & (VG_FILL_PATH | VG_STROKE_PATH)))
			return;
		auto ctx = Context::get_current();
		if(ctx == nullptr)
			return;

		ctx->select_conversion_matrix(Context::GNUVG_MATRIX_GLYPH_USER_TO_SURFACE);

		auto gly_p = glyphs.data();
		for(auto k = 0; k < glyphCount; ++k) {
			ADD_GNUVG_PROFILER_PROBE(render_glyph);
			auto glyphIndex = glyphIndices[k];
			if(glyphIndex < glyphs.size()) {
				auto glyph = gly_p[glyphIndex];

				Context::get_current()->use_glyph_origin_as_pre_translation(glyph->origin);

				if(glyph->path != VG_INVALID_HANDLE) {
					ADD_GNUVG_PROFILER_PROBE(glyph_drawPath);
					glyph->path->vgDrawPath(paintModes);
				}

				VGfloat adjustment[] = {
					glyph->escapement[0],
					glyph->escapement[1]
				};
				if(adjustments_x) adjustment[0] += adjustments_x[k];
				if(adjustments_y) adjustment[1] += adjustments_y[k];

				Context::get_current()->adjust_glyph_origin(adjustment);
			}
		}
	}
};

using namespace gnuVG;

inline static void prepare_fontloader() {
	static bool fontloader_prepared = false;
	if(!fontloader_prepared) {
		fontloader_prepared = true;
		fontLoader_setup();
	}
}

extern "C" {
	/*********************
	 *
	 *    gnuVG extensions API
	 *
	 *********************/

	VGFont VG_API_ENTRY gnuvgLoadFont(const char* family,
					  gnuVGFontStyle fontStyle) {
		VGFont font = vgCreateFont(256);
		if(font != VG_INVALID_HANDLE) {
			Font* f = (Font*)font;
			if(!(f->gnuVG_load_font(family, fontStyle))) {
				delete f;
				font = VG_INVALID_HANDLE;
				Context::get_current()->set_error(VG_ILLEGAL_ARGUMENT_ERROR);
			}
		} else {
			Context::get_current()->set_error(VG_BAD_HANDLE_ERROR);
		}
		return font;
	}

	void VG_API_ENTRY gnuvgRenderText(
		VGFont font, VGfloat size, gnuVGTextAnchor anchor,
		const char* utf8, VGfloat x_anchor, VGfloat y_anchor) {
		if(font != VG_INVALID_HANDLE) {
			Font* f = (Font*)font;
			f->gnuVG_render_text(size, anchor, utf8, x_anchor, y_anchor);
		} else {
			Context::get_current()->set_error(VG_BAD_HANDLE_ERROR);
		}
	}

	/*********************
	 *
	 *    regular OpenVG API
	 *
	 *********************/

	VGFont VG_API_ENTRY vgCreateFont(VGint glyphCapacityHint) VG_API_EXIT {
		prepare_fontloader();

		Font* font = new Font(glyphCapacityHint);
		return (VGFont)font;
	}

	void VG_API_ENTRY vgDestroyFont(VGFont font) VG_API_EXIT {
		if(font != VG_INVALID_HANDLE) {
			Font* f = (Font*)font;
			Object::dereference(f);
		} else {
			Context::get_current()->set_error(VG_BAD_HANDLE_ERROR);
		}
	}

	void VG_API_ENTRY vgSetGlyphToPath(VGFont font,
					   VGuint glyphIndex,
					   VGPath path,
					   VGboolean isHinted,
					   const VGfloat glyphOrigin [2],
					   const VGfloat escapement[2]) VG_API_EXIT {
		if(font != VG_INVALID_HANDLE && path != VG_INVALID_HANDLE) {
			Font* f = (Font*)font;
			Path* p = (Path*)path;
			f->vgSetGlyphToPath(glyphIndex, p, isHinted, glyphOrigin, escapement);
		} else {
			Context::get_current()->set_error(VG_BAD_HANDLE_ERROR);
		}
	}

	void VG_API_ENTRY vgSetGlyphToImage(VGFont font,
					    VGuint glyphIndex,
					    VGImage image,
					    const VGfloat glyphOrigin [2],
					    const VGfloat escapement[2]) VG_API_EXIT {
		if(font != VG_INVALID_HANDLE && image != VG_INVALID_HANDLE) {
			Font* f = (Font*)font;
			Image* i = (Image*)image;
			f->vgSetGlyphToImage(glyphIndex, i, glyphOrigin, escapement);
		} else {
			Context::get_current()->set_error(VG_BAD_HANDLE_ERROR);
		}
	}

	void VG_API_ENTRY vgClearGlyph(VGFont font,VGuint glyphIndex) VG_API_EXIT {
		if(font != VG_INVALID_HANDLE) {
			Font* f = (Font*)font;
			f->vgClearGlyph(glyphIndex);
		} else {
			Context::get_current()->set_error(VG_BAD_HANDLE_ERROR);
		}
	}

	void VG_API_ENTRY vgDrawGlyph(VGFont font,
				      VGuint glyphIndex,
				      VGbitfield paintModes,
				      VGboolean allowAutoHinting) VG_API_EXIT {
		if(font != VG_INVALID_HANDLE) {
			Font* f = (Font*)font;
			f->vgDrawGlyph(glyphIndex, paintModes, allowAutoHinting);
		} else {
			Context::get_current()->set_error(VG_BAD_HANDLE_ERROR);
		}
	}

	void VG_API_ENTRY vgDrawGlyphs(VGFont font,
				       VGint glyphCount,
				       const VGuint *glyphIndices,
				       const VGfloat *adjustments_x,
				       const VGfloat *adjustments_y,
				       VGbitfield paintModes,
				       VGboolean allowAutoHinting) VG_API_EXIT {
		if(font != VG_INVALID_HANDLE) {
			Font* f = (Font*)font;
			f->vgDrawGlyphs(glyphCount, glyphIndices,
					adjustments_x, adjustments_y,
					paintModes, allowAutoHinting);
		} else {
			Context::get_current()->set_error(VG_BAD_HANDLE_ERROR);
		}
	}
}
