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

uniform mat4 modelViewProjectionMatrix;
uniform mat4 modelViewMatrix;
uniform mat3 normalMatrix;

uniform vec3 qt_mAmbient;
uniform vec3 qt_mDiffuse;
uniform vec3 qt_mSpecular;

uniform float qt_Shininess;
uniform float qt_Opacity;

uniform vec3 qt_lAmbient;
uniform vec3 qt_lDiffuse;
uniform vec3 qt_lSpecular;
uniform vec3 qt_lPosition;

uniform bool hasDiffuse;

uniform sampler2D diffuseTexture;

in modelVertex
{
    vec3 position;
    vec3 normal;
    vec2 texcoord;
} In;

void main()
{
    vec3 myAmbient = qt_mAmbient;
    vec3 myDiffuse = qt_mDiffuse;

    if(hasDiffuse)
    {
        myAmbient *= texture2D(diffuseTexture, In.texcoord).xyz;
        myDiffuse  = myAmbient;

        if(texture2D(diffuseTexture, In.texcoord).a < 0.1f)
            discard;
    }

    vec3 diffuse = (dot(In.normal, normalize(qt_lPosition - In.position)) * myDiffuse + qt_mAmbient) * qt_lDiffuse;

    vec3 specular = vec3(0, 0, 0);

    vec3 eyeDir  = normalize(-In.position);
    vec3 r       = normalize(reflect(-normalize(qt_lPosition - In.position), In.normal));

    specular = qt_lSpecular * pow(max(dot(r, eyeDir), 0.0), qt_Shininess);

    vec3 ambient;

    /*vec3 ambient    = myAmbient * qt_lAmbient;
    vec3 surf2light = normalize(qt_lPosition - In.position);
    vec3 norm       = normalize(In.normal);

    float dcont = max(0.0, dot(norm, surf2light));

    vec3 diffuse    = dcont * (myDiffuse * qt_lDiffuse);
    vec3 surf2view  = normalize(-In.position);
    vec3 reflection = reflect(-surf2light, norm);

    float scont = pow(max(0.0, dot(surf2view, reflection)), qt_Shininess);

    vec3 specular = scont * qt_lSpecular * qt_mSpecular;*/

    //vec4(ambientAndDiff, 1.0) * diffuseColor + vec4(spec, 1.0);
    //fragColor = vec4(diffuse + specular, qt_Opacity);
    fragColor = vec4(texture2D(diffuseTexture, In.texcoord).rgb, 1.0f);
    //gl_FragColor=vec4(ambient+diffuse+specular,qt_Opacity);
}
