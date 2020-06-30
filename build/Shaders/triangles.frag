#version 330 core

in vec4 vs_fs_color;

out vec4 color;

void main()
{
	color = vs_fs_color;
}
