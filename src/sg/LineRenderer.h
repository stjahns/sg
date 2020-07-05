#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>

#include "ShaderProgram.h">

using namespace glm;

class Skeleton;
class Pose;

class LineRenderer
{
public:

    LineRenderer();

    void AddLine(vec3 start, vec3 end, vec4 startColor, vec4 endColor);
    void AddLine(vec3 start, vec3 end, vec4 color);

    void AddPose(const Skeleton& skeleton, const Pose& pose, vec4 color);

    void Render(const mat4& view, const mat4& projection);

private:

    struct Vertex
    {
        Vertex(vec3 position, vec4 color)
            : position(position)
            , color(color)
        {
        }

        vec3 position;
        vec4 color;
    };

    ShaderProgram lineShader;

	GLuint vao;
	GLuint vertexBuffer;

    std::vector<Vertex> vertices;

};