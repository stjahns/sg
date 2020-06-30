#version 330 core

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;

uniform mat4 mtxModel;
uniform mat4 mtxView;
uniform mat4 mtxProjection;

uniform Matrices
{
	mat4 Model;
	mat4 View;
	mat4 Projection;
};

out vec3 Normal;
out vec4 Position;

void main()
{
	mat4 mtxModelView = mtxView * mtxModel;
	Normal = normalize(mat3(mtxModelView) * VertexNormal);
	gl_Position = mtxProjection * (mtxModelView * vec4(VertexPosition, 1.0));
}
