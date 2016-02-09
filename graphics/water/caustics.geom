#version 330 core

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

out float strength;

void main()
{
    float area = length(cross(
            (gl_in[1].gl_Position - gl_in[0].gl_Position).xyz,
            (gl_in[2].gl_Position - gl_in[1].gl_Position).xyz
        ));

    strength = clamp(1 - (area * 2000), 0.3, 1);

    for(int i = 0; i < gl_in.length(); i++) {
        gl_Position = vec4(gl_in[i].gl_Position.xyz / 1.3, 1.0);
        EmitVertex();
    }
}

