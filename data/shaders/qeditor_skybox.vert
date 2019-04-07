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

in vec3 qt_Vertex;
out vec3 vertexVector;

void main()
{
    vec4 a = modelViewProjectionMatrix * vec4(qt_Vertex, 1.0);

    gl_Position = vec4(a.xyz, a.z + 0.000001);//modelViewProjectionMatrix * vec4(qt_Vertex, 1.0);

    vertexVector = qt_Vertex;
}
