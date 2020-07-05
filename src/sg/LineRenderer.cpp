#include "defines.h"
#include "LineRenderer.h"

LineRenderer::LineRenderer() : lineShader("shaders/line.vs.glsl", "shaders/line.fs.glsl")
{
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), BUFFER_OFFSET(offsetof(Vertex, position)));
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), BUFFER_OFFSET(offsetof(Vertex, color)));

    glBindVertexArray(0);
}

void LineRenderer::AddLine(vec3 start, vec3 end, vec4 startColor, vec4 endColor)
{
    vertices.emplace_back(start, startColor);
    vertices.emplace_back(end, endColor);
}

void LineRenderer::AddLine(vec3 start, vec3 end, vec4 color)
{
    vertices.emplace_back(start, color);
    vertices.emplace_back(end, color);
}

void LineRenderer::Render(const mat4& view, const mat4& projection)
{
    glBindVertexArray(vao);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    lineShader.Use();
    lineShader.SetUniform("view", view);
    lineShader.SetUniform("projection", projection);

    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), &vertices[0], GL_DYNAMIC_DRAW);
    glDrawArrays(GL_LINES, 0, vertices.size());

    glDisable(GL_BLEND);
    glBindVertexArray(0);

    vertices.clear();
}