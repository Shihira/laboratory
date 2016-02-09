#version 330 core

uniform light {
    vec4 posLight;
};

uniform sampler2D heightFields;
uniform float cellSize;

in vertexInterpBlock {
    vec4 posView;
    vec4 position;
    vec2 uvcoord;
} vertexInterp;

out vec4 fragColor;

void main()
{
    vec2 uvcoord = vertexInterp.uvcoord;
    vec3 normal = normalize(vec3(
            texture(heightFields, uvcoord + vec2(cellSize, 0)).r-
            texture(heightFields, uvcoord + vec2(-cellSize, 0)).r,
            2 * cellSize * 20,
            texture(heightFields, uvcoord + vec2(0, cellSize)).r-
            texture(heightFields, uvcoord + vec2(0, -cellSize).r)
        ));

    vec3 ray = normalize((vertexInterp.position - posLight / posLight.w).xyz);
    vec3 view = -normalize(vertexInterp.posView.xyz);
    vec3 refl = reflect(ray, normalize(normal));

    float diffuseStrength = dot(-ray, normalize(normal));
    float specularStrength = pow(max(dot(view, refl)*1.2, 0.0), 64.0);
    float strength = clamp(diffuseStrength + specularStrength * 0.0002, 0.2, 1);

    fragColor = vec4(strength * 0.8, strength * 1.1, strength * 1.2, 1);
}

