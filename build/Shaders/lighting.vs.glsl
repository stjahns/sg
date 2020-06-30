#version 330 core

layout (location = 0) in vec4 VertexPosition;
layout (location = 1) in vec4 VertexColor;
layout (location = 2) in vec3 VertexNormal;

uniform mat4 MVPMatrix;
uniform mat4 MVMatrix; // mat3?

out vec4 Color;
out vec3 Normal;
out vec4 Position;

void main()
{
	Color = VertexColor;

	Normal = normalize(mat3(MVMatrix) * VertexNormal);

	Position = MVMatrix * VertexPosition; // pre-perspective

	gl_Position = MVPMatrix * VertexPosition;
}
