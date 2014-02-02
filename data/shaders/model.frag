#version 400

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

void main(){
	vec3 myAmbient = qt_mAmbient;
	vec3 myDiffuse = qt_mDiffuse;
	if(hasDiffuse)
	{
		myAmbient = texture2D(diffuseTexture,In.texcoord).xyz;
		myDiffuse = myAmbient;
	}

	vec3 ambient=myAmbient*qt_lAmbient;
	vec3 surf2light=normalize(qt_lPosition-In.position);
	vec3 norm=normalize(In.normal);
	float dcont=max(0.0,dot(norm,surf2light));
	vec3 diffuse=dcont*(myDiffuse*qt_lDiffuse);
	vec3 surf2view=normalize(-In.position);
	vec3 reflection=reflect(-surf2light,norm);
	float scont=pow(max(0.0,dot(surf2view,reflection)),qt_Shininess);
	vec3 specular=scont*qt_lSpecular*qt_mSpecular;
	gl_FragColor=vec4(ambient+diffuse+specular,qt_Opacity);
}