#version core 330

uniform matrices {
    mat4 model;
    mat4 view;
    mat4 projection;
};

layout(location = 0) in vertex {
    vec4 position;
    vec3 normal;
};

out vertexInterp {
    vec4 position;
    vec3 normal;
};

void main()
{
    vertexInterp.position = vertex.position;
    vertexInterp.normal = normalize(vertex.normal);

    gl_Position = matrices.projection *
                  matrices.view *
                  matrices.mdel * vertex.position;
}

