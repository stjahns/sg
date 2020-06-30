#version 330 core

out vec4 color;
in vec2 TexCoords;

uniform sampler2D s1;
uniform sampler2D s2;
uniform sampler2D s3;
uniform sampler2D s4;

void main()
{
    sampler2D s = (TexCoords.s < 0.5 && TexCoords.t < 0.5) ? s1
                : (TexCoords.s > 0.5 && TexCoords.t < 0.5) ? s2
                : (TexCoords.s < 0.5 && TexCoords.t > 0.5) ? s3
                : s4;

    vec2 offset = vec2(
        (offset.s > 0.5) ? -0.5 : 0
        (offset.t > 0.5) ? -0.5 : 0
    );

    vec2 offsetCoords = (TexCoords + offset) * 2.0;

    color = texture(s, offsetCoords);
}