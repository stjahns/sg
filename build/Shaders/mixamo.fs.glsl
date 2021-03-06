#version 330 core
out vec4 color;

in vec3 position;
in vec3 normal;
in vec2 texcoord;
in vec4 FragPosLightSpace;
in mat3 TBN;

uniform vec3 viewPos;

uniform vec3 colorFactor;

struct Material
{
    vec3 ambient;

    sampler2D diffuse;
    sampler2D specular;
    sampler2D normal;

    float shininess;
};

uniform Material material;

struct DirectionalLight 
{
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform DirectionalLight directionalLight;

uniform mat4 model;

uniform sampler2D shadowMap;

float CalcShadow(vec4 fragPosLightSpace)
{
    float bias = 0.005;
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w; // [-1, 1]
    projCoords = projCoords * 0.5 + 0.5; // [0, 1] to match shadow depth map

    float fragDepth = projCoords.z;

    float shadow = 0.0f;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);

    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x,y) * texelSize).r;
            shadow += fragDepth - bias  > pcfDepth ? 1.0 : 0.0;
        }
    }

    shadow /= 9.0;

    return fragDepth > 1.0 ? 1.0 : 1.0 - shadow;
}

vec3 CalcDirectionalLight(DirectionalLight light)
{
    vec3 ambient = light.ambient * 1.0f;

    vec3 norm = normal;
    vec3 lightDir = normalize(-light.direction);

    vec3 diff = vec3(max(dot(normal, lightDir), 0.0));
    vec3 diffuse = light.diffuse * diff;

    vec3 viewDir = normalize(viewPos - position);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);

    vec3 specular = light.specular * spec;

    float shadow = CalcShadow(FragPosLightSpace);

    return (ambient + (diffuse + specular) * shadow) * colorFactor;
}

void main()
{
    vec3 result = CalcDirectionalLight(directionalLight);

    color = vec4(result, 1.0f);
}