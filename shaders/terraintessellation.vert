#version 400

layout (location = 0) in vec2 vertexPosition;

out terrainVertex {
    vec2 position;
} Out;

void main()
{
    Out.position = vertexPosition;
}
