#version 330 core

out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D colorBuffer;

vec3 invertColor(vec3 color)
{
    return vec3(1.0 - color);
}

void main()
{
    vec3 color = texture(colorBuffer, TexCoords).rgb;

    // gamma correction 
    const float gamma = 2.2;
    color = pow(color, vec3(1.0 / gamma));

    FragColor = vec4(color, 1.0);
}