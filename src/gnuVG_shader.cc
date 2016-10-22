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

#include <sstream>
#include "gnuVG_shader.hh"

//#define __DO_GNUVG_DEBUG
#include "gnuVG_debug.hh"

#define ENABLE_GNUVG_PROFILER
#include <VG/gnuVG_profiler.hh>

#include "gnuVG_gaussianblur.hh"

namespace gnuVG {
	std::map<int, Shader*> Shader::shader_library;

	const Shader* Shader::get_shader(int caps) {
		auto found = shader_library.find(caps);
		if(found != shader_library.end()) {
			return (*found).second;
		}

		auto new_shader = new Shader(caps);
		shader_library[caps] = new_shader;
		return new_shader;
	}

	void Shader::use_shader() const {
		glUseProgram(program_id);
		set_blending(blend_src_over);
	}

	void Shader::set_blending(Blending bmode) const {
		switch(bmode) {
		case blend_src: // blend_src equals no blending
			glDisable(GL_BLEND);
			break;

		case blend_dst_over:
		case blend_src_in:
		case blend_dst_in:
		case blend_multiply:
		case blend_screen:
		case blend_darken:
		case blend_lighten:
		case blend_additive:
		default:
		case blend_src_over:
			// everything not supported
			// will default into src_over
			glEnable(GL_BLEND);
			glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			break;
		}
	}

	void Shader::set_matrix(const GLfloat *mtrx) const {
		glUniformMatrix4fv(Matrix, 1, GL_FALSE, mtrx);
	}

	void Shader::set_pre_translation(const GLfloat *ptrans) const {
		glUniform2fv(preTranslation, 1, ptrans);
	}

	void Shader::set_surf2paint_matrix(const GLfloat *s2p_matrix) const {
		glUniformMatrix4fv(surf2paint, 1, GL_FALSE, s2p_matrix);
	}

	void Shader::set_mask_texture(GLuint tex) const {
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, tex);
		glUniform1i(maskTexture, 1);
	}

	void Shader::set_pattern_size(GLint width_in_pixels, GLint height_in_pixels) const {
		auto pixel_width = (GLfloat)1.0 / (GLfloat)width_in_pixels;
		auto pixel_height = (GLfloat)1.0 / (GLfloat)height_in_pixels;

		GLfloat pxs[] = {
			pixel_width,
			pixel_height,
			pixel_width * (GLfloat)0.5,
			pixel_height * (GLfloat)0.5
		};

		glUniform4fv(pxl_sizes, 1, pxs);
	}

	void Shader::set_pattern_matrix(const GLfloat *mtrx) const {
		glUniformMatrix4fv(patternMatrix, 1, GL_FALSE, mtrx);
	}

	void Shader::set_pattern_texture(GLuint tex) const {
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, tex);
		glUniform1i(patternTexture, 2);
	}

	void Shader::set_color(const GLfloat *clr) const {
		glUniform4fv(ColorHandle, 1, clr);
	}

	void Shader::set_linear_parameters(const GLfloat *vec) const {
		glUniform2fv(linear_start, 1, &(vec[0]));
		glUniform2fv(linear_normal, 1, &(vec[2]));
		glUniform1fv(linear_length, 1, &(vec[4]));
	}

	void Shader::set_radial_parameters(const GLfloat *vec) const {
		glUniform4fv(radial, 1, &(vec[0]));
		glUniform1fv(radius2, 1, &(vec[4]));
		glUniform1fv(radial_denom, 1, &(vec[5]));
	}

	void Shader::set_color_ramp(GLuint max_stops,
				    const GLfloat *offsets,
				    const GLfloat *invfactor,
				    const GLfloat *colors) const {
		glUniform1fv(stop_offsets,
			     max_stops, offsets);
		glUniform1fv(stop_invfactor,
			     max_stops, invfactor);
		glUniform4fv(stop_colors,
			     max_stops, colors);
		glUniform1i(nr_stops, max_stops);
	}

	void Shader::load_2dvertex_array(const GLfloat *verts, GLint stride) const {
		ADD_GNUVG_PROFILER_PROBE(SH_load_2dvertex_array);

		glVertexAttribPointer(position_handle, 2, GL_FLOAT, GL_FALSE,
				      stride * sizeof(GLfloat), verts);
		glEnableVertexAttribArray(position_handle);
	}

	void Shader::load_klm_array(const GLfloat *klm, GLint stride) const {
		glVertexAttribPointer(klm_handle, 3, GL_FLOAT, GL_FALSE, stride * sizeof(GLfloat), klm);
		glEnableVertexAttribArray(klm_handle);
	}

	void Shader::render_triangles(GLint first, GLsizei count) const {
		glDrawArrays(GL_TRIANGLES, first, count);
	}

	void Shader::render_elements(const GLuint *indices, GLsizei nr_indices) const {
		ADD_GNUVG_PROFILER_PROBE(SH_render_elements);
		ADD_GNUVG_PROFILER_COUNTER(SH_render_elements, nr_indices);

		glDrawElements(GL_TRIANGLES, nr_indices, GL_UNSIGNED_INT, indices);
	}

	std::string  Shader::build_vertex_shader(int caps) {
		std::stringstream vshad;

		vshad <<
			"uniform mat4 modelview_projection;\n"
			"attribute vec2 v_position;\n"
			;

		if(caps & do_pretranslate)
			vshad <<
				"uniform vec2 pre_translation;\n"
				;

		if(caps & do_mask)
			vshad <<
				"varying vec2 v_textureCoord;\n"
				;

		if((caps & primary_mode_mask) == do_pattern)
			vshad <<
				"uniform mat4 p_projection;\n"
				"varying vec4 p_textureCoord;\n"
				;

		if(caps & gradient_spread_mask)
			vshad <<
				"varying vec2 gradient_coord;\n";

		vshad <<
			"void main() {\n"
			;

		if(caps & do_pretranslate)
			vshad <<
			"  gl_Position = modelview_projection * vec4(v_position + pre_translation, 0.0, 1.0);\n";
		else
			vshad <<
			"  gl_Position = modelview_projection * vec4(v_position, 0.0, 1.0);\n";

		if(caps & do_mask)
			vshad <<
				"  v_textureCoord = vec2(gl_Position.x * 0.5 + 0.5, gl_Position.y * 0.5 + 0.5);\n"
				;

		if((caps & primary_mode_mask) == do_pattern)
			vshad <<
				"  p_textureCoord = p_projection * vec4(gl_Position.xy, 0.0, 1.0);\n"
				;

		if(caps & gradient_spread_mask)
			vshad <<
				"  vec4 gc_tmp = surf2paint * gl_Position;\n"
				"  gradient_coord = vec2(gc_tmp.x, gc_tmp.y);\n"
				;


		vshad <<
			"}\n";

		return vshad.str();
	}

	std::string Shader::build_fragment_shader(int caps) {
		std::stringstream fshad;

		bool horizontal_gauss = false;
		bool do_gauss = false;

		if(caps & do_horizontal_gaussian) {
			horizontal_gauss = true;
			do_gauss = true;
		} else if(caps & do_vertical_gaussian)
			do_gauss = true;

		fshad <<
			"precision highp float;\n"
			;

		if(caps & gradient_spread_mask)
			fshad <<
				"varying vec2 gradient_coord;\n";

		if(caps & do_mask)
			fshad <<
				"varying vec2 v_textureCoord;\n"
				"uniform sampler2D m_texture;\n";

		auto primary_mode = caps & primary_mode_mask;
		if(primary_mode == do_linear_gradient ||
		   primary_mode == do_radial_gradient) {
			// gradient coordinates are in paint space - use surface2paint conversion matrix
			fshad << "uniform mat4 surf2paint;\n";
			fshad <<
				"uniform vec4 stop_colors[32];\n" // stop position, R, G, B, A
				"uniform float stop_invfactor[32];\n" // used to calculate distance between stops
				"uniform float stop_offsets[32];\n" // stop position, R, G, B, A
				"uniform int nr_stops;\n"       // number of active stops, max 32
				"uniform int color_ramp_mode;\n"   // 0 = pad, 1 = repeat, 2 = mirror
				;

			if(primary_mode == do_linear_gradient) {
				fshad <<
					"uniform vec2 linear_normal;\n" // normal - for calculating distance in linear gradient
					"uniform vec2 linear_start;\n"     // linear gradient (x0, y0)
					"uniform float linear_length;\n"  // length of gradient
					;
			} else {
				fshad <<
					"uniform vec4 radial;\n"    // (focus.x, f.y, focus.x - center.x, f.y - c.y)
					"uniform float radius2;\n"  // radius ^ 2
					"uniform float radial_denom;\n"  // 1 / (r^2 − (fx'^2 + fy'^2))
					;
			}
		} else if(primary_mode == do_pattern)
			fshad <<
				"varying vec4 p_textureCoord;\n"
				"uniform sampler2D p_texture;\n";
		else // flat color
			fshad <<
				"uniform vec4 v_color;\n"
				;


		if(do_gauss)
			fshad
				<< "uniform vec4 pxl_n_half_pxl_size;\n"
				<< GenerateGaussFunctionCode(
					(caps & gauss_krn_diameter_mask) >> 16,
					true
					)
				;

		fshad <<
			"\n"
			"void main() {\n";

		GNUVG_ERROR("caps: %x -- gradient_spread_mask: %x -- %x\n",
			    caps, gradient_spread_mask,
			    caps & gradient_spread_mask);

		if(caps & do_mask)
			fshad <<
				"  vec4 m = texture2D( m_texture, v_textureCoord );\n";

		switch(primary_mode) {
		case do_flat_color:
			fshad << "  vec4 c = v_color;\n";
			break;

		case do_linear_gradient:
		case do_radial_gradient:
			if(primary_mode == do_linear_gradient)
				fshad <<
					"  vec2 BA = gradient_coord - linear_start;\n"
					"  float g = ((BA.x*linear_normal.y) - (BA.y*linear_normal.x)) / linear_length;\n"
					;
			else
				fshad <<
					"  vec2 dxy = coord - radial.xy;\n"
					"  float g = dot(dxy, radial.zw);\n"
					"  float h = dxy.x * radial.w - dxy.y * radial.z;\n"
					"  g += sqrt(radius2 * dot(dxy, dxy) - h*h);\n"
					"  g *= radial_denom;\n"
					;

			switch(caps & gradient_spread_mask) {
			case do_gradient_pad:
				fshad << "    g = clamp(g, 0.0, 1.0);\n";
				break;
			case do_gradient_repeat:
				fshad << "    g = mod(g, 1.0);\n";
				break;
			case do_gradient_reflect:
				fshad <<
					"    float uneven;\n"
					"    g = mod(g, 1.0);\n"
					"    uneven = floor(0.6 + mod(abs(floor(g))/2.0, 1.0));\n"
					"    g = g * (1.0 - uneven) + (1.0 - g) * uneven;\n";
				break;
			}

			fshad <<
				"  vec4 previous_color = vec4(0.0f, 0.0f, 0.0f, 0.0f);\n"
				"  vec4 c = stop_colors[nr_stops - 1];\n"
				"  for(int i = 0; i < nr_stops; i++) {\n"
				"    if(g < stop_offsets[i]) {\n"
				"      float coef = (stop_offsets[i] - g) * stop_invfactor[i];\n"
				"      c = mix(stop_colors[i], previous_color, coef);\n"
				"    }\n"
				"    previous_color = stop_colors[i];\n"
				"  }\n";
			break;

		case do_pattern:
			if(do_gauss) {
				auto hpx_size = horizontal_gauss ?
					"pxl_n_half_pxl_size.z, 0" :
					"0, pxl_n_half_pxl_size.w"
					;
				auto px_size = horizontal_gauss ?
					"pxl_n_half_pxl_size.x, 0" :
					"0, pxl_n_half_pxl_size.y"
					;
				fshad <<
					"  vec4 c = GaussianBlur( p_texture, p_textureCoord.xy, \n"
					"                    vec2( " << hpx_size << " ),\n"
					"                    vec2( " <<  px_size << " ) );\n"
					;
			} else {
				fshad <<
					"  vec4 c = texture2D( p_texture, p_textureCoord.xy );\n";
			}
			break;
		}

		if(caps & do_mask)
			fshad << "  c = m.a * c;\n";

		fshad <<
			"  gl_FragColor = c;\n"
			"}\n"
			;

		return fshad.str();
	}

	static GLuint compile_shader(GLenum shader_type, const char *shader_source) {
		GLuint shader = glCreateShader(shader_type);
		if(shader) {
			glShaderSource(shader, 1, &shader_source, NULL);
			glCompileShader(shader);
			GLint compiled = 0;
			glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
			if(!compiled) {
				GNUVG_ERROR("Shader program failed to compile.\n");

				GLint infoLen = 0;
				glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
				if(infoLen) {
					char buf[infoLen];
					glGetShaderInfoLog(shader, infoLen, NULL, buf);
					GNUVG_ERROR("Could not compile shader %d:\n%s\n",
						    shader_type, buf);
					glDeleteShader(shader);
					shader = 0;
				}
			}
		} else
			GNUVG_ERROR("Could not create shader object.\n");

		return shader;
	}

	GLuint Shader::create_program(const char *vertexshader_source, const char *fragmentshader_source) {
		GLuint vertexShader = compile_shader(GL_VERTEX_SHADER, vertexshader_source);
		if (!vertexShader)
			return 0;

		GLuint pixelShader = compile_shader(GL_FRAGMENT_SHADER, fragmentshader_source);
		if (!pixelShader)
			return 0;

		GLuint program = glCreateProgram();
		if (program) {
			glAttachShader(program, vertexShader);
			checkGlError("glAttachShader - vertex shader");
			glAttachShader(program, pixelShader);
			checkGlError("glAttachShader - fragment shader");
			glLinkProgram(program);
			GLint linkStatus = GL_FALSE;
			glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
			if (linkStatus != GL_TRUE) {
				GLint bufLength = 0;
				glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);
				if (bufLength) {
					char* buf = (char*) malloc(bufLength);
					if (buf) {
						glGetProgramInfoLog(program, bufLength, NULL, buf);
						GNUVG_ERROR("Could not link program:\n%s\n", buf);
						free(buf);
					}
				}
				glDeleteProgram(program);
				program = 0;
			}
		}
		return program;
	}

	static void print_shader(const std::string &title, const std::string &content_s) {
		GNUVG_ERROR("%s\n", title.c_str());

		auto content = content_s.c_str();
		std::vector<char> bfr;
		size_t k = 0, l = 0;

		while(content[k] != '\0') {
			bfr.clear();
			while(content[k] != '\n' && content[k] != '\0') {
				bfr.push_back(content[k]);
				k += 1;
			}
			bfr.push_back('\0');
			GNUVG_ERROR("%04d: %s\n", l, bfr.data());
			if(content[k] == '\n') k += 1;
			l++;
		}
	}

	Shader::Shader(int caps) {
		auto vertex_shader = build_vertex_shader(caps);
		auto fragment_shader = build_fragment_shader(caps);

		if(caps & do_horizontal_gaussian || caps & do_vertical_gaussian)
			GNUVG_ERROR("Caps debug, gauss: %x\n", caps);

		print_shader("Vertex Shader:", vertex_shader);
		print_shader("Fragment Shader:", fragment_shader);

		program_id = create_program(vertex_shader.c_str(),
					    fragment_shader.c_str());

		position_handle = glGetAttribLocation(program_id, "v_position");
		klm_handle = glGetAttribLocation(program_id, "klm");

		ColorHandle = glGetUniformLocation(program_id, "v_color");
		Matrix = glGetUniformLocation(program_id, "modelview_projection");
		preTranslation = glGetUniformLocation(program_id, "pre_translation");

		maskTexture = glGetUniformLocation(program_id , "m_texture" );

		pxl_sizes = glGetUniformLocation(program_id, "pxl_n_half_pxl_size");
		patternTexture = glGetUniformLocation(program_id , "p_texture" );
		patternMatrix = glGetUniformLocation(program_id, "p_projection");

		surf2paint = glGetUniformLocation(program_id, "surf2paint");

		linear_normal = glGetUniformLocation(program_id, "linear_normal");
		linear_start = glGetUniformLocation(program_id, "linear_start");
		linear_length = glGetUniformLocation(program_id, "linear_length");

		radial = glGetUniformLocation(program_id, "radial");
		radius2 = glGetUniformLocation(program_id, "radius2");
		radial_denom = glGetUniformLocation(program_id, "radial_denom");

		stop_colors = glGetUniformLocation(program_id, "stop_colors");
		stop_invfactor = glGetUniformLocation(program_id, "stop_invfactor");
		stop_offsets = glGetUniformLocation(program_id, "stop_offsets");
		nr_stops = glGetUniformLocation(program_id, "nr_stops");
	}
};
