// cflags: -lSDL2 -lGL -lGLEW
/*
 * Copyright(c) 2015, Shihira Fung <fengzhiping@hotmail.com>
 */

#include <iostream>
#include <fstream>
#include <vector>
#include <cassert>

#include "../include/shader.h"
#include "../include/matrix.h"

using namespace std;
using namespace labgl;

// tuple of triangle vertex coord stream and normal vectors
tuple<vector<col<4>>, vector<col<3>>>
make_cube(col<3> a, col<3> b)
{
    /*
     *   2-----3    +-----+
     *  /|    /|   /| 3  /|
     * 6-+---7 |  +-+--4+ |
     * | |   | |  |0|   |1|
     * | 0---+-1  | +5--+-+
     * |/    |/   |/  2 |/
     * 4-----5    +-----+
     */

    for(int i = 0; i < 3; i++)
        if(a[i] > b[i]) swap(a[i], b[i]);

    vector<col<4>> v;
    for(int i = 0; i < 8; i++) {
        v.push_back(col<4>{
            (i & 1 ? b : a)[0],
            (i & 2 ? b : a)[1],
            (i & 4 ? b : a)[2], 1 });
    }

    static vector<col<3>> n;
    if(n.empty())
        for(int i = 0; i < 6; i++) {
            col<3> new_n = { 0, 0, 0 };
            new_n[i / 2] = i % 2 ? 1 : -1;
            n.push_back(new_n);
        }

    vector<col<4>> triangles = {
            v[0], v[1], v[5], v[5], v[4], v[0],
            v[0], v[2], v[3], v[3], v[1], v[0],
            v[0], v[4], v[6], v[6], v[2], v[0],
            v[7], v[3], v[2], v[2], v[6], v[7],
            v[7], v[5], v[1], v[1], v[3], v[7],
            v[7], v[6], v[4], v[4], v[5], v[7],
        };
    vector<col<3>> normals = {
            n[2], n[2], n[2], n[2], n[2], n[2],
            n[4], n[4], n[4], n[4], n[4], n[4],
            n[0], n[0], n[0], n[0], n[0], n[0],
            n[3], n[3], n[3], n[3], n[3], n[3],
            n[1], n[1], n[1], n[1], n[1], n[1],
            n[5], n[5], n[5], n[5], n[5], n[5],
        };

    return make_tuple(move(triangles), move(normals));
}

#define clamp_(i, mini, maxi) ((i)<(mini)?(mini):((i)>(maxi)?(maxi):(i)))
#define i_(x, y) (clamp_((x), 0, subd*2-1) * subd*2 + clamp_((y), 0, subd*2-1))

int main(int argc, char** argv)
{
    int mode = 1; // film mode
    if(argc > 1) mode = min(1, atoi(argv[1]));

    windowgl w("Water Pool", 0x0303);
    w.init_glew();

    glClearColor(0.2, 0.2, 0.2, 1);
    glClearDepth(1);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    ifstream vert_shader("waterpool-vert.glsl");
    ifstream frag_shader("waterpool-frag.glsl");
    shader vs = compile(shader::vertex, string(
            istreambuf_iterator<char>(vert_shader),
            istreambuf_iterator<char>()
        ));
    shader fs = compile(shader::fragment, string(
            istreambuf_iterator<char>(frag_shader),
            istreambuf_iterator<char>()
        ));
    program prog = link(vs, fs);

    const int subd = 20;

    vector<col<4>> triangles;
    vector<col<3>> normals;
    vector<int> height_indices;
    vector<float> height(subd * subd * 4, 0);
    vector<float> height_temp(subd * subd * 4, 0);
    vector<float> velocity(subd * subd * 4, 0);

    for(int i = -subd; i < subd; i++)
    for(int j = -subd; j < subd; j++) {
        int ind = i_(i + subd, j + subd);

        if(mode == 0) {
            vector<col<4>> tris;
            vector<col<3>> nmls;
            tie(tris, nmls) = make_cube(
                    col<3>{double(i), 0, double(j)},
                    col<3>{i + 1., 10, j + 1.});

            for(col<4>& t : tris) {
                triangles.push_back(t);
                height_indices.push_back(t[1] > 5 ? ind : -1);
            }
            for(col<3>& n : nmls) normals.push_back(n);
        } else {
            triangles.push_back(col<4>{i * 1., 0, j * 1., 1});
            triangles.push_back(col<4>{i + 1., 0, j * 1., 1});
            triangles.push_back(col<4>{i + 1., 0, j + 1., 1});
            triangles.push_back(col<4>{i + 1., 0, j + 1., 1});
            triangles.push_back(col<4>{i * 1., 0, j + 1., 1});
            triangles.push_back(col<4>{i * 1., 0, j * 1., 1});
            height_indices.push_back(i_(i+subd, j+subd));
            height_indices.push_back(i_(i+subd+1, j+subd));
            height_indices.push_back(i_(i+subd+1, j+subd+1));
            height_indices.push_back(i_(i+subd+1, j+subd+1));
            height_indices.push_back(i_(i+subd, j+subd+1));
            height_indices.push_back(i_(i+subd, j+subd));
        }
    }

    vertex_array vao;
    vao.input(0, triangles);
    if(mode == 0) vao.input(1, normals);
    vao.input(2, height_indices);

    matrix<4, 4>
        mat_modl = transform::identity(),
        mat_view =
            transform::translate(col<4>{0, 10, -40, 1}) *
            transform::rotate(-M_PI / 4, transform::yOz) *
            transform::rotate(M_PI / 6, transform::zOx),
        mat_proj = transform::perspective(M_PI / 4, 4. / 3, 1, 100);

    prog.uniform_block("matrices", make_tuple(mat_modl, mat_view, mat_proj));
    prog.uniform_block("light", col<4>{5, 20, -20, 1});
    prog.uniform("height", height);
    prog.uniform("mode", mode);

    app().register_on_paint([&]() {
        for(int i = 0; i < subd * 2; i++)
        for(int j = 0; j < subd * 2; j++) {
            const static double c = 1.5;
            const static double dx = 1;
            const static double dt = 0.1;
            double f = c * c / (dx * dx) * (
                    height[i_(i+1, j)] + height[i_(i-1, j)] +
                    height[i_(i, j+1)] + height[i_(i, j-1)] -
                    4 * height[i_(i, j)]);
            velocity[i_(i, j)] += f * dt; 
            if(i == 0 || i == 39 || j == 0  || j == 39)
                velocity[i_(i, j)] *= 1.02;
            velocity[i_(i, j)] *= 0.990;
            height_temp[i_(i, j)] = height[i_(i, j)] + velocity[i_(i, j)] * dt;
        }

        swap(height_temp, height);
        prog.uniform("height", height);

        prog.render(frame_buffer::screen, vao);

        w.swap_buffer();
    });


    matrix<4, 4> mat_proj_inv = mat_proj.inverse();
    matrix<4, 4> mat_view_inv = mat_view.inverse();
    app().register_on_mouse_up([&](int x, int y, application::mouse_button b) {
        static int prev_x = 0;
        static int prev_y = 0;
        if(prev_x == x && prev_y == y) return;

        col<4> p = { (x-400)/400.0, (300-y)/300.0, 0, 1 };
        p = mat_proj_inv * p.to_mat();
        p = mat_view_inv * p.to_mat();
        p = p * (1 / p[3]);
        col<4> o = mat_view_inv * col<4>{ 0, 0, 0, 1 }.to_mat();
        o = o * (1 / o[3]);

        col<4> op = p - o;
        double l = -o[1] / op[1];
        p = o + op * l;

        cout << p << endl;

        for(int i = -2; i < 2; i++)
        for(int j = -2; j < 2; j++) {
            double r = sqrt(i * i + j * j);
            height[i_(i + int(p[0] + subd), j + int(p[2]) + subd)] =
                2 * 1.414 - r;
        }

        prev_x = x;
        prev_y = y;
    });

    app().run();
}

