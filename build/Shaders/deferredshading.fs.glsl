#version 330 core

out vec4 FragColor;

in vec2 TexCoords;

struct DirectionalLight 
{
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    //sampler2D depthMap;
};

struct PointLight 
{
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float constant;
    float linear;
    float quadratic;
    float far_plane;
    samplerCube depthMap;
};

uniform float numPointLights;
uniform PointLight pointLights[4];

uniform sampler2D gPositionBuffer;
uniform sampler2D gNormalBuffer;
uniform sampler2D gAlbedoSpecBuffer;
uniform sampler2D ssaoBuffer;

uniform vec3 uViewPos;

uniform DirectionalLight uDirectionalLight;

/*
float CalcShadowDirectional(vec4 fragPosLightSpace)
{
    float bias = 0.05;
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w; // [-1, 1]
    projCoords = projCoords * 0.5 + 0.5; // [0, 1] to match shadow depth map

    float fragDepth = projCoords.z;

    float shadow = 0.0f;
    vec2 texelSize = 1.0 / textureSize(uDirectionalLight.depthMap, 0);

    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(uDirectionalLight.depthMap, projCoords.xy + vec2(x,y) * texelSize).r;
            shadow += fragDepth - bias  > pcfDepth ? 1.0 : 0.0;
        }
    }

    shadow /= 9.0;

    return fragDepth > 1.0 ? 1.0 : 1.0 - shadow;
}
*/


vec3 CalcDirectionalLight(vec3 FragPos, vec3 FragNormal)
{
    vec3 ambient = uDirectionalLight.ambient;

    vec3 lightDir = normalize(-uDirectionalLight.direction);

    vec3 diff = vec3(max(dot(FragNormal, lightDir), 0.0));
    vec3 diffuse = uDirectionalLight.diffuse * diff;

    vec3 viewDir = normalize(uViewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, FragNormal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);

    // TODO fetch from g buffer? should this be color or intensity?
    vec3 materialSpecular = vec3(0.0); 

    vec3 specular = uDirectionalLight.specular * spec * materialSpecular;

    //float shadow = CalcShadow(FragPosLightSpace);
    //return ambient + (diffuse + specular) * shadow;

    return ambient + diffuse + specular;
}

float CalcShadowPointLight(int i, vec3 fragPos)
{
    vec3 fragToLight = fragPos - pointLights[i].position;
    float sampleDepth = texture(pointLights[i].depthMap, fragToLight).r * pointLights[i].far_plane;
    float fragDepth = length(fragToLight);
    float bias = 0.05;
    float shadow = fragDepth - bias > sampleDepth ? 0.0 : 1.0;
    return shadow;
}

float CalcShadowPointLightPCF(int i, vec3 fragPos)
{
    vec3 fragToLight = fragPos - pointLights[i].position;
    float fragDepth = length(fragToLight);
    float bias = 0.05;
    /*
    float sampleDepth = texture(pointLights[i].depthMap, fragToLight).r * pointLights[i].far_plane;

    float fragDepth = length(fragToLight);

    float bias = 0.05;
    float shadow = fragDepth - bias > sampleDepth ? 0.0 : 1.0;
    return shadow;

    */

    vec3 sampleOffsetDirections[20] = vec3[]
    (
        vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
        vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
        vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
        vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
        vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
    );   

    int samples = 20;
    float samplingRadius = 0.05;
    float shadow = 0.0f;

    for (int i = 0; i < samples; ++i)
    {
        float sampleDepth = texture(pointLights[i].depthMap, fragToLight + sampleOffsetDirections[i] * samplingRadius).r;
        sampleDepth *= pointLights[i].far_plane; // undo mapping
        if (fragDepth - bias > sampleDepth)
        {
            shadow += 1.0;
        }
    }

    shadow /= float(samples);
    return 1.0 - shadow;
}

vec3 CalcPointLight(int i, vec3 FragPos, vec3 FragNorm)
{
    float ssao = texture(ssaoBuffer, TexCoords).r;
    vec3 ambient = pointLights[i].ambient * ssao;

    vec3 lightDir = normalize(pointLights[i].position - FragPos);

    vec3 diff = vec3(max(dot(FragNorm, lightDir), 0.0));
    vec3 diffuse = pointLights[i].diffuse * diff;

    vec3 viewDir = normalize(uViewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, FragNorm);

    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = pointLights[i].specular * spec;

    float d = length(pointLights[i].position - FragPos);   
    float attenuation = 1.0f / (pointLights[i].constant + pointLights[i].linear * d + pointLights[i].quadratic * d * d);

    float shadow = CalcShadowPointLight(i, FragPos);  

    return ambient + (diffuse + specular) * attenuation * shadow;
}

void main()
{

    vec3 FragPos = texture(gPositionBuffer, TexCoords).rgb;
    vec3 FragNormal = texture(gNormalBuffer, TexCoords).rgb;
    FragColor = vec4(CalcDirectionalLight(FragPos, FragNormal), 1.0);

    //FragColor += vec4(CalcPointLight(0, FragPos, FragNormal), 1.0);
    //FragColor *= texture(pointLights[0].depthMap, FragPos).r;

    for (int i = 0; i < numPointLights; ++i)
    {
        FragColor += vec4(CalcPointLight(i, FragPos, FragNormal), 1.0);
    }

    FragColor *= texture(gAlbedoSpecBuffer, TexCoords);
}