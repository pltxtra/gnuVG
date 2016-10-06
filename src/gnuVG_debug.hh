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

#pragma once

#define WHERESTR  "[file %s, line %d]: "
#define WHEREARG  __FILE__, __LINE__

#ifdef __DISABLE_GNUVG_DEBUG_GLOBAL
#warn "Disabling __DO_GNUVG_DEBUG"
#undef __DO_GNUVG_DEBUG
#endif

#ifdef __DO_GNUVG_DEBUG

#ifdef __ANDROID__

#include <android/log.h>
#define GNUVG_DEBUG(...)       __android_log_print(ANDROID_LOG_INFO, "GNUVG_NDK", __VA_ARGS__)

#else

#include <stdio.h>
#define GNUVG_DEBUG(...)  printf(__VA_ARGS__)

#endif

#else
// disable debugging

#define GNUVG_DEBUG(...)

#endif

#ifdef __ANDROID__

#include <android/log.h>
#define GNUVG_ERROR(...)       __android_log_print(ANDROID_LOG_ERROR, "GNUVG_NDK", __VA_ARGS__)

#else

#include <stdio.h>
#define GNUVG_ERROR(...)  printf(__VA_ARGS__)

#endif

#include <GLES2/gl2.h>
static inline void checkGlError(const char* op) {
	for (GLint error = glGetError(); error; error
		     = glGetError()) {
		GNUVG_ERROR("after %s() glError (0x%x)\n", op, error);
	}
}
