#version 400

layout (location = 0) out vec4 fragColor;

uniform struct FogInfo
{
    vec4 color;

    float minDistance;
    float maxDistance;
} fog;

//in vec4 fragColor;

in wireFrameVertex
{
    noperspective vec3 edgeDistance;
    vec4 worldPosition;
    vec3 worldNormal;
    vec4 position;
    vec3 normal;
    vec2 texCoords;
};

uniform sampler2D heightMap;

void main()
{
    // Blend with fog color
    float dist      = abs(position.z);
    float fogFactor = clamp((fog.maxDistance - dist) / (fog.maxDistance - fog.minDistance), 0.0, 1.0);

    vec4 outColor = mix(fog.color, fragColor, fogFactor);

    //fragColor = vec4(0.0, 1.0, 0.0, 1.0);//outColor;
    fragColor = texture(heightMap, texCoords);
}
