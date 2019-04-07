#version 400

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
uniform bool animationEnabled;

uniform sampler2D diffuseTexture;

uniform mat4 boneMatrix[100];

in vec3 qt_Vertex;
in vec3 qt_Normal;
in vec2 qt_TexCoord;
in vec3 qt_Tangent;
in vec4 qt_BoneIDs;
in vec4 qt_Weights;

out modelVertex
{
    vec3 position;
    vec3 normal;
    vec2 texcoord;
} Out;

void main()
{
    mat4 animationMatrix = mat4(1, 0, 0, 0,
                                0, 1, 0, 0,
                                0, 0, 1, 0,
                                0, 0, 0, 1); // identity matrix

    if(animationEnabled == true)
    {
        animationMatrix =  boneMatrix[int(qt_BoneIDs[0])] * qt_Weights[0];
        animationMatrix += boneMatrix[int(qt_BoneIDs[1])] * qt_Weights[1];
        animationMatrix += boneMatrix[int(qt_BoneIDs[2])] * qt_Weights[2];
        animationMatrix += boneMatrix[int(qt_BoneIDs[3])] * qt_Weights[3];
    }

    gl_Position = modelViewProjectionMatrix /* animationMatrix*/ * vec4(qt_Vertex, 1.0);

    Out.position = (modelViewMatrix /* animationMatrix*/ * vec4(qt_Vertex, 1.0)).xyz;
    //Out.normal   = (modelViewMatrix * animationMatrix * vec4(qt_Normal, 0.0)).xyz;
    Out.normal   = normalize((modelViewMatrix * vec4(qt_Normal, 1.0f)).xyz);
    Out.texcoord = qt_TexCoord;
}
