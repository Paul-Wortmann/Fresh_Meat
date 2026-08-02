#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec4 aTangent;

uniform mat4 model;
uniform mat4 MVP;

out vec2 vTexCoord;
out vec3 vFragPos;
out vec3 vNormal;
out vec3 vTangent;
out vec3 vBitangent;
flat out uint vTileIndex;   // per‑tile index

void main()
{
    gl_Position = MVP * vec4(aPos, 1.0);
    vFragPos = vec3(model * vec4(aPos, 1.0));
    mat3 normalMat = mat3(transpose(inverse(model)));

    vNormal = normalize(normalMat * aNormal);
    vTangent = normalize(normalMat * aTangent.xyz);
    vBitangent = cross(vNormal, vTangent) * aTangent.w;
    vTexCoord = aTexCoord;

    // 4 vertices per tile
    vTileIndex = uint(gl_VertexID) / 4u;
}

