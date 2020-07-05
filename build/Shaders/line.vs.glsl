#version 330 core

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec4 in_color;

uniform mat4 view;
uniform mat4 projection;

out vec4 color;

void main() 
{
    mat4 VP = projection * view;
    gl_Position = VP * vec4(in_position, 1.0);
    color = in_color;
}