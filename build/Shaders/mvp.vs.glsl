#version 330 core

// TODO - rename this to "default" ??

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_texcoord;
layout(location = 3) in vec3 in_tangent;
layout(location = 4) in vec3 in_bitangent;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;

out vec3 normal;
out vec3 position;
out vec2 texcoord;
out vec4 FragPosLightSpace;

out mat3 TBN;

void main() 
{
    mat4 MVP = projection * view * model;
    gl_Position = MVP * vec4(in_position, 1.0);
    position = vec3(model * vec4(in_position, 1.0));
	normal = in_normal;
    texcoord = in_texcoord;
    FragPosLightSpace = lightSpaceMatrix * vec4(position, 1.0);
    TBN = mat3(in_tangent, in_bitangent, in_normal);
}