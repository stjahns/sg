#include "ShaderProgram.h"

ShaderSource::ShaderSource(const char* filename) : m_Source(nullptr)
{
#ifdef WIN32
	FILE* infile;
	fopen_s(&infile, filename, "rb");
#else
	FILE* infile = fopen(filename, "rb");
#endif // WIN32

	if (!infile) {
		std::cerr << "Unable to open file '" << filename << "'" << std::endl;
		return;
	}

	fseek(infile, 0, SEEK_END);
	int len = ftell(infile);
	fseek(infile, 0, SEEK_SET);

	m_Source = new GLchar[len+1];

	fread(m_Source, 1, len, infile);
	fclose(infile);

	m_Source[len] = 0;
}

ShaderSource::~ShaderSource()
{
	if (m_Source)
	{
		delete[] m_Source;
	}
}

////////////////////////////////////////////////////////////////////////////////

void Shader::Init()
{
	ShaderSource source(m_Path);
	if (!source.IsValid())
	{
		return;
	}

	m_Handle = glCreateShader(m_Type);

	const GLchar* strSource = source.Get();

	glShaderSource(m_Handle, 1, &strSource, NULL);

	glCompileShader(m_Handle);

	GLint compiled;
	glGetShaderiv(m_Handle, GL_COMPILE_STATUS, &compiled);
	if (compiled)
	{
		m_Valid = true;
	}
	else
	{
		GLsizei len;
		glGetShaderiv(m_Handle, GL_INFO_LOG_LENGTH, &len);

		GLchar* log = new GLchar[len+1];
		glGetShaderInfoLog(m_Handle, len, &len, log);
		std::cerr << "Shader compilation (" << m_Path << ") failed: " << log << std::endl;
		delete [] log;
	}
}

Shader::Shader(GLenum type, const char* path) : m_Handle(-1), m_Valid(false), m_Type(type), m_Path(path)
{
	if (path)
		Init();
}

Shader::~Shader()
{
	glDeleteShader(m_Handle);
}

////////////////////////////////////////////////////////////////////////////////

ShaderProgram::ShaderProgram(const char* vsSource, const char* fsSource)
	: m_VertexShader(GL_VERTEX_SHADER, vsSource)
	, m_GeometryShader(GL_GEOMETRY_SHADER, nullptr)
	, m_FragmentShader(GL_FRAGMENT_SHADER, fsSource)
{
	m_Handle = glCreateProgram();
	AttachShader(m_VertexShader);
	AttachShader(m_FragmentShader);
	Link();
}

ShaderProgram::ShaderProgram(const char* vsSource, const char* gsSource, const char* fsSource)
	: m_VertexShader(GL_VERTEX_SHADER, vsSource)
	, m_GeometryShader(GL_GEOMETRY_SHADER, gsSource)
	, m_FragmentShader(GL_FRAGMENT_SHADER, fsSource)
{
	m_Handle = glCreateProgram();
	AttachShader(m_VertexShader);
	AttachShader(m_GeometryShader);
	AttachShader(m_FragmentShader);
	Link();
}

ShaderProgram::~ShaderProgram()
{
	glDeleteProgram(m_Handle);
}

bool ShaderProgram::AttachShader(Shader& shader)
{
	if (shader.IsValid())
	{
		glAttachShader(m_Handle, shader.GetHandle());
		return true;
	}
	else
	{
		return false;
	}
}

bool ShaderProgram::Link()
{
	glLinkProgram(m_Handle);

	GLint linked;
	glGetProgramiv(m_Handle, GL_LINK_STATUS, &linked);
	if (!linked) {
		GLsizei len;
		glGetProgramiv(m_Handle, GL_INFO_LOG_LENGTH, &len);

		GLchar* log = new GLchar[len+1];
		glGetProgramInfoLog(m_Handle, len, &len, log);
		std::cerr << "Shader linking failed: " << log << std::endl;
		delete [] log;
		return false;
	}
	else
	{
		return true;
	}
}

////////////////////////////////////////////////////////////////////////////////

//SimpleShaderProgram::SimpleShaderProgram(const char* vsSource, const char* fsSource)
//	: m_VertexShader(GL_VERTEX_SHADER, vsSource)
//	, m_FragmentShader(GL_FRAGMENT_SHADER, fsSource)
//{
//	m_Program.AttachShader(m_VertexShader);
//	m_Program.AttachShader(m_FragmentShader);
//	m_Program.Link();
//}
