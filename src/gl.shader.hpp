#ifndef HYDROGEN_GL_SHADER_HPP_INCLUDED
#define HYDROGEN_GL_SHADER_HPP_INCLUDED

namespace aspect { namespace gl {

class shader : boost::noncopyable
{
public:
	shader(GLuint type, const char* source)
		: id_(0)
		, program_(0)
	{
		if (type && source)
		{
			build(type, source);
		}
	}

	~shader()
	{
		cleanup();
	}

	GLuint id() const { return id_; }
	GLuint program() const { return program_; }

private:
	void cleanup()
	{
		if (program_)
		{
			glDeleteProgram(program_);
			program_ = 0;
		}

		if (id_)
		{
			glDeleteShader(id_);
			id_ = 0;
		}
	}

private:
	GLuint id_;
	GLuint program_;

	static GLuint shader_type_from_filename(const char *filename)
	{
		if (strstr(filename, ".frag"))
			return GL_FRAGMENT_SHADER;
		else
		if (strstr(filename, ".fsl"))
			return GL_FRAGMENT_SHADER;
		else
		if (strstr(filename, ".vsl"))
			return GL_VERTEX_SHADER;

		trace("error - unable to determine shader type: %s\n", filename);

		return 0;
	}

	bool read_file(char const* filename)
	{
		trace("loading shader: %s\n", filename);

		GLuint const type = shader_type_from_filename(filename);
		if (!type)
		{
			return false;
		}

		std::ifstream file(filename);
		if (!file)
		{
			std::cerr << "error - unable to open: " << filename << std::endl;
			return false;
		}

		std::stringstream ss;
		ss << file.rdbuf();
		return build(type, ss.str().c_str());
	}

	GLint compile(GLuint type, const char* source)
	{
		id_ = glCreateShader(type);

		glShaderSource(id_, 1, &source, NULL);

		glCompileShader(id_);

		// Print the compilation log.
		GLint status = 0;
		glGetShaderiv(id_, GL_COMPILE_STATUS, &status);
		if (!status)
		{
			GLint size = 0;
			glGetShaderiv(id_, GL_INFO_LOG_LENGTH, &size);
			if (size)
			{
				std::vector<char> buf(size);
				glGetShaderInfoLog(id_, size, NULL, &buf[0]);
				aspect::error("shader compilation failed\n%s\n", &buf[0]);
			}
		}
		return status;
	}

	GLint link()
	{
		program_ = glCreateProgram();

		glAttachShader(program_, id_);

		glLinkProgram(program_);

		GLint link_status;
		glGetProgramiv(program_,GL_LINK_STATUS,&link_status);
		if (!link_status)
		{
			GLint size = 0;
			glGetProgramiv(program_,GL_INFO_LOG_LENGTH,&size);
			if (size)
			{
				std::vector<char> buf(size);
				glGetProgramInfoLog(program_, size, NULL, &buf[0]);
				error("program link failed","\n%s\n", &buf[0]);
			}
		}

		return link_status;
	}

	bool build(GLuint type, const char* source)
	{
		return compile(type, source) && link();
	}
};

}} // aspect::gl

#endif // HYDROGEN_GL_SHADER_HPP_INCLUDED
