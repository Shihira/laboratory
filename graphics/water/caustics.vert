#version 330 core

layout(location = 0) in vec4 vertex;
layout(location = 1) in vec3 uvcoord_;

uniform float cellSize;
uniform sampler2D heightFields;

const vec3 ray = normalize(vec3(-1, -0.5, 0));
const float nAir = 1.000028, nWater = 1.333333;

void main()
{
    vec2 uvcoord = uvcoord_.xy / uvcoord_.z * (1 - 4 * cellSize) + 
        vec2(2 * cellSize, 2 * cellSize);

    vec3 normal = normalize(vec3(
            texture(heightFields, uvcoord + vec2(cellSize, 0)).r-
            texture(heightFields, uvcoord + vec2(-cellSize, 0)).r,
            2 * cellSize * 20,
            texture(heightFields, uvcoord + vec2(0, cellSize)).r-
            texture(heightFields, uvcoord + vec2(0, -cellSize).r)
        ));

    float cosIn = dot(-ray, normal), sinIn = sqrt(1 - cosIn * cosIn);
    float sinOut = sinIn * nAir / nWater,
          cosOut = sqrt(1 - sinOut * sinOut);
    sinIn = sinIn < 0.000001 ? 0.000001 : sinIn;

    vec3 outRay = (ray + cosIn * normal) / sinIn * sinOut - normal * cosOut;
    outRay /= outRay.y;

    gl_Position = vertex;
    gl_Position.x += outRay.x * cellSize * 50;
    gl_Position.y += outRay.z * cellSize * 50;
}

