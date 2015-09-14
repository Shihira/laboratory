#ifndef SECTION
#define SECTION 5
#endif

#include <iostream>
#include <vector>
#include <algorithm>

#include "include/util.h"
#include "include/matrix.h"
#include "include/image.h"
#include "scanline-fill.cc"

using namespace std;

int main()
{
        ifstream wavefront("assets/teapot.obj");
        //read_points(wavefront, points, faces, vnormal);
        model m;
        m.read(wavefront);

        vector<col<4>>& points = m.vertexes;
        vector<col<4>> projs;
        vector<col<3>>& vnormal = m.normals;
        vector<vector<int>> faces;

        for(auto _f : m.faces) {
            vector<int> f;
            for(auto _fe : _f) {
                f.push_back(get<0>(_fe));
            }
            faces.push_back(f);
        }

        image img(800, 600);
        ofstream fimg("assets/vertex-coloring.ppm");

        double s/*Scaling*/ = 6,
               d/*Depression angle*/ = M_PI/6,
               u/*focUs distance*/ = 400;

        ////////////////////////////////////////////////////////////////////////
        // transformation
        matrix<4, 4>
        scaling = {
                s, 0, 0, 0,
                0, s, 0, 0,
                0, 0, s, 0,
                0, 0, 0, 1,
        },
        translation = {
                1, 0, 0, 0,
                0, 1, 0, 0,
                0, 0, 1, -u,
                0, 0, 0, 1,
        },
        center = {
                1, 0, 0, 400,
                0, 1, 0, 300,
                0, 0, 1, 0,
                0, 0, 0, 1,
        },
        rotation = {
                cos(d), 0, sin(d), 0,
                0, 1, 0, 0,
                -sin(d), 0, cos(d), 0,
                0, 0, 0, 1,
        },
        rotation2 = {
                1, 0, 0, 0,
                0, cos(d), -sin(d), 0,
                0, sin(d), cos(d), 0,
                0, 0, 0, 1,
        },
        projection = {
                1, 0, 0, 0,
                0, 1, 0, 0,
                0, 0, 0, 0,
                0, 0, -1/u, 1,
        };

        for(col<4>& p : points) {
                p = translation * scaling * rotation2 * rotation * p.to_mat();
                col<4> proj = center * projection * p.to_mat();

                projs.push_back(proj);
        }

        ////////////////////////////////////////////////////////////////////////
        // initialize depth buffer matrix
        matrix<600, 800> z_buffer;
        for(col<600>& i : z_buffer) for(double& j : i)
                j = numeric_limits<double>::lowest();

        // fill faces
        for(vector<int>& f : faces) {
                point_list ps;
                vector<double> bs;

                for(int i : f) {
                        ps.push_back(point(
                                int(projs[i][0] / projs[i][3]), img.height() -
                                int(projs[i][1] / projs[i][3])));
                        col<3> light = { 1.l, 0.l, -1.l/2 };
                        col<3> normal = vnormal[i];

                        double cos2_nl = pow(normal * light , 2)
                                / (normal * normal) / (light * light);
                        double brightness = 240 * sqrt(1 - cos2_nl);

                        bs.push_back(brightness);
                }

                col<3> p_1 = points[f[0]].reduce(),
                       p_2 = points[f[1]].reduce(),
                       p_3 = points[f[2]].reduce();
                col<3> normal = (p_2 - p_1) % (p_3 - p_2);

                scanline_fill(ps, [&](const point& p) {
                        // line (0, 0, u) -> (o_x, o_y, 0)
                        // eq x = o_x l; y = o_y l; z = u - ul;
                        // face p_0, p_1, p_2
                        // eq (n_i, n_j, n_k) . (P - (p_x, p_y, p_z)) = 0
                        double o_x = p.x - center(0, 3),
                               o_y = (img.height() - p.y) - center(1, 3);
                        col<3> sight_line = { o_x, o_y, -u };
                        // solve l from equation:
                        // n_i(o_xl - p_x) +
                        // n_j(o_yl - p_y) +
                        // n_k(u  l - p_z + u) = 0
                        double l = (normal * points[f[0]].reduce() -
                                normal[2] * u) / (normal * sight_line);
                        double X = o_x * l;
                        double Y = o_y * l;
                        double Z = u - u * l;

                        col<3> P = { X, Y, Z };
                        double C1 = ((P - p_2) % (P - p_3)).abs() / normal.abs();
                        double C2 = ((P - p_1) % (P - p_3)).abs() / normal.abs();
                        double C3 = ((P - p_1) % (P - p_2)).abs() / normal.abs();
                        
                        double err = C1 + C2 + C3;
                        if(err > 1.0) { C1 /= err; C2 /= err; C3 /= err; }

                        double brightness = C1 * bs[0] + C2 * bs[1] + C3 * bs[2];

                        if(Z <= z_buffer(p.y, p.x)) return;
                        z_buffer(p.y, p.x) = Z;

                        img[p] = color(brightness, brightness, brightness);

                });
        }

        fimg << img;
}
