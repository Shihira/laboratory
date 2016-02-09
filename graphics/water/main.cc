// cflags: -lSDL2 -lGL -lGLEW
/*
 * Copyright(c) 2015, Shihira Fung <fengzhiping@hotmail.com>
 */

#include <iostream>

#include "height_field.h"
#include "water_surface.h"
#include "caustics.h"

const char* render_texture_vert = R"EOF(
#version 330 core

layout(location = 0) in vec4 vertex;
out vec2 uv;

void main()
{
    gl_Position = vertex;
    uv = (vertex.xy) / 2.0 + vec2(0.5, 0.5);
}
)EOF";

const char* render_texture_frag = R"EOF(
#version 330 core

uniform sampler2D tex;
in vec2 uv;
out vec4 fragColor;

void main()
{
    float c = texture(tex, uv).r;
    fragColor = vec4(c, c, c, 1);
}
)EOF";

using namespace std;

int main(int argc, char** argv)
{
    windowgl w("Water", 0x0303, 800, 600);
    w.init_glew();

    glClearDepth(1);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    shader vs = compile(shader::vertex, render_texture_vert);
    shader fs = compile(shader::fragment, render_texture_frag);
    program prog = link(vs, fs);

    plane_model m(col<2>{-1, -1}, col<2>{1, 1});
    vertex_array vao;
    vao.input(0, m.triangles());

    height_field hf;
    matrix<4, 4>
        mat_view = 
            transform::translate(col<4>{0, 0, -40, 1}) *
            transform::rotate(-M_PI / 6, transform::yOz),
        mat_proj =
            transform::perspective(M_PI / 6, 4. / 3, 1, 100);
    water_surface ws(hf, mat_view, mat_proj, col<4>{0, 10, 0, 1});
    caustics caus(hf);

    app().register_on_paint([&]() {
        hf.render();
        //hf.render();

        glViewport(0, 0, 800, 600);
        glClearColor(0.2, 0.2, 0.2, 1);
        ws.render();

        glViewport(0, 0, 256, 256);
        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        caus.render(false);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        w.swap_buffer();
    });

    typedef application::mouse_button mouse_button;
    app().register_on_mouse_up([&](int x, int y, mouse_button btn) {
        if(btn == application::left_button)
            ws.render_drop(col<4>{x / 400. - 1, 1 - y / 300., -1, 1}, 5, 0.4);
        if(btn == application::right_button)
            ws.render_drop(col<4>{x / 400. - 1, 1 - y / 300., -1, 1}, 20, 2);
    });

    int last_x, last_y;
    app().register_on_mouse_down([&](int x, int y, mouse_button btn) {
        if(btn == application::middle_button) {
            last_x = x;
            last_y = y;
        }
    });

    app().register_on_mouse_move([&](int x, int y, uint32_t mask) {
        if(mask & application::middle_button) {
            double d_x = x - last_x, d_y = y - last_y;
            ws.matrix_view =
                transform::translate(col<4>{0, 0, -40, 1}) *
                transform::rotate(-M_PI / 6, transform::yOz) *
                transform::rotate(- d_x * 2 * M_PI / 720, transform::zOx) *
                transform::rotate(- d_y * 2 * M_PI / 720, transform::yOz) *
                transform::rotate(M_PI / 6, transform::yOz) *
                transform::translate(col<4>{0, 0, 40, 1}) *
                matrix<4, 4>(ws.matrix_view);

            last_x = x; last_y = y;
        }
    });


    app().set_fps(argc > 1 ? atoi(argv[1]) : 56);
    app().run();
}
