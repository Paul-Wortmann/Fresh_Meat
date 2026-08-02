#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;

uniform mat4 MVP;
uniform mat4 model;
uniform vec4 particleColor;

out vec2 TexCoord;
out vec4 Color;

void main()
{
    gl_Position = MVP * vec4(aPos, 0.0, 1.0);
    TexCoord = aTexCoord;
    Color = particleColor;
}

