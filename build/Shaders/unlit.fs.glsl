#version 330 core

in vec3 normal;
in vec3 position;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec4 color;

void main() 
{
    vec4 n = view * model * vec4(normal, 0.0f);
	color = vec4(max(n.z, 0.0f));
}