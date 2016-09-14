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

#include <time.h>
#include <string>
#include <map>

#ifndef ENABLE_GNUVG_PROFILER

#define GNUVG_DECLARE_PROFILER_PROBE(NAME)
#define GNUVG_APPLY_PROFILER_GUARD(NAME)
#define GNUVG_GET_PROFILER_TIME(NAME)
#define GNUVG_GET_PROFILER_COUNT(NAME)

#define GNUVG_READ_PROFILER_COUNTER(NAME)

#define ADD_GNUVG_PROFILER_PROBE(NAME)
#define ADD_GNUVG_PROFILER_COUNTER(NAME, VALUE)
#define FOR_ALL_GNUVG_PROFILE_PROBES(CODE)

#else

#define GNUVG_DECLARE_PROFILER_PROBE(NAME) gnuVG::ProfileProbe NAME##_probe(#NAME)
#define GNUVG_APPLY_PROFILER_GUARD(NAME) gnuVG::ProbeGuard NAME##_guard(&NAME##_probe)
#define GNUVG_GET_PROFILER_TIME(NAME) gnuVG::ProfileProbe::get(#NAME)->get_total_time_in_seconds()
#define GNUVG_GET_PROFILER_COUNT(NAME) gnuVG::ProfileProbe::get(#NAME)->get_count()

#define GNUVG_READ_PROFILER_COUNTER(NAME) gnuVG::ProfilerCounter::read_and_clear(#NAME)

#define ADD_GNUVG_PROFILER_PROBE(NAME) \
	static gnuVG::ProfileProbe NAME##_gvg_probe(#NAME); \
	gnuVG::ProbeGuard NAME##_gvg_probe_guard(&NAME##_gvg_probe)

#define ADD_GNUVG_PROFILER_COUNTER(NAME, VALUE)	\
	static gnuVG::ProfilerCounter NAME##_gvg_counter(#NAME, 0);	\
	NAME##_gvg_counter.add(VALUE);

#define FOR_ALL_GNUVG_PROFILE_PROBES(CODE) \
	for(auto prb : gnuVG::ProfileProbe::get_all()) { \
		auto gvgp_name = prb.first.c_str();		\
		auto gvgp_time = prb.second->get_total_time_in_seconds();	\
		auto gvgp_count = prb.second->get_count(); \
		CODE; \
	}

namespace gnuVG {

	class ProfilerCounter {
	private:
		static std::map<std::string, ProfilerCounter*> all_counters;

		std::string name;
		int value = 0;

	public:
		ProfilerCounter(const std::string &_name, int start_value)
			: name(_name)
			, value(start_value) {
			all_counters[_name] = this;
		}

		void add(int added_value) {
			value += added_value;
		}

		static int read_and_clear(const std::string &_name) {
			int retval = -1;
			auto item = all_counters.find(_name);
			if(item != all_counters.end()) {
				retval = (*item).second->value;
				(*item).second->value = 0;
			}
			return retval;
		}
	};

	class ProfileProbe {
	private:
		static std::map<std::string, ProfileProbe*> all_probes;

		std::string name;
		struct timespec total_time;
		struct timespec start_time;
		int count = 0;

	public:
		ProfileProbe(const std::string &_name) : name(_name) {
			total_time.tv_sec = 0;
			total_time.tv_nsec = 0;
			all_probes[name] = this;
		}

		static std::map<std::string, ProfileProbe*> get_all() {
			return all_probes;
		}

		static ProfileProbe* get(const std::string &name) {
			return all_probes[name];
		}

		std::string get_name() {
			return name;
		}

		void start_timer() {
			(void) clock_gettime(CLOCK_MONOTONIC_RAW, &start_time);
		}

		void stop_timer() {
			++count;

			struct timespec stop_time;
			(void) clock_gettime(CLOCK_MONOTONIC_RAW, &stop_time);

			stop_time.tv_sec -= start_time.tv_sec;
			stop_time.tv_nsec -= start_time.tv_nsec;
			if(stop_time.tv_nsec < 0) {
				stop_time.tv_sec  -= 1;
				stop_time.tv_nsec += 1000000000;
			}

			total_time.tv_sec += stop_time.tv_sec;
			total_time.tv_nsec += stop_time.tv_nsec;
			if(total_time.tv_nsec > 1000000000) {
				total_time.tv_sec  += 1;
				total_time.tv_nsec -= 1000000000;
			}
		}

		double get_total_time_in_seconds() {
			return (double)total_time.tv_sec +
				(double)total_time.tv_nsec / 1000000000.0;
		}

		int get_count() {
			return count;
		}
	};

	class ProbeGuard {
	private:
		ProfileProbe *prb;
	public:
		ProbeGuard(ProfileProbe *p) {
			prb = p;
			prb->start_timer();
		}

		~ProbeGuard() {
			prb->stop_timer();
		}
	};
};

#endif
