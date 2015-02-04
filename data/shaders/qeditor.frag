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

#extension GL_EXT_texture_array : enable

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

uniform sampler2D layer1Alpha;
uniform sampler2D layer2Alpha;
uniform sampler2D layer3Alpha;

//
uniform sampler2DArray textures;
uniform sampler2DArray depthTextures;
uniform sampler2DArray alphamaps;

uniform sampler2D vertexShading;
uniform sampler2D vertexLighting;

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

uniform vec4 textureScaleOption = vec4(0, 0, 0, 0);
uniform vec4 textureScaleFar    = vec4(0.4, 0.4, 0.4, 0.4);
uniform vec4 textureScaleNear   = vec4(0.4, 0.4, 0.4, 0.4);

uniform vec3 automaticTexture      = vec3(0, 0, 0);
uniform vec3 automaticTextureStart = vec3(0, 0, 0);
uniform vec3 automaticTextureEnd   = vec3(0, 0, 0);

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
    if(textureScaleOption.x == 0 || textureScaleOption.x == 3)
        uvNear *= textureScaleNear.x;

    // multiply far
    if(textureScaleOption.x == 0 || textureScaleOption.x == 2)
        uvFar *= textureScaleFar.x;

    // divide near
    if(textureScaleOption.x == 1 || textureScaleOption.x == 2)
        uvNear /= textureScaleNear.x;

    // divide far
    if(textureScaleOption.x == 1 || textureScaleOption.x == 3)
        uvFar /= textureScaleFar.x;
}

void nearAndFarTexCoords(out vec2 uvNear, out vec2 uvFar, int layer)
{
    uvNear = texCoords * 100.0;
    uvFar  = texCoords * 10.0;

    /// Not sure if AMD supports switch cases on all drivers => IFs
    if(layer == 0)
    {
        // multiply near
        if(textureScaleOption.x == 0 || textureScaleOption.x == 3)
            uvNear *= textureScaleNear.x;

        // multiply far
        if(textureScaleOption.x == 0 || textureScaleOption.x == 2)
            uvFar *= textureScaleFar.x;

        // divide near
        if(textureScaleOption.x == 1 || textureScaleOption.x == 2)
            uvNear /= textureScaleNear.x;

        // divide far
        if(textureScaleOption.x == 1 || textureScaleOption.x == 3)
            uvFar /= textureScaleFar.x;
    }
    else if(layer == 1)
    {
        // multiply near
        if(textureScaleOption.y == 0 || textureScaleOption.y == 3)
            uvNear *= textureScaleNear.y;

        // multiply far
        if(textureScaleOption.y == 0 || textureScaleOption.y == 2)
            uvFar *= textureScaleFar.y;

        // divide near
        if(textureScaleOption.y == 1 || textureScaleOption.y == 2)
            uvNear /= textureScaleNear.y;

        // divide far
        if(textureScaleOption.y == 1 || textureScaleOption.y == 3)
            uvFar /= textureScaleFar.y;
    }
    else if(layer == 2)
    {
        // multiply near
        if(textureScaleOption.z == 0 || textureScaleOption.z == 3)
            uvNear *= textureScaleNear.z;

        // multiply far
        if(textureScaleOption.z == 0 || textureScaleOption.z == 2)
            uvFar *= textureScaleFar.z;

        // divide near
        if(textureScaleOption.z == 1 || textureScaleOption.z == 2)
            uvNear /= textureScaleNear.z;

        // divide far
        if(textureScaleOption.z == 1 || textureScaleOption.z == 3)
            uvFar /= textureScaleFar.z;
    }
    else if(layer == 3)
    {
        // multiply near
        if(textureScaleOption.w == 0 || textureScaleOption.w == 3)
            uvNear *= textureScaleNear.w;

        // multiply far
        if(textureScaleOption.w == 0 || textureScaleOption.w == 2)
            uvFar *= textureScaleFar.w;

        // divide near
        if(textureScaleOption.w == 1 || textureScaleOption.w == 2)
            uvNear /= textureScaleNear.w;

        // divide far
        if(textureScaleOption.w == 1 || textureScaleOption.w == 3)
            uvFar /= textureScaleFar.w;
    }
}

// blend function
vec4 blend(vec4 baseTexture, vec4 texture2, vec4 baseDepth, vec4 depth2, float alpha)
{
    float baseAlpha = 1.0f - alpha;

    float ma = max(baseDepth.a + baseAlpha, depth2.a + alpha) - 0.2;

    float b1 = max(baseDepth.a + baseAlpha - ma, 0);
    float b2 = max(depth2.a    + alpha     - ma, 0);

    return vec4((baseTexture.rgb * b1 + texture2.rgb * b2) / (b1 + b2), 1.0);
}

vec4 blend2(vec4 baseTexture, vec4 texture1, vec4 texture2, vec4 texture3, float alpha1, float alpha2, float alpha3, vec4 baseDepth, vec4 depth1, vec4 depth2, vec4 depth3)
{
    float baseAlpha = 1.0f - alpha2;//- alpha1 - alpha2 - alpha3;

    float ma = max(baseDepth.a + baseAlpha, depth2.a + alpha2) - 0.2;

    float b1 = max(baseDepth.a + baseAlpha - ma, 0);
    float b2 = max(depth2.a    + alpha2    - ma, 0);

    return vec4((baseTexture.rgb * b1 + texture2.rgb * b2) / (b1 + b2), 1.0);
}

vec4 blendAll(vec4 baseTexture, vec4 texture1, vec4 texture2, vec4 texture3, float alpha1, float alpha2, float alpha3, vec4 baseDepth, vec4 depth1, vec4 depth2, vec4 depth3)
{
    float baseAlpha = 1.0f - alpha1 - alpha2 - alpha3;

    float ma1 = max(max(baseDepth.a + baseAlpha, depth1.a + alpha1), max(depth2.a + alpha2, depth3.a + alpha3)) - 0.2;

    float b1 = max(baseDepth.a + baseAlpha - ma1, 0);
    float b2 = max(depth1.a    + alpha1    - ma1, 0);
    float b3 = max(depth2.a    + alpha2    - ma1, 0);
    float b4 = max(depth3.a    + alpha3    - ma1, 0);

    return normalize(vec4((baseTexture.rgb * b1 + texture1.rgb * b2 + texture2.rgb * b3 + texture3.rgb * b4) / (b1 + b2 + b3 + b4), 1.0));
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

    nearAndFarTexCoords(uvNear, uvFar, 0);

    float textureDistanceFactor = textureDistanceBlendFactor();

    // Get base texture color
    vec4 baseNear  = texture2DArray(textures, vec3(uvNear, 0));
    vec4 baseFar   = texture2DArray(textures, vec3(texCoords, 0));
    vec4 baseColor = mix(baseNear, baseFar, textureDistanceFactor);

    // Get base texture depth
    vec4 baseDNear  = texture2DArray(depthTextures, vec3(uvNear, 0));
    vec4 baseDFar   = texture2DArray(depthTextures, vec3(texCoords, 0));
    vec4 baseDColor = mix(baseDNear, baseDFar, textureDistanceFactor);

    vec4 diffuseColor = blendAll(baseColor, vec4(0, 0, 0, 0), vec4(0, 0, 0, 0), vec4(0, 0, 0, 0), 0, 0, 0,
                                 baseDColor, vec4(0, 0, 0, 0), vec4(0, 0, 0, 0), vec4(0, 0, 0, 0));

    return mix(diffuseColor, texture(vertexShading, texCoords), texture(vertexShading, texCoords).a);
}

subroutine(ShaderModelType)
vec4 shadeBaseAndLayer1()
{
    vec2 uvNear, uvFar;

    nearAndFarTexCoords(uvNear, uvFar, 0);

    float textureDistanceFactor = textureDistanceBlendFactor();

    // Get base texture color
    vec4 baseNear  = texture2DArray(textures, vec3(uvNear, 0));
    vec4 baseFar   = texture2DArray(textures, vec3(texCoords, 0));
    vec4 baseColor = mix(baseNear, baseFar, textureDistanceFactor);

    // Get base texture depth
    vec4 baseDNear  = texture2DArray(depthTextures, vec3(uvNear, 0));
    vec4 baseDFar   = texture2DArray(depthTextures, vec3(texCoords, 0));
    vec4 baseDColor = mix(baseDNear, baseDFar, textureDistanceFactor);

    nearAndFarTexCoords(uvNear, uvFar, 1);

    /// Get layer 1 texture color
    vec4 layer1Near  = texture2DArray(textures, vec3(uvNear, 1));
    vec4 layer1Far   = texture2DArray(textures, vec3(uvFar, 1));
    vec4 layer1Color = mix(layer1Near, layer1Far, textureDistanceFactor);

    /// Get layer 1 texture depth
    vec4 layer1DNear  = texture2DArray(depthTextures, vec3(uvNear, 1));
    vec4 layer1DFar   = texture2DArray(depthTextures, vec3(uvFar, 1));
    vec4 layer1DColor = mix(layer1DNear, layer1DFar, textureDistanceFactor);

    // Blend layer 1 and base texture based upon alphamap
    vec4 diffuseColor = blendAll(baseColor, layer1Color, vec4(0, 0, 0, 0), vec4(0, 0, 0, 0), texture(layer1Alpha, texCoords).r, 0, 0,
                                 baseDColor, layer1DColor, vec4(0, 0, 0, 0), vec4(0, 0, 0, 0));

    return mix(diffuseColor, texture(vertexShading, texCoords), texture(vertexShading, texCoords).a);
}

subroutine(ShaderModelType)
vec4 shadeBaseLayer1AndLayer2()
{
    vec2 uvNear, uvFar;

    nearAndFarTexCoords(uvNear, uvFar, 0);

    float textureDistanceFactor = textureDistanceBlendFactor();

    // Get base texture color
    vec4 baseNear  = texture2DArray(textures, vec3(uvNear, 0));
    vec4 baseFar   = texture2DArray(textures, vec3(texCoords, 0));
    vec4 baseColor = mix(baseNear, baseFar, textureDistanceFactor);

    // Get base texture depth
    vec4 baseDNear  = texture2DArray(depthTextures, vec3(uvNear, 0));
    vec4 baseDFar   = texture2DArray(depthTextures, vec3(texCoords, 0));
    vec4 baseDColor = mix(baseDNear, baseDFar, textureDistanceFactor);

    nearAndFarTexCoords(uvNear, uvFar, 1);

    // Get layer 1 texture color
    vec4 layer1Near  = texture2DArray(textures, vec3(uvNear, 1));
    vec4 layer1Far   = texture2DArray(textures, vec3(uvFar, 1));
    vec4 layer1Color = mix(layer1Near, layer1Far, textureDistanceFactor);

    /// Get layer 1 texture depth
    vec4 layer1DNear  = texture2DArray(depthTextures, vec3(uvNear, 1));
    vec4 layer1DFar   = texture2DArray(depthTextures, vec3(uvFar, 1));
    vec4 layer1DColor = mix(layer1DNear, layer1DFar, textureDistanceFactor);

    nearAndFarTexCoords(uvNear, uvFar, 2);

    // Get layer 2 texture color
    vec4 layer2Near  = texture2DArray(textures, vec3(uvNear, 2));
    vec4 layer2Far   = texture2DArray(textures, vec3(uvFar, 2));
    vec4 layer2Color = mix(layer2Near, layer2Far, textureDistanceFactor);

    /// Get layer 2 texture depth
    vec4 layer2DNear  = texture2DArray(depthTextures, vec3(uvNear, 2));
    vec4 layer2DFar   = texture2DArray(depthTextures, vec3(uvFar, 2));
    vec4 layer2DColor = mix(layer2DNear, layer2DFar, textureDistanceFactor);

    // Blend everything
    vec4 diffuseColor = blendAll(baseColor, layer1Color, layer2Color, vec4(0, 0, 0, 0), texture(layer1Alpha, texCoords).r, texture(layer2Alpha, texCoords).r, 0,
                                 baseDColor, layer1DColor, layer2DColor, vec4(0, 0, 0, 0));

    return mix(diffuseColor, texture(vertexShading, texCoords), texture(vertexShading, texCoords).a);
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

    nearAndFarTexCoords(uvNear, uvFar, 0);

    float textureDistanceFactor = textureDistanceBlendFactor();

    /// Get base texture color
    vec4 baseNear  = texture2DArray(textures, vec3(uvNear, 0));
    vec4 baseFar   = texture2DArray(textures, vec3(texCoords, 0));
    vec4 baseColor = mix(baseNear, baseFar, textureDistanceFactor);

    // Get base texture depth
    vec4 baseDNear  = texture2DArray(depthTextures, vec3(uvNear, 0));
    vec4 baseDFar   = texture2DArray(depthTextures, vec3(texCoords, 0));
    vec4 baseDColor = mix(baseDNear, baseDFar, textureDistanceFactor);

    nearAndFarTexCoords(uvNear, uvFar, 1);

    /// Get layer 1 texture color
    vec4 layer1Near  = texture2DArray(textures, vec3(uvNear, 1));
    vec4 layer1Far   = texture2DArray(textures, vec3(uvFar, 1));
    vec4 layer1Color = mix(layer1Near, layer1Far, textureDistanceFactor);

    /// Get layer 1 texture depth
    vec4 layer1DNear  = texture2DArray(depthTextures, vec3(uvNear, 1));
    vec4 layer1DFar   = texture2DArray(depthTextures, vec3(uvFar, 1));
    vec4 layer1DColor = mix(layer1DNear, layer1DFar, textureDistanceFactor);

    nearAndFarTexCoords(uvNear, uvFar, 2);

    /// Get layer 2 texture color
    vec4 layer2Near  = texture2DArray(textures, vec3(uvNear, 2));
    vec4 layer2Far   = texture2DArray(textures, vec3(uvFar, 2));
    vec4 layer2Color = mix(layer2Near, layer2Far, textureDistanceFactor);

    /// Get layer 2 texture depth
    vec4 layer2DNear  = texture2DArray(depthTextures, vec3(uvNear, 2));
    vec4 layer2DFar   = texture2DArray(depthTextures, vec3(uvFar, 2));
    vec4 layer2DColor = mix(layer2DNear, layer2DFar, textureDistanceFactor);

    nearAndFarTexCoords(uvNear, uvFar, 3);

    /// Get layer 3 texture color
    vec4 layer3Near  = texture2DArray(textures, vec3(uvNear, 3));
    vec4 layer3Far   = texture2DArray(textures, vec3(uvFar, 3));
    vec4 layer3Color = mix(layer3Near, layer3Far, textureDistanceFactor);

    /// Get layer 1 texture depth
    vec4 layer3DNear  = texture2DArray(depthTextures, vec3(uvNear, 3));
    vec4 layer3DFar   = texture2DArray(depthTextures, vec3(uvFar, 3));
    vec4 layer3DColor = mix(layer3DNear, layer3DFar, textureDistanceFactor);

    /// Blend layer 1 and base texture based upon alphamap
    vec4 diffuseColor = vec4(0, 0, 0, 0);

    if(automaticTexture == vec3(0, 0, 0))
        diffuseColor = blendAll(baseColor, layer1Color, layer2Color, layer3Color, texture(layer1Alpha, texCoords).r, texture(layer2Alpha, texCoords).r, texture(layer3Alpha, texCoords).r,
                                baseDColor, layer1DColor, layer2DColor, layer3DColor);
    else
    {
        vec4 baseAndLayer1Color = vec4(0, 0, 0, 0);

        if(automaticTexture.x == 0)
            baseAndLayer1Color = mix(baseColor, layer1Color, texture(layer1Alpha, texCoords).r);
        else
            baseAndLayer1Color = mix(baseColor, layer1Color, smoothstep(automaticTextureStart.x, automaticTextureEnd.x, worldPosition.y));

        /// Blend baseAndLayer1Color and layer 1 based upon alphamap
        vec4 layer1AndLayer2Color = vec4(0, 0, 0, 0);

        if(automaticTexture.y == 0)
            layer1AndLayer2Color = mix(baseAndLayer1Color, layer2Color, texture(layer2Alpha, texCoords).r);
        else
            layer1AndLayer2Color = mix(baseAndLayer1Color, layer2Color, smoothstep(automaticTextureStart.y, automaticTextureEnd.y, worldPosition.y));

        /// Now blend with layer 3 based upon alphamap

        if(automaticTexture.z == 0)
            diffuseColor = mix(layer1AndLayer2Color, layer3Color, texture(layer3Alpha, texCoords).r);
        else
            diffuseColor = mix(layer1AndLayer2Color, layer3Color, smoothstep(automaticTextureStart.z, automaticTextureEnd.z, worldPosition.y));
    }

    // overlay effect
    if(!shadingOff)
    {
        //diffuseColor += texture(vertexShading, texCoords);

        vec4 vertexShadingColor = texture(vertexShading, texCoords);
        vertexShadingColor.rgb *= vertexShadingColor.a;

        diffuseColor.rgb += vertexShadingColor.rgb;

        //diffuseColor.rgb = mix(diffuseColor.rgb, vertexShadingColor.rgb, vertexShadingColor.a);
    }

    // vertex lighting
    vec3 surfaceToLight = vec3(position.x, position.y + 10.0f, position.z) - position.xyz;

    float brightness = clamp(dot(normal, surfaceToLight) / length(surfaceToLight) * length(normal), 0.0f, 1.0f) * 2.0;

    vec4 vertexLightingColor = texture(vertexLighting, texCoords);
    vertexLightingColor.rgb  = vertexLightingColor.rgb * 2.0 - 1.0;

    diffuseColor += vec4(brightness * (vertexLightingColor.rgb * vertexLightingColor.a) * diffuseColor.rgb, diffuseColor.a);

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
