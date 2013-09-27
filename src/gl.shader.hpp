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

		std::string	source_;
		GLuint	type_;
		GLuint	id_;
		GLuint	program_;

		shader(GLuint type = 0, const char *source = NULL)
			: id_(0), program_(0), type_(type), source_(source)
		{
			if(type && source)
				build(type_,source_.c_str());
		}

		~shader()
		{
			cleanup();
		}

		void cleanup(void)
		{
			if(program_)
			{
				glDeleteObjectARB(program_);
				program_ = 0;
			}

			if(id_)
			{
				glDeleteShader(id_);
				id_ = 0;
			}
		}

		GLuint get_id(void) const { return id_; }
		GLuint get_program(void) const { return program_; }

/*		GLuint get_shader_type_from_filename(const char *filename)
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
*/
/*
		bool read_file(const std::string &filename)
		{
			source_.clear();

			printf("loading shader: %s\n",filename.c_str());

			type_ = get_shader_type_from_filename(filename.c_str());
			if(!type_)
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
			source_ = ss.str();

			m_filename = filename;

			return true;
		}
*/

		GLint compile(GLuint type, const char *source)
		{
//			printf("compiling shader '%s'\n",m_filename.c_str());

//			source_ = source;

			id_ = glCreateShader(type);
//			if(!source)
//				source = source_.c_str();
			glShaderSource(id_,1,&source,NULL);

			glCompileShader(id_);

			// Print the compilation log.
			
			GLint status = 0;
			//glGetObjectParameterivARB(id_,GL_OBJECT_COMPILE_STATUS_ARB,&status);
			glGetShaderiv(id_,GL_COMPILE_STATUS,&status);
			if(!status)
			{
				GLint size = 0;
				glGetShaderiv(id_,GL_INFO_LOG_LENGTH,&size);
				char *buffer = (char*)malloc(size);
				glGetShaderInfoLog(id_,size,NULL,buffer);
				aspect::error("shader compilation failed\n%s\n", buffer);

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
			program_ = glCreateProgramObjectARB();

			glAttachShader(program_,get_id());

			glLinkProgram(program_);

			GLint link_status;
			glGetProgramiv(program_,GL_LINK_STATUS,&link_status);
			if(!link_status)
			{
				GLint size = 0;
				glGetProgramiv(program_,GL_INFO_LOG_LENGTH,&size);
				char *buffer = (char*)malloc(size);
				glGetProgramInfoLog(program_,size,NULL,buffer);
				error("program link failed","\n%s\n", buffer);
				free(buffer);
			}

			return link_status;
		}

		bool build(GLuint type, const char *source)
		{
			if(!compile(type, source))
				return false;

			if(!link())
				return false;

			return true;
		}

		bool restore()
		{
			if(!compile(type_, source_.c_str()))
				return false;

			if(!link())
				return false;

			return true;
		}

		bool operator() (GLuint type, const char *source) //(const std::string &filename)
		{
			cleanup();

			source_ = source;
			type_ = type;
			return build(type, source);
		}
};


} } // aspect::gl

#endif // __GL_SHADER_HPP__
