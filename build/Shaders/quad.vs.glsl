#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

uniform vec2 min;
uniform vec2 max;

void main()
{
    vec2 w = max - min;
    vec2 s = (aPos / 2.0) + 0.5;
    gl_Position = vec4(w * s + min, -0.9999999990, 1.0); 
    TexCoords = aTexCoords;
}  