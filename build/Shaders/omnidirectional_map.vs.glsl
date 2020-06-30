#version 330 core

layout(location = 0) in vec3 in_position;

uniform mat4 model;
uniform mat4 shadowMatrix;

out vec4 FragPos;

void main() 
{
    FragPos = model * vec4(in_position, 1.0f);
    gl_Position = shadowMatrix * FragPos;
}