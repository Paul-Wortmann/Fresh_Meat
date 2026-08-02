#version 330 core

in vec2 vTexCoord;
in vec3 vFragPos;
in vec3 vNormal;
in vec3 vTangent;
in vec3 vBitangent;
flat in uint vTileIndex;

uniform sampler2D diffuseTexture;
uniform sampler2D normalTexture;
uniform sampler2D specularTexture;

// TBOs for per‑tile data
uniform usamplerBuffer tileTypeBuffer;
uniform samplerBuffer tileStateBuffer;

uniform vec3 lightDir;
uniform vec3 lightColor;
uniform vec3 ambientColor;
uniform vec3 viewPos;

out vec4 FragColor;

void main()
{
    // Fetch per‑tile data
    uint tileType = texelFetch(tileTypeBuffer, int(vTileIndex)).r;
    vec2 tileState = texelFetch(tileStateBuffer, int(vTileIndex)).rg;

    // Determine atlas rectangle (same logic)
    vec2 uvMin, uvMax;
    if (tileType == 1u) {        // floor
        uvMin = vec2(0.0, 0.0); uvMax = vec2(0.5, 0.5);
    } else if (tileType == 2u) { // wall
        uvMin = vec2(0.5, 0.0); uvMax = vec2(1.0, 0.5);
    } else if (tileType == 3u) { // path
        uvMin = vec2(0.0, 0.5); uvMax = vec2(0.5, 1.0);
    } else {                     // none / default
        uvMin = vec2(0.5, 0.5); uvMax = vec2(1.0, 1.0);
    }

    vec2 finalUV = uvMin + vTexCoord * (uvMax - uvMin);

    vec4 albedo = texture(diffuseTexture, finalUV);
    if (albedo.a < 0.1) discard;

    vec3 normalMap = texture(normalTexture, finalUV).rgb;
    normalMap = normalize(normalMap * 2.0 - 1.0);

    vec3 N = normalize(vNormal);
    vec3 T = normalize(vTangent);
    vec3 B = normalize(vBitangent);
    mat3 TBN = mat3(T, B, N);
    vec3 worldNormal = normalize(TBN * normalMap);

    vec3 lightDirWorld = normalize(-lightDir);
    float diff = max(dot(worldNormal, lightDirWorld), 0.0);
    vec3 diffuse = diff * lightColor;

    vec3 viewDir = normalize(viewPos - vFragPos);
    vec3 halfVec = normalize(lightDirWorld + viewDir);
    float spec = pow(max(dot(worldNormal, halfVec), 0.0), 32.0);
    vec3 specular = spec * lightColor;

    vec3 result = (ambientColor + diffuse + specular) * albedo.rgb;

    if (tileState.y > 0.5)
        result = mix(result, vec3(1.0, 0.0, 0.0), 0.45);
    if (tileState.x > 0.5)
        result = mix(result, vec3(1.0, 1.0, 0.0), 0.45);

    FragColor = vec4(result, albedo.a);
}
