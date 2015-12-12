#version 330 core

uniform matrices {
    mat4 matModel;
    mat4 matView;
    mat4 matProjection;
};

layout(location = 0) in vec4 vertexPosition;
layout(location = 1) in vec3 vertexNormal;
layout(location = 2) in int heightIndex;

out vertexInterpBlock {
    vec4 position;
    vec3 normal;
} vertexInterp;

uniform float waveHeight[1601];

void main()
{
    vec4 posReal = vertexPosition / vertexPosition.w;
    posReal.y = (heightIndex > 0) ? waveHeight[heightIndex] / 2.0 : -10;

    vertexInterp.position = posReal;
    vertexInterp.normal = normalize(vertexNormal);

    gl_Position = matProjection *
                  matView *
                  matModel * posReal;
}

