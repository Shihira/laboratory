/*
 * Copyright(c) 2015, Shihira Fung <fengzhiping@hotmail.com>
 */
#ifndef MODEL_H_INCLUDED
#define MODEL_H_INCLUDED

#include <fstream>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

#include "matrix.h"

class model {
public:
    virtual std::vector<col<4>> triangles() const = 0;
    virtual std::vector<col<3>> normals() const = 0;
    virtual std::vector<col<3>> uvcoords() const = 0;
};

class plane_model : public model {
    size_t grid_w_;
    size_t grid_h_;

    col<3> origin_;
    col<3> df_dx_;
    col<3> df_dy_;

    size_t i_(size_t i, size_t j) {
        return i * (grid_w_ + 1) + j;
    }

public:

    plane_model(col<2> tl, col<2> br,
            col<3>xaxis = col<3>{1, 0, 0},
            col<3>yaxis = col<3>{0, 1, 0}) :
        grid_w_(1), grid_h_(1),
        origin_(xaxis * tl[0] + yaxis * tl[1]),
        df_dx_(xaxis * (br[0] - tl[0])),
        df_dy_(yaxis * (br[1] - tl[1])) { }

    void subdivide(size_t w, size_t h) {
        df_dx_ = df_dx_ * (1.0 / w);
        df_dy_ = df_dy_ * (1.0 / h);

        grid_h_ *= h;
        grid_w_ *= w;
    }

    std::vector<col<4>> triangles() const {
        std::vector<col<4>> tris;

        for(size_t i = 0; i < grid_h_; i++)
        for(size_t j = 0; j < grid_w_; j++) {
            col<4> tl = (origin_ + df_dx_ * j + df_dy_ * i).homo(),
                   tr = (origin_ + df_dx_ * (j+1) + df_dy_ * i).homo(),
                   bl = (origin_ + df_dx_ * j + df_dy_ * (i+1)).homo(),
                   br = (origin_ + df_dx_ * (j+1) + df_dy_ * (i+1)).homo();
            tris.push_back(tl); tris.push_back(tr); tris.push_back(br);
            tris.push_back(br); tris.push_back(bl); tris.push_back(tl);
        }

        return tris;
    }

    std::vector<col<3>> normals() const {
        col<3> n = df_dx_ % df_dy_;
        n = n * (1 / n.abs());
        return std::vector<col<3>>(6 * grid_w_ * grid_h_, n);
    }

    std::vector<col<3>> uvcoords() const {
        std::vector<col<3>> uvs;
        double step_h = 1.0 / grid_h_, step_w = 1.0 / grid_w_;
        for(size_t i = 0; i < grid_h_; i++)
        for(size_t j = 0; j < grid_w_; j++) {
            col<3> tl = { j * step_w, i * step_h, 1 },
                   tr = { (j+1) * step_w, i * step_h, 1 },
                   bl = { j * step_w, (i+1) * step_h, 1 },
                   br = { (j+1) * step_w, (i+1) * step_h, 1 };
            uvs.push_back(tl); uvs.push_back(tr); uvs.push_back(br);
            uvs.push_back(br); uvs.push_back(bl); uvs.push_back(tl);
        }
        return uvs;
    }
};

class cube_model : public model {
};

class wavefront_model : public model {
public:
    typedef col<4> vertex;
    typedef col<3> normal;
    typedef col<3> texture;

    typedef std::tuple<size_t, size_t, size_t> face_elem;
    typedef std::vector<face_elem> face;

protected:
    // _face_elem uses integer to index faces_
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

        vertices_.push_back(v);
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

        textures_.push_back(vt);
    }

    void read_vn(const std::string& str_vn) {
        normal vn;
        std::istringstream is(str_vn);

        for(int i = 0; i < 3; i++) {
            is >> vn[i];
            if(is.fail()) 
                throw std::runtime_error("Bad obj");
        }

        normals_.push_back(vn);
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
                    static_cast<size_t>(v  >= 0 ? v  - 1 : vertices_.size() + v ),
                    static_cast<size_t>(vn >= 0 ? vn - 1 : vertices_.size() + vn),
                    static_cast<size_t>(vt >= 0 ? vt - 1 : vertices_.size() + vt)
                ));

            start_i = std::find_if(space_i, str_f.end(), [] (char c) {
                    return c != ' ' && c != '\t' && c != '\r' && c != '\n'; });
            if(start_i == str_f.end()) break;
        }

        faces_.push_back(f);
    }

    std::vector<vertex> vertices_;
    std::vector<texture> textures_;
    std::vector<normal> normals_;
    std::vector<face> faces_;

    template<typename T>
    auto foreach_mesh(T func) const ->
            decltype(func(size_t(), size_t(), size_t())) {
        typedef decltype(func(size_t(), size_t(), size_t())) ret_type;
        ret_type v;
        for(const face& f : faces_) {
            size_t fe_i[] = { 0, 1, 2 };
            for(; fe_i[2] < f.size(); fe_i[1]++, fe_i[2]++) {
                for(size_t j = 0; j < 3; j++) {
                    size_t v_i, n_i, t_i;
                    std::tie(v_i, n_i, t_i) = f[fe_i[j]];

                    ret_type vf = func(v_i, n_i, t_i);
                    v.insert(v.end(), vf.begin(), vf.end());
                }
            }
        }

        return v;
    }

public:

    std::string name;

    void read(std::istream& fin, bool ignore = false) {
        name.clear();
        vertices_.clear();
        normals_.clear();
        textures_.clear();
        faces_.clear();

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
            for(vertex& v : vertices_)
                ret += v[static_cast<size_t>(item)];
            ret /= vertices_.size();
            break;
        case min_x: case min_y: case min_z:
            ret = vertices_[0][static_cast<size_t>(item) - 3];
            for(vertex& v : vertices_)
                ret = std::min(ret, v[static_cast<size_t>(item) - 3]);
            break;
        case max_x: case max_y: case max_z:
            ret = vertices_[0][static_cast<size_t>(item) - 3];
            for(vertex& v : vertices_)
                ret = std::max(ret, v[static_cast<size_t>(item) - 3]);
            break;
        }

        return ret;
    }

    void extract_triangles(std::vector<vertex>& vs,
            std::vector<normal>& ns, std::vector<texture>& ts) {
        vs = triangles();
        ns = normals();
        ts = uvcoords();
    }

    std::vector<col<4>> triangles() const {
        return foreach_mesh([&](size_t v_i, size_t, size_t) {
            if(v_i >= vertices_.size()) return std::vector<col<4>>();
            return std::vector<col<4>>(1, vertices_[v_i]);
        });
    }

    std::vector<col<3>> normals() const {
        return foreach_mesh([&](size_t, size_t n_i, size_t) {
            if(n_i >= normals_.size()) return std::vector<col<3>>();
            return std::vector<col<3>>(1, normals_[n_i]);
        });
    }

    std::vector<col<3>> uvcoords() const {
        return foreach_mesh([&](size_t, size_t, size_t t_i) {
            if(t_i >= textures_.size()) return std::vector<col<3>>();
            return std::vector<col<3>>(1, textures_[t_i]);
        });
    }

    decltype(vertices_)& get_vertices() { return vertices_; }
    decltype(normals_)& get_normals() { return normals_; }
    decltype(textures_)& get_textures() { return textures_; }
    decltype(faces_)& get_meshes() { return faces_; }
};

#endif // MODEL_H_INCLUDED
