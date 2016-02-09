// cflags: -lSDL2 -lGL -lGLEW

#include <iostream>
#include <vector>
#include <tuple>
#include <memory>

#include <GL/glew.h>

#include "../include/gui.h"
#include "../include/util.h"
#include "../include/shader.h"
#include "../include/model.h"

using namespace std;
using namespace labgl;

const char* vs_light_src = R"EOF(
#version 330 core

layout(location = 0) in vec4 position;

uniform mat4 mvp;

void main()
{
    gl_Position = mvp * position;
}
)EOF";

const char* fs_light_src = R"EOF(
#version 330 core

layout(location = 0) out float depth;

void main()
{
    //depth = gl_FragCoord.z;
}
)EOF";

const char* vs_src = R"EOF(
#version 330 core

layout(location = 0) in vec4 position;
layout(location = 1) in vec3 normal;

out vec3 fragNormal;
out vec4 worldPos;

uniform mat4 mvp;
uniform mat4 mmat;

void main()
{
    worldPos = mmat * position;
    fragNormal = (transpose(inverse(mmat)) * vec4(normal, 0)).xyz;

    gl_Position = mvp * position;
}
)EOF";

const char* fs_src = R"EOF(
#version 330 core

in vec4 worldPos;
in vec3 fragNormal;

out vec4 color;

uniform mat4 mmat;
uniform mat4 lightmvp;
uniform vec4 lightPos;

uniform sampler2D shadowMap;

void main()
{
    vec3 ray = (lightPos / lightPos.w - worldPos / worldPos.w).xyz;
    float diffuse = clamp(dot(normalize(fragNormal), normalize(ray)), 0.1, 1);

    vec4 posView = lightmvp * inverse(mmat) * worldPos;
    vec3 mapCoord = (vec3(1, 1, 1) + posView.xyz / posView.w ) / 2.0;

    float shadow = 0;
    float lightDepth = texture(shadowMap, mapCoord.xy).r;
    shadow = step(mapCoord.z, lightDepth + 0.001);

    diffuse *= clamp(shadow, 0.3, 1);
    diffuse = clamp(diffuse, 0.1, 1);
    color = vec4(diffuse, diffuse, diffuse, 1);
}
)EOF";

const char* vs_tt_src /*tt = test texture*/= R"EOF(
#version 330 core

layout(location = 0) in vec4 sqrCoord;
out vec2 texCoord;

void main()
{
    gl_Position = sqrCoord;
    texCoord = (vec2(1, 1) + sqrCoord.xy / sqrCoord.w) / 2.0;
}
)EOF";

const char* fs_tt_src = R"EOF(
#version 330 core

uniform sampler2D texDepth;
in vec2 texCoord;

out vec4 color;

void main()
{
    float g = texture(texDepth, texCoord).r;
    color = vec4(g, g, g, 1);
}
)EOF";

vector<col<4>> tt_pos = {
    col<4>{ -1,  1, 0, 1 },
    col<4>{ -1, -1, 0, 1 },
    col<4>{  1, -1, 0, 1 },
    col<4>{  1, -1, 0, 1 },
    col<4>{  1,  1, 0, 1 },
    col<4>{ -1,  1, 0, 1 },
};

int main()
{
    windowgl w("Hello", 0x0303);
    w.init_glew();

    glClearColor(0.2, 0.2, 0.2, 1);
    glClearDepth(1);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    ////////////////////////////////////////////////////////////////////////////
    // Program Part

    shader vs_light = compile(shader::vertex, vs_light_src);
    shader fs_light = compile(shader::fragment, fs_light_src);
    program prog_light = link(vs_light, fs_light);

    shader vs_main = compile(shader::vertex, vs_src);
    shader fs_main = compile(shader::fragment, fs_src);
    program prog_main = link(vs_main, fs_main);

    shader vs_test = compile(shader::vertex, vs_tt_src);
    shader fs_test = compile(shader::fragment, fs_tt_src);
    program prog_test = link(vs_test, fs_test);

    ////////////////////////////////////////////////////////////////////////////
    // Read in Model

    wavefront_model m;
    ifstream fobj("../assets/shadow.obj");
    m.read(fobj);

    vector<col<4>> vertices;
    vector<col<3>> normals;
    vector<col<3>> uvs;

    m.extract_triangles(vertices, normals, uvs);

    vertex_array vao;
    vao.input(0, vertices);
    vao.input(1, normals);

    vertex_array vao_test;
    vao_test.input(0, tt_pos);

    ////////////////////////////////////////////////////////////////////////////
    // Transform and render

    texture tex_depth(1024, 1024, texture::depth_f);

    frame_buffer fbo;
    fbo.bind(frame_buffer::depth_buffer, tex_depth);

    matrix<4, 4> mmat_light = transform::translate(col<4>{ 0, 0, -20, 1 });
    mmat_light *= transform::rotate(-M_PI / 4, transform::yOz);
    matrix<4, 4> pmat_light = transform::perspective(M_PI / 4, 4. / 3, 8, 50);

    matrix<4, 4> mmat = transform::translate(col<4>{ 0, 0, -20, 1 });
    mmat *= transform::rotate(-M_PI / 4, transform::yOz);
    mmat *= rotate(M_PI / 3 * 2, transform::zOx);
    matrix<4, 4> pmat = transform::perspective(M_PI / 4, 4. / 3, 12, 50);

    app().register_on_paint([&]() {
        mmat_light *= rotate(M_PI / 360, transform::zOx);

        glViewport(0, 0, 1024, 1024);
        prog_light.uniform("mvp", pmat_light * mmat_light);
        prog_light.render(fbo, vao);

        glViewport(0, 0, 800, 600);
        prog_main.uniform("mvp", pmat * mmat);
        prog_main.uniform("mmat", mmat);
        prog_main.uniform("lightmvp", pmat_light * mmat_light);
        prog_main.uniform("lightPos", col<4>(mmat * mmat_light.inverse()
                * matrix<4, 1>{ 0, 0, 0, 1 }));
        prog_main.uniform("shadowMap", tex_depth);
        prog_main.render(frame_buffer::screen, vao);

        glViewport(0, 0, 200, 150);
        prog_test.uniform("texDepth", tex_depth);
        prog_test.render(frame_buffer::screen, vao_test, frame_buffer::none);

        w.swap_buffer();
    });

    app().run();

    return 0;
}

