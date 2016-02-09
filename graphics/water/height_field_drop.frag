#version 330 core

uniform sampler2D heightFields;
uniform vec2 center;

uniform float radius;
uniform float strength;

in vec2 uvcoord;

const float M_PI = 3.141592653589;

layout(location = 0) out vec4 outInfo;

float ripple_height(float r) {
    if(r > radius) return 0.f;

    r /= radius;
    r *= M_PI;

    float drop = cos(r);
    drop *= 0.5;
    drop += 0.5;

    return drop;
}

void main()
{
    outInfo = texture(heightFields, uvcoord);

    float r = length(uvcoord - center);
    outInfo += strength * ripple_height(r);
}

