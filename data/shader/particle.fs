#version 330 core
in vec2 TexCoord;
in vec4 Color;

uniform sampler2D particleTex;

out vec4 FragColor;

void main()
{
    vec4 texColor = texture(particleTex, TexCoord);
    FragColor = texColor * Color;
}

