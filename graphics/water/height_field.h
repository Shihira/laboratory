/*
 * Copyright(c) 2015, Shihira Fung <fengzhiping@hotmail.com>
 */
#ifndef HEIGHT_FIELD_H_INCLUDED
#define HEIGHT_FIELD_H_INCLUDED

#include <iostream>
#include <fstream>
#include <memory>

#include "../include/shader.h"
#include "../include/model.h"

using namespace std;
using namespace labgl;

class height_field {
    frame_buffer front_; // display buffer
    frame_buffer back_; // render buffer
    texture texfront_; // input for rendering
    texture texback_;

    vertex_array vao_;

    plane_model model_;

    shader vs_;
    shader fs_;
    program prog_;
    shader drop_fs_;
    program drop_prog_;

    double drop_func_(double r) {
        if(r > 5.0) return 0;
        r /= 5.0;
        r *= M_PI / 2;
        r += M_PI / 2;
        double drop = cos(r);
        drop = 1 - drop * drop;
        return drop;
    }

    void swap_buffer_() {
        swap(front_, back_);
        swap(texfront_, texback_);
    }

public:
    static const int cells = 256;


    height_field() :
            texfront_(cells, cells, texture::rg_ff),
            texback_(cells, cells, texture::rg_ff),
            model_(col<2>{-1, -1},   col<2>{1, 1}),

            vs_(compile(shader::vertex, ifstream("height_field.vert"))),
            fs_(compile(shader::fragment, ifstream("height_field.frag"))),
            prog_(link(vs_, fs_)),

            drop_fs_(compile(shader::fragment, ifstream("height_field_drop.frag"))),
            drop_prog_(link(vs_, drop_fs_))
    {
        front_.bind(frame_buffer::color_buffer_0, texfront_);
        back_.bind(frame_buffer::color_buffer_0, texback_);

        front_.clear();

        vao_.input(0, model_.triangles());

        prog_.uniform("cellSize", 1.f/cells);
    }

    void render_drop(const col<2>& drop, double radius, double height) {
        if(drop[0] > 1 || drop[1] > 1 || drop[0] < 0 || drop[1] < 0) {
            cerr << "Drop is ignored: " << drop << endl;
            return;
        }

        glViewport(0, 0, cells, cells);
        drop_prog_.uniform("radius", float(radius));
        drop_prog_.uniform("strength", float(height));
        drop_prog_.uniform("heightFields", texfront_);
        drop_prog_.uniform("center", drop);

        drop_prog_.render(back_, vao_);

        swap_buffer_();
    }

    void render() {
        glViewport(0, 0, cells, cells);
        prog_.uniform("heightFields", texfront_);
        prog_.render(back_, vao_);

        swap_buffer_();
    }

    texture& get_height_fields() { return texfront_; }
};

#endif // HEIGHT_FIELD_H_INCLUDED
