unifrom light {
    vec4 position;
};

in vertexInterp {
    vec4 position;
    vec3 normal;
};

void main()
{
    vec3 ray = normalize(light.position / light.position.w
             - vertexInterp.position / vertexInterp.position.w).xyz;
    float diffuseStrength = dot(ray * normal);

    gl_FragColor = vec4(diffuseStrength, diffuseStrength, diffuseStrength, 1);
}

