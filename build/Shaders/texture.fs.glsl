#version 330 core

uniform sampler2D tex1;
uniform sampler2D tex2;

in vec2 vs_tex_coord_1;
in vec2 vs_tex_coord_2;

layout (location = 0) out vec4 color;

void main(void)
{
	color = texture(tex1, vs_tex_coord_1) * texture(tex2, vs_tex_coord_2);
}

