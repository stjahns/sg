#pragma once;

#include <glm/glm.hpp>
#include <glad/glad.h>

using namespace glm;

struct PointLight
{
	PointLight() 
		: ambient(0.1f)
		, diffuse(1.0f)
		, specular(1.0f)
		, constant(0.1f)
		, linear(0.01f)
		, quadratic(0.001f)
	{

		glGenTextures(1, &depthCubemap);

		const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
		glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
		for (unsigned int i = 0; i < 6; ++i)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
				SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		}

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	}

	vec3 position;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

    float constant;
    float linear;
    float quadratic;

	GLuint depthCubemap;
};

struct DirectionalLight
{
	DirectionalLight()
		: direction(-0.3f, -1.0f, -0.1f)
		, diffuse(1.0f, 1.0f, 1.0f)
		, specular(1.0f, 1.0f, 1.0f)
		, ambient(0.1f)
	{
	}

	vec3 direction;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

struct SpotLight
{
	SpotLight()
		: direction(0.0f, -1.0f, 0.0f)
		, ambient(0.1f)
		, diffuse(1.0f)
		, specular(1.0f)
		, constant(1.0f)
		, linear(0.7f)
		, quadratic(1.8f)
		, cutOff(0.5f)
		, outerCutOff(0.5f)
	{
	}

	vec3 position;
	vec3 direction;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

    float constant;
    float linear;
    float quadratic;

	float cutOff;
	float outerCutOff;
};

