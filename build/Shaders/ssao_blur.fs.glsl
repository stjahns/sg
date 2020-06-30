#version 330 core

out float FragColor;

in vec2 TexCoords;

uniform sampler2D ssaoInput;

void main()
{
    vec2 texelSize = 1.0 / vec2(textureSize(ssaoInput, 0));
    float result = 0.0;

    for (int x = -2; x < 2; ++x)
    {
        for (int y = -2; y < 2; ++y)
        {
            result += texture(ssaoInput, TexCoords + vec2((float(x),float(y)) * texelSize)).r;
        }
    }

    result /= 16.0;

    FragColor = result;
    //FragColor = texture(ssaoInput, TexCoords).r;
}