#version 330 core

uniform light {
    vec4 posLight;
};

in vertexInterpBlock {
    vec4 position;
    vec3 normal;
} vertexInterp;

void main()
{
    vec3 ray = normalize((posLight / posLight.w - vertexInterp.position).xyz);
    float diffuseStrength = dot(ray, vertexInterp.normal);

    gl_FragColor = vec4(diffuseStrength, diffuseStrength, diffuseStrength, 1);
}

