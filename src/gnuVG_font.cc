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
#include <VG/vgu.h>

namespace gnuVG {

	class FailedToCreateFontCacheException {};

	FontCache::FontCache(int w, int h) : SkylineBinPack(w, h, true) {
		auto ctx = Context::get_current();
		if(!ctx->create_framebuffer(&framebuffer, VG_sRGBA_8888, w, h, VG_IMAGE_QUALITY_BETTER))
			throw FailedToCreateFontCacheException();

		ctx->save_current_framebuffer();
		ctx->render_to_framebuffer(&framebuffer);
		ctx->trivial_fill_area(0, 0, w, h, 0.0, 0.0, 0.0, 0.0);
		ctx->restore_current_framebuffer();

		GLfloat w_f = (float)w;
		GLfloat h_f= (float)h;
		tex_mtrx[0] = 1.0 / w_f; tex_mtrx[3] = 0.0      ; tex_mtrx[6] = 0.0;
		tex_mtrx[1] = 0.0;       tex_mtrx[4] = 1.0 / h_f; tex_mtrx[7] = 0.0;
		tex_mtrx[2] = 0.0;       tex_mtrx[5] = 0.0;       tex_mtrx[8] = 1.0;
	}

	void Font::Glyph::set_path(std::shared_ptr<Path> _path,
				   VGboolean _isHinted,
				   const VGfloat _origin[2], const VGfloat _escapement[2]) {
		clear();
		path = _path;
		isHinted = _isHinted;
		for(int k = 0; k < 2; ++k) {
			origin[k] = _origin[k];
			escapement[k] = _escapement[k];
		}
	}

	void Font::Glyph::set_image(std::shared_ptr<Image> _image,
				    const VGfloat _origin[2], const VGfloat _escapement[2]) {
		clear();
		image = _image;

		for(int k = 0; k < 2; ++k) {
			origin[k] = _origin[k];
			escapement[k] = _escapement[k];
		}
	}

	void Font::Glyph::clear() {
		path.reset();
		image.reset();
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
		std::string path = "/home/pltxtra/Android/Sdk/platforms/android-27/data/fonts/"; //"/system/fonts/";
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

		if(get_handle() !=
		   fontLoader_load_font(get_handle(), &freetype_face, path.c_str(), VG_FALSE)) {
			printf("dang ... couldn't load %s\n", path.c_str());
			return false;
		}
			printf("Loaded %s\n", path.c_str());
		freetype_face_set = true;
		return true;
	}

	static inline void p_mul(VGfloat pp[], VGfloat mtrx[], VGfloat p[]) {
		pp[0] = mtrx[0] * p[0] + mtrx[3] * p[1] + mtrx[6] * p[2];
		pp[1] = mtrx[1] * p[0] + mtrx[4] * p[1] + mtrx[7] * p[2];
		pp[2] = mtrx[2] * p[0] + mtrx[5] * p[1] + mtrx[8] * p[2];
	}

	int Font::get_fc_scale() {
		VGfloat mtrx[9];
		vgGetMatrix(mtrx);

		mtrx[6] = mtrx[7] = 0.0; // skip translation

		VGfloat p_1[] = {0.0, 0.0, 1.0};
		VGfloat p_2[] = {0.0, 1.0, 1.0};
		VGfloat pp2[] = {0.0, 0.0, 0.0};
		VGfloat pp1[] = {0.0, 0.0, 0.0};
		p_mul(pp1, mtrx, p_1);
		p_mul(pp2, mtrx, p_2);

		VGfloat pp[] = {
			pp2[0] - pp1[0],
			pp2[1] - pp1[1],
			pp2[2] - pp1[2]
		};
		VGfloat l = sqrtf(pp[0] * pp[0] + pp[1] * pp[1]);

		l = 1.0 / l;
		return (int)l;
	}

	FontCache* Font::get_font_cache(int fc_scale) {
		auto fc_p = font_caches.find(fc_scale);
		if(fc_p != font_caches.end()) {
			return (*fc_p).second;
		}

		GNUVG_DEBUG("Created font cache for fc_scale %d\n", fc_scale);
		auto fc = new FontCache(512, 512);
		font_caches[fc_scale] = fc;
		return fc;
	}

	void Font::prefill_cache(int fc_scale, const std::vector<VGuint> &glyph_indices) {
		auto fc = get_font_cache(fc_scale);

		float scale = 1.0 / ((float)fc_scale);

		VGfloat min_x, min_y, width, height;

		auto ctx = Context::get_current();
		if(ctx == nullptr)
			return;

		auto old_clr_transform = vgGeti(VG_COLOR_TRANSFORM);
		VGfloat old_clr_transform_values[8];
		vgGetfv(VG_COLOR_TRANSFORM_VALUES, 8, old_clr_transform_values);
		vgSeti(VG_COLOR_TRANSFORM, VG_FALSE);

		auto old_mtrx_mode = vgGeti(VG_MATRIX_MODE);
		vgSeti(VG_MATRIX_MODE, VG_MATRIX_GLYPH_USER_TO_SURFACE);

		// remember old matrix
		VGfloat mtrx[9];
		vgGetMatrix(mtrx);

		static VGPath temp_path = VG_INVALID_HANDLE;
		if(temp_path == VG_INVALID_HANDLE) {
			temp_path = vgCreatePath(VG_PATH_FORMAT_STANDARD, VG_PATH_DATATYPE_F,
						 1,0,0,0, VG_PATH_CAPABILITY_ALL);
		}
		auto tepa = Object::get<Path>(temp_path);

		ctx->save_current_framebuffer();
		ctx->render_to_framebuffer(&(fc->framebuffer));

		if(!(cache_render_paint)) {
			cache_render_paint = Object::create<Paint>();
			cache_render_paint->vgSetColor(0xffffffff); // RGBA
		}
		auto old_stroke_paint = vgGetPaint(VG_STROKE_PATH);
		auto old_fill_paint = vgGetPaint(VG_FILL_PATH);

		vgSetPaint(cache_render_paint->get_handle(), VG_FILL_PATH);

		for(auto gi : glyph_indices) {
			FontCache::Rect result_ignored;
			if(!fc->lookup(gi, result_ignored)) {
				auto g = glyphs[gi];

				vgLoadIdentity();
				vgScale(scale, scale);
				vgTranslate(g->origin[0], g->origin[1]);
				ctx->select_conversion_matrix(Context::GNUVG_MATRIX_GLYPH_USER_TO_SURFACE);

				g->path->vgPathTransformedBounds(&min_x, &min_y,
								 &width, &height);

				auto r = fc->pack(gi, g->origin[0], g->origin[1], width + 4.0, height + 4.0);

				if(r.width == 0) {
					GNUVG_ERROR("Couldn't fit to cache. (%f, %f, %f, %f)\n", g->origin[0], g->origin[1], width + 4.0, height + 4.0);
					continue; // couldn't fit
				}

				vgLoadIdentity();
				vgTranslate(r.x, r.y);

				if(r.rotated) {
					vgTranslate(0.0, r.height);
					vgRotate(-90.0);
				}
				vgTranslate(2.0 - min_x, 2.0 - min_y);

				vgScale(scale, scale);
				vgTranslate(g->origin[0], g->origin[1]);

				ctx->select_conversion_matrix(Context::GNUVG_MATRIX_GLYPH_USER_TO_SURFACE);
				g->path->vgDrawPath(VG_FILL_PATH);
			}
		}
		vgSetPaint(old_stroke_paint, VG_STROKE_PATH);
		vgSetPaint(old_fill_paint, VG_FILL_PATH);

		ctx->restore_current_framebuffer();

		vgLoadMatrix(mtrx);

		vgSetfv(VG_COLOR_TRANSFORM_VALUES, 8, old_clr_transform_values);
		vgSeti(VG_COLOR_TRANSFORM, old_clr_transform);

		// restore matrix mode
		vgSeti(VG_MATRIX_MODE, old_mtrx_mode);
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

		VGfloat mtrx[9], mtrx2[9];

		VGint old_matrixmode;
		old_matrixmode = vgGeti(VG_MATRIX_MODE);
		vgSeti(VG_MATRIX_MODE, VG_MATRIX_GLYPH_USER_TO_SURFACE);

		vgGetMatrix(mtrx);
		vgTranslate(x_anchor, y_anchor);
		vgScale(conversion_factor, conversion_factor);

		auto fc_scale = get_fc_scale();

		{
			VGint scissoring = vgGeti(VG_SCISSORING);
			vgSeti(VG_SCISSORING, VG_FALSE);
			prefill_cache(fc_scale, glyph_indices);
			vgSeti(VG_SCISSORING, scissoring);
		}

		VGfloat* adj_x = adjustments_x.size() == 0 ? nullptr : adjustments_x.data();
		vgGetMatrix(mtrx2);
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
				    std::shared_ptr<Path> path,
				    VGboolean isHinted,
				    const VGfloat glyphOrigin[2],
				    const VGfloat escapement[2]) {
		secure_max_glyphIndex(glyphIndex);
		glyphs[glyphIndex]->set_path(path, isHinted, glyphOrigin, escapement);
	}

	void Font::vgSetGlyphToImage(VGuint glyphIndex,
				     std::shared_ptr<Image> image,
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
		if(glyph->path != VG_INVALID_HANDLE) {
			glyph->path->vgDrawPath(paintModes);
		}
		Context::get_current()->adjust_glyph_origin(glyph->escapement);
	}

	static VGfloat cord_cache_buffer[] = {0.0, 0.0};
	static std::vector<VGfloat> vrtc_cache_buffer; // vertices
	static std::vector<VGfloat> txtc_cache_buffer; // texture coordinates
	static std::vector<VGuint>  indc_cache_buffer; // indices

	static void reset_cache_buffer() {
		cord_cache_buffer[0] = 0.0;
		cord_cache_buffer[1] = 0.0;
		vrtc_cache_buffer.clear();
		txtc_cache_buffer.clear();
		indc_cache_buffer.clear();
	}

	inline static void push_coords(std::vector<GLfloat> &a, GLfloat x, GLfloat y) {
		a.push_back(x); a.push_back(y);
	}

	static void push_to_cache_buffer(float fc_scale, const FontCache::Rect &data) {
		int i = vrtc_cache_buffer.size() / 2;

		const auto x = cord_cache_buffer[0] - fc_scale * data.offset_x;
		const auto y = cord_cache_buffer[1] - fc_scale * data.offset_y;
		const auto w = fc_scale * data.width;
		const auto h = fc_scale * data.height;
		const auto w_t = data.width;
		const auto h_t = data.height;
		const auto x_t = data.x;
		const auto y_t = data.y;

		if(data.rotated) {
			// push vertice coords
			push_coords(vrtc_cache_buffer, x    , y);
			push_coords(vrtc_cache_buffer, x    , y + w);
			push_coords(vrtc_cache_buffer, x + h, y + w);
			push_coords(vrtc_cache_buffer, x + h, y);

			// push texture coords
			push_coords(txtc_cache_buffer, x_t      , y_t + h_t);
			push_coords(txtc_cache_buffer, x_t + w_t, y_t + h_t);
			push_coords(txtc_cache_buffer, x_t + w_t, y_t);
			push_coords(txtc_cache_buffer, x_t      , y_t);
		} else {
			// push vertice coords
			push_coords(vrtc_cache_buffer, x    , y);
			push_coords(vrtc_cache_buffer, x    , y + h);
			push_coords(vrtc_cache_buffer, x + w, y + h);
			push_coords(vrtc_cache_buffer, x + w, y);

			// push texture coords
			push_coords(txtc_cache_buffer, x_t      , y_t);
			push_coords(txtc_cache_buffer, x_t      , y_t + h_t);
			push_coords(txtc_cache_buffer, x_t + w_t, y_t + h_t);
			push_coords(txtc_cache_buffer, x_t + w_t, y_t);
		}

		// push indices
		indc_cache_buffer.push_back(i);
		indc_cache_buffer.push_back(i + 1);
		indc_cache_buffer.push_back(i + 2);

		indc_cache_buffer.push_back(i);
		indc_cache_buffer.push_back(i + 2);
		indc_cache_buffer.push_back(i + 3);
	}

	static void render_cache_buffer(Context *ctx, const FontCache *fc) {
		if(indc_cache_buffer.size() == 0) return;

		ctx->render_texture_alpha_triangle_array(
			&(fc->framebuffer),
			vrtc_cache_buffer.data(), 0,
			txtc_cache_buffer.data(), 0,
			indc_cache_buffer.data(), indc_cache_buffer.size(),
			fc->get_texture_matrix()
			);
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

		auto fc_scale = get_fc_scale();
		auto fc = get_font_cache(fc_scale);

		ctx->select_conversion_matrix(Context::GNUVG_MATRIX_GLYPH_USER_TO_SURFACE);

		reset_cache_buffer();

		auto gly_p = glyphs.data();
		for(auto k = 0; k < glyphCount; ++k) {
			ADD_GNUVG_PROFILER_PROBE(render_glyph);
			auto glyphIndex = glyphIndices[k];

			FontCache::Rect cached_result;
			if(glyphIndex < glyphs.size()) {
				auto glyph = gly_p[glyphIndex];

				VGfloat adjustment[] = {
					glyph->escapement[0],
					glyph->escapement[1]
				};
				if(adjustments_x) adjustment[0] += adjustments_x[k];
				if(adjustments_y) adjustment[1] += adjustments_y[k];

				if(fc->lookup(glyphIndex, cached_result)) {
					push_to_cache_buffer(fc_scale, cached_result);
					for(auto k = 0; k < 2; ++k) {
						cord_cache_buffer[k] += adjustment[k];
					}
				} else {
					Context::get_current()->use_glyph_origin_as_pre_translation(glyph->origin);

					if(glyph->path != VG_INVALID_HANDLE) {
						ADD_GNUVG_PROFILER_PROBE(glyph_drawPath);
						glyph->path->vgDrawPath(paintModes);
					}
				}


				Context::get_current()->adjust_glyph_origin(adjustment);
			}
		}

		render_cache_buffer(ctx, fc);
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
			auto f = Object::get<Font>(font);
			if(!(f->gnuVG_load_font(family, fontStyle))) {
				Object::dereference(font);
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
		auto f = Object::get<Font>(font);
		if(f)
			f->gnuVG_render_text(size, anchor, utf8, x_anchor, y_anchor);
	}

	/*********************
	 *
	 *    regular OpenVG API
	 *
	 *********************/

	VGFont VG_API_ENTRY vgCreateFont(VGint glyphCapacityHint) VG_API_EXIT {
		prepare_fontloader();

		auto font = Object::create<Font>(glyphCapacityHint);

		if(font)
			return (VGFont)font->get_handle();

		return (VGFont)VG_INVALID_HANDLE;
	}

	void VG_API_ENTRY vgDestroyFont(VGFont font) VG_API_EXIT {
		Object::dereference(font);
	}

	void VG_API_ENTRY vgSetGlyphToPath(VGFont font,
					   VGuint glyphIndex,
					   VGPath path,
					   VGboolean isHinted,
					   const VGfloat glyphOrigin [2],
					   const VGfloat escapement[2]) VG_API_EXIT {
		auto f = Object::get<Font>(font);
		auto p = Object::get<Path>(path);
		if(f && p)
			f->vgSetGlyphToPath(glyphIndex, p, isHinted, glyphOrigin, escapement);
	}

	void VG_API_ENTRY vgSetGlyphToImage(VGFont font,
					    VGuint glyphIndex,
					    VGImage image,
					    const VGfloat glyphOrigin [2],
					    const VGfloat escapement[2]) VG_API_EXIT {
		auto f = Object::get<Font>(font);
		auto i = Object::get<Image>(image);
		if(f && i)
			f->vgSetGlyphToImage(glyphIndex, i, glyphOrigin, escapement);
	}

	void VG_API_ENTRY vgClearGlyph(VGFont font,VGuint glyphIndex) VG_API_EXIT {
		auto f = Object::get<Font>(font);
		if(f)
			f->vgClearGlyph(glyphIndex);
	}

	void VG_API_ENTRY vgDrawGlyph(VGFont font,
				      VGuint glyphIndex,
				      VGbitfield paintModes,
				      VGboolean allowAutoHinting) VG_API_EXIT {
		auto f = Object::get<Font>(font);
		if(f)
			f->vgDrawGlyph(glyphIndex, paintModes, allowAutoHinting);
	}

	void VG_API_ENTRY vgDrawGlyphs(VGFont font,
				       VGint glyphCount,
				       const VGuint *glyphIndices,
				       const VGfloat *adjustments_x,
				       const VGfloat *adjustments_y,
				       VGbitfield paintModes,
				       VGboolean allowAutoHinting) VG_API_EXIT {
		auto f = Object::get<Font>(font);
		if(f)
			f->vgDrawGlyphs(glyphCount, glyphIndices,
					adjustments_x, adjustments_y,
					paintModes, allowAutoHinting);
	}
}
