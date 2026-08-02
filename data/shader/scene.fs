#version 330 core

in vec2 vTexCoord;
in vec3 vFragPos;
in vec3 vNormal;
in vec3 vTangent;
in vec3 vBitangent;

uniform sampler2D diffuseTexture;
uniform sampler2D normalTexture;
uniform sampler2D specularTexture;   // optional, can be used for specular map

uniform vec3 lightDir;        // direction towards the light (in world space)
uniform vec3 lightColor;
uniform vec3 ambientColor;
uniform vec3 viewPos;         // camera position in world space

out vec4 FragColor;

void main()
{
    // Sample textures
    vec4 albedo = texture(diffuseTexture, vTexCoord);
    if (albedo.a < 0.1) discard;

    vec3 normalMap = texture(normalTexture, vTexCoord).rgb;
    normalMap = normalize(normalMap * 2.0 - 1.0);   // convert from [0,1] to [-1,1]

    // Construct TBN matrix (world to tangent space)
    vec3 N = normalize(vNormal);
    vec3 T = normalize(vTangent);
    vec3 B = normalize(vBitangent);
    mat3 TBN = mat3(T, B, N);   // maps tangent-space vector to world space

    // Transform sampled normal from tangent space to world space
    vec3 worldNormal = normalize(TBN * normalMap);

    // Directional light (lightDir points *towards* the light)
    vec3 lightDirWorld = normalize(-lightDir);   // assuming lightDir is direction from light to fragment
    float diff = max(dot(worldNormal, lightDirWorld), 0.0);
    vec3 diffuse = diff * lightColor;

    // Specular (Blinn-Phong for example)
    vec3 viewDir = normalize(viewPos - vFragPos);
    vec3 halfVec = normalize(lightDirWorld + viewDir);
    float spec = pow(max(dot(worldNormal, halfVec), 0.0), 32.0);
    vec3 specular = spec * lightColor;

    // Combine (ambient + diffuse + specular) * albedo
    vec3 result = (ambientColor + diffuse + specular) * albedo.rgb;
    FragColor = vec4(result, albedo.a);
}
