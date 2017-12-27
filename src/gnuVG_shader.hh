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

#pragma once

#include <map>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

namespace gnuVG {
	class Shader {
	public:
		enum Values {
			gauss_krn_diameter_shift = 16,
		};
		enum Capabilities {
			// primary mode
			do_flat_color		= 0x00000000,
			do_linear_gradient	= 0x00000001,
			do_radial_gradient	= 0x00000002,
			do_pattern		= 0x00000003,
			do_texture		= 0x00000004,

			primary_mode_mask	= 0x0000000f,

			// gradient spread mode
			do_gradient_pad		= 0x00000010,
			do_gradient_repeat	= 0x00000020,
			do_gradient_reflect	= 0x00000030,

			gradient_spread_mask	= 0x00000030,

			// additional flags
			do_mask			= 0x00000100,
			do_pretranslate		= 0x00000200,
			do_horizontal_gaussian	= 0x00000400,
			do_vertical_gaussian	= 0x00000800,
			do_color_transform	= 0x01000000,
			do_texture_alpha	= 0x02000000,

			// gaussian kernel configuration
			gauss_krn_diameter_mask = 0x00ff0000,
		};

		enum Blending {
			blend_src_over      	= 0x00000000, // default
			blend_src		= 0x00001000,
			blend_dst_over      	= 0x00002000,
			blend_src_in      	= 0x00003000,
			blend_dst_in      	= 0x00004000,
			blend_multiply      	= 0x00005000,
			blend_screen      	= 0x00006000,
			blend_darken      	= 0x00007000,
			blend_lighten      	= 0x00008000,
			blend_additive      	= 0x00009000
		};

		static GLuint create_program(const char *vertexshader_source,
					     const char *fragmentshader_source);

		// returns a shader program ID number
		static const Shader* get_shader(int capabilities);

		void use_shader() const;

		void set_blending(Blending bmode) const;

		void set_matrix(const GLfloat *mtrx) const;
		void set_pre_translation(const GLfloat *ptrans) const;

		void set_surf2paint_matrix(const GLfloat *s2p_matrix) const;

		void set_mask_texture(GLuint tex) const;

		void set_pattern_size(GLint width_in_pixels, GLint height_in_pixels) const;
		void set_pattern_matrix(const GLfloat *mtrx) const;
		void set_pattern_texture(GLuint tex) const;
		void set_wrap_mode(GLuint wrap_mode) const;

		void set_color_transform(const GLfloat *scale,
					 const GLfloat *bias) const;

		void set_color(const GLfloat *clr) const;
		void set_linear_parameters(const GLfloat *vec) const;
		void set_radial_parameters(const GLfloat *vec) const;
		void set_color_ramp(GLuint max_stops,
				    const GLfloat *offsets,
				    const GLfloat *invfactor,
				    const GLfloat *colors) const;

		void load_2dvertex_array(const GLfloat *verts, GLint stride) const;
		void load_2dvertex_texture_array(const GLfloat *verts, GLint stride) const;
		void set_texture(GLuint tex) const;
		void set_texture_matrix(const GLfloat *mtrx_3by3) const;
		void render_triangles(GLint first, GLsizei count) const;
		void render_elements(const GLuint *indices, GLsizei nr_indices) const;

	private:
		static std::map<int, Shader*> shader_library;

		GLfloat pixel_width, pixel_height;

		GLuint program_id;

		/* shader handles */
		GLint position_handle;

		GLint textureCoord_handle;
		GLint textureMatrix_handle;
		GLint textureSampler_handle;

		GLint ColorHandle;
		GLint Matrix;
		GLint preTranslation;

		GLint maskTexture;

		GLint pxl_sizes;
		GLint patternTexture, patternMatrix;

		GLint surf2paint;

		GLint ctransform_scale, ctransform_bias;

		GLint linear_normal;
		GLint linear_start;
		GLint linear_length;

		GLint radial;
		GLint radius2;
		GLint radial_denom;

		GLint stop_colors;
		GLint stop_invfactor;
		GLint stop_offsets;
		GLint nr_stops;

		Shader(int caps);

		std::string build_vertex_shader(int caps);
		std::string build_fragment_shader(int caps);
	};
};
