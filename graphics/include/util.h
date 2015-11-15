#ifndef UTIL_H_DEFINED
#define UTIL_H_DEFINED

#include <utility>
#include <vector>
#include <tuple>
#include <cmath>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cstdio>

#include "matrix.h"

class point : public std::pair<double, double> {
public:
    point() : pair(0.0, 0.0) { }
    point(double _x, double _y) : pair(_x, _y) { }
    point(const point& p) : pair(p) { }
    template <typename T> point(std::initializer_list<T> l) {
        first = *l.begin();
        second = *(l.begin() + 1);
    }

    double &x = first;
    double &y = second;

    point operator=(const point& other)
        { x = other.x; y = other.y; return *this; }
    point operator+(const point& other)
        { return { x + other.x, y + other.y }; }
    point operator-(const point& other)
        { return { x - other.x, y - other.y }; }
    point operator+=(const point& other)
        { x += other.x; y += other.y; return *this; } 

    double operator|(const point& other) { // NORM
        double dx = x - other.x;
        double dy = y - other.y;

        return std::sqrt(dx * dx + dy * dy);
    }

    point operator*(double coefficient)
        { return { coefficient * x, coefficient * y }; }
};

inline point operator*(const point& p, double coefficient)
    { return p * coefficient; }

inline std::ostream& operator<<(std::ostream& s, const point& p)
{
    return (s << '(' << p.first << ", " << p.second << ')');
}

inline std::istream& operator>>(std::istream& s, point& p)
{
    s.clear();

    auto ignore_char = [&](char c) {
        s >> std::ws;
        if(s.peek() == c) {
            s.ignore(1);
            return true;
        } else return false;
    }; 

    bool bracket = ignore_char('(');
    if(s >> p.first, s.fail()) return s;
    ignore_char(',');
    if(s >> p.second, s.fail()) return s;
    if(bracket) ignore_char(')');

    return s;
}

typedef std::vector<point> point_list;

////////////////////////////////////////////////////////////////////////////////

class model {
public:
    typedef col<4> vertex;
    typedef col<3> normal;
    typedef col<3> texture;

    typedef std::tuple<size_t, size_t, size_t> face_elem;
    typedef std::vector<face_elem> face;

protected:
    // _face_elem uses integer to index faces
    // and face_elem contains actual vertex/normal/texture information

    void read_v(const std::string& str_v) {
        vertex v;
        std::istringstream is(str_v);

        for(int i = 0; i < 4; i++) {
            is >> v[i];
            if(is.fail()) {
                if(i == 3) {
                    v[3] = 1;
                    is.clear();
                } else
                    throw std::runtime_error("Bad obj");
            }
        }

        vertices.push_back(v);
    }

    void read_vt(const std::string& str_vt) {
        texture vt;
        std::istringstream is(str_vt);

        for(int i = 0; i < 3; i++) {
            is >> vt[i];
            if(is.fail()) {
                if(i == 2) {
                    vt[2] = 1;
                    is.clear();
                } else
                    throw std::runtime_error("Bad obj");
            }
        }

        textures.push_back(vt);
    }

    void read_vn(const std::string& str_vn) {
        normal vn;
        std::istringstream is(str_vn);

        for(int i = 0; i < 3; i++) {
            is >> vn[i];
            if(is.fail()) 
                throw std::runtime_error("Bad obj");
        }

        normals.push_back(vn);
    }

    void read_f(const std::string& str_f) {
        face f;

        // 3 choices for face element:
        //   i. v1 v2 v3 ...
        //  ii. v1//vt1 ...
        // iii. v1/vn1/vt1 ...

        int v, vn, vt;
        std::string::const_iterator start_i = str_f.begin(), space_i;

        while(true) {
            space_i = std::find(start_i, str_f.end(), ' ');

            std::string str_fe(start_i, space_i);

            if(str_fe.find("//") != str_fe.npos) { // ii.
                int assigned = std::sscanf(str_fe.c_str(), "%d//%d", &v, &vn);
                if(assigned != 2)
                    throw std::runtime_error("Bad obj");
                vt = v;
            } else if(str_fe.find('/') != str_fe.npos) { // iii.
                int assigned = std::sscanf(str_fe.c_str(), "%d/%d/%d", &v, &vt, &vn);
                if(assigned != 3)
                    throw std::runtime_error("Bad obj");
            } else { // i.
                int assigned = std::sscanf(str_fe.c_str(), "%d", &v);
                if(assigned != 1)
                    throw std::runtime_error("Bad obj");
                vn = v;
                vt = v;
            }

            f.push_back(face_elem(
                    static_cast<size_t>(v  >= 0 ? v  - 1 : vertices.size() + v ),
                    static_cast<size_t>(vn >= 0 ? vn - 1 : vertices.size() + vn),
                    static_cast<size_t>(vt >= 0 ? vt - 1 : vertices.size() + vt)
                ));

            start_i = std::find_if(space_i, str_f.end(), [] (char c) {
                    return c != ' ' && c != '\t' && c != '\r' && c != '\n'; });
            if(start_i == str_f.end()) break;
        }

        faces.push_back(f);
    }

public:
    std::vector<vertex> vertices;
    std::vector<texture> textures;
    std::vector<normal> normals;
    std::vector<face> faces;

    std::string name;

    void read(std::istream& fin, bool ignore = false) {
        name.clear();
        vertices.clear();
        normals.clear();
        textures.clear();
        faces.clear();

        while(!fin.eof() && !fin.fail()) {
            std::string cmd;
            fin >> std::ws >> cmd >> std::ws;

            if(cmd == "g") {
                if(name.empty()) {
                    fin >> name;
                    continue;
                } else {
                    fin.putback(' ');
                    fin.putback('g');
                    break; // next model
                }
            }

            if(ignore) {
                fin.ignore(0x7fffffff, '\n');
                continue;
            }

            std::string line;
            std::getline(fin, line);

            if(cmd == "v") {
                read_v(line);
            } else if(cmd == "f") {
                read_f(line);
            } else if(cmd == "vn") {
                read_vn(line);
            } else if(cmd == "vt") {
                read_vt(line);
            }
        }
    }

    typedef enum {
        avr_x = 0, avr_y = 1, avr_z = 2,
        min_x = 3, min_y = 4, min_z = 5,
        max_x = 6, max_y = 7, max_z = 9,
    } stat_item;
    double statistic(stat_item item) {
        double ret;

        switch(item) {
        case avr_x: case avr_y: case avr_z:
            ret = 0.0;
            for(vertex& v : vertices)
                ret += v[static_cast<size_t>(item)];
            ret /= vertices.size();
            break;
        case min_x: case min_y: case min_z:
            ret = vertices[0][static_cast<size_t>(item) - 3];
            for(vertex& v : vertices)
                ret = std::min(ret, v[static_cast<size_t>(item) - 3]);
            break;
        case max_x: case max_y: case max_z:
            ret = vertices[0][static_cast<size_t>(item) - 3];
            for(vertex& v : vertices)
                ret = std::max(ret, v[static_cast<size_t>(item) - 3]);
            break;
        }

        return ret;
    }

    void extract_triangles(std::vector<vertex>& vs,
            std::vector<normal>& ns, std::vector<texture>& ts) {
        for(face& f : faces) {
            size_t fe_i[] = { 0, 1, 2 };
            for(; fe_i[2] < f.size(); fe_i[1]++, fe_i[2]++) {
                for(size_t j = 0; j < 3; j++) {
                    size_t v_i, n_i, t_i;
                    std::tie(v_i, n_i, t_i) = f[fe_i[j]];

                    if(v_i < vertices.size()) {
                        vertex& v = vertices[v_i];
                        vs.push_back(v);
                    }

                    if(n_i < normals.size()) {
                        normal& n = normals[n_i];
                        ns.push_back(n);
                    }

                    if(t_i < textures.size()) {
                        texture& t = textures[t_i];
                        ts.push_back(t);
                    }
                }
            }
        }
    }
};

#define EST(code, repeat) \
        { \
                long est_t = clock(); \
                for(int lp = 0; lp < repeat; lp++) { code; } \
                printf("ln.%d used: %ld us\n", __LINE__, clock() - est_t); \
        }
#define EST_10K(code) EST(code, 10000)

#endif
