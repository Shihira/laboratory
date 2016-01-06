#version 330 core

layout(location = 0) in vec4 vertex;

out vec2 uvcoord;

void main()
{
    gl_Position = vertex;
    uvcoord = vertex.xy / vertex.w / 2.0 + vec2(0.5, 0.5);
}
