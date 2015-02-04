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

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in worldVertex
{
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
} In[];

out wireFrameVertex
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
} Out;

uniform mat4 viewportMatrix;

void main()
{
    // Transform each vertex into viewport space
    vec2 p0 = vec2(viewportMatrix * (gl_in[0].gl_Position / gl_in[0].gl_Position.w));
    vec2 p1 = vec2(viewportMatrix * (gl_in[1].gl_Position / gl_in[1].gl_Position.w));
    vec2 p2 = vec2(viewportMatrix * (gl_in[2].gl_Position / gl_in[2].gl_Position.w));

    // Calculate lengths of 3 edges of triangle
    float a = length(p1 - p2);
    float b = length(p2 - p0);
    float c = length(p1 - p0);

    // Calculate internal angles using the cosine rule
    float alpha = acos((b * b + c * c - a * a) / (2.0 * b * c));
    float beta  = acos((a * a + c * c - b * b) / (2.0 * a * c));

    // Calculate the perpendicular distance of each vertex from the opposing edge
    float ha = abs(c * sin(beta));
    float hb = abs(c * sin(alpha));
    float hc = abs(b * sin(alpha));

    // Now add this perpendicular distance as a per-vertex property in addition to
    // the position calculated in the vertex shader.

    // Vertex 0 (a)
    Out.edgeDistance        = vec3(ha, 0, 0);
    Out.worldPosition       = In[0].worldPosition;
    Out.worldNormal         = In[0].worldNormal;
    Out.position            = In[0].position;
    Out.genericPosition     = In[0].genericPosition;
    Out.normal              = In[0].normal;
    Out.texCoords           = In[0].texCoords;
    /*Out.lightVertexPosition = In[0].lightVertexPosition;
    Out.lightPosition	    = In[0].lightPosition;*/
    Out.reflection          = In[0].reflection;

    gl_Position = gl_in[0].gl_Position;

    EmitVertex();

    // Vertex 1 (b)
    Out.edgeDistance        = vec3(0, hb, 0);
    Out.worldPosition       = In[1].worldPosition;
    Out.worldNormal         = In[1].worldNormal;
    Out.position            = In[1].position;
    Out.genericPosition     = In[1].genericPosition;
    Out.normal              = In[1].normal;
    Out.texCoords           = In[1].texCoords;
    /*Out.lightVertexPosition = In[1].lightVertexPosition;
    Out.lightPosition	    = In[1].lightPosition;*/
    Out.reflection          = In[1].reflection;

    gl_Position = gl_in[1].gl_Position;

    EmitVertex();

    // Vertex 2 (c)
    Out.edgeDistance        = vec3(0, 0, hc);
    Out.worldPosition       = In[2].worldPosition;
    Out.worldNormal         = In[2].worldNormal;
    Out.position            = In[2].position;
    Out.genericPosition     = In[2].genericPosition;
    Out.normal              = In[2].normal;
    Out.texCoords           = In[2].texCoords;
    /*Out.lightVertexPosition = In[2].lightVertexPosition;
    Out.lightPosition	    = In[2].lightPosition;*/
    Out.reflection          = In[2].reflection;

    gl_Position = gl_in[2].gl_Position;

    EmitVertex();

    // Finish the primitive off
    EndPrimitive();
}
