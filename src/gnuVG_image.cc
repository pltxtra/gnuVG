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

#include "gnuVG_image.hh"

//#define __DO_GNUVG_DEBUG
#include "gnuVG_debug.hh"

namespace gnuVG {
	Image::Image(Context* ctx, VGImageFormat format,
		     VGint width, VGint height,
		     VGbitfield allowedQuality) {
		if(ctx->create_framebuffer(&framebuffer, format, width, height, allowedQuality))
			throw FailedToCreateImageException();
	}

	Image::~Image() {
		auto ctx = Context::get_current();
		if(ctx) {
			ctx->delete_framebuffer(&framebuffer);
		}
	}

	void Image::vgClearImage(VGint x, VGint y, VGint width, VGint height) {
		auto ctx = Context::get_current();
		if(ctx) {
			ctx->save_current_framebuffer();
			ctx->render_to_framebuffer(&framebuffer);
			ctx->clear(x, y, width, height);
			ctx->restore_current_framebuffer();
		}
	}

	void Image::vgImageSubData(const void * data, VGint dataStride,
				   VGImageFormat dataFormat,
				   VGint x, VGint y, VGint width, VGint height) {
	}

	void Image::vgGetImageSubData(void * data, VGint dataStride,
				      VGImageFormat dataFormat,
				      VGint x, VGint y,
				      VGint width, VGint height) {
	}

	Image* Image::vgChildImage(VGint x, VGint y, VGint width, VGint height) {
		return VG_INVALID_HANDLE;
	}

	Image* Image::vgGetParent() {
		return parent;
	}

	void Image::vgCopyImage(VGint dx, VGint dy,
				Image* src, VGint sx, VGint sy,
				VGint width, VGint height,
				VGboolean dither) {
		auto ctx = Context::get_current();
		if(ctx) {
			ctx->copy_framebuffer_to_framebuffer(
				&framebuffer, &src->framebuffer,
				dx, dy, sx, sy, width, height);
		}
	}

	void Image::vgDrawImage() {
	}

	void Image::vgSetPixels(VGint dx, VGint dy,
				Image* src, VGint sx, VGint sy,
				VGint width, VGint height) {
	}

	void Image::vgGetPixels(Image* dst, VGint dx, VGint dy,
				VGint sx, VGint sy,
				VGint width, VGint height) {
	}

	void Image::vgWritePixels(const void * data, VGint dataStride,
				  VGImageFormat dataFormat,
				  VGint dx, VGint dy,
				  VGint width, VGint height) {
	}

	void Image::vgReadPixels(void * data, VGint dataStride,
				 VGImageFormat dataFormat,
				 VGint sx, VGint sy,
				 VGint width, VGint height) {
	}

	void Image::vgCopyPixels(VGint dx, VGint dy,
				 VGint sx, VGint sy,
				 VGint width, VGint height) {
	}


	/* inherited virtual interface */
	void Image::vgSetParameterf(VGint paramType, VGfloat value) {
	}

	void Image::vgSetParameteri(VGint paramType, VGint value) {
	}

	void Image::vgSetParameterfv(VGint paramType, VGint count, const VGfloat *values) {
	}

	void Image::vgSetParameteriv(VGint paramType, VGint count, const VGint *values) {
	}


	VGfloat Image::vgGetParameterf(VGint paramType) {
		Context::get_current()->set_error(VG_ILLEGAL_ARGUMENT_ERROR);
		return 0.0f;
	}

	VGint Image::vgGetParameteri(VGint paramType) {
		switch(paramType) {
		case VG_IMAGE_FORMAT:
			return VG_sRGBA_8888;
		case VG_IMAGE_WIDTH:
			return framebuffer.width;
		case VG_IMAGE_HEIGHT:
			return framebuffer.height;
		}
		Context::get_current()->set_error(VG_ILLEGAL_ARGUMENT_ERROR);
		return -1;
	}


	VGint Image::vgGetParameterVectorSize(VGint paramType) {
		return 1;
	}


	void Image::vgGetParameterfv(VGint paramType, VGint count, VGfloat *values) {
		Context::get_current()->set_error(VG_ILLEGAL_ARGUMENT_ERROR);
	}

	void Image::vgGetParameteriv(VGint paramType, VGint count, VGint *values) {
		if(count != 1)
			Context::get_current()->set_error(VG_ILLEGAL_ARGUMENT_ERROR);
		switch(paramType) {
		case VG_IMAGE_FORMAT:
		case VG_IMAGE_WIDTH:
		case VG_IMAGE_HEIGHT:
			values[0] = vgGetParameteri(paramType);
			return;
		}
		Context::get_current()->set_error(VG_ILLEGAL_ARGUMENT_ERROR);
	}

};

using namespace gnuVG;

extern "C" {
	VGImage VG_API_ENTRY vgCreateImage(VGImageFormat format,
					   VGint width, VGint height,
					   VGbitfield allowedQuality) VG_API_EXIT {
		auto ctx = Context::get_current();
		if(ctx) {
			try {
				auto image =
					Object::create<Image>(ctx, format, width, height, allowedQuality);
				return (VGImage)image->get_handle();
			} catch(Image::FailedToCreateImageException e) {
				GNUVG_ERROR("vgCreateImage() - Failed to prepare framebuffer.\n");
			}
		} else
			GNUVG_ERROR("vgCreateImage() - No valid context available.\n");

		return VG_INVALID_HANDLE;
	}

	void VG_API_ENTRY vgDestroyImage(VGImage image) VG_API_EXIT {
		Object::dereference(image);
	}

	void VG_API_ENTRY gnuVG_render_to_image(VGImage image) VG_API_EXIT {
		auto i = Object::get<Image>(image);
		if(i)
			Context::get_current()->render_to_framebuffer(i->get_framebuffer());
	}

	void VG_API_ENTRY vgClearImage(VGImage image,
				       VGint x, VGint y, VGint width, VGint height) VG_API_EXIT {
		auto i = Object::get<Image>(image);
		if(i)
			i->vgClearImage(x, y, width, height);
	}

	void VG_API_ENTRY vgImageSubData(VGImage image,
					 const void * data, VGint dataStride,
					 VGImageFormat dataFormat,
					 VGint x, VGint y, VGint width, VGint height) VG_API_EXIT {
	}

	void VG_API_ENTRY vgGetImageSubData(VGImage image,
					    void * data, VGint dataStride,
					    VGImageFormat dataFormat,
					    VGint x, VGint y,
					    VGint width, VGint height) VG_API_EXIT {
	}

	VGImage VG_API_ENTRY vgChildImage(VGImage parent,
					  VGint x, VGint y, VGint width, VGint height) VG_API_EXIT {
		return VG_INVALID_HANDLE;
	}

	VGImage VG_API_ENTRY vgGetParent(VGImage image) VG_API_EXIT {
		return VG_INVALID_HANDLE;
	}

	void VG_API_ENTRY vgCopyImage(VGImage dst, VGint dx, VGint dy,
				      VGImage src, VGint sx, VGint sy,
				      VGint width, VGint height,
				      VGboolean dither) VG_API_EXIT {
	}

	void VG_API_ENTRY vgDrawImage(VGImage image) VG_API_EXIT {
	}

	void VG_API_ENTRY vgSetPixels(VGint dx, VGint dy,
				      VGImage src, VGint sx, VGint sy,
				      VGint width, VGint height) VG_API_EXIT {
	}

	void VG_API_ENTRY vgGetPixels(VGImage dst, VGint dx, VGint dy,
				      VGint sx, VGint sy,
				      VGint width, VGint height) VG_API_EXIT {
	}

	void VG_API_ENTRY vgWritePixels(const void * data, VGint dataStride,
					VGImageFormat dataFormat,
					VGint dx, VGint dy,
					VGint width, VGint height) VG_API_EXIT {
	}

	void VG_API_ENTRY vgReadPixels(void * data, VGint dataStride,
				       VGImageFormat dataFormat,
				       VGint sx, VGint sy,
				       VGint width, VGint height) VG_API_EXIT {
	}

	void VG_API_ENTRY vgCopyPixels(VGint dx, VGint dy,
				       VGint sx, VGint sy,
				       VGint width, VGint height) VG_API_EXIT {
	}

}
