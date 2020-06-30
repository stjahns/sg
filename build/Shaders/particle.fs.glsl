#version 330 core

layout (location=0) out vec4 color;

in float intensity;

void main(void)
{
	color = mix(vec4(0.0f, 0.2, 1.0, 1.0), vec4(0.2, 0.05, 0.0, 1.0), intensity);
}
