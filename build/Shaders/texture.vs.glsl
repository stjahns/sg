#version 330 core

layout (location = 0) in vec4 in_position;
layout (location = 1) in vec2 in_tex_coord;

out vec2 vs_tex_coord_1;
out vec2 vs_tex_coord_2;

uniform float time;

void main(void)
{
	float t = time / 10;
	mat2 m = mat2(cos(t), sin(t),
				  -sin(t), cos(t));

	gl_Position = in_position;
	vs_tex_coord_1 = in_tex_coord;
	//vs_tex_coord_1 = in_tex_coord * m;

	vs_tex_coord_2 = (in_tex_coord - vec2(1, 1)) * m;
}
