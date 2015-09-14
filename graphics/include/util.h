#ifndef UTIL_H_DEFINED
#define UTIL_H_DEFINED

#include <utility>
#include <vector>
#include <tuple>
#include <cmath>
#include <iostream>
#include <sstream>
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

protected:
    // _face_elem uses integer to index faces
    // and face_elem contains actual vertex/normal/texture information
    typedef std::tuple<size_t, size_t, size_t> _face_elem;
    typedef std::vector<_face_elem> _face;

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

        vertexes.push_back(v);
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
        _face f;

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
                int assigned = std::sscanf(str_fe.c_str(), "%d//%d", &v, &vt);
                if(assigned != 2)
                    throw std::runtime_error("Bad obj");
                vn = v;
            } else if(str_fe.find('/') != str_fe.npos) { // iii.
                int assigned = std::sscanf(str_fe.c_str(), "%d/%d/%d", &v, &vn, &vt);
                if(assigned != 3)
                    throw std::runtime_error("Bad obj");
            } else { // i.
                int assigned = std::sscanf(str_fe.c_str(), "%d", &v);
                if(assigned != 1)
                    throw std::runtime_error("Bad obj");
                vn = v;
                vt = v;
            }

            f.push_back(_face_elem(
                    static_cast<size_t>(v  >= 0 ? v  - 1 : vertexes.size() + v ),
                    static_cast<size_t>(vn >= 0 ? vn - 1 : vertexes.size() + vn),
                    static_cast<size_t>(vt >= 0 ? vt - 1 : vertexes.size() + vt)
                ));

            start_i = std::find_if(space_i, str_f.end(), [] (char c) {
                    return c != ' ' && c != '\t' && c != '\r' && c != '\n'; });
            if(start_i == str_f.end()) break;
        }

        faces.push_back(f);
    }

public:
    typedef std::tuple<vertex, normal, texture> face_elem;
    typedef std::vector<face_elem> face;

    std::vector<vertex> vertexes;
    std::vector<texture> textures;
    std::vector<normal> normals;
    std::vector<_face> faces;

    std::string name;

    void read(std::istream& fin, bool ignore = false) {
        name.clear();
        vertexes.clear();
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

    face at(size_t i) {
        _face& _f = faces[i];
        face f;

        for(_face_elem& fe : _f) {
            vertex v;
            normal n;
            texture t;

            if(vertexes.size() > std::get<0>(fe))
                v = vertexes[std::get<0>(fe)];
            if(normals.size() > std::get<1>(fe))
                n = normals[std::get<1>(fe)];
            if(textures.size() > std::get<2>(fe))
                t = textures[std::get<2>(fe)];
            f.push_back(face_elem(v, n, t));
        }

        return f;
    }

    size_t size() { return faces.size(); }

    typedef enum { avr_x = 0, avr_y = 1, avr_z = 2 } stat_item;
    double statistic(stat_item item) {
        double sum = 0;
        for(vertex& v : vertexes)
            sum += v[static_cast<size_t>(item)];
        return sum / vertexes.size();
    }
};

#define EST(code, repeat) \
        { \
                long est_t = clock(); \
                for(int lp = 0; lp < repeat; lp++) { code; } \
                printf("ln.%d used: %ld us\n", __LINE__, clock() - est_t); \
        }
#define EST_M(code) EST(code, 10000)

#endif
