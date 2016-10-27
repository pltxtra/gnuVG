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

using namespace gnuVG;

extern "C" {
	void VG_API_ENTRY vgGaussianBlur(VGImage dst, VGImage src,
					 VGfloat stdDeviationX,
					 VGfloat stdDeviationY,
					 VGTilingMode tilingMode) VG_API_EXIT {
		auto d = Object::get<Image>(dst);
		auto s = Object::get<Image>(src);
		if(s) {
			auto sfb = s->get_framebuffer();
			auto ctx = Context::get_current();

			// calculate matrix size
			stdDeviationX = stdDeviationX * 8.0 + 1.0;
			stdDeviationY = stdDeviationY * 8.0 + 1.0;

			// if d is found, render to d, otherwise
			// render to current target - this is a gnuVG extension
			if(d) {
				auto dfb = d->get_framebuffer();
				ctx->save_current_framebuffer();
				ctx->render_to_framebuffer(dfb);
			}
			ctx->trivial_render_framebuffer(
				sfb,
				(int)stdDeviationX,
				(int)stdDeviationY
				);
			if(d)
				ctx->restore_current_framebuffer();
		}
	}
}
