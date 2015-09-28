#include <vector>
#include <list>
#include <algorithm>
#include <cassert>

#include "include/image.h"
#include "include/util.h"
#include "include/util.h"

using namespace std;

bool is_inside(const vector<col<4>>& points, col<4> pt_test)
{
    /*
    if(pt_test[0] <= 1 && pt_test[2] <= 1 &&
        pt_test[0] >= -1 && pt_test[2] >= -1)
        return true;
    else return false;
    */

    col<3> center = points[0].reduce(); // center point
    col<3> v3 = pt_test.reduce() - center;

    bool ret = false;

    for(size_t p1 = 1, p2 = 2; p2 < points.size(); p1++, p2++) {
        col<3> v1 = points[p1].reduce() - center,
               v2 = points[p2].reduce() - center;

        double a = (v1 * v2) * (v3 * v2) - (v2 * v2) * (v3 * v1);
        double b = (v2 * v1) * (v3 * v1) - (v1 * v1) * (v3 * v2);
        double c = (v1 * v2) * (v1 * v2) - (v1 * v1) * (v2 * v2);

        a /= c; b /= c;

        ret |= (a >= 0 && b >= 0 && a + b <= 1);
    }

    return ret;
}

double intersection(const vector<col<4>>& points,
        const col<3>& dire, const col<4>& sp)
{
    // finding the normal vector of a plane requires at most 3 vertices
    col<3> v1 = (points[1] - points[0]).to_vec<3>(),
           v2 = (points[2] - points[1]).to_vec<3>();

    col<3> n = v1 % v2; // outer product/normal
    //    (x - p0)n = 0; x = sp + a dire;
    // => a = (- sp n + p0 n) / dire n
    double a = (-sp.reduce() * n + points[0].reduce() * n) / (dire * n);
    //cout << sp.reduce() << " + " << a << " * " << dire << endl;
    // radial line: only faces intersect with ray in positive direction count

    return a;
}

const model::face* find_intersection(const model& m,
        const col<3>& dire, const col<4>& sp, double& min_a)
{
    min_a = 1e8;
    const model::face* inter_f = nullptr;

    for(const model::face& f : m.faces) {
        vector<col<4>> vs;
        for(const model::face_elem& fe : f)
            vs.push_back(m.vertices[get<0>(fe)]);

        double a = intersection(vs, dire, sp);

        //cout << "a: " << a << endl;

        if(a < 0 || a > min_a) continue;

        col<4> inter_p = (sp.reduce() + dire * a).homo();
        if(!is_inside(vs, inter_p)) continue;

        min_a = a;
        inter_f = &f;
    }

    return inter_f;
}

int main()
{
    ofstream fout("assets/ray-tracing.ppm");
    ifstream fin("assets/cube.obj");

    model m;
    m.read(fin);

    /*
    double avr_x = m.statistic(model::avr_x);
    double avr_y = m.statistic(model::avr_y);
    double avr_z = m.statistic(model::avr_z);
    */

    using namespace transform;

    matrix<4, 4> mat = identity();
    mat *= rotate(-M_PI / 12, yOz);
    mat *= rotate(M_PI / 12, zOx);
    //mat *= translate({-avr_x, -avr_y, -avr_z, 1.0});

    for(model::vertex& v : m.vertices)
        v = mat * v.to_mat();

    image img(200, 150);
    //image img(800, 600);

    //double vf = 400, vp = vf + 400;
    double vf = 2, vp = vf + 5;

    for(size_t y = 0; y < img.height(); y++) {
        for(size_t x = 0; x < img.width(); x++) {
            // triangle fan
            col<4> screen_pt{(double(x)-img.width()/2.) / 40.,
                    (-double(y)+img.height()/2.) / 40., vf, 1};
            col<3> dire_vec = (screen_pt - col<4>{0, 0, vp, 1}).to_vec<3>();

            double ray_length;
            const model::face* p_inter = find_intersection(
                    m, dire_vec, screen_pt, ray_length);
            cout << x << ' ' << y << ": " << p_inter << endl;

            if(p_inter) {
                col<4> inter_pt = (screen_pt.reduce() + dire_vec * ray_length).homo();

                col<4> light = { -1, 5, 5, 1 };
                //col<4> light = { 0, 10, 0, 1 };
                light = mat * light.to_mat();
                col<3> normal = m.normals[get<1>((*p_inter)[0])];
                col<3> ray_vec = light.reduce() - inter_pt.reduce();

                normal = normal * (1/normal.abs());
                col<3> ray_nvec = ray_vec * (1/ray_vec.abs());

                double strength = normal * ray_nvec;
                strength = min(abs(strength), 1.0);
                //strength = min(abs(strength), 1.0);

                const model::face* p_shadow = find_intersection(m, ray_vec,
                        (inter_pt.reduce() + normal * 0.00001).homo(), ray_length);
                if(ray_length < 1.0 && ray_length) {
                    cout << ray_length << endl;
                    strength *= ray_length;
                }

                img(x, y) = color::from_arithematic(
                        0x3f + 0xc0 * strength,
                        0x3f + 0xc0 * strength,
                        0x3f + 0xc0 * strength,
                        255
                    );
            } else {
                img(x, y) = 0xff333333;
            }
        }
    }

    fout << img;
}


