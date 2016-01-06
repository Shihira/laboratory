// cflags: -lSDL2 -lGL -lGLEW
/*
 * Copyright(c) 2015, Shihira Fung <fengzhiping@hotmail.com>
 */

#include <iostream>

#include "height_field.h"

using namespace std;

int main()
{
    windowgl w("Water", 0x0303, 512, 512);
    w.init_glew();

    glClearColor(0.2, 0.2, 0.2, 1);
    glClearDepth(1);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    plane_model m(col<2>{-1, -1},   col<2>{1, 1});
    shader vs = compile(shader::vertex, "#version 330 core\n"
            "layout(location = 0) in vec4 vertex; out vec2 uv;\nvoid main() {"
            "gl_Position = vertex; uv = (vertex.xy) / 2.0 + vec2(0.5, 0.5); }"),
           fs = compile(shader::fragment, "#version 330 core\n"
            "uniform sampler2D tex; in vec2 uv; out vec4 fragColor;\nvoid main() {"
            "float c = texture(tex, uv).r;"
            "fragColor = vec4(c, c, c, 1); }");
    program prog = link(vs, fs);

    height_field hf;
    image init_wave("init_wave.ppm");
    hf.get_height_fields().bitblt(init_wave);

    vertex_array vao;
    vao.input(0, m.triangles());

    app().register_on_paint([&]() {
        hf.render();

        glViewport(0, 0, 512, 512);
        prog.uniform("tex", hf.get_height_fields());
        prog.render(frame_buffer::screen, vao);

        w.swap_buffer();
    });

    app().run();
}
