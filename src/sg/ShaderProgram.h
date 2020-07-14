#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <cstdlib>
#include <iostream>

class ShaderSource
{
public:

	ShaderSource(const char* filename);

	~ShaderSource();

	bool IsValid() { return m_Source != nullptr; }
	const GLchar* Get() { return m_Source; }

private:

	GLchar* m_Source;
};

class Shader
{
public:

	Shader(GLenum type, const char* path);
	~Shader();

	void Init();

	bool IsValid() { return m_Valid; }
	GLuint GetHandle() { return m_Handle; }
	bool HasSource() { return m_Path; }

private:

	GLuint m_Handle;
	bool m_Valid;

	GLenum m_Type;
	const char* m_Path;
};


class ShaderProgram
{
public:

	ShaderProgram(const char* vsSource, const char* fsSource);
	ShaderProgram(const char* vsSource, const char* gsSource, const char* fsSource);

	~ShaderProgram();

	bool AttachShader(Shader& shader);

	bool Link();

	GLuint GetHandle() { return m_Handle; }

	void Reload()
	{
		m_VertexShader.Init();
		m_FragmentShader.Init();

		if (m_GeometryShader.HasSource())
		{
			m_GeometryShader.Init();
		}

		m_Handle = glCreateProgram();

		AttachShader(m_VertexShader);

		if (m_GeometryShader.HasSource())
		{
			AttachShader(m_GeometryShader);
		}

		AttachShader(m_FragmentShader);

		Link();
	}

	void Use()
	{
		glUseProgram(GetHandle());
	}

	void SetUniform(const char* id, glm::mat4 mat)
	{
		GLuint uniform = glGetUniformLocation(GetHandle(), id);
		glUniformMatrix4fv(uniform, 1, GL_FALSE, &mat[0][0]);
	}

	void SetUniform(const char* id, glm::vec3 vec)
	{
		GLuint uniform = glGetUniformLocation(GetHandle(), id);
		glUniform3fv(uniform, 1, &vec[0]);
	}

	void SetUniform(const char* id, glm::vec4 vec)
	{
		GLuint uniform = glGetUniformLocation(GetHandle(), id);
		glUniform4fv(uniform, 1, &vec[0]);
	}

	void SetUniform(const char* id, glm::vec2 vec)
	{
		GLuint uniform = glGetUniformLocation(GetHandle(), id);
		glUniform2fv(uniform, 1, &vec[0]);
	}

	void SetUniform(const char* id, float f)
	{
		GLuint uniform = glGetUniformLocation(GetHandle(), id);
		glUniform1f(uniform, f);
	}

	void SetUniformi(const char* id, int i)
	{
		GLuint uniform = glGetUniformLocation(GetHandle(), id);
		glUniform1i(uniform, i);
	}

private:

	Shader m_VertexShader;
	Shader m_FragmentShader;
	Shader m_GeometryShader;
	GLuint m_Handle;
};

//class SimpleShaderProgram
//{
//public:
//
//	SimpleShaderProgram(const char* vsSource, const char* fsSource);
//
//private:
//
//	ShaderProgram m_Program;
//	Shader m_VertexShader;
//	Shader m_FragmentShader;
//};
