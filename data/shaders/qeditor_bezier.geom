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

layout(points, invocations = 4) in;
layout(line_strip, max_vertices = 4) out;

in curveVertex
{
    vec3 position;
} In[];

uniform int detail;

uniform mat4 mvp;

uniform vec3 point1;
uniform vec3 point2;
uniform vec3 point3;
uniform vec3 point4;

vec3 evaluateBezierPosition(vec3 v[4], float t)
{
    vec3 p;

    float _t = 1.0 - t;

    float b0 = _t * _t * _t;
    float b1 = 3.0 * t * _t * _t;
    float b2 = 3.0 * t * t * _t;
    float b3 = t * t * t;

    return b0 * v[0] + b1 * v[1] + b2 * v[2] + b3 * v[3];
}

void main(void)
{
    vec3 pos[4];
    pos[0] = point1;
    pos[1] = point2;
    pos[2] = point3;
    pos[3] = point4;

    float OneOverDetail = 1.0 / float(detail - 1.0);

    for(int i = 0; i < detail; i++)
    {
        float t = i * OneOverDetail;

        vec3 p = evaluateBezierPosition(pos, t);

        gl_Position = mvp * vec4(p, 1.0);

        EmitVertex();
    }

    EndPrimitive();
}