#include <vector>
#include <list>
#include <algorithm>
#include <cassert>

#include "include/image.h"
#include "include/util.h"
#include "include/util.h"

using namespace std;

#define F faces[face_i]
#define FN fnormals[face_i]
#define FC fcolors[face_i]
#define V(fe) vertices[std::get<0>(fe)]
#define VN(fe) normals[std::get<1>(fe)]

class renderer : public model {
public:
    image& img;

    vector<col<3>> fnormals;
    col<4> light;
    vector<color> fcolors;

    renderer(ifstream& fin, image& _img)
        : img(_img), light{0, 0, 0, 1} {
        read(fin);
    }

    void _gen_fnormals() {
        for(face& f : faces) {
            assert(f.size() >= 3);

            col<3> v1 = V(f[1]).reduce() - V(f[0]).reduce();
            col<3> v2 = V(f[2]).reduce() - V(f[1]).reduce();

            col<3> n = v1 % v2;
            fnormals.push_back(n * (1/n.abs()));
        }
    }

    bool is_inside(size_t face_i, const vertex& v) {
        assert(F.size() >= 3);

        col<3> center = V(F[0]).reduce();
        col<3> v3 = v.reduce() - center;

        bool ret = false;

        for(size_t p1 = 1, p2 = 2; p2 < F.size() && !ret; p1++, p2++) {
            col<3> v1 = V(F[p1]).reduce() - center,
                   v2 = V(F[p2]).reduce() - center;

            double  v1_v2 = (v1 * v2), v2_v3 = (v2 * v3), v1_v3 = (v1 * v3),
                    v1_v1 = (v1 * v1), v2_v2 = (v2 * v2); // v3_v3 = (v3 * v3);

            double a = v1_v2 * v2_v3 - v2_v2 * v1_v3;
            double b = v1_v2 * v1_v3 - v1_v1 * v2_v3;
            double c = v1_v2 * v1_v2 - v1_v1 * v2_v2;

            a /= c; b /= c;

            ret |= (a >= 0 && b >= 0 && a + b <= 1);
        }

        return ret;
    }

    // This function find the intersection point of line:(p+au) and the face F
    // is in, and doesn't ensure the intersection in that polygon
    double intersection(size_t face_i, const vertex& p, const col<3>& u) {
        col<3>& n = FN;

        //    (x - p0)n = 0; x = p + a u;
        // => a = (- p n + p0 n) / (u n)
        return ( - p.reduce() * n + V(F[0]).reduce() * n) / (u * n);
    }

    size_t emit(const vertex& p, const col<3>& u, double& min_a) {
        min_a = 1e8;
        size_t min_face_i = ~0UL;

        for(size_t face_i = 0; face_i < faces.size(); face_i++) {
            double a = intersection(face_i, p, u);

            if(a < 0 || a > min_a) continue;

            col<4> inter_p = (p.reduce() + u * a).homo();
            if(!is_inside(face_i, inter_p)) continue;

            min_a = a;
            min_face_i = face_i;
        }

        return min_face_i;
    }

    void multiply(const matrix<4, 4>& mat) {
        matrix<4, 4> mat_1_t = mat.inverse().t();

        for(vertex& v : vertices)
            v = mat * v.to_mat();
        for(normal& n : normals) {
            n = col<4>(mat_1_t * n.to_vec<4>().to_mat()).to_vec<3>();
            n = n * (1/n.abs());
        }
        light = mat * light.to_mat();
    }

    color trace(const vertex& p, const col<3>& u, size_t level = 0) {
        double ray_len;
        size_t face_i = emit(p, u, ray_len);
        cout << int(face_i) << p.reduce() + u * ray_len;
        if(face_i == ~0UL) return 0xff333333;

        vertex ep = (p.reduce() + u * ray_len).homo();
        col<3> normal = VN(F[0]);
        col<3> ray = light.reduce() - ep.reduce();
        col<3> rayi = ray * (1 / ray.abs());

        ep = (ep.reduce() + normal * 0.000001).homo();
        col<3> ui = u * (1 / u.abs());

        //////////////////////// DYE ///////////////////////////////////////////
        // diffuse light
        double dye_strength = normal * rayi;
        dye_strength = min(abs(dye_strength), 1.0);

        // drop shadow
        double light_len;
        emit(ep, ray, light_len);
        if(light_len < 1.0)
            dye_strength *= light_len * 3;

        color dye = FC;
        dye.r *= 0.3 + 0.7 * dye_strength;
        dye.g *= 0.3 + 0.7 * dye_strength;
        dye.b *= 0.3 + 0.7 * dye_strength;

        //////////////////////// FURTHER TRACING ///////////////////////////////
        if(level < 1 && (face_i == 2 || face_i == 0)) {
            // reflection
            col<3> r = ui - normal * (ui * normal * 2);
            color rc = trace(ep, r, level + 1);
            dye = dye.blend(rc, 0.3);
        }

        return dye;
    }

    void render() {
        _gen_fnormals();

        double vf = 2, vp = vf + 5;

        for(size_t y = 0; y < img.height(); y++)
        for(size_t x = 0; x < img.width(); x++) {
            //if(x != 30 || y != 100) continue;

            // triangle fan
            col<4> screen_pt{(double(x)-img.width()/2.) / 40.,
                    (-double(y)+img.height()/2.) / 40., vf, 1};
            col<3> dire_vec = (screen_pt - col<4>{0, 0, vp, 1}).to_vec<3>();

            cout << x << '\t' << y << ": ";
            img(x, y) = trace(screen_pt, dire_vec);
            cout << endl;
        }
    }
};

int main(int argc, char* argv[])
{
    ofstream fout("assets/ray-tracing.ppm");
    ifstream fin("assets/cube.obj");

    image img(200, 150);

    renderer m(fin, img);
    m.light = col<4>{ -1, 6, 5, 1 };
    m.fcolors = {
        color(0xffffcba4),
        color(0xff9172ec),
        color(0xff5efb6e),
        color(0xffffffff),
        color(0xffffffff),
        color(0xffffffff),
        color(0xffffffff),
        color(0xffffffff),
        color(0xffffffff),
    };

    /*
    double avr_x = m.statistic(model::avr_x);
    double avr_y = m.statistic(model::avr_y);
    double avr_z = m.statistic(model::avr_z);
    */

    using namespace transform;

    matrix<4, 4> mat = identity();
    mat *= rotate(-M_PI / 6, yOz);
    mat *= rotate(M_PI / 3, zOx);
    //mat *= translate({-avr_x, -avr_y, -avr_z, 1.0});
    m.multiply(mat);

    m.render();

    fout << img;
}


