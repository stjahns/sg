#version 330 core
out vec4 color;

in vec3 position;
in vec3 normal;
in vec2 texcoord;
in vec4 FragPosLightSpace;
in mat3 TBN;

uniform vec3 viewPos;

struct Material
{
    vec3 ambient;

    sampler2D diffuse;
    sampler2D specular;
    sampler2D normal;

    float shininess;
};

uniform Material material;

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

struct SpotLight 
{
    vec3 position;
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float constant;
    float linear;
    float quadratic;
	float cutoff;
	float outercutoff;
};

struct DirectionalLight 
{
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform float numPointLights;
uniform PointLight pointLights[4];

uniform float numSpotLights;
uniform SpotLight spotLights[4];

uniform DirectionalLight directionalLight;

uniform mat4 model;

uniform sampler2D shadowMap;

float CalcShadowPointLight(int i, vec3 fragPos)
{
    vec3 fragToLight = fragPos - pointLights[i].position;
    float sampleDepth = texture(pointLights[i].depthMap, fragToLight).r * pointLights[i].far_plane;

    float fragDepth = length(fragToLight);

    float bias = 0.005;
    float shadow = fragDepth - bias > sampleDepth ? 0.0 : 1.0;

    return shadow;
}

vec3 CalcPointLight(int i)
{
    // FIXME -- many of these
    vec3 ambient = pointLights[i].ambient * material.ambient;

    vec3 lightDir = normalize(pointLights[i].position - position);

    //vec3 n = texture(material.normal, texcoord).rgb;
    vec3 n = normal;
    n = n * 2.0 - 1.0; // Fix range
    vec3 norm = normalize(TBN * n); // now in world space!

    vec3 diff = vec3(max(dot(norm, lightDir), 0.0));
    vec3 diffuse = pointLights[i].diffuse * diff;

    vec3 viewDir = normalize(viewPos - position);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);

    vec3 materialSpecular = texture(material.specular, texcoord).xyz;

    vec3 specular = pointLights[i].specular * spec * materialSpecular;

    float d = length(pointLights[i].position - position);   
    float attenuation = 1.0f / (pointLights[i].constant + pointLights[i].linear * d + pointLights[i].quadratic * d * d);

    float shadow = CalcShadowPointLight(i, position);  

    return (ambient + shadow * (diffuse + specular)) * attenuation;
}

vec3 CalcSpotLight(SpotLight light)
{
    vec3 ambient = light.ambient * material.ambient;

    vec3 lightDir = normalize(light.position - position);
    vec3 spotDir = normalize(-light.direction);

    float cosTheta = dot(lightDir, spotDir);

    float d = length(light.position - position);   
    float attenuation = 1.0f / (light.constant + light.linear * d + light.quadratic * d * d);

    if (cosTheta < light.outercutoff)
    {
        return ambient * attenuation;
    }
    else
    {
        vec3 n = texture(material.normal, texcoord).rgb;
        n = normalize(n * 2.0 - 1.0); // Fix range
        vec3 norm = normalize(TBN * n); // now in world space!

        vec3 diff = vec3(max(dot(norm, lightDir), 0.0));
        //vec3 diffuse = light.diffuse * diff * material.diffuse;
        vec3 diffuse = light.diffuse * diff;

        vec3 viewDir = normalize(viewPos - position);
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);

        vec3 materialSpecular = texture(material.specular, texcoord).xyz;
        vec3 specular = light.specular * spec * materialSpecular;


        float d = length(light.position - position);   
        float attenuation = 1.0f / (light.constant + light.linear * d + light.quadratic * d * d);

        float intensity = clamp((cosTheta - light.outercutoff) / (light.cutoff - light.outercutoff), 0.0, 1.0);

        return (ambient + intensity * (diffuse + specular)) * attenuation;
    }
}

// 1.0 -> no shadow, 0.0 -> fully in shadow
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

    vec3 materialSpecular = texture(material.specular, texcoord).xyz;
    vec3 specular = light.specular * spec * materialSpecular;

    float shadow = CalcShadow(FragPosLightSpace);

    return ambient + (diffuse + specular) * shadow;
}

void main()
{
    vec3 result = CalcDirectionalLight(directionalLight);

    //for (int i = 0; i < numPointLights; ++i)
    //{
        //result += CalcPointLight(0);
    //}

    /*
    for (int i = 0; i < numPointLights; ++i)
    {
        result += CalcPointLight(pointLights[i]);
    }
    for (int i = 0; i < numSpotLights; ++i)
    {
        result += CalcSpotLight(spotLights[i]);
    }
    */

    color = vec4(result, 1.0f);

    vec2 tCorrected = vec2(texcoord.s, 1-texcoord.t);

    color *= texture(material.diffuse, tCorrected);
}