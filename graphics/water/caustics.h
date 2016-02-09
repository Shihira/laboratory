#include <fstream>

#include "../include/shader.h"
#include "../include/model.h"
#include "../include/matrix.h"

#include "height_field.h"

using namespace std;
using namespace labgl;

class caustics {
    height_field& hf_;

    vertex_array vao_;
    plane_model surf_;

    shader vs_;
    shader fs_;
    shader gs_;
    program prog_;

    frame_buffer fb_;
    texture caus_tex_;

public:
    caustics(height_field& hf) :
            hf_(hf),
            surf_(col<2>{-1, -1}, col<2>{1, 1}, col<3>{1, 0, 0}, col<3>{0, 1, 0}),
            vs_(compile(shader::vertex, ifstream("caustics.vert"))),
            fs_(compile(shader::fragment, ifstream("caustics.frag"))),
            gs_(compile(shader::geometry, ifstream("caustics.geom"))),
            prog_(link(vs_, fs_, gs_)),
            caus_tex_(256, 256, texture::r_f)
    {
        surf_.subdivide(200, 200);

        vao_.input(0, surf_.triangles());
        vao_.input(1, surf_.uvcoords());

        fb_.bind(frame_buffer::color_buffer_0, caus_tex_);

        prog_.uniform("cellSize", 1.0f / hf_.cells);
    }

    texture& get_texture() {
        return caus_tex_;
    }

    void render(bool clear = true) {
        glDisable(GL_CULL_FACE);
        glDisable(GL_DEPTH_TEST);

        prog_.uniform("heightFields", hf_.get_height_fields());
        prog_.render(frame_buffer::screen, vao_,
                clear ? frame_buffer::all : frame_buffer::none);

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
    }
};
