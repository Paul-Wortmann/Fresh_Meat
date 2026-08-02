#version 330 core

out vec4 FragColor;
in vec2 TexCoord;

uniform sampler2D sceneTex;
uniform sampler2D uiTex;

void main()
{
    vec4 scene = texture(sceneTex, TexCoord);
    vec4 ui    = texture(uiTex, TexCoord);

    FragColor = mix(scene, ui, ui.a);
}