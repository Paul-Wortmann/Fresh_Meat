#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform vec4 color;
uniform sampler2D uiTexture;   // new uniform
uniform bool useTexture;       // if true, sample texture

void main()
{
    if (useTexture)
    {
        vec4 texColor = texture(uiTexture, TexCoord);
        FragColor = texColor * color;   // modulate with color (e.g., for tint)
    }
    else
    {
        FragColor = color;
    }
}

