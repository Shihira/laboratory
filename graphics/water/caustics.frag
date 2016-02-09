#version 330 core

layout(location = 0) out vec4 color;

in float strength;

void main()
{
    //color = vec4(1, 1, 1, 1);
    color = vec4(strength, strength, strength, 1);
}
