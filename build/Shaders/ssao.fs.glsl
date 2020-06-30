#version 330 core

out float ssao;
in vec2 TexCoords;

uniform sampler2D gPositionBuffer;
uniform sampler2D gNormalBuffer;
uniform sampler2D texNoise;

uniform vec3 samples[64];

uniform int kernelSize = 64;
uniform float radius = 0.25;
uniform float bias = 0.005;

uniform mat4 view;
uniform mat4 projection;

const vec2 noiseScale = vec2(1280.0/4.0, 800.0/4.0);

float ComputeSSAO()
{
    vec3 fragPos = texture(gPositionBuffer, TexCoords).rgb;
    fragPos = (view * vec4(fragPos, 1.0)).xyz;

    vec3 normal = normalize(texture(gNormalBuffer, TexCoords).rgb);
    normal = (view * vec4(normal, 0.0)).xyz;

    vec3 randomVec = normalize(texture(texNoise, TexCoords * noiseScale).xyz);

    vec3 tangent   = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN       = mat3(tangent, bitangent, normal);  

    float occlusion = 0.0;

    for (int i = 0; i < kernelSize; ++i)
    {
        vec3 sample = TBN * samples[i];
        sample = fragPos + sample * radius;

        vec4 offset = vec4(sample, 1.0);
        offset      = projection * offset; // to clip space
        offset.xyz /= offset.w; // perspective divide
        offset.xyz  = offset.xyz * 0.5 + 0.5; // to [0.0 , 1.0]

        vec3 samplePos = texture(gPositionBuffer, offset.xy).rgb;
        float sampleDepth = (view * vec4(samplePos, 1.0)).z;

        // range check & accumulate
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
        occlusion += (sampleDepth >= sample.z + bias ? 1.0 : 0.0) * rangeCheck;  
    }

    return 1.0 - (occlusion / kernelSize);
}

void main()
{
    ssao = ComputeSSAO();
}