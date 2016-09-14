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

#ifndef __GNUVG_IMAGE_HH
#define __GNUVG_IMAGE_HH

#include <VG/openvg.h>

#include "gnuVG_context.hh"
#include "gnuVG_object.hh"

namespace gnuVG {

	class Image : public Object {
	private:
		Image* parent = NULL;
		Context::FrameBuffer framebuffer;

	public:
		class FailedToCreateImageException {};

		const Context::FrameBuffer* get_framebuffer() {
			return &framebuffer;
		}

		Image(Context* ctx, VGImageFormat format,
		      VGint width, VGint height,
		      VGbitfield allowedQuality);
		virtual ~Image();

		/* OpenVG equivalent API - image manipulation */
		void vgClearImage(VGint x, VGint y, VGint width, VGint height);
		void vgImageSubData(const void * data, VGint dataStride,
				    VGImageFormat dataFormat,
				    VGint x, VGint y, VGint width, VGint height);
		void vgGetImageSubData(void * data, VGint dataStride,
				       VGImageFormat dataFormat,
				       VGint x, VGint y,
				       VGint width, VGint height);
		Image* vgChildImage(VGint x, VGint y, VGint width, VGint height);
		Image* vgGetParent();
		void vgCopyImage(VGint dx, VGint dy,
				 Image* src, VGint sx, VGint sy,
				 VGint width, VGint height,
				 VGboolean dither);
		void vgDrawImage();
		static void vgSetPixels(VGint dx, VGint dy,
					Image* src, VGint sx, VGint sy,
					VGint width, VGint height);
		static void vgWritePixels(const void * data, VGint dataStride,
					  VGImageFormat dataFormat,
					  VGint dx, VGint dy,
					  VGint width, VGint height);
		static void vgGetPixels(Image* dst, VGint dx, VGint dy,
					VGint sx, VGint sy,
					VGint width, VGint height);
		static void vgReadPixels(void * data, VGint dataStride,
					 VGImageFormat dataFormat,
					 VGint sx, VGint sy,
					 VGint width, VGint height);
		static void vgCopyPixels(VGint dx, VGint dy,
					 VGint sx, VGint sy,
					 VGint width, VGint height);

		/* inherited virtual interface */
		virtual void vgSetParameterf(VGint paramType, VGfloat value) override;
		virtual void vgSetParameteri(VGint paramType, VGint value) override;
		virtual void vgSetParameterfv(VGint paramType, VGint count, const VGfloat *values) override;
		virtual void vgSetParameteriv(VGint paramType, VGint count, const VGint *values) override;

		virtual VGfloat vgGetParameterf(VGint paramType) override;
		virtual VGint vgGetParameteri(VGint paramType) override;

		virtual VGint vgGetParameterVectorSize(VGint paramType) override;

		virtual void vgGetParameterfv(VGint paramType, VGint count, VGfloat *values) override;
		virtual void vgGetParameteriv(VGint paramType, VGint count, VGint *values) override;

	};
}

#endif
