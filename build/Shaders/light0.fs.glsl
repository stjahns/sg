#version 330 core
out vec4 color;

in vec3 position;
in vec3 normal;

//uniform vec3 objectColor;
//uniform vec3 lightColor;
//uniform vec3 lightPosition;
uniform vec3 viewPos;

struct Material
{
    vec3 diffuse;
    vec3 ambient;
    vec3 specular;
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

vec3 CalcPointLight(PointLight light)
{
    vec3 ambient = light.ambient * material.ambient;

    vec3 norm = normalize(normal);
    vec3 lightDir = normalize(light.position - position);

    vec3 diff = vec3(max(dot(norm, lightDir), 0.0));
    vec3 diffuse = light.diffuse * diff * material.diffuse;

    vec3 viewDir = normalize(viewPos - position);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = light.specular * spec * material.specular;

    float d = length(light.position - position);   
    float attenuation = 1.0f / (light.constant + light.linear * d + light.quadratic * d * d);

    return (ambient + diffuse + specular) * attenuation;
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
        vec3 norm = normalize(normal);

        vec3 diff = vec3(max(dot(norm, lightDir), 0.0));
        vec3 diffuse = light.diffuse * diff * material.diffuse;

        vec3 viewDir = normalize(viewPos - position);
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
        vec3 specular = light.specular * spec * material.specular;

        float d = length(light.position - position);   
        float attenuation = 1.0f / (light.constant + light.linear * d + light.quadratic * d * d);

        float intensity = clamp((cosTheta - light.outercutoff) / (light.cutoff - light.outercutoff), 0.0, 1.0);

        return (ambient + intensity * (diffuse + specular)) * attenuation;
    }
}

vec3 CalcDirectionalLight(DirectionalLight light)
{
    vec3 ambient = light.ambient * material.ambient;

    vec3 norm = normalize(normal);
    vec3 lightDir = normalize(-light.direction);

    vec3 diff = vec3(max(dot(norm, lightDir), 0.0));
    vec3 diffuse = light.diffuse * diff * material.diffuse;

    vec3 viewDir = normalize(viewPos - position);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = light.specular * spec * material.specular;

    return ambient + diffuse + specular;
}

void main()
{
    vec3 result = CalcDirectionalLight(directionalLight);

    for (int i = 0; i < numPointLights; ++i)
    {
        result += CalcPointLight(pointLights[i]);
    }

    for (int i = 0; i < numSpotLights; ++i)
    {
        result += CalcSpotLight(spotLights[i]);
    }
    
    color = vec4(result, 1.0f);
}