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

in wireFrameVertex
{
    noperspective vec3 edgeDistance;
    vec4 worldPosition;
    vec3 worldNormal;
    vec4 position;
    vec4 genericPosition;
    vec3 normal;
    vec2 texCoords;
    /*vec4 lightVertexPosition;
    vec3 lightPosition;*/
    vec3 reflection;
    vec3 refraction;
};

uniform mat4 skyboxMatrix;
uniform mat4 reflectionMatrix;
uniform mat4 projMatrix;
uniform mat4 modelMatrix;
uniform mat4 modelViewMatrix;
uniform mat3 worldNormalMatrix;
uniform mat3 normalMatrix;
uniform mat4 mvp;

uniform vec3 cameraPosition;

uniform float horizontalScale = 533.33333;

uniform struct FogInfo
{
    vec4 color;
    float discardDistance;

    float minDistance;
    float maxDistance;

    float minAlphaDistance;
    float maxAlphaDistance;
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

uniform float deltaTime = 0.0;

uniform sampler2D baseTexture;
uniform sampler2D heightMap;

uniform sampler2D reflectionTexture;
/*uniform sampler2D depthTexture;*/

//uniform sampler2D shadowMap;
uniform samplerCube cubeMap;

/*uniform mat4 lightModelViewProjectionMatrix;
uniform mat4 lightMatrix;*/

mat4 reflectionMat = mat4(0.5f, 0.0f, 0.0f, 0.5f,
                          0.0f, 0.5f, 0.0f, 0.5f,
                          0.0f, 0.0f, 0.0f, 0.5f,
                          0.0f, 0.0f, 0.0f, 1.0f);

vec4 linearGradient(const in float value, const in float limit1, const in float limit2, const in vec4 color1, const in vec4 color2)
{
    float tLocal = (value - limit1) / (limit2 - limit1);

    return mix(color1, color2, tLocal);
}

vec4 worldHeight(const in float t, const in float limit1, const in float limit2, const in float limit3, const in float limit4)
{
    vec4 color1 = vec4(0.00, 0.55, 0.00, 1.00);
    vec4 color2 = vec4(0.89, 0.68, 0.00, 1.00);
    vec4 color3 = vec4(0.75, 0.00, 0.00, 1.00);
    vec4 color4 = vec4(1.00, 1.00, 1.00, 1.00);

    vec4 color;

    if(t < limit1)
        color = color1;
    else if(t < limit2)
        color = linearGradient(t, limit1, limit2, color1, color2);
    else if(t < limit3)
        color = linearGradient(t, limit2, limit3, color2, color3);
    else if(t < limit4)
        color = linearGradient(t, limit3, limit4, color3, color4);
    else
        color = color4;

    return color;
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

void phongModel(out vec3 ambientAndDiff, out vec3 spec)
{
    // Some useful vectors
    vec3 s = normalize(vec3(light.position.xyz * normalMatrix) /*- position.xyz*/);
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

float Fresnel(float NdotL, float bias, float power)
{
    float facing = (1.0 - NdotL);

    return max(bias + (1.0 - bias) * pow(facing, power), 0.0);
}

float getTimeProgress(float speed)
{
    float time = deltaTime * 1000.0;
    speed      = 10000.0 / speed;

    return float(int(time) / int(speed));
}

void nearAndFarTexCoords(out vec2 uvNear, out vec2 uvFar)
{
    uvNear = texCoords;
    uvFar  = texCoords;
}

/*float getShadowValue()
{
    float shadowValue = 0.0;

    vec4 lightVertexPosition2 = lightVertexPosition;
         lightVertexPosition2 /= lightVertexPosition2.w;

        //if(texture2D(shadowMap, lightVertexPosition2.xy).r > lightVertexPosition2.z)
        //	return 0.3;
        //return 0.0;

    for(float x = -0.0001; x <= 0.0001; x += 0.0001)
    {
        for(float y = -0.0001 ; y <= 0.0001; y += 0.0001)
        {
            vec2 moments = vec2(1.0, 1.0) - texture2D(shadowMap, lightVertexPosition2.xy + vec2(x, y)).rg;

            if(lightVertexPosition2.z <= moments.x)
                shadowValue += 1.0;
            else
            {
                float variance = moments.y - (moments.x * moments.x);
                      variance = max(variance, 0.002);

                float d     = lightVertexPosition2.z - moments.x;
                float p_max = variance / (variance + d * d);

                shadowValue += p_max;
            }
        }
    }

    return shadowValue / 9;
}*/

subroutine(ShaderModelType)
vec4 shadeTexture()
{
        /*vec3 eye       = normalize(worldPosition.xyz - eyePos);
    vec3 reflected = normalize(refract(eye, normalize(worldNormal), 1.0 / 1.333)); // 1.3330

    vec4 diffuseColor = mix(vec4(0.0, 0.5, 1.0, 1.0), texture(reflectionTexture, reflected), 0.6);*/

    vec4 diffuseColor = texture(baseTexture, texCoords + deltaTime * 0.015);

    // Calculate the lighting model, keeping the specular component separate
    vec3 ambientAndDiff, spec;

    phongModel(ambientAndDiff, spec);

    vec4 color = vec4(ambientAndDiff, 1.0) * diffuseColor + vec4(spec, 1.0);

    float dist        = abs(position.z);
    float fogFactor   = clamp((fog.maxDistance - dist) / (fog.maxDistance - fog.minDistance), 0.0, 1.0);
    float alphaFactor = clamp(1.0 - ((fog.maxAlphaDistance - dist) / (fog.maxAlphaDistance - fog.minAlphaDistance)), 0.4, 1.0);

    vec4 skyRefletion      = texture(cubeMap, vec3(inverse(skyboxMatrix) * vec4(reflection, 0.0)));
    //vec4 terrainRefraction = texture(reflectionTexture, vec3(inverse(skyboxMatrix) * vec4(refraction, 0.0)));
    //vec4 terrainReflection = texture(reflectionTexture, vec3(inverse(skyboxMatrix) * vec4(reflection, 0.0)));

    mat4 reflectionViewProjection      = reflectionMatrix * projMatrix;
    mat4 reflectionWorldViewProjection = modelMatrix * reflectionViewProjection;
//----------------------------------------------------------------------------------------
    vec4 pos = /*(position * projMatrix)*/ genericPosition * reflectionWorldViewProjection;

    vec2 projectedTexcoords;
    projectedTexcoords.x = pos.x / pos.w / 2.0f + 0.5f;
    projectedTexcoords.y = pos.z / pos.w / 2.0f + 0.5f; // y/z

    vec4 terrainReflection = texture(reflectionTexture, projectedTexcoords);

    return mix(terrainReflection, vec4(0.5, 0.5, 0.5, 0.5), 0.5);

    if(cameraPosition.y >= 0.0)
        return (skyRefletion * skyRefletion) * 0.4 + vec4(mix(fog.color, color, fogFactor).rgb, alphaFactor) * 0.6;// = vec4(mix(fog.color, outColor, fogFactor).rgb, 1.0); vec4(color.rgb, 0.5)

    return vec4(color.rgb, 0.5);
}

subroutine(ShaderModelType)
vec4 shadeTextureWireFrame()
{
    return wireFrame(shadeTexture(), line.color2);
}

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
    vec4 color = worldHeight(worldPosition.y, 0.0, 4.0, 8.0, 12.0);

    return vec4(wireFrame(color, 0.7 * color).rgb, 1.0);
}

subroutine(ShaderModelType)
vec4 shadeWorldNormal()
{
    return vec4(vec4(0.5 * (worldNormal + vec3(1.0)), 1.0).rgb, 1.0);
}

subroutine(ShaderModelType)
vec4 shadeHidden()
{
    return vec4(0);
}

void main()
{
    //if(fog.discardDistance < abs(position.z + worldPosition.y))
    //    discard;

    //if(fog.discardDistance < abs(position.z) + worldPosition.y * worldPosition.y)
      //  discard;

    vec4 outColor = shaderModel();

    fragColor = outColor;
}