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
		enum Capabilities {
			// primary mode - painting
			do_flat_color		= 0x00,
			do_linear_gradient	= 0x01,
			do_radial_gradient	= 0x02,
			do_pattern		= 0x03,

			// primary mode - blending
			do_blend_src		= 0x04,
			do_blend_src_over      	= 0x05,
			do_blend_dst_over      	= 0x06,
			do_blend_src_in      	= 0x07,
			do_blend_dst_in      	= 0x08,
			do_blend_multiply      	= 0x09,
			do_blend_screen      	= 0x0a,
			do_blend_darken      	= 0x0b,
			do_blend_lighten      	= 0x0c,
			do_blend_additive      	= 0x0d,
			do_blend_subtract_alpha = 0x0e,

			primary_mode_mask	= 0x0f,

			// gradient spread mode
			do_gradient_pad		= 0x10,
			do_gradient_repeat	= 0x20,
			do_gradient_reflect	= 0x30,
			gradient_spread_mask	= 0x30,

			// mask
			do_mask			= 0x100,

			// pre-translation
			do_pretranslate		= 0x400
		};

		static GLuint create_program(const char *vertexshader_source,
					     const char *fragmentshader_source);

		// returns a shader program ID number
		static const Shader* get_shader(int capabilities);

		void use_shader() const;

		void set_matrix(GLfloat *mtrx) const;
		void set_pre_translation(GLfloat *ptrans) const;

		void set_surf2paint_matrix(GLfloat *s2p_matrix) const;

		void set_mask_texture(GLuint tex) const;

		void set_blend_source_texture(GLuint tex) const;
		void set_blend_destination_texture(GLuint tex) const;

		void set_color(GLfloat *clr) const;
		void set_linear_parameters(GLfloat *vec) const;
		void set_radial_parameters(GLfloat *vec) const;
		void set_color_ramp(GLuint max_stops,
				    GLfloat *offsets,
				    GLfloat *invfactor,
				    GLfloat *colors) const;

		void load_2dvertex_array(const GLfloat *verts, GLint stride) const;
		void load_klm_array(const GLfloat *klm, GLint stride) const;
		void render_triangles(GLint first, GLsizei count) const;
		void render_elements(const GLuint *indices, GLsizei nr_indices) const;

	private:
		static std::map<int, Shader*> shader_library;

		std::string vertex_shader, fragment_shader;
		GLuint program_id;

		GLint position_handle, klm_handle;

		GLint ColorHandle;
		GLint Matrix;
		GLint preTranslation;
		GLint maskTexture;

		GLint blend_sTexture, blend_dTexture;

		GLint surf2paint;

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

		void build_vertex_shader(int caps);
		void build_fragment_shader(int caps);
	};
};
