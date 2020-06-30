#pragma once

#include "ShaderProgram.h"

#include <cstdint>

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "defines.h"

using namespace glm;

class PrimitiveCube
{
public:

	PrimitiveCube(shared_ptr<ShaderProgram> shaderProgram, mat4 transform);
	~PrimitiveCube();

	void Render(mat4 view, mat4 projection);

	void SetTransform(mat4 t) { m_Transform = t; }

private:

	mat4 m_Transform;

	GLuint m_VertexArrayObject;
	GLuint m_ArrayBuffer;
	GLuint m_ElementArrayBuffer;

	GLuint m_uniformModel;
	GLuint m_uniformView;
	GLuint m_uniformProjection;

	shared_ptr<ShaderProgram> m_ShaderProgram;

	enum
	{
		VertexCount = 24,
		IndexCount = 29,
	};

	static const float m_vertexPositions[VertexCount * 3];
	static const float m_vertexNormals[VertexCount * 3];
	static const u16 m_vertexIndices[IndexCount];

};
