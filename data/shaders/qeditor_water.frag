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

uniform samplerCube reflectionTexture;
uniform samplerCube depthTexture;

uniform float colorStop1 = 0.0;
uniform float colorStop2 = 4.0;
uniform float colorStop3 = 8.0;
uniform float colorStop4 = 12.0;

uniform vec4 color1 = vec4(0.00, 0.55, 0.00, 1.00);
uniform vec4 color2 = vec4(0.89, 0.68, 0.00, 1.00);
uniform vec4 color3 = vec4(0.75, 0.00, 0.00, 1.00);
uniform vec4 color4 = vec4(1.00, 1.00, 1.00, 1.00);

uniform float horizontalScale = 533.33333;
uniform float deltaTime       = 0.0;

uniform vec2 viewportSize;

uniform sampler2D heightMap;

uniform float baseX = 0.0f;
uniform float baseY = 0.0f;

uniform int chunkX = 0;
uniform int chunkY = 0;

uniform vec3 eyePos;

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

float Fresnel(float NdotL, float bias, float power)
{
    float facing = (1.0 - NdotL);

    return max(bias + (1.0 - bias) * pow(facing, power), 0.0);
}

float textureDistanceBlendFactor()
{
    float dist = abs(position.z);

    return (dist - 30.0) / (30.0 - 5.0) / 5.3; // 5.3 = brushmultiplier
}

// divide = +size, multiply = -size
void nearAndFarTexCoords(out vec2 uvNear, out vec2 uvFar)
{
    uvNear = texCoords * 100.0;
    uvFar  = texCoords * 10.0;
}

// ShaderModelType Subroutines
subroutine(ShaderModelType)
vec4 shadeWireFrame()
{
    vec4 color = vec4(0, 0, 0, 0);

    return wireFrame(color, line.color);
}

subroutine(ShaderModelType)
vec4 shadeWater()
{
    /*vec3 eye       = normalize(worldPosition.xyz - eyePos);
    vec3 reflected = normalize(refract(eye, normalize(worldNormal), 1.1)); // 1.3330

    vec4 diffuseColor = mix(vec4(0.0, 0.5, 1.0, 1.0), texture(reflectionTexture, reflected), 0.6);*/

    vec4 diffuseColor = texture(baseTexture, texCoords + deltaTime * 0.01);

    // Calculate the lighting model, keeping the specular component separate
    vec3 ambientAndDiff, spec;

    phongModel(ambientAndDiff, spec);

    vec4 color = vec4(ambientAndDiff, 1.0) * diffuseColor + vec4(spec, 1.0);
    color.a    = 0.5;

    return color;
}

subroutine(ShaderModelType)
vec4 shadeWaterWireframed()
{
    return wireFrame(shadeWater(), vec4(0));
}

subroutine(ShaderModelType)
vec4 shadeHidden()
{
    return vec4(0);
}

float fresnel(vec3 incident, vec3 normal, float bias, float power)
{
    float scale = 1.0 - bias;

    return bias + pow(1.0 - dot(incident, normal), power) * scale;
}

void main()
{
    // Compute fragment color depending upon selected shading mode
    vec4 outColor = shadeWater();//shaderModel();

    float dist      = abs(position.z);
    float fogFactor = clamp((fog.maxDistance - dist) / (fog.maxDistance - fog.minDistance), 0.0, 1.0);

    fragColor = mix(fog.color, outColor, fogFactor);
}