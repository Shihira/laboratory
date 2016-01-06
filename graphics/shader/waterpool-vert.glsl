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
    vec4 posView;
    vec4 position;
    vec3 normal;
} vertexInterp;

uniform float height[1600];
uniform int mode;

int getInd(int x, int z)
{
    return clamp(x, 0, 39) * 40 + clamp(z, 0, 39);
}

void main()
{
    vec4 posReal = vertexPosition / vertexPosition.w;
    posReal.y = (heightIndex >= 0) ? height[heightIndex] / 2.0 : -10;

    vertexInterp.position = posReal;
    vertexInterp.posView = matView * matModel * posReal;
    vertexInterp.posView /= vertexInterp.posView.w;

    if(mode == 0)
        vertexInterp.normal = normalize(vertexNormal);
    else if(mode == 1) {
        int x = heightIndex / 40, z = heightIndex % 40;
        float rad_x = atan((height[getInd(x+1, z)] -
                height[getInd(x-1, z)]) / 2);
        vec3 normal_x = vec3(-sin(rad_x), cos(rad_x), 0);
        float rad_z = atan((height[getInd(x, z+1)] -
                height[getInd(x, z-1)]) / 2);
        vec3 normal_z = vec3(0, cos(rad_z), -sin(rad_z));
        vertexInterp.normal = normalize(normal_x + normal_z);
    }

    gl_Position = matProjection * vertexInterp.posView;
}

