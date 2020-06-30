#version 330 core

uniform mat4 model_matrix;
uniform mat4 projection_matrix;

layout(location = 0) in vec4 position;
layout(location = 1) in vec4 color;

out vec4 vs_fs_color;

const int INSTANCES = 5;

void main()
{
	vs_fs_color = color;

	vec4 position = model_matrix * position;
	position = position + vec4(3.0, 0.0, 0.0, 0.0) * gl_InstanceID;
	gl_Position = projection_matrix * position;
}
