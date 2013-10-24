#version 400

subroutine vec4 ShaderModelType();
subroutine uniform ShaderModelType shaderModel;

uniform struct LineInfo
{
  float width;
  vec4 color;
} line;

uniform struct FogInfo
{
    vec4 color;
    float minDistance;
    float maxDistance;
} fog;

uniform struct LightInfo
{
    vec4 position;  // Light position in eye coords.
    vec3 intensity;
} light;

uniform struct MaterialInfo
{
    vec3 Ka;         // Ambient reflectivity
    vec3 Kd;         // Diffuse reflectivity
    vec3 Ks;         // Specular reflectivity
    float shininess; // Specular shininess exponent
} material;

uniform sampler2D grassTexture;
uniform sampler2D rockTexture;
uniform sampler2D snowTexture;

uniform float colorStop1 = 0.0;
uniform float colorStop2 = 4.0;
uniform float colorStop3 = 8.0;
uniform float colorStop4 = 12.0;

uniform vec4 color1 = vec4(0.00, 0.55, 0.00, 1.00);
uniform vec4 color2 = vec4(0.89, 0.68, 0.00, 1.00);
uniform vec4 color3 = vec4(0.75, 0.00, 0.00, 1.00);
uniform vec4 color4 = vec4(1.00, 1.00, 1.00, 1.00);

uniform int   brush           = 0;
uniform vec2  cursorPos       = vec2(0, 0);
uniform float brushRadius     = 10;
uniform float brushMultiplier = 5.33333;
uniform vec4  brushColor      = vec4(0, 1, 0, 1);

uniform float horizontalScale = 10.0;

uniform vec2 viewportSize;

in wireFrameVertex
{
    noperspective vec3 edgeDistance;
    vec4 worldPosition;
    vec3 worldNormal;
    vec4 position;
    vec3 normal;
    vec2 texCoords;
};

layout (location = 0) out vec4 fragColor;

// Helper functions
vec4 linearGradient(const in float t)
{
    vec4 color;

    if(t < colorStop1)
        color = color1;
    else if(t < colorStop2)
    {
        float tLocal = (t - colorStop1) / (colorStop2 - colorStop1);

        color = mix(color1, color2, tLocal);
    }
    else if(t < colorStop3)
    {
        float tLocal = (t - colorStop2) / (colorStop3 - colorStop2);

        color = mix(color2, color3, tLocal);
    }
    else if(t < colorStop4)
    {
        float tLocal = (t - colorStop3) / (colorStop4 - colorStop3);

        color = mix(color3, color4, tLocal);
    }
    else
        color = color4;

    return color;
}

void phongModel(out vec3 ambientAndDiff, out vec3 spec)
{
    // Some useful vectors
    vec3 s = normalize(vec3(light.position)/* - position.xyz*/);
    vec3 v = normalize(-position.xyz);
    vec3 n = normalize(normal);
    vec3 r = reflect(-s, n);

    // Calculate the ambient contribution
    vec3 ambient = light.intensity * material.Ka;

    // Calculate the diffuse contribution
    float sDotN  = max(dot(s, n), 0.0);
    vec3 diffuse = light.intensity * material.Kd * sDotN;

    // Sum the ambient and diffuse contributions
    ambientAndDiff = ambient + diffuse;

    // Calculate the specular highlight component
    spec = vec3(0.0);

    if(sDotN > 0.0)
        spec = light.intensity * material.Ks * pow(max(dot(r, v), 0.0), material.shininess);
}

vec4 wireFrame(const in vec4 color, const in vec4 wireFrameColor)
{
    // Find the smallest distance between the fragment and a triangle edge
    float d = min(edgeDistance.x, edgeDistance.y);
    d       = min(d, edgeDistance.z);

    // Blend between line color and triangle color
    float mixVal;

    if(d < line.width - 0.5)
        mixVal = 1.0;
    else if(d > line.width + 0.5)
        mixVal = 0.0;
    else
    {
        float x = d - (line.width - 1.0);

        mixVal = exp2(-2.0 * (x * x));
    }

    vec4 c = mix(color, wireFrameColor, mixVal);

    return c;
}

float textureDistanceBlendFactor()
{
    float dist = abs(position.z);

    return (dist - 30.0) / (30.0 - 5.0) / brushMultiplier;
}

void nearAndFarTexCoords(out vec2 uvNear, out vec2 uvFar)
{
    uvNear = texCoords * 100.0;
    uvFar  = texCoords * 10.0;
}

// ShaderModelType Subroutines
subroutine(ShaderModelType)
vec4 shadeSimpleWireFrame()
{
    vec4 color = vec4(0, 0, 0, 0);

    return wireFrame(color, line.color);
}

subroutine(ShaderModelType)
vec4 shadeWorldHeight()
{
    vec4 color = linearGradient(worldPosition.y);

    return wireFrame(color, 0.7 * color);
}

subroutine(ShaderModelType)
vec4 shadeWorldNormal()
{
    return vec4(0.5 * (worldNormal + vec3(1.0)), 1.0);
}

subroutine(ShaderModelType)
vec4 shadeGrass()
{
    vec2 uvNear, uvFar;

    nearAndFarTexCoords(uvNear, uvFar);

    float textureDistanceFactor = textureDistanceBlendFactor();

    // Get grass texture color
    vec4 grassNear  = texture(grassTexture, uvNear);
    vec4 grassFar   = texture(grassTexture, texCoords);
    vec4 grassColor = mix(grassNear, grassFar, textureDistanceFactor);

    return grassColor;
}

subroutine(ShaderModelType)
vec4 shadeGrassAndRocks()
{
    vec2 uvNear, uvFar;

    nearAndFarTexCoords(uvNear, uvFar);

    float textureDistanceFactor = textureDistanceBlendFactor();

    // Get grass texture color
    vec4 grassNear  = texture(grassTexture, uvNear);
    vec4 grassFar   = texture(grassTexture, texCoords);
    vec4 grassColor = mix(grassNear, grassFar, textureDistanceFactor);

    // Get rock texture color
    vec4 rockNear  = texture(rockTexture, uvNear);
    vec4 rockFar   = texture(rockTexture, uvFar);
    vec4 rockColor = mix(rockNear, rockFar, textureDistanceFactor);

    // Blend rock and grass texture based upon the worldNormal vector
    vec4 grassRockColor = mix(rockColor, grassColor, smoothstep(0.75, 0.95, clamp(worldNormal.y, 0.0, 1.0)));

    return grassRockColor;
}

subroutine(ShaderModelType)
vec4 shadeGrassRocksAndSnow()
{
    vec2 uvNear, uvFar;

    nearAndFarTexCoords(uvNear, uvFar);

    float textureDistanceFactor = textureDistanceBlendFactor();

    // Get grass texture color
    vec4 grassNear  = texture(grassTexture, uvNear);
    vec4 grassFar   = texture(grassTexture, texCoords);
    vec4 grassColor = mix(grassNear, grassFar, textureDistanceFactor);

    // Get rock texture color
    vec4 rockNear  = texture(rockTexture, uvNear);
    vec4 rockFar   = texture(rockTexture, uvFar);
    vec4 rockColor = mix(rockNear, rockFar, textureDistanceFactor);

    // Blend rock and grass texture based upon the worldNormal vector
    vec4 grassRockColor = mix(rockColor, grassColor, smoothstep(0.75, 0.95, clamp(worldNormal.y, 0.0, 1.0)));

    // Now blend with snow based upon world height
    vec4 snowNear  = texture(snowTexture, uvNear);
    vec4 snowFar   = texture(snowTexture, 5.0 * uvFar);
    vec4 snowColor = mix(snowNear, snowFar, textureDistanceFactor);

    vec4 diffuseColor = mix(grassRockColor, snowColor, smoothstep(10.0, 15.0, worldPosition.y));

    return diffuseColor;
}

subroutine(ShaderModelType)
vec4 shadeLightingFactors()
{
    vec3 ambientAndDiff, spec;

    phongModel(ambientAndDiff, spec);

    vec4 lightingFactors = vec4(ambientAndDiff, 1.0) + vec4(spec, 1.0);

    return lightingFactors;
}

subroutine(ShaderModelType)
vec4 shadeTexturedAndLit()
{
    vec2 uvNear, uvFar;

    nearAndFarTexCoords(uvNear, uvFar);

    float textureDistanceFactor = textureDistanceBlendFactor();

    // Get grass texture color
    vec4 grassNear  = texture(grassTexture, uvNear);
    vec4 grassFar   = texture(grassTexture, texCoords);
    vec4 grassColor = mix(grassNear, grassFar, textureDistanceFactor);

    // Get rock texture color
    vec4 rockNear  = texture(rockTexture, uvNear);
    vec4 rockFar   = texture(rockTexture, uvFar);
    vec4 rockColor = mix(rockNear, rockFar, textureDistanceFactor);

    // Blend rock and grass texture based upon the worldNormal vector
    vec4 grassRockColor = mix(rockColor, grassColor, smoothstep(0.75, 0.95, clamp(worldNormal.y, 0.0, 1.0)));

    // Now blend with snow based upon world height
    vec4 snowNear  = texture(snowTexture, uvNear);
    vec4 snowFar   = texture(snowTexture, 5.0 * uvFar);
    vec4 snowColor = mix(snowNear, snowFar, textureDistanceFactor);

    vec4 diffuseColor = mix(grassRockColor, snowColor, smoothstep(10.0, 15.0, worldPosition.y));

    // Calculate the lighting model, keeping the specular component separate
    vec3 ambientAndDiff, spec;

    phongModel(ambientAndDiff, spec);

    vec4 color = vec4(ambientAndDiff, 1.0) * diffuseColor + vec4(spec, 1.0);

    return color;
}

subroutine(ShaderModelType)
vec4 shadeWorldTexturedWireframed()
{
    return wireFrame(shadeTexturedAndLit(), vec4(0));
}

subroutine(ShaderModelType)
vec4 shadeHidden()
{
    return vec4(0);
}

void main()
{
    // Compute fragment color depending upon selected shading mode
    vec4 c = shaderModel();

    // Blend with fog color
    float dist      = abs(position.z);
    float fogFactor = (fog.maxDistance - dist) / (fog.maxDistance - fog.minDistance);

    fogFactor = clamp(fogFactor, 0.0, 1.0);
    vec4 outColor = mix(fog.color, c, fogFactor);

    // Terrain brush
    if(brush == 1)
    {
        float dx = texCoords.x * horizontalScale - cursorPos.x;
        float dy = texCoords.y * horizontalScale - cursorPos.y;

        float bDist = sqrt(dx * dx + dy * dy) * (brushMultiplier);

        if(bDist < brushRadius)
        {
            float str = max(0, mix(-1.5, 0.5, bDist / brushRadius));
            outColor += brushColor * str;
        }
    }

    // Borders
    if(brush == 1)
    {
        float lineWidth = 0.1;

        float maxVal = 1.0 - (lineWidth / horizontalScale);
        float minVal = lineWidth / horizontalScale;

        if(texCoords.x > maxVal || texCoords.y > maxVal || texCoords.x < minVal || texCoords.y < minVal)
            outColor = vec4(1.0, 0.0, 0.0, 1.0);
    }

    fragColor = outColor;
}
