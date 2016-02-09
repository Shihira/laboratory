#include <iostream>

#include "../include/shader.h"
#include "../include/model.h"
#include "../include/matrix.h"

#include "height_field.h"

using namespace std;
using namespace labgl;

class water_surface {
    height_field& hf_;

    vertex_array vao_;
    plane_model surf_;

    matrix<4, 4> mat_model_;
    matrix<4, 4> mat_view_;
    matrix<4, 4> mat_proj_;

    col<4> light_;

    shader vs_;
    shader fs_;
    program prog_;

public:
    water_surface(height_field& hf,
            const matrix<4, 4>& mat_view,
            const matrix<4, 4>& mat_proj,
            col<4> light) : hf_(hf),
            surf_(col<2>{-20, -20}, col<2>{20, 20},
                col<3>{1, 0, 0}, col<3>{0, 0, -1}),
            mat_model_(transform::identity()),
            mat_view_(mat_view),
            mat_proj_(mat_proj),
            light_(light),
            vs_(compile(shader::vertex, ifstream("water_surface.vert"))),
            fs_(compile(shader::fragment, ifstream("water_surface.frag"))),
            prog_(link(vs_, fs_)),
            matrix_view(this)
    {
        surf_.subdivide(80, 80);

        vao_.input(0, surf_.triangles());
        vao_.input(1, surf_.uvcoords());

        prog_.uniform("cellSize", 1.f / hf_.cells);
        prog_.uniform_block("matrices", make_tuple(
                mat_model_, mat_view_, mat_proj_
            ));
    }

    void render_drop(const col<4>& scr, double radius, double height) {
        auto mat_proj_inv = mat_proj_.inverse(),
             mat_view_inv = mat_view_.inverse(),
             mat_model_inv = mat_model_.inverse();

        col<4>
            p_view = mat_proj_inv * scr.to_mat(),
            p = mat_model_inv * mat_view_inv * p_view.to_mat(),
            o = mat_model_inv * mat_view_inv * col<4>{0, 0, 0, 1}.to_mat();
        p = p * (1 / p[3]); o = o * (1 / o[3]);

        col<4> sight = p - o;

        // o.y + C * s.y = 0, find C
        double C = o[1] / sight[1];
        col<4> uv = o - sight * C;

        hf_.render_drop(col<2>{uv[0] / 40 + 0.5, 0.5 - uv[2] / 40},
            radius / hf_.cells, height);
    }

    void render() {
        glEnable(GL_DEPTH_TEST);
        prog_.uniform_block("light", mat_view_ * light_.to_mat());
        prog_.uniform("heightFields", hf_.get_height_fields());
        prog_.render(frame_buffer::screen, vao_);
    }

    class descriptor_ {
        water_surface* self;
    public:
        descriptor_(water_surface* s) : self(s) { }
        inline descriptor_& operator=(const matrix<4, 4>& m) {
            self->mat_view_ = m;
            self->prog_.uniform_block("matrices", make_tuple(
                   self->mat_model_, self->mat_view_, self->mat_proj_
                ));
            return *this;
        }
        inline operator matrix<4, 4>() const {
            return self->mat_view_;
        }
    } matrix_view;

    friend class water_surface::descriptor_;
};

