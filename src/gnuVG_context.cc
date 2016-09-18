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

#include <VG/openvg.h>

#include "gnuVG_context.hh"
#include "gnuVG_config.hh"

//#define __DO_GNUVG_DEBUG
#include "gnuVG_debug.hh"

#define ENABLE_GNUVG_PROFILER
#include <VG/gnuVG_profiler.hh>

#define POSITION_ATTRIB_HANDLE 3
#define KLM_ATTRIB_HANDLE      4

namespace gnuVG {
	Context *Context::current_context = 0;

	Context::Context()
		: user_matrix(GNUVG_MATRIX_PATH_USER_TO_SURFACE)
		, conversion_matrix(GNUVG_MATRIX_PATH_USER_TO_SURFACE)
		, stroke_width(1.0f)
		, stroke_dash_phase_reset(false)
		, miter_limit(4.0f)
		, join_style(VG_JOIN_MITER)
		, current_framebuffer(&screen_buffer)
	{
		fill_paint = &default_fill_paint;
		stroke_paint = &default_stroke_paint;
	}

	Context::~Context() {}

	Context *Context::get_current() {
		return current_context;
	}

	void Context::set_current(Context *ctx) {
		current_context = ctx;
		ctx->prepare_pipeline();
	}

	void Context::set_error(VGErrorCode new_error) {
		last_error = new_error;
	}

	VGErrorCode Context::get_error() {
		auto retval = last_error;
		last_error = VG_NO_ERROR;
		return retval;
	}

	void Context::matrix_resize(VGint pxlw, VGint pxlh) {
		/* prepare screen matrix */
		VGfloat scale_w = pxlw, scale_h = pxlh;

		screen_matrix.init_identity();
		screen_matrix.translate(-1.0, -1.0);
		screen_matrix.scale(2.0, 2.0);
		screen_matrix.scale(1.0f/scale_w, 1.0f/scale_h);

		for(int k = 0; k < GNUVG_MATRIX_MAX; ++k)
			matrix_is_dirty[k] = true;
	}

	VGfloat Context::get_stroke_width() {
		return stroke_width;
	}

	VGfloat Context::get_miter_limit() {
		return miter_limit;
	}

	VGJoinStyle Context::get_join_style() {
		return join_style;
	}

	std::vector<VGfloat> Context::get_dash_pattern() {
		return dash_pattern;
	}

	VGfloat Context::get_dash_phase() {
		return stroke_dash_phase;
	}

	bool Context::get_dash_phase_reset() {
		return stroke_dash_phase_reset;
	}

	void Context::vgFlush() {
	}

	void Context::vgFinish() {
	}

	/* OpenVG equivalent API - Paint Manipulation */
	void Context::vgSetPaint(Paint *p, VGbitfield paintModes) {
		if(p == NULL) {
			if(paintModes & VG_FILL_PATH)
				fill_paint = &default_fill_paint;
			if(paintModes & VG_STROKE_PATH)
				stroke_paint = &default_stroke_paint;
		} else {
			if(paintModes & VG_FILL_PATH) {
				fill_paint = p;
			}
			if(paintModes & VG_STROKE_PATH) {
				stroke_paint = p;
			}
		}
	}

	Paint *Context::vgGetPaint(VGbitfield paintModes) {
		if(
			!(
				paintModes == VG_FILL_PATH
				||
				paintModes == VG_STROKE_PATH

				)

			){
			set_error(VG_ILLEGAL_ARGUMENT_ERROR);
			return VG_INVALID_HANDLE;
		}
		if(paintModes == VG_FILL_PATH) {
			return fill_paint == &default_fill_paint ? NULL : fill_paint;
		}
		if(paintModes == VG_STROKE_PATH) {
			return stroke_paint == &default_stroke_paint ? NULL : stroke_paint;
		}

		return VG_INVALID_HANDLE;
	}

	/* OpenVG equivalent API - Matrix Manipulation */
	void Context::vgLoadIdentity(void) {
		matrix[user_matrix].init_identity();
		matrix_is_dirty[user_matrix] = true;
	}

	void Context::vgLoadMatrix(const VGfloat * m) {
		matrix[user_matrix].a = m[0];
		matrix[user_matrix].b = m[1];
		matrix[user_matrix].c = m[2];
		matrix[user_matrix].d = m[3];
		matrix[user_matrix].e = m[4];
		matrix[user_matrix].f = m[5];
		matrix[user_matrix].g = m[6];
		matrix[user_matrix].h = m[7];
		matrix[user_matrix].i = m[8];

		matrix_is_dirty[user_matrix] = true;
	}

	void Context::vgGetMatrix(VGfloat * m) {
		m[0] = matrix[user_matrix].a;
		m[1] = matrix[user_matrix].b;
		m[2] = matrix[user_matrix].c;
		m[3] = matrix[user_matrix].d;
		m[4] = matrix[user_matrix].e;
		m[5] = matrix[user_matrix].f;
		m[6] = matrix[user_matrix].g;
		m[7] = matrix[user_matrix].h;
		m[8] = matrix[user_matrix].i;
	}

	void Context::vgMultMatrix(const VGfloat * m) {
		Matrix b(
			m[0], m[3], m[6],
			m[1], m[4], m[7],
			m[2], m[5], m[8]
			);

		Matrix result; result.multiply(matrix[user_matrix], b);

		matrix[user_matrix].set_to(result);
		matrix_is_dirty[user_matrix] = true;
	}

	void Context::vgTranslate(VGfloat tx, VGfloat ty) {
		matrix[user_matrix].translate(tx, ty);
		matrix_is_dirty[user_matrix] = true;
	}

	void Context::vgScale(VGfloat sx, VGfloat sy) {
		matrix[user_matrix].scale(sx, sy);
		matrix_is_dirty[user_matrix] = true;
	}

	void Context::vgShear(VGfloat shx, VGfloat shy) {
		matrix[user_matrix].shear(shx, shy);
		matrix_is_dirty[user_matrix] = true;
	}

	void Context::vgRotate(VGfloat angle_in_degrees) {
		matrix[user_matrix].rotate(M_PI * angle_in_degrees * (1.0f / 180.0f));
		matrix_is_dirty[user_matrix] = true;
	}

	void Context::reset_bounding_box() {
		bounding_box_was_reset = true;
	}

	void Context::get_bounding_box(VGfloat *sp_ep) {
		// we must convert the bbox to screen coordinates
		auto w = current_framebuffer->width;
		auto h = current_framebuffer->height;
		sp_ep[0] = w * (0.5f + bounding_box[0].x / 2.0f);
		sp_ep[1] = h * (0.5f - bounding_box[1].y / 2.0f);
		sp_ep[2] = w * (0.5f + bounding_box[1].x / 2.0f);
		sp_ep[3] = h * (0.5f - bounding_box[0].y / 2.0f);
	}

	Point Context::map_point(const Point &p) {
		return matrix[user_matrix].map_point(p);
	}

	/* Inherited Object API */
	void Context::vgSetf(VGint paramType, VGfloat value) {
		switch(paramType) {
			/* For the following paramType values this call is illegal */
		case VG_STROKE_DASH_PATTERN:
		case VG_STROKE_DASH_PHASE_RESET:
		case VG_MATRIX_MODE:
		case VG_CLEAR_COLOR:
		case VG_STROKE_CAP_STYLE:
		case VG_STROKE_JOIN_STYLE:
			set_error(VG_ILLEGAL_ARGUMENT_ERROR);
			break;
			/* Mode settings */
		case VG_FILL_RULE:
		case VG_IMAGE_QUALITY:
		case VG_RENDERING_QUALITY:
		case VG_BLEND_MODE:
		case VG_IMAGE_MODE:
			break;

			/* Color Transformation */
		case VG_COLOR_TRANSFORM:
		case VG_COLOR_TRANSFORM_VALUES:
			break;

			/* Stroke parameters */
		case VG_STROKE_LINE_WIDTH:
			stroke_width = value;
			break;
		case VG_STROKE_DASH_PHASE:
			stroke_dash_phase = value;
			break;

		case VG_STROKE_MITER_LIMIT:
			miter_limit = value;
			if(miter_limit < 1.0f) miter_limit = 1.0f; // silently clamp small/negative values to 1.0f
			break;

			/* Edge fill color for VG_TILE_FILL tiling mode */
		case VG_TILE_FILL_COLOR:
			break;


			/* Glyph origin */
		case VG_GLYPH_ORIGIN:
			break;

			/* Pixel layout information */
		case VG_PIXEL_LAYOUT:
		case VG_SCREEN_LAYOUT:
			break;

			/* Source format selection for image filters */
		case VG_FILTER_FORMAT_LINEAR:
		case VG_FILTER_FORMAT_PREMULTIPLIED:
			break;

			/* Destination write enable mask for image filters */
		case VG_FILTER_CHANNEL_MASK:
			break;

		default:
			set_error(VG_ILLEGAL_ARGUMENT_ERROR);
			break;
		}
	}

	void Context::vgSeti(VGint paramType, VGint value) {
		switch(paramType) {
			/* For the following paramType values this call is illegal */
		default:
		case VG_STROKE_DASH_PATTERN:
		case VG_STROKE_DASH_PHASE:
		case VG_STROKE_LINE_WIDTH:
		case VG_CLEAR_COLOR:
		case VG_STROKE_MITER_LIMIT:
			set_error(VG_ILLEGAL_ARGUMENT_ERROR);
			break;
			/* Mode settings */
		case VG_MATRIX_MODE:
		{
			switch((VGMatrixMode)value) {
			case VG_MATRIX_PATH_USER_TO_SURFACE:
				user_matrix = GNUVG_MATRIX_PATH_USER_TO_SURFACE;
				break;
			case VG_MATRIX_IMAGE_USER_TO_SURFACE:
				user_matrix = GNUVG_MATRIX_IMAGE_USER_TO_SURFACE;
				break;
			case VG_MATRIX_FILL_PAINT_TO_USER:
				user_matrix = GNUVG_MATRIX_FILL_PAINT_TO_USER;
				break;
			case VG_MATRIX_STROKE_PAINT_TO_USER:
				user_matrix = GNUVG_MATRIX_STROKE_PAINT_TO_USER;
				break;
			case VG_MATRIX_GLYPH_USER_TO_SURFACE:
				user_matrix = GNUVG_MATRIX_GLYPH_USER_TO_SURFACE;
				break;
			default:
				set_error(VG_ILLEGAL_ARGUMENT_ERROR);
				break;
			}
		}

		case VG_FILL_RULE:
		case VG_IMAGE_QUALITY:
		case VG_RENDERING_QUALITY:
		case VG_BLEND_MODE:
		case VG_IMAGE_MODE:
			break;

			/* Color Transformation */
		case VG_COLOR_TRANSFORM:
		case VG_COLOR_TRANSFORM_VALUES:
			break;

			/* Stroke parameters */
		case VG_STROKE_CAP_STYLE:
			break;
		case VG_STROKE_JOIN_STYLE:
			switch((VGJoinStyle)value) {
			case VG_JOIN_MITER:
				join_style = VG_JOIN_MITER;
				break;
			case VG_JOIN_ROUND:
				join_style = VG_JOIN_ROUND;
				break;
			case VG_JOIN_BEVEL:
				join_style = VG_JOIN_BEVEL;
				break;
			default:
				set_error(VG_ILLEGAL_ARGUMENT_ERROR);
				break;
			}
			break;
		case VG_STROKE_DASH_PHASE_RESET:
			stroke_dash_phase_reset = (((VGboolean)value) == VG_TRUE) ? true : false;
			break;

			/* Edge fill color for VG_TILE_FILL tiling mode */
		case VG_TILE_FILL_COLOR:
			break;


			/* Glyph origin */
		case VG_GLYPH_ORIGIN:
			break;

			/* Enable/disable alpha masking and scissoring */
		case VG_MASKING:
			mask_is_active = (((VGboolean)value) == VG_TRUE) ? true : false;
			break;

		case VG_SCISSORING:
			scissors_are_active = (((VGboolean)value) == VG_TRUE) ? true : false;
			break;

			/* Pixel layout information */
		case VG_PIXEL_LAYOUT:
		case VG_SCREEN_LAYOUT:
			break;

			/* Source format selection for image filters */
		case VG_FILTER_FORMAT_LINEAR:
		case VG_FILTER_FORMAT_PREMULTIPLIED:
			break;

			/* Destination write enable mask for image filters */
		case VG_FILTER_CHANNEL_MASK:
			break;
		}
	}

	void Context::vgSetfv(VGint paramType, VGint count, const VGfloat *values) {
		switch(paramType) {
		case VG_STROKE_DASH_PATTERN:
		{
			VGfloat total_length = 0.0f;
			dash_pattern.clear();
			for(int k = 0; k < count; k++) {
				dash_pattern.push_back(values[k]);
				total_length += values[k];
			}
			if(total_length <= 0.0f) // can't have a dash pattern of length zero
				dash_pattern.clear();
		}
		break;
		case VG_CLEAR_COLOR: /* Color for vgClear */
			if(count == 4) {
				for(int k = 0; k < 4; k++) {
					clear_color.c[k] = values[k];
				}
			} else {
				set_error(VG_ILLEGAL_ARGUMENT_ERROR);
			}
			break;
		case VG_GLYPH_ORIGIN:
			if(count == 2)
				for(int k = 0; k < 2; ++k)
					glyph_origin[k] = values[k];
			break;
		default:
			set_error(VG_ILLEGAL_ARGUMENT_ERROR);
		}
	}

	void Context::vgSetiv(VGint paramType, VGint count, const VGint *values) {
		switch(paramType) {
			/* Scissoring rectangles */
		case VG_SCISSOR_RECTS:
		{
			count = count > (GNUVG_MAX_SCISSORS * 4) ? (GNUVG_MAX_SCISSORS * 4) : count;

			GLsizei l = 0, max_k = count / 4;
			for(int k = 0; k < max_k; k++) {
				int soffset = k * 4;

				GLfloat x = (GLfloat)(values[soffset + 0]);
				GLfloat y = (GLfloat)(values[soffset + 1]);
				GLfloat w = (GLfloat)(values[soffset + 2]);
				GLfloat h = (GLfloat)(values[soffset + 3]);

				// we ignore zero size rectangles
				if(w != 0.0f && h != 0.0f) {
					int vcoffset = (l * 4) << 1;
					int toffset = l * 3 * 2;
					l++;

					scissor_vertices[vcoffset + 0] = x;
					scissor_vertices[vcoffset + 1] = y;
					scissor_vertices[vcoffset + 2] = x + w;
					scissor_vertices[vcoffset + 3] = y;
					scissor_vertices[vcoffset + 4] = x + w;
					scissor_vertices[vcoffset + 5] = y + h;
					scissor_vertices[vcoffset + 6] = x;
					scissor_vertices[vcoffset + 7] = y + h;

					scissor_triangles[toffset + 0] = soffset + 0;
					scissor_triangles[toffset + 1] = soffset + 1;
					scissor_triangles[toffset + 2] = soffset + 2;
					scissor_triangles[toffset + 3] = soffset + 2;
					scissor_triangles[toffset + 4] = soffset + 3;
					scissor_triangles[toffset + 5] = soffset + 0;
				}
			}
			nr_active_scissors = l;

			render_scissors();
		}
		break;

		default:
			set_error(VG_ILLEGAL_ARGUMENT_ERROR);
			break;
		}
	}

	VGfloat Context::vgGetf(VGint paramType) {
		switch(paramType) {
			/* For the following paramType values this call is illegal */
		case VG_STROKE_DASH_PATTERN:
		case VG_STROKE_DASH_PHASE_RESET:
		case VG_MATRIX_MODE:
		case VG_CLEAR_COLOR:
		case VG_STROKE_CAP_STYLE:
		case VG_STROKE_JOIN_STYLE:
			set_error(VG_ILLEGAL_ARGUMENT_ERROR);
			break;
			/* Mode settings */
		case VG_FILL_RULE:
		case VG_IMAGE_QUALITY:
		case VG_RENDERING_QUALITY:
		case VG_BLEND_MODE:
		case VG_IMAGE_MODE:
			break;

			/* Scissoring rectangles */
		case VG_SCISSOR_RECTS:
			break;

			/* Color Transformation */
		case VG_COLOR_TRANSFORM:
		case VG_COLOR_TRANSFORM_VALUES:
			break;

			/* Stroke parameters */
		case VG_STROKE_LINE_WIDTH:
			return stroke_width;
		case VG_STROKE_DASH_PHASE:
			return stroke_dash_phase;

		case VG_STROKE_MITER_LIMIT:
			return miter_limit;

			/* Edge fill color for VG_TILE_FILL tiling mode */
		case VG_TILE_FILL_COLOR:
			break;

			/* Enable/disable alpha masking and scissoring */
		case VG_MASKING:
		case VG_SCISSORING:
			break;

			/* Pixel layout information */
		case VG_PIXEL_LAYOUT:
		case VG_SCREEN_LAYOUT:
			break;

			/* Source format selection for image filters */
		case VG_FILTER_FORMAT_LINEAR:
		case VG_FILTER_FORMAT_PREMULTIPLIED:
			break;

			/* Destination write enable mask for image filters */
		case VG_FILTER_CHANNEL_MASK:
			break;

			/* Implementation limits (read-only) */
		case VG_MAX_DASH_COUNT:
		case VG_MAX_KERNEL_SIZE:
		case VG_MAX_SEPARABLE_KERNEL_SIZE:
		case VG_MAX_IMAGE_WIDTH:
		case VG_MAX_IMAGE_HEIGHT:
		case VG_MAX_IMAGE_PIXELS:
		case VG_MAX_IMAGE_BYTES:
		case VG_MAX_FLOAT:
		case VG_MAX_GAUSSIAN_STD_DEVIATION:
			break;
		}

		// illegal argument (/not implemented)
		set_error(VG_ILLEGAL_ARGUMENT_ERROR);
		return 0.0f;
	}

	VGint Context::vgGeti(VGint paramType) {
		switch(paramType) {
			/* For the following paramType values this call is illegal */
		default:
		case VG_STROKE_DASH_PATTERN:
		case VG_STROKE_DASH_PHASE:
		case VG_STROKE_LINE_WIDTH:
		case VG_CLEAR_COLOR:
		case VG_STROKE_MITER_LIMIT:
			set_error(VG_ILLEGAL_ARGUMENT_ERROR);
			break;
			/* Mode settings */
		case VG_MATRIX_MODE:
		{
			return user_matrix;
		}

		case VG_FILL_RULE:
		case VG_IMAGE_QUALITY:
		case VG_RENDERING_QUALITY:
		case VG_BLEND_MODE:
		case VG_IMAGE_MODE:
			break;

			/* Scissoring rectangles */
		case VG_SCISSOR_RECTS:
			break;

			/* Color Transformation */
		case VG_COLOR_TRANSFORM:
		case VG_COLOR_TRANSFORM_VALUES:
			break;

			/* Stroke parameters */
		case VG_STROKE_CAP_STYLE:
			break;
		case VG_STROKE_JOIN_STYLE:
			return join_style;
		case VG_STROKE_DASH_PHASE_RESET:
			return stroke_dash_phase_reset ? VG_TRUE : VG_FALSE;

			/* Edge fill color for VG_TILE_FILL tiling mode */
		case VG_TILE_FILL_COLOR:
			break;


			/* Glyph origin */
		case VG_GLYPH_ORIGIN:
			break;

			/* Enable/disable alpha masking and scissoring */
		case VG_MASKING:
			return mask_is_active ? VG_TRUE : VG_FALSE;

		case VG_SCISSORING:
			return scissors_are_active ? VG_TRUE : VG_FALSE;

			/* Pixel layout information */
		case VG_PIXEL_LAYOUT:
		case VG_SCREEN_LAYOUT:
			break;

			/* Source format selection for image filters */
		case VG_FILTER_FORMAT_LINEAR:
		case VG_FILTER_FORMAT_PREMULTIPLIED:
			break;

			/* Destination write enable mask for image filters */
		case VG_FILTER_CHANNEL_MASK:
			break;

			/* Implementation limits (read-only) */
		case VG_MAX_SCISSOR_RECTS:
			return GNUVG_MAX_SCISSORS;
			break;

		case VG_MAX_COLOR_RAMP_STOPS:
			return GNUVG_MAX_COLOR_RAMP_STOPS;

		case VG_MAX_KERNEL_SIZE:
		case VG_MAX_SEPARABLE_KERNEL_SIZE:
		case VG_MAX_IMAGE_WIDTH:
		case VG_MAX_IMAGE_HEIGHT:
		case VG_MAX_IMAGE_PIXELS:
		case VG_MAX_IMAGE_BYTES:
		case VG_MAX_FLOAT:
		case VG_MAX_GAUSSIAN_STD_DEVIATION:
			break;
		case VG_MAX_DASH_COUNT:
			return 128;
		}

		// illegal argument (/not implemented)
		set_error(VG_ILLEGAL_ARGUMENT_ERROR);
		return -1; // not implemented
	}

	VGint Context::vgGetVectorSize(VGint paramType) {
		switch(paramType) {
		case VG_MATRIX_MODE:
		case VG_FILL_RULE:
		case VG_IMAGE_QUALITY:
		case VG_RENDERING_QUALITY:
		case VG_BLEND_MODE:
		case VG_IMAGE_MODE:
		case VG_COLOR_TRANSFORM:
		case VG_STROKE_LINE_WIDTH:
		case VG_STROKE_CAP_STYLE:
		case VG_STROKE_JOIN_STYLE:
		case VG_STROKE_MITER_LIMIT:
		case VG_STROKE_DASH_PHASE:
		case VG_STROKE_DASH_PHASE_RESET:
		case VG_MASKING:
		case VG_SCISSORING:
		case VG_SCREEN_LAYOUT:
		case VG_PIXEL_LAYOUT:
		case VG_FILTER_FORMAT_LINEAR:
		case VG_FILTER_FORMAT_PREMULTIPLIED:
		case VG_FILTER_CHANNEL_MASK:
		case VG_MAX_SCISSOR_RECTS:
		case VG_MAX_DASH_COUNT:
		case VG_MAX_KERNEL_SIZE:
		case VG_MAX_SEPARABLE_KERNEL_SIZE:
		case VG_MAX_GAUSSIAN_STD_DEVIATION:
		case VG_MAX_COLOR_RAMP_STOPS:
		case VG_MAX_IMAGE_WIDTH:
		case VG_MAX_IMAGE_HEIGHT:
		case VG_MAX_IMAGE_PIXELS:
		case VG_MAX_IMAGE_BYTES:
		case VG_MAX_FLOAT:
			return 1;

		case VG_SCISSOR_RECTS:
			return nr_active_scissors * 4;

		case VG_COLOR_TRANSFORM_VALUES:
			return 8;

		case VG_STROKE_DASH_PATTERN:
			return (VGint)dash_pattern.size();

		case VG_TILE_FILL_COLOR:
		case VG_CLEAR_COLOR:
			return 4;

		case VG_GLYPH_ORIGIN:
			return 2;
		}
		return 0;
	}

	void Context::vgGetfv(VGint paramType, VGint count, VGfloat *values) {
		switch(paramType) {
		case VG_GLYPH_ORIGIN:
			if(count == 2)
				for(int k = 0; k < 2; ++k)
					values[k] = glyph_origin[k];
			break;
		default:
			set_error(VG_ILLEGAL_ARGUMENT_ERROR);
		}
	}

	void Context::vgGetiv(VGint paramType, VGint count, VGint *values) {
	}

}

/* Backend implementation specific functions - OpenGL */
namespace gnuVG {

	void Context::render_scissors() {
		if(scissors_are_active) {
			if(nr_active_scissors > 0) {
				glEnable(GL_STENCIL_TEST);
				glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

				// first we clear the stencil to zero
				glClearStencil(0);
				glClear(GL_STENCIL_BUFFER_BIT);

				// then we render the scissor elements
				glStencilFunc(GL_ALWAYS,
					      1,
					      1);
				glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);

				trivial_render_elements(scissor_vertices,
							scissor_triangles,
							6 * nr_active_scissors,
							// colors will be ignored
							// because of disabled color mask
							1.0, 0.0, 0.0, 1.0);

				glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
				glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
			} else {
				glClearStencil(0);
				glClear(GL_STENCIL_BUFFER_BIT);
				glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
			}
		}

		// return to previous framebuffer and pipeline
		use_pipeline(active_pipeline, pipeline_mode);
	}

	void Context::recreate_buffers() {
		// Then the mask
		if(mask.framebuffer != 0)
			delete_framebuffer(&mask);
		if(!create_framebuffer(&mask, VG_sRGBA_8888,
				       current_framebuffer->width, current_framebuffer->height,
				       VG_IMAGE_QUALITY_FASTER))
			GNUVG_ERROR("failed to create framebuffer for mask.\n");

		// Then two temporary buffers
		if(temporary_a.framebuffer != 0)
			delete_framebuffer(&temporary_a);
		if(!create_framebuffer(&temporary_a, VG_sRGBA_8888,
				       current_framebuffer->width, current_framebuffer->height,
				       VG_IMAGE_QUALITY_FASTER))
			GNUVG_ERROR("failed to create framebuffer for temporary_a.\n");
		if(temporary_b.framebuffer != 0)
			delete_framebuffer(&temporary_b);
		if(!create_framebuffer(&temporary_b, VG_sRGBA_8888,
				       current_framebuffer->width, current_framebuffer->height,
				       VG_IMAGE_QUALITY_FASTER))
			GNUVG_ERROR("failed to create framebuffer for temporary_b.\n");
	}

	void Context::resize(VGint pxlw, VGint pxlh) {
		screen_buffer.width = pxlw;
		screen_buffer.height = pxlh;

		if(current_framebuffer == &screen_buffer)
			render_to_framebuffer(&screen_buffer);
	}

	void Context::prepare_pipeline() {
		glEnable(GL_BLEND);
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	void Context::clear(VGint x_not_implemented, VGint y_not_implemented,
			    VGint width_not_implemented, VGint height_not_implemented) {
		GNUVG_DEBUG("Context::clear() - this method is not properly implemented yet... \n");
		glClearColor(clear_color.r,
			     clear_color.g,
			     clear_color.b,
			     clear_color.a);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void Context::trivial_render_elements(
		GLfloat *vertices, GLuint *indices, GLsizei indices_count,
		VGfloat r, VGfloat g, VGfloat b, VGfloat a) {

		Matrix *m = &screen_matrix;
		GLfloat mat[] = {
			m->a, m->b, 0.0f, 0.0f,
			m->d, m->e, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			m->g, m->h, 0.0f, 1.0f
		};

		VGfloat col[] = {r, g, b, a};

		active_shader = Shader::get_shader(Shader::do_flat_color);
		active_shader->set_matrix(mat);
		active_shader->set_color(col);
		active_shader->load_2dvertex_array(vertices, 0);
		active_shader->render_elements(indices, indices_count);

		use_pipeline(active_pipeline, pipeline_mode);
	}

	void Context::trivial_fill_area(
		VGint _x, VGint _y, VGint _width, VGint _height,
		VGfloat r, VGfloat g, VGfloat b, VGfloat a) {

		VGfloat x = (VGfloat)_x;
		VGfloat y = (VGfloat)_y;
		VGfloat width = (VGfloat)_width;
		VGfloat height = (VGfloat)_height;

		GLfloat vertices[] = {
			x,         y,
			x + width, y,
			x + width, y + height,
			x        , y + height
		};

		GLuint indices[] = {
			0, 1, 2,
			0, 2, 3
		};

		trivial_render_elements(vertices, indices, 6, r, g, b, a);
	}

	void Context::select_conversion_matrix(MatrixMode _conversion_matrix) {
		if(conversion_matrix != _conversion_matrix ||
		   matrix_is_dirty[_conversion_matrix] ||
		   matrix_is_dirty[GNUVG_MATRIX_FILL_PAINT_TO_USER] ||
		   matrix_is_dirty[GNUVG_MATRIX_STROKE_PAINT_TO_USER]) {

			matrix_is_dirty[_conversion_matrix] = false;
			matrix_is_dirty[GNUVG_MATRIX_FILL_PAINT_TO_USER] = false;
			matrix_is_dirty[GNUVG_MATRIX_STROKE_PAINT_TO_USER] = false;

			/* calculate regualar conversion matrix */
			final_matrix[_conversion_matrix].multiply(
				screen_matrix,
				matrix[_conversion_matrix]);

			/* then also the surface -> paint matrices */
			final_matrix[GNUVG_MATRIX_FILL_PAINT_TO_USER].multiply(
				final_matrix[_conversion_matrix],
				matrix[GNUVG_MATRIX_FILL_PAINT_TO_USER]
				);
			final_matrix[GNUVG_MATRIX_FILL_PAINT_TO_USER].invert();

			final_matrix[GNUVG_MATRIX_STROKE_PAINT_TO_USER].multiply(
				final_matrix[_conversion_matrix],
				matrix[GNUVG_MATRIX_STROKE_PAINT_TO_USER]
				);
			final_matrix[GNUVG_MATRIX_STROKE_PAINT_TO_USER].invert();
		}

		conversion_matrix = _conversion_matrix;

		/* convert user2surface conversion matrix to GL friendly format */
		{
			Matrix *m = &final_matrix[conversion_matrix];
			GLfloat mat[] = {
				m->a, m->b, 0.0f, 0.0f,
				m->d, m->e, 0.0f, 0.0f,
				0.0f, 0.0f, 1.0f, 0.0f,
				m->g, m->h, 0.0f, 1.0f
			};
			memcpy(conversion_matrix_data, mat, sizeof(mat));
		}
		/* convert surface2fill conversion matrix to GL friendly format */
		{
			Matrix *m = &final_matrix[GNUVG_MATRIX_FILL_PAINT_TO_USER];
			GLfloat mat[] = {
				m->a, m->b, 0.0f, 0.0f,
				m->d, m->e, 0.0f, 0.0f,
				0.0f, 0.0f, 1.0f, 0.0f,
				m->g, m->h, 0.0f, 1.0f
			};
			memcpy(surf2fill_matrix_data, mat, sizeof(mat));
		}
		/* convert surface2stroke conversion matrix to GL friendly format */
		{
			Matrix *m = &final_matrix[GNUVG_MATRIX_STROKE_PAINT_TO_USER];
			GLfloat mat[] = {
				m->a, m->b, 0.0f, 0.0f,
				m->d, m->e, 0.0f, 0.0f,
				0.0f, 0.0f, 1.0f, 0.0f,
				m->g, m->h, 0.0f, 1.0f
			};
			memcpy(surf2stroke_matrix_data, mat, sizeof(mat));
		}
	}

	void Context::use_pipeline(gnuVGPipeline new_pipeline, VGPaintMode _mode) {
		ADD_GNUVG_PROFILER_PROBE(use_pipeline);
		pipeline_mode = _mode;
		active_pipeline = new_pipeline;

		auto active_paint =
			pipeline_mode == VG_STROKE_PATH ? stroke_paint : fill_paint;
		int caps = Shader::do_flat_color | Shader::do_pretranslate;

		if(active_pipeline == GNUVG_LB_PIPELINE)
			caps |= Shader::do_loop_n_blinn;

		bool gradient_enabled = false;
		switch(active_paint->ptype) {
		case VG_PAINT_TYPE_COLOR:
		case VG_PAINT_TYPE_PATTERN:
		case VG_PAINT_TYPE_FORCE_SIZE:
			break;
		case VG_PAINT_TYPE_LINEAR_GRADIENT:
			caps |= Shader::do_linear_gradient;
			gradient_enabled = true;
			break;

		case VG_PAINT_TYPE_RADIAL_GRADIENT:
			caps |= Shader::do_radial_gradient;
			gradient_enabled = true;
			break;
		}

		if(mask_is_active) caps |= Shader::do_mask;

		if(gradient_enabled)
			switch(active_paint->spread_mode) {
			case VG_COLOR_RAMP_SPREAD_MODE_FORCE_SIZE:
				break;
			case VG_COLOR_RAMP_SPREAD_PAD:
				caps |= Shader::do_gradient_pad;
				break;
			case VG_COLOR_RAMP_SPREAD_REPEAT:
				caps |= Shader::do_gradient_repeat;
				break;
			case VG_COLOR_RAMP_SPREAD_REFLECT:
				caps |= Shader::do_gradient_reflect;
				break;
			}

		active_shader = Shader::get_shader(caps);
		active_shader->use_shader();
		active_shader->set_matrix(conversion_matrix_data);
		active_shader->set_pre_translation(pre_translation);

		switch(pipeline_mode) {
		case VG_COLOR_RAMP_SPREAD_MODE_FORCE_SIZE:
			break;
		case VG_FILL_PATH:
			active_shader->set_surf2paint_matrix(surf2fill_matrix_data);
			break;
		case VG_STROKE_PATH:
			active_shader->set_surf2paint_matrix(surf2stroke_matrix_data);
			break;
		}

		if(mask_is_active) active_shader->set_mask_texture(mask.texture);

		if(scissors_are_active) {
			glEnable(GL_STENCIL_TEST);
			glStencilFunc(GL_EQUAL,
				      1,
				      1);
			glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
		} else {
			glDisable(GL_STENCIL_TEST);
			glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
		}

		switch(active_paint->ptype) {
		case VG_PAINT_TYPE_FORCE_SIZE:
		case VG_PAINT_TYPE_PATTERN:
			break;
		case VG_PAINT_TYPE_COLOR:
			active_shader->set_color(active_paint->color.c);
			return;

		case VG_PAINT_TYPE_LINEAR_GRADIENT:
			active_shader->set_linear_parameters(active_paint->gradient_parameters);
			active_shader->set_color_ramp(
				active_paint->max_stops,
				active_paint->color_ramp_stop_offset,
				active_paint->color_ramp_stop_invfactor,
				active_paint->color_ramp_stop_color);
			break;

		case VG_PAINT_TYPE_RADIAL_GRADIENT:
			active_shader->set_radial_parameters(active_paint->gradient_parameters);
			active_shader->set_color_ramp(
				active_paint->max_stops,
				active_paint->color_ramp_stop_offset,
				active_paint->color_ramp_stop_invfactor,
				active_paint->color_ramp_stop_color);
			break;
		}
	}

	void Context::reset_pre_translation() {
		for(int k = 0; k < 2; ++k)
			pre_translation[k] = 0.0f;
	}

	void Context::use_glyph_origin_as_pre_translation(VGfloat specific_glyph_origin[2]) {
		for(int k = 0; k < 2; ++k)
			pre_translation[k] = glyph_origin[k] - specific_glyph_origin[k];
	}

	void Context::adjust_glyph_origin(VGfloat escapement[2]) {
		for(int k = 0; k < 2; ++k)
			glyph_origin[k] += escapement[k];
	}

	static const GLfloat *__veerts;
	static GLint __striide;

	void Context::load_2dvertex_array(const GLfloat *verts, GLint stride) {
		if(active_shader) {
			active_shader->load_2dvertex_array(verts, stride);
			return;
		}
		ADD_GNUVG_PROFILER_PROBE(load_2dvertex_array);
		__veerts = verts;
		__striide = stride;

		glVertexAttribPointer(POSITION_ATTRIB_HANDLE, 2, GL_FLOAT, GL_FALSE,
				      stride * sizeof(GLfloat), verts);
		glEnableVertexAttribArray(POSITION_ATTRIB_HANDLE);
	}

	void Context::load_klm_array(const GLfloat *klm, GLint stride) {
		if(active_shader) {
			active_shader->load_klm_array(klm, stride);
			return;
		}
		glVertexAttribPointer(KLM_ATTRIB_HANDLE, 3, GL_FLOAT, GL_FALSE, stride * sizeof(GLfloat), klm);
		glEnableVertexAttribArray(KLM_ATTRIB_HANDLE);
	}

	void Context::render_triangles(GLint first, GLsizei count) {
		if(active_shader) {
			active_shader->render_triangles(first, count);
			return;
		}
		glDrawArrays(GL_TRIANGLES, first, count);
	}

	void Context::render_elements(const GLuint *indices, GLsizei nr_indices) {
		ADD_GNUVG_PROFILER_COUNTER(render_elements, nr_indices);
		if(active_shader) {
			active_shader->render_elements(indices, nr_indices);
			return;
		}
		ADD_GNUVG_PROFILER_PROBE(render_elements);
		glDrawElements(GL_TRIANGLES, nr_indices, GL_UNSIGNED_INT, indices);
	}

	static inline void add_to_bounding_box(Point* bbox, const Point& p) {
		if(p.x < bbox[0].x)
			bbox[0].x = p.x;
		if(p.y < bbox[0].y)
			bbox[0].y = p.y;

		if(p.x > bbox[1].x)
			bbox[1].x = p.x;
		if(p.y > bbox[1].y)
			bbox[1].y = p.y;
	}

	void Context::calculate_bounding_box(Point* bbox) {
		Point transformed[4];
		for(int k = 0; k < 4; k++) {
			transformed[k] = final_matrix[conversion_matrix].map_point(bbox[k]);
		}

		int k = 0;
		if(bounding_box_was_reset) {
			k = 1;
			bounding_box_was_reset = false;
			bounding_box[0] = transformed[0];
			bounding_box[1] = transformed[0];
		}

		for(; k < 4; k++) {
			add_to_bounding_box(bounding_box, transformed[k]);
		}
	}

	void Context::transform_bounding_box(Point* bbox, VGfloat *sp_ep) {
		Point transformed[4];
		for(int k = 0; k < 4; k++) {
			transformed[k] = final_matrix[conversion_matrix].map_point(bbox[k]);
		}

		Point bounding_box[2];

		bounding_box[0] = transformed[0];
		bounding_box[1] = transformed[0];

		for(auto k = 1; k < 4; k++) {
			add_to_bounding_box(bounding_box, transformed[k]);
		}

		// we must convert the bbox to screen coordinates
		auto w = current_framebuffer->width;
		auto h = current_framebuffer->height;
		sp_ep[0] = w * (0.5f + bounding_box[0].x / 2.0f);
		sp_ep[1] = h * (0.5f - bounding_box[1].y / 2.0f);
		sp_ep[2] = w * (0.5f + bounding_box[1].x / 2.0f);
		sp_ep[3] = h * (0.5f - bounding_box[0].y / 2.0f);
	}

	void Context::get_pixelsize(VGint& w, VGint& h) {
		w = current_framebuffer->width;
		h = current_framebuffer->height;
	}

	void Context::blend_framebuffers(const FrameBuffer* fb_src,
					 const FrameBuffer* fb_dst,
					 gnuVGBlendMode blend_mode) {

		int caps = 0;

		switch(blend_mode) {
		case GNUVG_BLEND_SRC:
			caps |= Shader::do_blend_src;
			break;
		case GNUVG_BLEND_SRC_OVER:
			caps |= Shader::do_blend_src_over;
			break;
		case GNUVG_BLEND_DST_OVER:
			caps |= Shader::do_blend_dst_over;
			break;
		case GNUVG_BLEND_SRC_IN:
			caps |= Shader::do_blend_src_in;
			break;
		case GNUVG_BLEND_DST_IN:
			caps |= Shader::do_blend_dst_in;
			break;
		case GNUVG_BLEND_MULTIPLY:
			caps |= Shader::do_blend_multiply;
			break;
		case GNUVG_BLEND_SCREEN:
			caps |= Shader::do_blend_screen;
			break;
		case GNUVG_BLEND_DARKEN:
			caps |= Shader::do_blend_darken;
			break;
		case GNUVG_BLEND_LIGHTEN:
			caps |= Shader::do_blend_lighten;
			break;
		case GNUVG_BLEND_ADDITIVE:
			caps |= Shader::do_blend_additive;
			break;
		case GNUVG_BLEND_SUBTRACT_ALPHA:
			caps |= Shader::do_blend_subtract_alpha;
			break;
		}

		active_shader = Shader::get_shader(caps);
		active_shader->use_shader();

		active_shader->set_blend_source_texture(fb_src->texture);
		active_shader->set_blend_destination_texture(fb_dst->texture);

		// unit matrix
		GLfloat mat[] = {
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		};
		active_shader->set_matrix(mat);

		/* cover the whole framebuffer with two triangles */
		GLfloat vertices[] = {
			-1.0f, -1.0f,
			1.0f, -1.0f,
			1.0f, 1.0f,
			-1.0f, 1.0f
		};

		GLuint indices[] = {
			0, 1, 2,
			0, 2, 3
		};

		active_shader->load_2dvertex_array(vertices, 0);
		active_shader->render_elements(indices, 6);

		// restore pipeline
		use_pipeline(active_pipeline, pipeline_mode);
	}

	void Context::switch_mask_to(gnuVGFrameBuffer to_this_temporary) {
		auto temporary = mask;
		switch(to_this_temporary) {
		case Context::GNUVG_TEMPORARY_A:
			mask = temporary_a;
			temporary_a = temporary;
			break;
		case Context::GNUVG_TEMPORARY_B:
			mask = temporary_b;
			temporary_b = temporary;
			break;
		default:
			GNUVG_ERROR("Context::switch_mask_to() not called"
				    " with temporary buffer as parameter.\n");
			break;
		}
	}

	bool Context::create_framebuffer(FrameBuffer* destination,
					 VGImageFormat format,
					 VGint w, VGint h,
					 VGbitfield allowedQuality) {
		destination->width = w;
		destination->height = h;
		glGenFramebuffers(1, &destination->framebuffer);
		glGenTextures(1, &destination->texture);

		glBindTexture(GL_TEXTURE_2D, destination->texture);
		glTexImage2D(GL_TEXTURE_2D, 0,
			     GL_RGBA, w, h, 0,
			     GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4,
			     NULL);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		glBindFramebuffer(GL_FRAMEBUFFER, destination->framebuffer);

		// specify texture as color attachment
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
				       destination->texture, 0 );

		glBindFramebuffer(GL_FRAMEBUFFER, current_framebuffer->framebuffer);

		return glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
	}

	void Context::delete_framebuffer(FrameBuffer* framebuffer) {
		if(framebuffer == current_framebuffer) {
			render_to_framebuffer(&screen_buffer);
		}
		glDeleteFramebuffers(1, &framebuffer->framebuffer);
		glDeleteTextures(1, &framebuffer->texture);
		framebuffer->framebuffer = 0;
		framebuffer->texture = 0;
	}

	void Context::render_to_framebuffer(const FrameBuffer* framebuffer) {
		current_framebuffer = framebuffer == nullptr ? (&screen_buffer) : framebuffer;

		glBindFramebuffer(GL_FRAMEBUFFER, current_framebuffer->framebuffer);

		if(current_framebuffer->width ==
		   buffer_width &&
		   current_framebuffer->height ==
		   buffer_height)
			return; // we can stop here - all is same from here

		buffer_width = current_framebuffer->width;
		buffer_height = current_framebuffer->height;

		matrix_resize(buffer_width, buffer_height);
		render_scissors();
		recreate_buffers();
	}

	auto Context::get_internal_framebuffer(gnuVGFrameBuffer selection) -> const FrameBuffer* {
		switch(selection) {
		case Context::GNUVG_CURRENT_FRAMEBUFFER:
			return current_framebuffer;
		case Context::GNUVG_TEMPORARY_A:
			return &temporary_a;
		case Context::GNUVG_TEMPORARY_B:
			return &temporary_b;
		case Context::GNUVG_MASK_BUFFER:
			return &mask;
		}
		GNUVG_ERROR("Context::get_internal_framebuffer() returning nullptr\n");
		return nullptr;
	}

	void Context::clear_framebuffer(const FrameBuffer* framebuffer,
					VGint x, VGint y,
					VGint width, VGint height) {
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer->framebuffer);
		clear(x, y, width, height);
		glBindFramebuffer(GL_FRAMEBUFFER, current_framebuffer->framebuffer);
	}

	void Context::save_current_framebuffer() {
		framebuffer_storage.push(current_framebuffer);
	}

	void Context::restore_current_framebuffer() {
		if(framebuffer_storage.size()) {
			render_to_framebuffer(framebuffer_storage.top());
			framebuffer_storage.pop();
		}
	}

}

using namespace gnuVG;

extern "C" {

	/*********************
	 *
	 *  Getters and Setters
	 *
	 *********************/
	VGErrorCode VG_API_ENTRY vgGetError(void) VG_API_EXIT {
		return gnuVG::Context::get_current()->get_error();
	}

	void VG_API_ENTRY vgSetf (VGParamType type, VGfloat value) VG_API_EXIT {
		gnuVG::Context::get_current()->vgSetf((VGint)type, value);
	}

	void VG_API_ENTRY vgSeti (VGParamType type, VGint value) VG_API_EXIT {
		gnuVG::Context::get_current()->vgSeti((VGint)type, value);
	}

	void VG_API_ENTRY vgSetfv(VGParamType type, VGint count,
				  const VGfloat * values) VG_API_EXIT {
		gnuVG::Context::get_current()->vgSetfv((VGint)type, count, values);
	}

	void VG_API_ENTRY vgSetiv(VGParamType type, VGint count,
				  const VGint * values) VG_API_EXIT {
		gnuVG::Context::get_current()->vgSetiv((VGint)type, count, values);
	}

	VGfloat VG_API_ENTRY vgGetf(VGParamType type) VG_API_EXIT {
		return gnuVG::Context::get_current()->vgGetf((VGint)type);
	}

	VGint VG_API_ENTRY vgGeti(VGParamType type) VG_API_EXIT {
		return gnuVG::Context::get_current()->vgGeti((VGint)type);
	}

	VGint VG_API_ENTRY vgGetVectorSize(VGParamType type) VG_API_EXIT {
		return gnuVG::Context::get_current()->vgGetVectorSize((VGint)type);
	}

	void VG_API_ENTRY vgGetfv(VGParamType type, VGint count, VGfloat * values) VG_API_EXIT {
		gnuVG::Context::get_current()->vgGetfv((VGint)type, count, values);
	}

	void VG_API_ENTRY vgGetiv(VGParamType type, VGint count, VGint * values) VG_API_EXIT {
		gnuVG::Context::get_current()->vgGetiv((VGint)type, count, values);
	}

	void VG_API_ENTRY vgSetParameterf(VGHandle object,
					  VGint paramType,
					  VGfloat value) VG_API_EXIT {
		if(object != VG_INVALID_HANDLE) {
			Object *o = (Object *)object;
			o->vgSetParameterf(paramType, value);
		} else {
			gnuVG::Context::get_current()->set_error(VG_BAD_HANDLE_ERROR);
		}
	}

	void VG_API_ENTRY vgSetParameteri(VGHandle object,
					  VGint paramType,
					  VGint value) VG_API_EXIT {
		if(object != VG_INVALID_HANDLE) {
			Object *o = (Object *)object;
			o->vgSetParameteri(paramType, value);
		} else {
			gnuVG::Context::get_current()->set_error(VG_BAD_HANDLE_ERROR);
		}
	}

	void VG_API_ENTRY vgSetParameterfv(VGHandle object,
					   VGint paramType,
					   VGint count, const VGfloat * values) VG_API_EXIT {
		if(object != VG_INVALID_HANDLE) {
			Object *o = (Object *)object;
			o->vgSetParameterfv(paramType, count, values);
		} else {
			gnuVG::Context::get_current()->set_error(VG_BAD_HANDLE_ERROR);
		}
	}

	void VG_API_ENTRY vgSetParameteriv(VGHandle object,
					   VGint paramType,
					   VGint count, const VGint * values) VG_API_EXIT {
		if(object != VG_INVALID_HANDLE) {
			Object *o = (Object *)object;
			o->vgSetParameteriv(paramType, count, values);
		} else {
			gnuVG::Context::get_current()->set_error(VG_BAD_HANDLE_ERROR);
		}
	}


	VGfloat VG_API_ENTRY vgGetParameterf(VGHandle object,
					     VGint paramType) VG_API_EXIT {
		if(object != VG_INVALID_HANDLE) {
			Object *o = (Object *)object;
			return o->vgGetParameteri(paramType);
		}

		gnuVG::Context::get_current()->set_error(VG_BAD_HANDLE_ERROR);
		return 0;
	}

	VGint VG_API_ENTRY vgGetParameteri(VGHandle object,
					   VGint paramType) {
		if(object != VG_INVALID_HANDLE) {
			Object *o = (Object *)object;
			return o->vgGetParameteri(paramType);
		}

		gnuVG::Context::get_current()->set_error(VG_BAD_HANDLE_ERROR);
		return 0;
	}

	VGint VG_API_ENTRY vgGetParameterVectorSize(VGHandle object,
						    VGint paramType) VG_API_EXIT {
		if(object != VG_INVALID_HANDLE) {
			Object *o = (Object *)object;
			return o->vgGetParameterVectorSize(paramType);
		}

		gnuVG::Context::get_current()->set_error(VG_BAD_HANDLE_ERROR);
		return 0;
	}

	void VG_API_ENTRY vgGetParameterfv(VGHandle object,
					   VGint paramType,
					   VGint count, VGfloat * values) VG_API_EXIT {
		if(object != VG_INVALID_HANDLE) {
			Object *o = (Object *)object;
			o->vgGetParameterfv(paramType, count, values);
		}

		gnuVG::Context::get_current()->set_error(VG_BAD_HANDLE_ERROR);
	}

	void VG_API_ENTRY vgGetParameteriv(VGHandle object,
					   VGint paramType,
					   VGint count, VGint * values) VG_API_EXIT {
		if(object != VG_INVALID_HANDLE) {
			Object *o = (Object *)object;
			o->vgGetParameteriv(paramType, count, values);
		}

		gnuVG::Context::get_current()->set_error(VG_BAD_HANDLE_ERROR);
	}

	/**********************
	 *
	 * Matrix Manipulation
	 *
	 **********************/
	void VG_API_ENTRY vgLoadIdentity(void) VG_API_EXIT {
		gnuVG::Context::get_current()->vgLoadIdentity();
	}

	void VG_API_ENTRY vgLoadMatrix(const VGfloat * m) VG_API_EXIT {
		gnuVG::Context::get_current()->vgLoadMatrix(m);
	}

	void VG_API_ENTRY vgGetMatrix(VGfloat * m) VG_API_EXIT {
		gnuVG::Context::get_current()->vgGetMatrix(m);
	}

	void VG_API_ENTRY vgMultMatrix(const VGfloat * m) VG_API_EXIT {
		gnuVG::Context::get_current()->vgMultMatrix(m);
	}

	void VG_API_ENTRY vgTranslate(VGfloat tx, VGfloat ty) VG_API_EXIT {
		gnuVG::Context::get_current()->vgTranslate(tx, ty);
	}

	void VG_API_ENTRY vgScale(VGfloat sx, VGfloat sy) VG_API_EXIT {
		gnuVG::Context::get_current()->vgScale(sx, sy);
	}

	void VG_API_ENTRY vgShear(VGfloat shx, VGfloat shy) VG_API_EXIT {
		gnuVG::Context::get_current()->vgShear(shx, shy);
	}

	void VG_API_ENTRY vgRotate(VGfloat angle) VG_API_EXIT {
		gnuVG::Context::get_current()->vgRotate(angle);
	}

	void VG_API_ENTRY vgClear(VGint x, VGint y, VGint width, VGint height) VG_API_EXIT {
		gnuVG::Context::get_current()->clear(x, y, width, height);
	}

	const VGubyte * VG_API_ENTRY vgGetString(VGStringID name) VG_API_EXIT {
		static VGubyte *vendor = (VGubyte *)strdup("Anton Persson");
		static VGubyte *renderer = (VGubyte *)strdup("gnuVG");
		static VGubyte *version = (VGubyte *)strdup("1.1");
		static VGubyte *extensions = (VGubyte *)strdup("gnuVG");

		switch(name) {
		case VG_VENDOR:
			return vendor;
		case VG_RENDERER:
			return renderer;
		case VG_VERSION:
			return version;
		case VG_EXTENSIONS:
			return extensions;
		case VG_STRING_ID_FORCE_SIZE:
			break;
		}

		return NULL;
	}

	/*********************
	 *
	 *    gnuVG extensions API
	 *
	 *********************/
	VGHandle VG_API_ENTRY gnuvgCreateContext() VG_API_EXIT {
		return (VGHandle)(new gnuVG::Context());
	}

	void VG_API_ENTRY gnuvgDestroyContext(VGHandle context) VG_API_EXIT {
		delete ((gnuVG::Context *)context);
	}

	void VG_API_ENTRY gnuvgUseContext(VGHandle context) VG_API_EXIT {
		gnuVG::Context::set_current((gnuVG::Context *)context);
	}

	void VG_API_ENTRY gnuvgResize(VGint pixel_width, VGint pixel_height) VG_API_EXIT {
		gnuVG::Context::get_current()->resize(pixel_width, pixel_height);
	}

	void VG_API_ENTRY gnuvgResetBoundingBox() {
		gnuVG::Context::get_current()->reset_bounding_box();
	}

	void VG_API_ENTRY gnuvgGetBoundingBox(VGfloat *sp_ep) {
		gnuVG::Context::get_current()->get_bounding_box(sp_ep);
	}

}
