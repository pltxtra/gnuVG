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

#include "gnuVG_error.hh"

#ifdef __ANDROID__

#include <iostream>
#include <iomanip>

#include <unwind.h>
#include <dlfcn.h>
#include <sstream>

namespace {

	struct BacktraceState
	{
		void** current;
		void** end;
	};

	static _Unwind_Reason_Code unwindCallback(struct _Unwind_Context* context, void* arg)
	{
		BacktraceState* state = static_cast<BacktraceState*>(arg);
		uintptr_t pc = _Unwind_GetIP(context);
		if (pc) {
			if (state->current == state->end) {
				return _URC_END_OF_STACK;
			} else {
				*state->current++ = reinterpret_cast<void*>(pc);
			}
		}
		return _URC_NO_REASON;
	}

}

static size_t captureBacktrace(void** buffer, size_t max)
{
	BacktraceState state = {buffer, buffer + max};
	_Unwind_Backtrace(unwindCallback, &state);

	return state.current - buffer;
}

static void dumpBacktrace(std::ostream& os, void** buffer, size_t count)
{
	for (size_t idx = 0; idx < count; ++idx) {
		const void* addr = buffer[idx];
		const char* symbol = "";

		Dl_info info;
		if (dladdr(addr, &info) && info.dli_sname) {
			symbol = info.dli_sname;
		}

		os << "  #" << std::setw(2) << idx << ": " << addr << "  " << symbol << "\n";
	}
}

void print_trace()
{
	const size_t max = 30;
	void* buffer[max];
	std::ostringstream oss;

	dumpBacktrace(oss, buffer, captureBacktrace(buffer, max));

	GNUVG_ERROR("%s", oss.str().c_str());
}

#else
#include <execinfo.h>
void print_trace() {
	void *array[30];
	size_t size;
	char **strings;
	size_t i;

	size = backtrace (array, 30);
	strings = backtrace_symbols(array, size);

	GNUVG_ERROR("Obtained %zd stack frames.\n", size);

	for (i = 0; i < size; i++)
		GNUVG_ERROR("%s\n", strings[i]);

	free(strings);
}
#endif
