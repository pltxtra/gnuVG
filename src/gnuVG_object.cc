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
#define __DO_GNUVG_DEBUG
#include "gnuVG_debug.hh"

#include "gnuVG_object.hh"
#include "gnuVG_idallocator.hh"

#include "gnuVG_context.hh"

#include <map>
#include <stdlib.h>

namespace gnuVG {
	static std::map<VGHandle, std::shared_ptr<Object> > all_objects;
	static IDAllocator<VGHandle> idalloc(1);

	Object::Object() {}
	Object::~Object() {
		idalloc.free_id(obj_handle);
	}

	void Object::reference() {
		auto o = shared_from_this();
		obj_handle = idalloc.get_id();
		all_objects[obj_handle] = o;
		properly_created = true;
	}

	void Object::dereference(VGHandle handle) {
		auto o = all_objects.find(handle);
		if(o != all_objects.end())
			all_objects.erase(o);
		else
			Context::get_current()->set_error(VG_BAD_HANDLE_ERROR);
	}

	std::shared_ptr<Object> Object::get_by_handle(VGHandle handle) {
		std::shared_ptr<Object> retval;

		auto o = all_objects.find(handle);
		if(o != all_objects.end())
			retval = (*o).second;
		else
			Context::get_current()->set_error(VG_BAD_HANDLE_ERROR);

		return retval;
	}
};
