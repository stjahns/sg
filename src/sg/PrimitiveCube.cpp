#include "PrimitiveCube.h"

#include <glm/gtc/type_ptr.hpp>

PrimitiveCube::PrimitiveCube(shared_ptr<ShaderProgram> shaderProgram, mat4 transform)
	: m_Transform(transform)
	, m_ShaderProgram(shaderProgram)
{
	glUseProgram(m_ShaderProgram->GetHandle());

	glGenVertexArrays(1, &m_VertexArrayObject);
	glBindVertexArray(m_VertexArrayObject);

	glGenBuffers(1, &m_ArrayBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, m_ArrayBuffer);

	glBufferData(GL_ARRAY_BUFFER, sizeof(m_vertexPositions) + sizeof(m_vertexNormals), NULL, GL_STATIC_DRAW);

	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(m_vertexPositions), m_vertexPositions);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(m_vertexPositions), sizeof(m_vertexNormals), m_vertexNormals);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(m_vertexPositions)));
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &m_ElementArrayBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ElementArrayBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(m_vertexIndices), m_vertexIndices, GL_STATIC_DRAW);

	m_uniformModel = glGetUniformLocation(m_ShaderProgram->GetHandle(), "mtxModel");
	m_uniformView = glGetUniformLocation(m_ShaderProgram->GetHandle(), "mtxView");
	m_uniformProjection = glGetUniformLocation(m_ShaderProgram->GetHandle(), "mtxProjection");

	// TODO - use a uniform buffer object for:
	// View/Projection matrices
	// Scene Lights

	GLuint matrixBlock = glGetUniformBlockIndex(m_ShaderProgram->GetHandle(), "Matrices");
	GLuint matrixBuffer = -1;
	glBindBuffer(GL_UNIFORM_BUFFER, matrixBuffer);
	glBindBufferBase(GL_UNIFORM_BUFFER, matrixBlock, matrixBuffer);
}

PrimitiveCube::~PrimitiveCube()
{
	glDeleteVertexArrays(1, &m_VertexArrayObject);
	glDeleteBuffers(1, &m_ArrayBuffer);
	glDeleteBuffers(1, &m_ElementArrayBuffer);
}

void PrimitiveCube::Render(mat4 view, mat4 projection)
{
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	glBindVertexArray(m_VertexArrayObject);
	glBindBuffer(GL_ARRAY_BUFFER, m_ArrayBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ElementArrayBuffer);

	glUseProgram(m_ShaderProgram->GetHandle());

	glUniformMatrix4fv(m_uniformModel, 1, GL_FALSE, value_ptr(m_Transform));
	glUniformMatrix4fv(m_uniformView, 1, GL_FALSE, value_ptr(view));
	glUniformMatrix4fv(m_uniformProjection, 1, GL_FALSE, value_ptr(projection));

	glEnable(GL_PRIMITIVE_RESTART);
	glPrimitiveRestartIndex(0xFFFF);
	glDrawElements(GL_TRIANGLE_STRIP, 53, GL_UNSIGNED_SHORT, NULL);
}

const float PrimitiveCube::m_vertexPositions[] =
{
	-1,1,1,
	-1,1,1,
	-1,1,1,
	1,1,1,

	1,1,1,
	1,1,1,
	-1,-1,1,
	-1,-1,1,

	-1,-1,1,
	1,-1,1,
	1,-1,1,
	1,-1,1,

	-1,1,-1,
	-1,1,-1,
	-1,1,-1,
	1,1,-1,

	1,1,-1,
	1,1,-1,
	-1,-1,-1,
	-1,-1,-1,

	-1,-1,-1,
	1,-1,-1,
	1,-1,-1,
	1,-1,-1

};

const float PrimitiveCube::m_vertexNormals[] =
{
	0,0,1,
	0,1,0,
	-1,0,0,
	0,0,1,

	0,1,0,
	1,0,0,
	0,0,1,
	0,-1,0,

	-1,0,0,
	0,0,1,
	0,-1,0,
	1,0,0,

	0,1,0,
	-1,0,0,
	0,0,-1,
	0,1,0,

	1,0,0,
	0,0,-1,
	0,-1,0,
	-1,0,0,

	0,0,-1,
	0,-1,0,
	1,0,0,
	0,0,-1,
};

const u16 PrimitiveCube::m_vertexIndices[] =
{
	6,9,0,3,
	0xFFFF,
	12,1,15,4,
	0xFFFF,
	14,17,20,23,
	0xFFFF,
	5,11,16,22,
	0xFFFF,
	13,19,2,8,
	0xFFFF,
	21,10,18,7
};
