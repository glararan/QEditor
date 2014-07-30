/*This file is part of QEditor.

QEditor is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

QEditor is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with QEditor.  If not, see <http://www.gnu.org/licenses/>.*/

#version 400

layout (location = 0) out vec4 fragColor;

subroutine vec4 ShaderModelType();
subroutine uniform ShaderModelType shaderModel;

uniform struct FogInfo
{
    vec4 color;

    float minDistance;
    float maxDistance;
} fog;

uniform struct LineInfo
{
    float width;

    vec4 color;
    vec4 color2;
} line;

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

uniform sampler2D baseTexture;
uniform sampler2D layer1Texture;
uniform sampler2D layer2Texture;
uniform sampler2D layer3Texture;

uniform sampler2D layer1Alpha;
uniform sampler2D layer2Alpha;
uniform sampler2D layer3Alpha;

uniform sampler2D vertexShading;

uniform float colorStop1 = 0.0;
uniform float colorStop2 = 4.0;
uniform float colorStop3 = 8.0;
uniform float colorStop4 = 12.0;

uniform vec4 color1 = vec4(0.00, 0.55, 0.00, 1.00);
uniform vec4 color2 = vec4(0.89, 0.68, 0.00, 1.00);
uniform vec4 color3 = vec4(0.75, 0.00, 0.00, 1.00);
uniform vec4 color4 = vec4(1.00, 1.00, 1.00, 1.00);

uniform int   brush            = 0;
uniform vec2  cursorPos        = vec2(0, 0);
uniform float brushInnerRadius = 8;
uniform float brushOuterRadius = 10;
uniform float brushMultiplier  = 5.33333;
uniform vec4  outerBrushColor  = vec4(0, 1, 0, 1);
uniform vec4  innerBrushColor  = vec4(0, 1, 0, 1);

uniform float horizontalScale = 533.33333;

uniform vec2 viewportSize;

uniform sampler2D heightMap;

uniform float deltaTime;

uniform float baseX = 0.0f;
uniform float baseY = 0.0f;

uniform int chunkX = 0;
uniform int chunkY = 0;

uniform int textureScaleOption = 0;

uniform float textureScaleFar  = 0.4;
uniform float textureScaleNear = 0.4;

uniform bool chunkLines = false;
uniform bool highlight  = false;
uniform bool selected   = false;
uniform bool shadingOff = false;

in wireFrameVertex
{
    noperspective vec3 edgeDistance;
    vec4 worldPosition;
    vec3 worldNormal;
    vec4 position;
    vec3 normal;
    vec2 texCoords;
};

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

// divide = +size, multiply = -size
void nearAndFarTexCoords(out vec2 uvNear, out vec2 uvFar)
{
    uvNear = texCoords * 100.0;
    uvFar  = texCoords * 10.0;

    /// Not sure if AMD supports switch cases on all drivers => IFs
    // multiply near
    if(textureScaleOption == 0 || textureScaleOption == 3)
        uvNear *= textureScaleNear;

    // multiply far
    if(textureScaleOption == 0 || textureScaleOption == 2)
        uvFar *= textureScaleFar;

    // divide near
    if(textureScaleOption == 1 || textureScaleOption == 2)
        uvNear /= textureScaleNear;

    // divide far
    if(textureScaleOption == 1 || textureScaleOption == 3)
        uvFar /= textureScaleFar;
}

// ShaderModelType Subroutines
subroutine(ShaderModelType)
vec4 shadeSimpleWireFrame()
{
    vec4 color = vec4(0, 0, 0, 0);

    vec4 outColor = wireFrame(color, line.color);

    if(outColor == color)
        discard;

    return outColor;
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
vec4 shaderBaseLayer()
{
    vec2 uvNear, uvFar;

    nearAndFarTexCoords(uvNear, uvFar);

    float textureDistanceFactor = textureDistanceBlendFactor();

    // Get base texture color
    vec4 baseNear  = texture(baseTexture, uvNear);
    vec4 baseFar   = texture(baseTexture, texCoords);
    vec4 baseColor = mix(baseNear, baseFar, textureDistanceFactor);

    return mix(baseColor, texture(vertexShading, texCoords), texture(vertexShading, texCoords).a);
}

subroutine(ShaderModelType)
vec4 shadeBaseAndLayer1()
{
    vec2 uvNear, uvFar;

    nearAndFarTexCoords(uvNear, uvFar);

    float textureDistanceFactor = textureDistanceBlendFactor();

    // Get base texture color
    vec4 baseNear  = texture(baseTexture, uvNear);
    vec4 baseFar   = texture(baseTexture, texCoords);
    vec4 baseColor = mix(baseNear, baseFar, textureDistanceFactor);

    // Get layer 1 texture color
    vec4 layer1Near  = texture(layer1Texture, uvNear);
    vec4 layer1Far   = texture(layer1Texture, uvFar);
    vec4 layer1Color = mix(layer1Near, layer1Far, textureDistanceFactor);

    // Blend layer 1 and base texture based upon alphamap
    vec4 baseLayer1Color = mix(baseColor, layer1Color, texture(layer1Alpha, texCoords).r);

    return mix(baseLayer1Color, texture(vertexShading, texCoords), texture(vertexShading, texCoords).a);
}

subroutine(ShaderModelType)
vec4 shadeBaseLayer1AndLayer2()
{
    vec2 uvNear, uvFar;

    nearAndFarTexCoords(uvNear, uvFar);

    float textureDistanceFactor = textureDistanceBlendFactor();

    // Get base texture color
    vec4 baseNear  = texture(baseTexture, uvNear);
    vec4 baseFar   = texture(baseTexture, texCoords);
    vec4 baseColor = mix(baseNear, baseFar, textureDistanceFactor);

    // Get layer 1 texture color
    vec4 layer1Near  = texture(layer1Texture, uvNear);
    vec4 layer1Far   = texture(layer1Texture, uvFar);
    vec4 layer1Color = mix(layer1Near, layer1Far, textureDistanceFactor);

    // Blend layer 1 and base texture based upon alphamap
    vec4 baseAndLayer1Color = mix(baseColor, layer1Color, texture(layer1Alpha, texCoords).r);

    // Now blend with layer 2 based upon alphamap
    vec4 layer2Near  = texture(layer2Texture, uvNear);
    vec4 layer2Far   = texture(layer2Texture, 5.0 * uvFar);
    vec4 layer2Color = mix(layer2Near, layer2Far, textureDistanceFactor);

    vec4 diffuseColor = mix(baseAndLayer1Color, layer2Color, texture(layer2Alpha, texCoords).r);

    return mix(diffuseColor, texture(vertexShading, texCoords), texture(vertexShading, texCoords).a);;
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

    /// Get base texture color
    vec4 baseNear  = texture(baseTexture, uvNear);
    vec4 baseFar   = texture(baseTexture, texCoords);
    vec4 baseColor = mix(baseNear, baseFar, textureDistanceFactor);

    /// Get layer 1 texture color
    vec4 layer1Near  = texture(layer1Texture, uvNear);
    vec4 layer1Far   = texture(layer1Texture, uvFar);
    vec4 layer1Color = mix(layer1Near, layer1Far, textureDistanceFactor);

    /// Get layer 2 texture color
    vec4 layer2Near  = texture(layer2Texture, uvNear);
    vec4 layer2Far   = texture(layer2Texture, uvFar);
    vec4 layer2Color = mix(layer2Near, layer2Far, textureDistanceFactor);

    /// Get layer 3 texture color
    vec4 layer3Near  = texture(layer3Texture, uvNear);
    vec4 layer3Far   = texture(layer3Texture, 5.0 * uvFar);
    vec4 layer3Color = mix(layer3Near, layer3Far, textureDistanceFactor);

    /// Blend layer 1 and base texture based upon alphamap
    vec4 baseAndLayer1Color = mix(baseColor, layer1Color, texture(layer1Alpha, texCoords).r);

    /// Blend baseAndLayer1Color and layer 1 based upon alphamap
    vec4 layer1AndLayer2Color = mix(baseAndLayer1Color, layer2Color, texture(layer2Alpha, texCoords).r);

    /// Now blend with layer 3 based upon alphamap
    vec4 diffuseColor = mix(layer1AndLayer2Color, layer3Color, texture(layer3Alpha, texCoords).r);

    // overlay effect
    if(!shadingOff)
    {
        //diffuseColor += texture(vertexShading, texCoords);

        vec4 vertexShadingColor = texture(vertexShading, texCoords);
        vertexShadingColor.rgb *= vertexShadingColor.a;

        diffuseColor.rgb += vertexShadingColor.rgb;

        //diffuseColor.rgb = mix(diffuseColor.rgb, vertexShadingColor.rgb, vertexShadingColor.a);
    }

    // non overlay effect
    //diffuseColor = mix(diffuseColor, texture(vertexShading, texCoords), texture(vertexShading, texCoords).a);

    // Calculate the lighting model, keeping the specular component separate
    vec3 ambientAndDiff, spec;

    phongModel(ambientAndDiff, spec);

    vec4 color = vec4(ambientAndDiff, 1.0) * diffuseColor + vec4(spec, 1.0);

    return color;
}

subroutine(ShaderModelType)
vec4 shadeWorldTexturedWireframed()
{
    return wireFrame(shadeTexturedAndLit(), line.color2);
}

subroutine(ShaderModelType)
vec4 shadeHidden()
{
    return vec4(0);
}

vec4 ApplyCircle(vec4 colorIn, vec4 brushColor, vec2 distVec, float mouseDist, float maxRadius)
{
    if((maxRadius - mouseDist) > (maxRadius * 0.01f) || mouseDist >= maxRadius)
        return colorIn;

    distVec = normalize(distVec);

    float angle = atan(distVec.y, distVec.x);

    if(angle < 0)
        angle += 3.1415926535897932384626433832795 * 2;

    angle += deltaTime * 0.25f; // rotate circle

    angle = (angle * 180.0f) / 3.1415926535897932384626433832795;

    float resid = int(angle) % 22;

    if(resid < 14)
        colorIn += brushColor;

    return colorIn;
}

void main()
{
    // Compute fragment color depending upon selected shading mode
    vec4 outColor = shaderModel();

    float dist      = abs(position.z);
    float fogFactor = clamp((fog.maxDistance - dist) / (fog.maxDistance - fog.minDistance), 0.0, 1.0);

    outColor = mix(fog.color, outColor, fogFactor);

    // Borders
    if(chunkLines)
    {
        float lineWidthMin = 0.02;
        float lineWidthMax = 0.07;

        float lineWidth = mix(lineWidthMin, lineWidthMax, textureDistanceBlendFactor());

        float maxVal = 1.0 - (lineWidth / horizontalScale);
        float minVal = lineWidth / horizontalScale;

        if(texCoords.x > maxVal || texCoords.y > maxVal || texCoords.x < minVal || texCoords.y < minVal)
            outColor = mix(fog.color, vec4(1.0, 0.0, 0.0, 1.0), fogFactor);

        if(chunkX == 0 || chunkX == 3 || chunkY == 0 || chunkY == 3)
        {
            if(chunkX == 0 && texCoords.x < minVal)
                outColor = mix(fog.color, vec4(0.0, 1.0, 0.0, 1.0), fogFactor);

            if(chunkX == 3 && texCoords.x > maxVal)
                outColor = mix(fog.color, vec4(0.0, 1.0, 0.0, 1.0), fogFactor);

            if(chunkY == 0 && texCoords.y < minVal)
                outColor = mix(fog.color, vec4(0.0, 1.0, 0.0, 1.0), fogFactor);

            if(chunkY == 3 && texCoords.y > maxVal)
                outColor = mix(fog.color, vec4(0.0, 1.0, 0.0, 1.0), fogFactor);
        }

        if(selected && (texCoords.x > maxVal - 0.001 || texCoords.y > maxVal - 0.001 || texCoords.x < minVal + 0.001 || texCoords.y < minVal + 0.001))
            outColor = mix(fog.color, vec4(1.0, 0.5, 0.0, 1.0), fogFactor);
    }

    // Terrain brush
    /*if(brush == 0)
    {
        float dx = texCoords.x * horizontalScale - cursorPos.x + baseX;
        float dy = texCoords.y * horizontalScale - cursorPos.y + baseY;

        float bDist = sqrt(dx * dx + dy * dy) * brushMultiplier;

        if(bDist < brushOuterRadius)
        {
            float str = max(0, mix(-1.5, 0.5, bDist / brushOuterRadius));
            outColor += brushColor * str;
        }
    }*/

    // Terrain brush v2
    if(brush == 0)
    {
        float dx = texCoords.x * horizontalScale - cursorPos.x + baseX;
        float dy = texCoords.y * horizontalScale - cursorPos.y + baseY;

        float bDist = sqrt(dx * dx + dy * dy) * brushMultiplier;

        if(bDist < brushOuterRadius)
        {
            outColor = ApplyCircle(outColor, outerBrushColor, vec2(dx, dy), bDist, brushOuterRadius);

            if(bDist < brushInnerRadius)
                outColor = ApplyCircle(outColor, innerBrushColor, vec2(dx, dy), bDist, brushInnerRadius);
        }
    }

    if(highlight)
        outColor += vec4(0.1, 0.1, 0.1, 1.0);

    // Water
    /*if(brush == 1)
    {
        vec2 uvNormal0 = texCoords.xy * waterNoiseTile;
        uvNormal0.x += waterTime * 0.01;
        uvNormal0.y += waterTime * 0.01;

        vec2 uvNormal1 = texCoords.xy * waterNoiseTile;
        uvNormal1.x -= waterTime * 0.01;
        uvNormal1.y += waterTime * 0.01;

        vec3 normal0 = texture2D(waterNoise, uvNormal0).rgb * 2.0 - 1.0;
        vec3 normal1 = texture2D(waterNoise, uvNormal1).rgb * 2.0 - 1.0;

        vec3 normalF = normalize(normal0 + normal1); // == worldNormal or normal

        vec2 uvReflection = vec2(fragCoord.x / viewportSize.x, fragCoord.y / viewportSize.y);

        vec2 uvFinal = uvReflection.xy + waterNoiseFactor * normalF.xy;

        outColor += texture2D(waterReflection, uvFinal);

        float shadow = shadowMapping();
        shadow = shadow * 0.5 + 0.5;

        outColor *= shadow;
        outColor.a = fresnel(V, N, 0.5, 2.0);
    }*/

    fragColor = outColor;
}
