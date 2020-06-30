#version 330 core

uniform vec3 Ambient;
uniform vec3 LightColor;
uniform vec3 LightDirection;
uniform vec3 LightPosition;
uniform vec3 HalfVector;
uniform float Shininess;
uniform float Strength;

uniform vec3 EyeDirection;
uniform float ConstantAttenuation;
uniform float LinearAttenuation;
uniform float QuadraticAttenuation;

uniform vec3 ConeDirection;
uniform float SpotCosCutoff;
uniform float SpotExponent;

in vec4 Color;
in vec3 Normal;
in vec4 Position;

out vec4 FragColor;

void main()
{
	// SPOT LIGHT

	vec3 lightDirection = LightPosition - vec3(Position);
	float lightDistance = length(lightDirection);
	lightDirection = lightDirection / lightDistance; // normalize so dot gives cosines

	float attenuation = 1.0 /
		(ConstantAttenuation +
		 LinearAttenuation * lightDistance +
		 QuadraticAttenuation * lightDistance * lightDistance);

	float spotCos = dot(lightDirection, -ConeDirection);

	if (spotCos < 0.99)
		attenuation = 0.0;
	else
		attenuation *= pow(spotCos - 0.99, 0.4);

	vec3 halfVector = normalize(lightDirection + EyeDirection);
	float diffuse = max(0.0, dot(Normal, lightDirection));
	float specular = max(0.0, dot(Normal, halfVector));

	if (diffuse == 0.0)
		specular = 0.0;
	else
		specular = pow(specular, Shininess) * Strength;

	vec3 scatteredLight = Ambient + LightColor * diffuse * attenuation;
	vec3 reflectedLight = LightColor * specular * attenuation;

	vec3 rgb = min(Color.rgb * scatteredLight + reflectedLight, vec3(1.0));

	// POSITIONAL LIGHT

/*
	vec3 lightDirection = LightPosition - vec3(Position);
	float lightDistance = length(lightDirection);
	lightDirection = lightDirection / lightDistance; // normalize so dot gives cosines

	float attenuation = 1.0 /
		(ConstantAttenuation +
		 LinearAttenuation * lightDistance +
		 QuadraticAttenuation * lightDistance * lightDistance);

	vec3 halfVector = normalize(lightDirection + EyeDirection);
	float diffuse = max(0.0, dot(Normal, lightDirection));
	float specular = max(0.0, dot(Normal, halfVector));

	if (diffuse == 0.0)
		specular = 0.0;
	else
		specular = pow(specular, Shininess) * Strength;

	vec3 scatteredLight = Ambient + LightColor * diffuse * attenuation;
	vec3 reflectedLight = LightColor * specular * attenuation;

	vec3 rgb = min(Color.rgb * scatteredLight + reflectedLight, vec3(1.0));
	*/

	// DIRECTIONAL LIGHT

	/*
	float diffuse = max(0.0, dot(Normal, LightDirection));
	float specular = max(0.0, dot(Normal, HalfVector));
	
	if (diffuse == 0.0)
		specular = 0.0;
	else
		specular = pow(specular, Shininess);


	vec3 scatteredLight = Ambient + LightColor * diffuse;
	vec3 reflectedLight = LightColor * specular * Strength;

	vec3 rgb = min(Color.rgb * scatteredLight + reflectedLight, vec3(1.0));
	*/

	FragColor = vec4(rgb, Color.a);
}
