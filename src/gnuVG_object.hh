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
#include <memory>
#include <typeinfo>

#include "gnuVG_error.hh"

namespace gnuVG {

	class Context;

	class Object  : public std::enable_shared_from_this<Object> {
	private:
		bool properly_created = false;
		VGHandle obj_handle = VG_INVALID_HANDLE;

		void reference();
		static std::shared_ptr<Object> get_by_handle(VGHandle handle);

	public:

		class ObjectImproperlyCreated : public std::runtime_error {
		public:
			ObjectImproperlyCreated()
				: runtime_error("gnuVG Object not created with Object::create().")
				{}
			virtual ~ObjectImproperlyCreated() {}
		};

		Object();
		virtual ~Object();

		inline VGHandle get_handle() {
			if(!properly_created) {
				throw ObjectImproperlyCreated();
			}
			return obj_handle;
		}
		virtual void vgSetParameterf(VGint paramType, VGfloat value) = 0;
		virtual void vgSetParameteri(VGint paramType, VGint value) = 0;
		virtual void vgSetParameterfv(VGint paramType, VGint count, const VGfloat *values) = 0;
		virtual void vgSetParameteriv(VGint paramType, VGint count, const VGint *values) = 0;

		virtual VGfloat vgGetParameterf(VGint paramType) = 0;
		virtual VGint vgGetParameteri(VGint paramType) = 0;

		virtual VGint vgGetParameterVectorSize(VGint paramType) = 0;

		virtual void vgGetParameterfv(VGint paramType, VGint count, VGfloat *values) = 0;
		virtual void vgGetParameteriv(VGint paramType, VGint count, VGint *values) = 0;

		static void dereference(VGHandle handle);

		template<typename T>
		static std::shared_ptr<T> get(VGHandle handle) {
			auto obj = std::static_pointer_cast<T>(get_by_handle(handle));
			return obj;
		}

		template<class T, class ... Args>
		static std::shared_ptr<T> create(Args && ... args) {
			std::shared_ptr<T> ptr = std::make_shared<T>(args...);
			ptr->reference();
			return ptr;
		}
	};
}

#endif
