#version 330 core

layout (location = 0) in vec3  aPosition;
layout (location = 1) in vec3  aNormal;
layout (location = 2) in vec2  aTexCoord;
layout (location = 3) in vec4  aTangent;
layout (location = 4) in ivec4 aJoints;
layout (location = 5) in vec4  aWeights;

uniform mat4 boneMatrices[128];

uniform mat4 model;
uniform mat4 MVP;

out vec2 vTexCoord;
out vec3 vFragPos;
out vec3 vNormal;
out vec3 vTangent;
out vec3 vBitangent;

void main()
{
    mat4 skinMatrix = 
        aWeights.x * boneMatrices[aJoints.x] +
        aWeights.y * boneMatrices[aJoints.y] +
        aWeights.z * boneMatrices[aJoints.z] +
        aWeights.w * boneMatrices[aJoints.w];

    vec4 skinnedPos = skinMatrix * vec4(aPosition, 1.0);
    gl_Position = MVP * skinnedPos;

    vFragPos = vec3(model * skinnedPos);
    mat3 normalMat = mat3(transpose(inverse(model)));

    vNormal = normalize(normalMat * aNormal);
    vTangent = normalize(normalMat * aTangent.xyz);
    vBitangent = cross(vNormal, vTangent) * aTangent.w;

    vTexCoord = aTexCoord;
}
