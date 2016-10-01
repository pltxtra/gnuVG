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

#include <functional>

#include "gnuVG_mask.hh"

//#define __DO_GNUVG_DEBUG
#include "gnuVG_debug.hh"

#define MASK_R_CHANNEL_VALUE 0.0f
#define MASK_G_CHANNEL_VALUE 0.0f
#define MASK_B_CHANNEL_VALUE 0.0f

namespace gnuVG {
	MaskLayer::MaskLayer(Context* ctx, VGint width, VGint height) {
		if(ctx->create_framebuffer(
			   &framebuffer, VG_sRGBA_8888,
			   width, height,
			   VG_IMAGE_QUALITY_BETTER))
			throw FailedToCreateMaskLayerException();
	}

	MaskLayer::~MaskLayer() {
		auto ctx = Context::get_current();
		if(ctx) {
			ctx->delete_framebuffer(&framebuffer);
		}
	}

	void MaskLayer::vgFillMaskLayer(VGint x, VGint y, VGint width, VGint height, VGfloat value) {
		auto ctx = Context::get_current();
		if(ctx) {
			ctx->save_current_framebuffer();
			ctx->render_to_framebuffer(&framebuffer);
			ctx->trivial_fill_area(x, y, width, height,
					       MASK_R_CHANNEL_VALUE,
					       MASK_G_CHANNEL_VALUE,
					       MASK_B_CHANNEL_VALUE,
					       value);
			ctx->restore_current_framebuffer();
		}
	}

	void MaskLayer::vgCopyMask(VGint dx, VGint dy,
				   VGint sx, VGint sy,
				   VGint width, VGint height) {
		auto ctx = Context::get_current();
		if(ctx) {
			auto fbuf = ctx->get_internal_framebuffer(Context::GNUVG_CURRENT_FRAMEBUFFER);
			ctx->copy_framebuffer_to_framebuffer(
				fbuf, &framebuffer,
				dx, dy, sx, sy, width, height);
		}
	}

	/* inherited virtual interface */
	void MaskLayer::vgSetParameterf(VGint paramType, VGfloat value) {
		Context::get_current()->set_error(VG_ILLEGAL_ARGUMENT_ERROR);
	}

	void MaskLayer::vgSetParameteri(VGint paramType, VGint value) {
		Context::get_current()->set_error(VG_ILLEGAL_ARGUMENT_ERROR);
	}

	void MaskLayer::vgSetParameterfv(VGint paramType, VGint count, const VGfloat *values) {
		Context::get_current()->set_error(VG_ILLEGAL_ARGUMENT_ERROR);
	}

	void MaskLayer::vgSetParameteriv(VGint paramType, VGint count, const VGint *values) {
		Context::get_current()->set_error(VG_ILLEGAL_ARGUMENT_ERROR);
	}


	VGfloat MaskLayer::vgGetParameterf(VGint paramType) {
		Context::get_current()->set_error(VG_ILLEGAL_ARGUMENT_ERROR);
		return 0.0f;
	}

	VGint MaskLayer::vgGetParameteri(VGint paramType) {
		Context::get_current()->set_error(VG_ILLEGAL_ARGUMENT_ERROR);
		return 0;
	}


	VGint MaskLayer::vgGetParameterVectorSize(VGint paramType) {
		return 0;
	}


	void MaskLayer::vgGetParameterfv(VGint paramType, VGint count, VGfloat *values) {
		Context::get_current()->set_error(VG_ILLEGAL_ARGUMENT_ERROR);
	}

	void MaskLayer::vgGetParameteriv(VGint paramType, VGint count, VGint *values) {
		Context::get_current()->set_error(VG_ILLEGAL_ARGUMENT_ERROR);
	}

};

using namespace gnuVG;

extern "C" {
	VGMaskLayer VG_API_ENTRY vgCreateMaskLayer(VGint width, VGint height) VG_API_EXIT {
		auto ctx = Context::get_current();
		if(ctx) {
			try {
				auto mskl =
					Object::create<MaskLayer>(ctx, width, height);
				return (VGMaskLayer)mskl->get_handle();
			} catch(MaskLayer::FailedToCreateMaskLayerException e) {
				GNUVG_ERROR("vgCreateMaskLayer() - Failed to prepare framebuffer.\n");
			}
		} else
			GNUVG_ERROR("vgCreateMaskLayer() - No valid context available.\n");

		return VG_INVALID_HANDLE;
	}

	void VG_API_ENTRY vgDestroyMaskLayer(VGMaskLayer mskl) VG_API_EXIT {
		Object::dereference(mskl);
	}

	void VG_API_ENTRY vgFillMaskLayer(VGMaskLayer mskl,
					  VGint x, VGint y,
					  VGint width, VGint height,
					  VGfloat value) VG_API_EXIT {
		auto ml = Object::get<MaskLayer>(mskl);
		if(ml)
			ml->vgFillMaskLayer(x, y, width, height, value);
	}

	void VG_API_ENTRY vgCopyMask(VGMaskLayer mskl,
				     VGint dx, VGint dy,
				     VGint sx, VGint sy,
				     VGint width, VGint height) VG_API_EXIT {
		auto ml = Object::get<MaskLayer>(mskl);
		if(ml)
			ml->vgCopyMask(dx, dy, sx, sy, width, height);
	}

	static void render_helper(Context* ctx,
				  VGMaskOperation operation,
				  std::function<void()> perform_operation) {
		// save current framebuffer
		ctx->save_current_framebuffer();

		auto fb_mask = ctx->get_internal_framebuffer(Context::GNUVG_MASK_BUFFER);

		// Render to mask
		ctx->render_to_framebuffer(fb_mask);

		// perform the operation
		perform_operation();

		// restore previously saved framebuffer
		ctx->restore_current_framebuffer();
	}

	static void render_direct_helper(Context* ctx,
					 std::function<void()> perform_operation) {
		// save current framebuffer
		ctx->save_current_framebuffer();

		auto fb_mask = ctx->get_internal_framebuffer(Context::GNUVG_MASK_BUFFER);

		// switch to temporary buffer A for rendering new mask data
		ctx->render_to_framebuffer(fb_mask);

		// perform the operation
		perform_operation();

		// restore previously saved framebuffer
		ctx->restore_current_framebuffer();
	}

	void VG_API_ENTRY vgMask(VGHandle mask,
				 VGMaskOperation operation,
				 VGint x, VGint y,
				 VGint width, VGint height) VG_API_EXIT {
		auto ctx = Context::get_current();
		if(!ctx) {
			GNUVG_ERROR("vgMask() called without proper gnuVG context.\n");
			return;
		}

		if(operation == VG_CLEAR_MASK || operation == VG_FILL_MASK) {
			VGfloat value = operation == VG_CLEAR_MASK ? 0.0f : 1.0f;
			render_direct_helper(ctx,
					     [ctx, x, y, width, height, value]() {
						     ctx->trivial_fill_area(x, y, width, height,
									    MASK_R_CHANNEL_VALUE,
									    MASK_G_CHANNEL_VALUE,
									    MASK_B_CHANNEL_VALUE,
									    value);
					     }
				);
		} else {
			GNUVG_ERROR("vgMask() only supports VG_CLEAR_MASK and VG_FILL_MASK.\n");
			ctx->set_error(VG_BAD_HANDLE_ERROR);
			return;
		}
	}

	void VG_API_ENTRY vgRenderToMask(VGPath path,
					 VGbitfield paintModes,
					 VGMaskOperation operation) VG_API_EXIT {
		auto ctx = Context::get_current();
		if(!ctx) {
			GNUVG_ERROR("vgRenderToMask() called without proper gnuVG context.\n");
			return;
		}

		if(path == VG_INVALID_HANDLE) {
			ctx->set_error(VG_BAD_HANDLE_ERROR);
			return;
		}

		render_helper(ctx,
			      operation,
			      [ctx, path, paintModes]() {
				      VGPaint old_stroke = vgGetPaint(VG_STROKE_PATH);
				      VGPaint old_fill = vgGetPaint(VG_FILL_PATH);

				      static VGPaint mpaint = VG_INVALID_HANDLE;
				      if(mpaint == VG_INVALID_HANDLE)
					      mpaint = vgCreatePaint();

				      if(mpaint) {
					      VGfloat rgba[] = {MASK_R_CHANNEL_VALUE,
								MASK_G_CHANNEL_VALUE,
								MASK_B_CHANNEL_VALUE,
								1.0f};

					      // remeber old settings for mask/scissors
					      VGint do_mask, do_scissors;
					      do_mask = vgGeti(VG_MASKING);
					      do_scissors = vgGeti(VG_SCISSORING);

					      // disable mask/scissors
					      vgSeti(VG_MASKING, VG_FALSE);
					      vgSeti(VG_SCISSORING, VG_FALSE);

					      vgSetParameterfv(mpaint, VG_PAINT_COLOR, 4, rgba);
					      vgSetParameteri(mpaint, VG_PAINT_TYPE, VG_PAINT_TYPE_COLOR);

					      vgSetPaint(mpaint, paintModes);

					      vgDrawPath(path, paintModes);

					      // restore mask/scissor settings
					      vgSeti(VG_MASKING, do_mask);
					      vgSeti(VG_SCISSORING, do_scissors);
				      }

				      vgSetPaint(old_stroke, VG_STROKE_PATH);
				      vgSetPaint(old_fill, VG_FILL_PATH);
			      }
			);
	}


}
