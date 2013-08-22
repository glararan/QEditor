#version 400

layout (location = 0) in vec2 vertexPosition;
layout (location = 1) in vec2 vertexTexCoord;

out terrainVertex
{
    vec2 position;
} Out;

out vec2 VTexCoord;

void main()
{
    Out.position = vertexPosition;
    VTexCoord    = vertexTexCoord;
}
