#version 330 core

layout(location = 0) in vec3 in_position;
layout(location = 5) in ivec4 in_skinIndex;
layout(location = 6) in vec4 in_skinWeight;

uniform mat4 model;
uniform mat4 shadowMatrix;

uniform mat4 skinMatrix[128];

out vec4 FragPos;

void main() 
{
    mat4 skin = skinMatrix[in_skinIndex.x] * in_skinWeight.x
              + skinMatrix[in_skinIndex.y] * in_skinWeight.y
              + skinMatrix[in_skinIndex.z] * in_skinWeight.z
              + skinMatrix[in_skinIndex.w] * in_skinWeight.w;

    FragPos = model * skin * vec4(in_position, 1.0);

    gl_Position = shadowMatrix * FragPos;
}