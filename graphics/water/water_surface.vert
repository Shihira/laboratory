#version 330 core

uniform matrices {
    mat4 matModel;
    mat4 matView;
    mat4 matProjection;
};

layout(location = 0) in vec4 vertex;
layout(location = 1) in vec3 uvcoord;

uniform sampler2D heightFields;
uniform float cellSize;

out vertexInterpBlock {
    vec4 posView;
    vec4 position;
    vec2 uvcoord;
} vertexInterp;

void main()
{
    vertexInterp.uvcoord = uvcoord.xy / uvcoord.z * (1 - 4 * cellSize) + 
        vec2(2 * cellSize, 2 * cellSize);

    vertexInterp.position = matModel * vec4(vertex.xyz / vertex.w, 1);
    vertexInterp.position.y = texture(heightFields, vertexInterp.uvcoord).r;
    vertexInterp.posView = matView * vertexInterp.position;

    gl_Position = matProjection * vertexInterp.posView;
}
