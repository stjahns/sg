#version 330 core

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_texcoord;
layout(location = 3) in vec3 in_tangent;
layout(location = 4) in vec3 in_bitangent;
layout(location = 5) in ivec4 in_skinIndex;
layout(location = 6) in vec4 in_skinWeight;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 MVP;

uniform mat4 skinMatrix[128];

out vec3 position;
out vec2 texcoord;
out mat3 TBN;

void main() 
{
    mat4 skin = skinMatrix[in_skinIndex.x] * in_skinWeight.x
              + skinMatrix[in_skinIndex.y] * in_skinWeight.y
              + skinMatrix[in_skinIndex.z] * in_skinWeight.z
              + skinMatrix[in_skinIndex.w] * in_skinWeight.w;

    vec4 skinnedPosition = skin * vec4(in_position, 1.0);
    vec4 skinnedNormal = skin * vec4(in_normal, 0.0);

    gl_Position = MVP * skinnedPosition;

    position = vec3(model * skinnedPosition);
    texcoord = in_texcoord;
    TBN = mat3(skin) * mat3(in_tangent, in_bitangent, in_normal);
}