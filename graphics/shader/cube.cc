// cflags: -lSDL2 -lGL -lGLEW

#include <iostream>
#include <memory>
#include <GL/glew.h>

#include "../include/matrix.h"
#include "../include/gui.h"
#include "../include/shader.h"

using namespace std;
using namespace labgl;

const char* vs_src = R"EOF(
#version 330 core

layout(location = 0) in vec4 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;

out float diffuse;
out vec2 uv_coord;

uniform mat4 mmat;
uniform mat4 pmat;

void main(void)
{
    mat4 nmmat = transpose(inverse(mmat));
    vec3 nml = (nmmat * vec4(normal, 0)).xyz;

    vec4 light = vec4(2, 3, 4, 1);
    diffuse = clamp(
        dot(-nml, normalize((position - light).xyz)),
        0.1, 1);

    gl_Position = pmat * mmat * position;
    uv_coord = uv;
}
)EOF";

const char* fs_src = R"EOF(
#version 330 core

in float diffuse;
in vec2 uv_coord;

out vec4 color;

uniform sampler2D tex_logo;

void main(void)
{
    vec4 orgc = texture(tex_logo, uv_coord);
    color = vec4(orgc.rgb * diffuse, orgc[3]);
}
)EOF";

/*
 *      A y
 *      |
 *    5----1
 *   /|   /|
 *  4-+--0 |
 *  | 7--|-3
 *  |/   |/---> x
 *  6----2
 *   /
 *  L  z
 *
 */

const GLfloat vertices[] = {
     1,  1,  1, 1,
     1,  1, -1, 1,
     1, -1,  1, 1,
     1, -1, -1, 1,
    -1,  1,  1, 1,
    -1,  1, -1, 1,
    -1, -1,  1, 1,
    -1, -1, -1, 1,
};

const GLfloat normals[] = {
    1, 0, 0,
    -1, 0, 0,
    0, 1, 0,
    0, -1, 0,
    0, 0, 1,
    0, 0, -1,
};

const GLfloat uvs[] = {
    0, 0,
    0, 1,
    1, 0,
    1, 1,
};

// 0 1 3 2
// 4 5 7 6
const GLuint vindices[] = {
    2, 3, 1,    1, 0, 2,
    4, 5, 7,    7, 6, 4,
    1, 5, 4,    4, 0, 1,
    2, 6, 7,    7, 3, 2,
    0, 4, 6,    6, 2, 0,
    3, 7, 5,    5, 1, 3,
};

const GLuint nindices[] = {
    0, 0, 0,    0, 0, 0,
    1, 1, 1,    1, 1, 1,
    2, 2, 2,    2, 2, 2,
    3, 3, 3,    3, 3, 3,
    4, 4, 4,    4, 4, 4,
    5, 5, 5,    5, 5, 5,
};

const GLuint uindices[] = {
    0, 1, 3,    3, 2, 0,
    0, 1, 3,    3, 2, 0,
    0, 1, 3,    3, 2, 0,
    0, 1, 3,    3, 2, 0,
    0, 1, 3,    3, 2, 0,
    0, 1, 3,    3, 2, 0,
};

template<size_t M_, size_t ind_count>
vector<col<M_>> extend(const GLfloat *vectors,
        const GLuint (&indices)[ind_count])
{
    vector<col<M_>> arr(ind_count);
    for(size_t i : indices) {
        col<M_> vec;
        for(size_t c = 0; c < M_; c++)
            vec[c] = vectors[i * M_ + c];
        arr.push_back(vec);
    }

    return arr;
}

int main()
{
    windowgl w("Hello", 0x0303);
    w.init_glew();

    glClearColor(0.2, 0.2, 0.2, 1);
    glClearDepth(1);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    ////////////////////////////////////////////////////////

    shader vs = compile(shader::vertex, vs_src);
    shader fs = compile(shader::fragment, fs_src);
    program prog = link(vs, fs);

    auto ext_v = extend<4>(vertices, vindices);
    auto ext_n = extend<3>(normals, nindices);
    auto ext_u = extend<2>(uvs, uindices);
    texture tex_logo(image("../assets/texture-logo.ppm"));

    vertex_array vao;
    vao.input(0, ext_v); 
    vao.input(1, ext_n); 
    vao.input(2, ext_u); 

    matrix<4, 4> pmat = transform::perspective(M_PI / 4, 4. / 3, 3, 6);
    matrix<4, 4> mmat = transform::identity();
    mmat *= transform::translate(col<4>{ 0, 0, -5, 1 });
    mmat *= transform::rotate(-M_PI / 6, transform::yOz);

    prog.uniform("tex_logo", tex_logo);

    app().register_on_paint([&]() {
        mmat *= rotate(M_PI / 360, transform::zOx);

        prog.uniform("mmat", mmat);
        prog.uniform("pmat", pmat);
        prog.render(frame_buffer::screen, vao);

        w.swap_buffer();
    });

    app().run();

    return 0;
}
