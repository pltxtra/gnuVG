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

#ifndef __GNUVG_OBJECT_HH
#define __GNUVG_OBJECT_HH

#include <VG/openvg.h>

namespace gnuVG {

	class Object {
	private:
		unsigned int reference_counter = 0;

	public:
		virtual ~Object() {};

		static void reference(Object* ptr);
		static void dereference(Object* ptr);

		virtual void vgSetParameterf(VGint paramType, VGfloat value) = 0;
		virtual void vgSetParameteri(VGint paramType, VGint value) = 0;
		virtual void vgSetParameterfv(VGint paramType, VGint count, const VGfloat *values) = 0;
		virtual void vgSetParameteriv(VGint paramType, VGint count, const VGint *values) = 0;

		virtual VGfloat vgGetParameterf(VGint paramType) = 0;
		virtual VGint vgGetParameteri(VGint paramType) = 0;

		virtual VGint vgGetParameterVectorSize(VGint paramType) = 0;

		virtual void vgGetParameterfv(VGint paramType, VGint count, VGfloat *values) = 0;
		virtual void vgGetParameteriv(VGint paramType, VGint count, VGint *values) = 0;
	};
}

#endif
