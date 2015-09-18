#ifndef SECTION
#define SECTION 5
#endif

#include <iostream>
#include <vector>
#include <list>
#include <algorithm>
#include <cassert>

#include "include/util.h"
#include "include/image.h"

using namespace std;

#define EQUAL_FLOAT(a, b) (int(a) == int(b))

namespace {

struct vertex_info {
    point p;
    color c;
};

struct edge {
    const vertex_info* f; // from
    const vertex_info* t; // to
};

struct blender {
    blender(const color& _b, const color& _e, double times) {
        r_step = (_e.r - _b.r) / times;
        g_step = (_e.g - _b.g) / times;
        b_step = (_e.b - _b.b) / times;

        r_cur = _b.r;
        g_cur = _b.g;
        b_cur = _b.b;
    }

    bool advance() {
        r_cur += r_step;
        g_cur += g_step;
        b_cur += b_step;

        return true;
    }

    color get() {
        return color(r_cur, g_cur, b_cur);
    }

protected:
    double r_step;
    double g_step;
    double b_step;

    double r_cur;
    double g_cur;
    double b_cur;
};


struct active_edge {
    edge* e;
    double x;
    double x_inc;
    blender b;

    active_edge(edge* _e) : e(_e),
        b(e->f->c, e->t->c, e->t->p.y - e->f->p.y) {
        x = e->f->p.x;
        x_inc = (e->t->p.x - x) / (e->t->p.y - e->f->p.y);
    }

    bool advance(size_t y) {
        b.advance();
        x += x_inc;

        return size_t(e->t->p.y) <= y;
    }

    bool operator<(const active_edge& other) {
        return x < other.x;
    }
};

}

void convex_fill(const vector<vertex_info>& v, image& img) {
    // transform vertices to edge table
    vector<edge> edges;
    for(size_t i = 0; i < v.size(); i++) {
        size_t next_i = (i + 1) % v.size();
        if(v[i].p.y < v[next_i].p.y)
            edges.push_back({ &v[i], &v[next_i] });
        else edges.push_back({ &v[next_i], &v[i] });
    }

    // sort edges from top to bottom
    sort(edges.begin(), edges.end(),
        [](const edge& e1, const edge& e2) {
            return e1.f->p.y < e2.f->p.y; });

    vector<active_edge> active_edges;
    auto heap_cmp = [](const active_edge& ae1, const active_edge& ae2) {
                return ae1.x > ae2.x; };
    make_heap(active_edges.begin(), active_edges.end(), heap_cmp);
    auto cur_edge = edges.begin();
    
    for(size_t y = edges.front().f->p.y; y < edges.back().t->p.y; y++) {
        // increase the edges and remove inactive edges
        bool need_sort = false;

        remove_if(active_edges.begin(), active_edges.end(),
                [y, &need_sort](active_edge& ae) {
            bool is_to_removed = ae.advance(y);
            return need_sort |= is_to_removed, is_to_removed;
        });

        // supplement new active edges
        while(cur_edge != edges.end() && y >= size_t(cur_edge->f->p.y)) {
            need_sort = true;
            active_edges.push_back(active_edge(&*cur_edge++));
        }

        if(need_sort)
            sort(active_edges.begin(), active_edges.end());

        // sort active edges again on a new scanline. this algorithm
        // allows convex polygon only, so just 2 edges is required
        active_edge&
            el = *active_edges.begin(),
            er = *++active_edges.begin();

        blender line_b(el.b.get(), er.b.get(), er.x - el.x);

        double a = 0.0, a_inc = 1 / (er.x - el.x);
        for(size_t x = el.x; x < er.x; x++, line_b.advance()) {
            img(x, y) = line_b.get();
            a += a_inc;
        }
    }
}

#if SECTION == 5

int main()
{
    image img(800, 600);
    ofstream fout("assets/convex-fill.ppm");

    img(70, 70) = color(0x33, 0x44, 0x55);

    vector<vertex_info> vertices = {
        vertex_info { point(200, 450), color(0xffff00ff) },
        vertex_info { point(200, 150), color(0xffff0000) },
        vertex_info { point(600, 350), color(0x000000ff) },
        vertex_info { point(700, 500), color(0xff00ff00) },
        vertex_info { point(400, 550), color(0xffffff00) }
    };

    convex_fill(vertices, img);

    fout << img;
}

#endif
