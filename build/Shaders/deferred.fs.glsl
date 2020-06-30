#version 330 core

layout(location = 0) out vec3 gPosition;
layout(location = 1) out vec3 gNormal;
layout(location = 2) out vec4 gAlbedoSpec;

in vec3 position;
in vec2 texcoord;
in mat3 TBN;

struct Material
{
    sampler2D diffuse;
    sampler2D normal;
};

uniform Material material;

void main() 
{
    gPosition = position;

    //vec3 n = texture(material.normal, texcoord).rgb;
    //n = normalize(n * 2.0 - 1.0);
    vec3 n = vec3(0, 0, 1);
    gNormal = normalize(TBN * n);

    //gAlbedoSpec = vec4(texture(material.diffuse, texcoord).rgb, 1.0);
    gAlbedoSpec = vec4(1.0);
    //gAlbedoSpec.a = texture(TODO, texcoord).rgb;
}