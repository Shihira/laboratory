#version 330 core

uniform light {
    vec4 posLight;
};

in vertexInterpBlock {
    vec4 posView;
    vec4 position;
    vec3 normal;
} vertexInterp;

out vec4 fragColor;

void main()
{
    vec3 ray = normalize((vertexInterp.position - posLight / posLight.w).xyz);
    vec3 view = -normalize(vertexInterp.posView.xyz);
    vec3 refl = reflect(ray, normalize(vertexInterp.normal));

    float diffuseStrength = dot(-ray, normalize(vertexInterp.normal));
    float specularStrength = pow(max(dot(view, refl)*1.2, 0.0), 32.0);
    float strength = clamp(diffuseStrength + specularStrength, 0.2, 1);

    fragColor = vec4(strength * 0.8, strength * 1.1, strength * 1.2, 1);
}

