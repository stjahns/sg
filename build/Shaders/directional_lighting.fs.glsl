
#version 330 core

//uniform vec3 Ambient;
//uniform vec3 LightColor;
//uniform vec3 LightDirection;
//uniform vec3 HalfVector;
//uniform float Shininess;
//uniform float Strength;

in vec3 Normal;

out vec4 FragColor;

void main()
{
	vec3 Ambient = vec3(0.1, 0.1, 0.1);
	vec3 LightColor = vec3(1.0, 1.0, 1.0);
	vec3 LightDirection = vec3(1.0, 1.0, 1.0);
	vec3 HalfVector = vec3(1.0, 0.0, 0.0);
	float Shininess = 1.0;
	float Strength = 1.0;

	float diffuse = max(0.0, dot(Normal, LightDirection));
	float specular = max(0.0, dot(Normal, HalfVector));

	if (diffuse == 0.0)
		specular = 0.0;
	else
		specular = pow(specular, Shininess);

	vec3 scatteredLight = Ambient + LightColor * diffuse;
	vec3 reflectedLight = LightColor * specular * Strength;

	vec3 rgb = min(scatteredLight + reflectedLight, vec3(1.0));
	FragColor = vec4(rgb, 1.0);
}
