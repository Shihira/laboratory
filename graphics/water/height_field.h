/*
 * Copyright(c) 2015, Shihira Fung <fengzhiping@hotmail.com>
 */

#include <iostream>
#include <fstream>
#include <memory>

#include "../include/shader.h"
#include "../include/model.h"

using namespace std;
using namespace labgl;

class height_field {
    frame_buffer front_;
    frame_buffer back_;
    texture texfront_;
    texture texback_;

    vertex_array vao_;

    plane_model model_;

    shader vs_;
    shader fs_;
    program prog_;

    static const int cells_ = 256;

public:
    height_field() :
            texfront_(image(cells_, cells_, 0xff007f7f)),
            texback_(cells_, cells_),
            model_(col<2>{-1, -1},   col<2>{1, 1}),
            vs_(compile(shader::vertex, ifstream("height_field.vert"))),
            fs_(compile(shader::fragment, ifstream("height_field.frag"))),
            prog_(link(vs_, fs_))
    {
        front_.bind(frame_buffer::color_buffer_0, texfront_);
        back_.bind(frame_buffer::color_buffer_0, texback_);

        vao_.input(0, model_.triangles());

        try {
            prog_.uniform("cellSize", 1.f/cells_);
        } catch(runtime_error e) { }
    }

    void render() {
        glViewport(0, 0, cells_, cells_);
        prog_.uniform("heightFields", texfront_);
        prog_.render(back_, vao_);

        swap(front_, back_);
        swap(texfront_, texback_);
    }

    texture& get_height_fields() { return texfront_; }
};

