#pragma once
#ifndef __GL_SHADER_HPP__
#define __GL_SHADER_HPP__

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>

namespace aspect { namespace gl {

class shader
{
	public:

		typedef std::vector<shader*>::iterator iterator;

		std::string	m_filename;
		std::string	m_source;
		GLuint	m_type;
		GLuint	m_id;
		GLuint	m_program;

		shader()
			: m_id(0), m_type(0), m_program(0)
		{
		}

		~shader()
		{
			cleanup();
		}

		void cleanup(void)
		{
			if(m_program)
			{
				glDeleteObjectARB(m_program);
				m_program = 0;
			}

			if(m_id)
			{
				glDeleteShader(m_id);
				m_id = 0;
			}
		}

		GLuint get_id(void) const { return m_id; }
		GLuint get_program(void) const { return m_program; }

		GLuint get_shader_type_from_filename(const char *filename)
		{
			if(strstr(filename,".frag"))
				return GL_FRAGMENT_SHADER;
			else
			if(strstr(filename,".fsl"))
				return GL_FRAGMENT_SHADER;
			else
			if(strstr(filename,".vsl"))
				return GL_VERTEX_SHADER;

			trace("error - unable to determine shader type: %s\n",filename);

			return NULL;
		}

		bool read_file(const std::string &filename)
		{
			m_source.clear();

			printf("loading shader: %s\n",filename.c_str());

			m_type = get_shader_type_from_filename(filename.c_str());
			if(!m_type)
				return false;

			std::ifstream ifs;
			ifs.open(filename.c_str());
			
			if(!ifs) 
			{
				std::cerr << "error - unable to open: " << filename << std::endl;
				return false;
			}

			std::stringstream ss;
			ss << ifs.rdbuf();
			m_source = ss.str();

			m_filename = filename;

			return true;
		}

		GLint compile(void)
		{
			printf("compiling shader '%s'\n",m_filename.c_str());

			m_id = glCreateShader(m_type);
			const char *source = m_source.c_str();
			glShaderSource(m_id,1,&source,NULL);

			glCompileShader(m_id);

			// Print the compilation log.
			
			GLint status = 0;
			//glGetObjectParameterivARB(m_id,GL_OBJECT_COMPILE_STATUS_ARB,&status);
			glGetShaderiv(m_id,GL_COMPILE_STATUS,&status);
			if(!status)
			{
				GLint size = 0;
				glGetShaderiv(m_id,GL_INFO_LOG_LENGTH,&size);
				char *buffer = (char*)malloc(size);
				glGetShaderInfoLog(m_id,size,NULL,buffer);
				error("shader compilation failed","\n%s\n", buffer);

#if 0
				std::string filename;
				aspect::get_application_folder(filename);
				filename += "/shaders/shader-error.txt";
				std::ofstream ofs;
				ofs.open(filename.c_str());
				ofs << buffer << std::endl;
				ofs.close();
#endif

				free(buffer);
			}

			return status;						

		}

		GLint link(void)
		{
			m_program = glCreateProgramObjectARB();

			glAttachShader(m_program,get_id());

			glLinkProgram(m_program);

			GLint link_status;
			glGetProgramiv(m_program,GL_LINK_STATUS,&link_status);
			if(!link_status)
			{
				GLint size = 0;
				glGetProgramiv(m_program,GL_INFO_LOG_LENGTH,&size);
				char *buffer = (char*)malloc(size);
				glGetProgramInfoLog(m_program,size,NULL,buffer);
				error("program link failed","\n%s\n", buffer);
				free(buffer);
			}

			return link_status;
		}

		bool operator() (const std::string &filename)
		{
			cleanup();

//			std::string folder;
//			aspect::get_application_folder(folder);
//			folder += "/shaders/";
//			if(!read_file(folder + filename))
			if(!read_file(filename))
			{
				_aspect_assert(!"unable to load shader");
				return false;
			}

			if(!compile())
				return false;

			if(!link())
				return false;

			return true;
		}
};


} } // aspect::gl

#endif // __GL_SHADER_HPP__