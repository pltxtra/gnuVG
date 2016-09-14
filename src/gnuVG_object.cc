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
#include "gnuVG_object.hh"

#define __DO_GNUVG_DEBUG
#include "gnuVG_debug.hh"

namespace gnuVG {
	void Object::reference(Object* ptr) {
		ptr->reference_counter++;
	}

	void Object::dereference(Object* ptr) {
		if(ptr->reference_counter)
			ptr->reference_counter--;
		else
			delete ptr;
	}
};
