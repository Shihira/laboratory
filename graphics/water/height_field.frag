#version 330 core

uniform sampler2D heightFields;
uniform float cellSize;

layout(location = 0) out vec4 outInfo;

in vec2 uvcoord;

const float v = 3.2;
const float dt = 0.22;

void main()
{
    vec2 adj[4];
    adj[0] = vec2(cellSize, 0); adj[1] = vec2(-cellSize, 0);
    adj[2] = vec2(0, cellSize); adj[3] = vec2(0, -cellSize);

    outInfo = texture(heightFields, uvcoord);

    vec4 adjInfo[4];
    float f = 0;
    for(int i = 0; i < 4; i++) {
        vec2 uvcoord_i = uvcoord + adj[i];
        uvcoord_i.x = clamp(uvcoord_i.x, cellSize, 1 - cellSize);
        uvcoord_i.y = clamp(uvcoord_i.y, cellSize, 1 - cellSize);

        adjInfo[i] = texture(heightFields, uvcoord_i);
        f += adjInfo[i].r;
    }

    f -= 4 * outInfo.r;
    f *= v * v;

    outInfo.g += f * dt;
    outInfo.g *= 0.998;
    outInfo.r += outInfo.g * dt;
}
