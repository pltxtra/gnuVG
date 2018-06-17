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

#include "gnuVG_path.hh"

//#define __DO_GNUVG_DEBUG
#include "gnuVG_debug.hh"

#define ENABLE_GNUVG_PROFILER
#include <VG/gnuVG_profiler.hh>

#define TESS_POLY_SIZE 3

namespace gnuVG {

	inline void Path::cleanup_path() {
		simplified.simplify_path(s_segments.data(),
					 s_coordinates.data(),
					 s_segments.size());
		// get top left, bottom right
		simplified.get_bounding_box(bounding_box);

		// calculate the top right, bottom left
		auto w = bounding_box[1].x - bounding_box[0].x;
		auto h = bounding_box[1].y - bounding_box[0].y;
		bounding_box[2].x = bounding_box[0].x + w;
		bounding_box[2].y = bounding_box[0].y;
		bounding_box[3].x = bounding_box[0].x;
		bounding_box[3].y = bounding_box[0].y + h;
		path_dirty = false;
	}

	void Path::vgSetParameterf(VGint paramType, VGfloat value) {
		switch(paramType) {
		case VG_PATH_FORMAT:
		case VG_PATH_DATATYPE:
		case VG_PATH_NUM_SEGMENTS:
		case VG_PATH_NUM_COORDS:
			Context::get_current()->set_error(VG_ILLEGAL_ARGUMENT_ERROR);
			break;

		case VG_PATH_BIAS:
			bias = value;
			break;
		case VG_PATH_SCALE:
			scale = value;
			break;
		}
	}

	void Path::vgSetParameteri(VGint paramType, VGint value) {
		Context::get_current()->set_error(VG_ILLEGAL_ARGUMENT_ERROR);
	}

	void Path::vgSetParameterfv(VGint paramType, VGint count, const VGfloat *values) {
		if(count != 1) {
			Context::get_current()->set_error(VG_ILLEGAL_ARGUMENT_ERROR);
			return;
		}
		vgSetParameterf(paramType, values[0]);
	}

	void Path::vgSetParameteriv(VGint paramType, VGint count, const VGint *values) {
		if(count != 1) {
			Context::get_current()->set_error(VG_ILLEGAL_ARGUMENT_ERROR);
			return;
		}
		vgSetParameteri(paramType, values[0]);
	}

	VGfloat Path::vgGetParameterf(VGint paramType) {
		switch(paramType) {
		case VG_PATH_FORMAT:
		case VG_PATH_DATATYPE:
		case VG_PATH_NUM_SEGMENTS:
		case VG_PATH_NUM_COORDS:
			break;

		case VG_PATH_BIAS:
			return bias;

		case VG_PATH_SCALE:
			return scale;
		}

		Context::get_current()->set_error(VG_ILLEGAL_ARGUMENT_ERROR);
		return 0.0f;
	}

	VGint Path::vgGetParameteri(VGint paramType) {
		switch(paramType) {
		case VG_PATH_FORMAT:
			return VG_PATH_FORMAT_STANDARD;
		case VG_PATH_DATATYPE:
			return VG_PATH_DATATYPE_F;
		case VG_PATH_NUM_SEGMENTS:
			return (VGint)s_segments.size();
		case VG_PATH_NUM_COORDS:
			return (VGint)s_coordinates.size();

		case VG_PATH_BIAS:
		case VG_PATH_SCALE:
			break;
		}

		Context::get_current()->set_error(VG_ILLEGAL_ARGUMENT_ERROR);
		return 0.0f;
	}

	VGint Path::vgGetParameterVectorSize(VGint paramType) {
		switch(paramType) {
		case VG_PATH_FORMAT:
		case VG_PATH_DATATYPE:
		case VG_PATH_NUM_SEGMENTS:
		case VG_PATH_NUM_COORDS:
		case VG_PATH_BIAS:
		case VG_PATH_SCALE:
			return 1;
		}

		Context::get_current()->set_error(VG_ILLEGAL_ARGUMENT_ERROR);
		return 0;
	}

	void Path::vgGetParameterfv(VGint paramType, VGint count, VGfloat *values) {
		if(count == 1)
			values[0] = vgGetParameterf(paramType);
		else
			Context::get_current()->set_error(VG_ILLEGAL_ARGUMENT_ERROR);
	}

	void Path::vgGetParameteriv(VGint paramType, VGint count, VGint *values) {
		if(count == 1)
			values[0] = vgGetParameteri(paramType);
		else
			Context::get_current()->set_error(VG_ILLEGAL_ARGUMENT_ERROR);
	}


	Path::Path(VGPathDatatype _dataType,
		   VGfloat _scale, VGfloat _bias,
		   VGbitfield _capabilities)
		: dataType(_dataType)
		, scale(_scale)
		, bias(_bias)
		, capabilities(_capabilities)
		, path_dirty(false)
	{
	}


	Path::~Path() {
		vgClearPath(capabilities);
	}

	void Path::vgClearPath(VGbitfield _capabilities) {
		path_dirty = true;
		s_segments.clear();
		s_coordinates.clear();
	}

	VGbitfield Path::vgGetPathCapabilities() {
		return capabilities;
	}


	void Path::vgAppendPath(std::shared_ptr<Path> srcPath) {
		path_dirty = true;
		/* XXX not implemented */
	}

	void Path::vgModifyPathCoords(VGint startIndex, VGint numSegments, const void *pathData) {
		path_dirty = true;
		/* XXX not implemented */
	}

	void Path::vgTransformPath(std::shared_ptr<Path> srcPath) {
		path_dirty = true;
		/* XXX not implemented */
	}

	VGboolean Path::vgInterpolatePath(std::shared_ptr<Path> startPath,
					  std::shared_ptr<Path> endPath, VGfloat amount) {
		path_dirty = true;
		/* XXX not implemented */
		return VG_FALSE;
	}

	VGfloat Path::vgPathLength(VGint startSegment, VGint numSegments) {
		/* XXX not implemented */
		return 0.0f;
	}

	void Path::vgPointAlongPath(VGint startSegment, VGint numSegments,
				    VGfloat distance,
				    VGfloat *x, VGfloat *y,
				    VGfloat *tangentX, VGfloat *tangentY) {
		/* XXX not implemented */
	}

	void Path::vgPathBounds(VGfloat * minX, VGfloat * minY,
				VGfloat * width, VGfloat * height) {
		if(path_dirty) cleanup_path();

		*minX	= bounding_box[0].x;
		*minY	= bounding_box[0].y;
		*width	= bounding_box[1].x - bounding_box[0].x;
		*height	= bounding_box[1].y - bounding_box[0].y;
	}

	void Path::vgPathTransformedBounds(VGfloat * minX, VGfloat * minY,
					   VGfloat * width, VGfloat * height) {
		if(path_dirty) cleanup_path();

		VGfloat sp_ep[4];

		Context::get_current()->transform_bounding_box(bounding_box, sp_ep);

		*minX	= sp_ep[0];
		*minY	= sp_ep[1];
		*width	= sp_ep[2] - sp_ep[0];
		*height	= sp_ep[3] - sp_ep[1];
	}

	static TESSalloc ma;
	static TESSalloc* ma_p = nullptr;

	void Path::vgDrawPath_reset_tesselator() {
		if(!ma_p) {
			ma_p = &ma;
			memset(ma_p, 0, sizeof(ma));
			ma.memalloc = GvgAllocator::gvg_alloc;
			ma.memfree = GvgAllocator::gvg_free;
			ma.memrealloc = GvgAllocator::gvg_realloc;

		}
		if(!tess)
			tess = tessNewTess(ma_p);
	}

	void Path::vgDrawPath_fill_regular() {
		const GLfloat *vertices = NULL;
		const GLuint *indices = NULL;
		GLsizei nr_indices;

		bool was_tesselated;

		{
			ADD_GNUVG_PROFILER_PROBE(path_tesselate);
			was_tesselated = tessTesselate(tess,
						       TESS_WINDING_ODD,
						       TESS_POLYGONS, TESS_POLY_SIZE, 2, 0) ? true : false;
		}

		GNUVG_DEBUG("vgDrawPath_fill_regular()\n");
		if(was_tesselated) {
			{
				ADD_GNUVG_PROFILER_PROBE(path_get_tesselation);
				vertices = (const GLfloat *) tessGetVertices(tess);
				indices = (const GLuint *)      tessGetElements(tess);
				nr_indices = (GLsizei)          tessGetElementCount(tess) * TESS_POLY_SIZE;
			}

			if(vertices != NULL && indices != NULL) {
//			GNUVG_DEBUG("    vgDrawPath: vertices(%p), indices(%p), nr indices(%d)\n", vertices, indices, nr_indices);
				Context::get_current()->use_pipeline(Context::GNUVG_SIMPLE_PIPELINE,
								     VG_FILL_PATH);
				Context::get_current()->load_2dvertex_array(vertices, 0);
				Context::get_current()->render_elements(indices, nr_indices);
			}
		}
	}

	void Path::vgDrawPath(VGbitfield paintModes) {
		if(path_dirty) cleanup_path();

		if(paintModes & VG_FILL_PATH) {
			{
				ADD_GNUVG_PROFILER_PROBE(process_subpaths);
				vgDrawPath_reset_tesselator();
				simplified.tesselate_fill_shape(tess);
			}
			vgDrawPath_fill_regular();
		}

		if(paintModes & VG_STROKE_PATH) {
			ADD_GNUVG_PROFILER_PROBE(path_stroke);

			Context::get_current()->use_pipeline(
				Context::GNUVG_SIMPLE_PIPELINE,
				VG_STROKE_PATH);

			simplified.get_stroke_shape(
				[](const SimplifiedPath::StrokeData &stroke_data) {
					if(stroke_data.nr_vertices) {
						Context::get_current()->load_2dvertex_array(
							stroke_data.vertices, 0);
						Context::get_current()->render_elements(
							stroke_data.indices,
							stroke_data.nr_indices);
					}
				}
				);
		}
		Context::get_current()->calculate_bounding_box(bounding_box);
	}
}

using namespace gnuVG;

extern "C" {
	VGPath VG_API_ENTRY vgCreatePath(VGint pathFormat, VGPathDatatype datatype,
					 VGfloat scale, VGfloat bias,
					 VGint segmentCapacityHint, VGint coordCapacityHint,
					 VGbitfield capabilities ) VG_API_EXIT {
		auto path =
			Object::create<Path>(datatype, scale, bias, capabilities);

		if(path)
			return (VGPath)path->get_handle();

		return VG_INVALID_HANDLE;
	}

	void VG_API_ENTRY vgDestroyPath(VGPath path) VG_API_EXIT {
		Object::dereference(path);
	}

	void VG_API_ENTRY vgClearPath(VGPath path, VGbitfield capabilities) VG_API_EXIT {
		auto p = Object::get<Path>(path);
		if(p)
			p->vgClearPath(capabilities);
	}

	void VG_API_ENTRY vgRemovePathCapabilities(VGPath path,
						   VGbitfield capabilities) VG_API_EXIT {
	}

	VGbitfield VG_API_ENTRY vgGetPathCapabilities(VGPath path) VG_API_EXIT {
		auto p = Object::get<Path>(path);
		if(p)
			return p->vgGetPathCapabilities();
		return 0;
	}

	void VG_API_ENTRY vgAppendPath(VGPath dstPath, VGPath srcPath) VG_API_EXIT {
		auto sp = Object::get<Path>(srcPath);
		auto dp = Object::get<Path>(dstPath);

		if(sp && dp)
			dp->vgAppendPath(sp);
	}

	void VG_API_ENTRY vgAppendPathData(VGPath path, VGint numSegments,
					   const VGubyte *pathSegments, const void *pathData) VG_API_EXIT {
		auto p = Object::get<Path>(path);
		if(!p)
			return;

		switch(p->get_dataType()) {
		case VG_PATH_DATATYPE_FORCE_SIZE:
			break;
		case VG_PATH_DATATYPE_S_8:
			p->vgAppendPathData<int8_t>(numSegments, pathSegments, (const int8_t *)pathData);
			break;
		case VG_PATH_DATATYPE_S_16:
			p->vgAppendPathData<int16_t>(numSegments, pathSegments, (const int16_t *)pathData);
			break;
		case VG_PATH_DATATYPE_S_32:
			p->vgAppendPathData<int32_t>(numSegments, pathSegments, (const int32_t *)pathData);
			break;
		case VG_PATH_DATATYPE_F:
			p->vgAppendPathData<VGfloat>(numSegments, pathSegments, (const VGfloat *)pathData);
			break;
		}
	}

	void VG_API_ENTRY vgModifyPathCoords(VGPath dstPath, VGint startIndex,
					     VGint numSegments,
					     const void * pathData) VG_API_EXIT {
	}

	void VG_API_ENTRY vgTransformPath(VGPath dstPath, VGPath srcPath) VG_API_EXIT {
		auto sp = Object::get<Path>(srcPath);
		auto dp = Object::get<Path>(dstPath);
		if(dp && sp)
			dp->vgTransformPath(sp);
	}

	VGboolean VG_API_ENTRY vgInterpolatePath(VGPath dstPath,
						 VGPath startPath,
						 VGPath endPath,
						 VGfloat amount) VG_API_EXIT {
		auto sp = Object::get<Path>(startPath);
		auto dp = Object::get<Path>(dstPath);
		auto ep = Object::get<Path>(endPath);

		if(sp && ep && dp)
			return dp->vgInterpolatePath(sp, ep, amount);

		return VG_FALSE;
	}

	VGfloat VG_API_ENTRY vgPathLength(VGPath path,
					  VGint startSegment, VGint numSegments) VG_API_EXIT {
		auto p = Object::get<Path>(path);
		if(!p)
			return 0.0f;
		return p->vgPathLength(startSegment, numSegments);
	}

	void VG_API_ENTRY vgPointAlongPath(VGPath path,
					   VGint startSegment, VGint numSegments,
					   VGfloat distance,
					   VGfloat * x, VGfloat * y,
					   VGfloat * tangentX, VGfloat * tangentY) VG_API_EXIT {
		auto p = Object::get<Path>(path);
		if(!p)
			return;
		p->vgPointAlongPath(startSegment, numSegments, distance,
				    x, y, tangentX, tangentY);
	}

	void VG_API_ENTRY vgPathBounds(VGPath path,
				       VGfloat *minX, VGfloat *minY,
				       VGfloat *width, VGfloat *height) VG_API_EXIT {
		auto p = Object::get<Path>(path);
		if(!p)
			return;
		p->vgPathBounds(minX, minY, width, height);
	}

	void VG_API_ENTRY vgPathTransformedBounds(
		VGPath path,
		VGfloat * minX, VGfloat * minY,
		VGfloat * width, VGfloat * height) VG_API_EXIT {
		auto p = Object::get<Path>(path);
		if(!p)
			return;
		p->vgPathTransformedBounds(minX, minY, width, height);
	}

	void VG_API_ENTRY vgDrawPath(VGPath path, VGbitfield paintModes) VG_API_EXIT {
		auto p = Object::get<Path>(path);
		if(!p)
			return;
		if(!(paintModes & (VG_FILL_PATH | VG_STROKE_PATH)))
			return;

		auto ctx = Context::get_current();
		ctx->select_conversion_matrix(Context::GNUVG_MATRIX_PATH_USER_TO_SURFACE);
		ctx->reset_pre_translation();

//		try {
		p->vgDrawPath(paintModes);
//		} catch(...) {
//			int status = 0;
//			char * buff = __cxxabiv1::__cxa_demangle(
//				__cxxabiv1::__cxa_current_exception_type()->name(),
//				NULL, NULL, &status);
//			GNUVG_ERROR("gnuVG_path.cc caught an unknown exception: %s\n", buff);
//		}
	}

}
