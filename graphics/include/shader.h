/*
 * Copyright (C) Shihira Fung <fengzhiping@hotmail.com>
 *
 * Shader.h contains not only shader class but many other utilities for
 * rendering with shaders in OpenGL 3.x.
 */

#ifndef SHADER_H_INCLUDED
#define SHADER_H_INCLUDED

#include <GL/glew.h>
#include <string>
#include <memory>
#include <map>
#include <tuple>

#include "matrix.h"
#include "image.h"
#include "gui.h"

namespace labgl {

class shader {
protected:
    GLuint shader_id_;

public:
    shader(GLuint sdid) : shader_id_(sdid) { }
    shader(const shader& sd) : shader_id_(sd.shader_id_) { }
    shader& operator=(shader& sd) {
        shader_id_ = sd.shader_id_; return *this; }
    GLuint id() const { return shader_id_; }

    enum shader_type_e {
        vertex = GL_VERTEX_SHADER,
        fragment = GL_FRAGMENT_SHADER,
    };

};

inline shader compile(shader::shader_type_e type, const std::string& src)
{
    GLuint sdid = glCreateShader(GLenum(type));
    const char* s = src.c_str();
    glShaderSource(sdid, 1, &s, GL_NONE);
    glCompileShader(sdid);

    GLint cmpl_param;
    glGetShaderiv(sdid, GL_COMPILE_STATUS, &cmpl_param);
    if(cmpl_param != GL_TRUE) {
        glGetShaderiv(sdid, GL_INFO_LOG_LENGTH, &cmpl_param);
        std::unique_ptr<char[]> log(new char[cmpl_param + 1]);
        GLsizei log_length;
        glGetShaderInfoLog(sdid, cmpl_param, &log_length, log.get());
        log[log_length] = '\0';
        std::string log_str(log.get());

        throw std::runtime_error(log_str);
    }

    return shader(sdid);
}

/******** Uniform overloading functions induction *********/

template<typename T>
struct uniform_helper_ { };

#define declare_umat_func_(glfunc, t) \
    static void ufunc (GLint loc, GLsizei c, const t* v) \
    { glfunc(loc, c, false, v); }
#define declare_uvec_func_(glfunc, t) \
    static void ufunc (GLint loc, GLsizei c, const t* v) \
    { glfunc(loc, c, v); }

template<> struct uniform_helper_<matrix<4, 4>>
    { declare_umat_func_(glUniformMatrix4fv, GLfloat); };
template<> struct uniform_helper_<col<4>>
    { declare_uvec_func_(glUniform4fv, GLfloat); };
template<> struct uniform_helper_<col<1>>
    { declare_uvec_func_(glUniform1fv, GLfloat); };
template<> struct uniform_helper_<float>
    { declare_uvec_func_(glUniform1fv, GLfloat); };
template<typename T> struct uniform_helper_<std::vector<T>>
    : public uniform_helper_<T> { };

/**********************************************************/

/*************** Algebraic types conversion ***************/

// algebraic_cvt_ helper class reference:
//  - elem_type: type of the algebraic type
//  - value_type: input type [ <elem_type> <elem_type> ... ]
//  - raw_type: encodeed type for OpenGL, vector usually
//  - count: count of the elem_type
//  - size: count of raw_type::value_type of each elem_type
//  - encode: encode input type to raw type

template<typename T> struct algebraic_cvt_ { };

template<size_t M_> struct algebraic_cvt_<col<M_>> {
    enum { gl_elem_type = GL_FLOAT };
    typedef col<M_> elem_type;
    typedef col<M_> value_type;
    typedef std::vector<float> raw_type;
    constexpr static size_t count(const value_type&) { return 1; }
    constexpr static size_t size() { return M_; }
    static raw_type encode(const value_type& c) {
        return raw_type(c.begin(), c.end());
    }
};

template<size_t M_, size_t N_> struct algebraic_cvt_<matrix<M_, N_>> {
    enum { gl_elem_type = GL_FLOAT };
    typedef matrix<M_, N_> elem_type;
    typedef matrix<M_, N_> value_type;
    typedef std::vector<float> raw_type;
    constexpr static size_t count(const value_type&) { return 1; }
    constexpr static size_t size() { return M_ * N_; }
    static raw_type encode(const value_type& m) {
        raw_type v;
        for(auto c : m) for(auto e : c)
            v.push_back(e);
        return v;
    }
};

template<> struct algebraic_cvt_<GLfloat> {
    enum { gl_elem_type = GL_FLOAT };
    typedef GLfloat elem_type;
    typedef GLfloat value_type;
    typedef std::vector<GLfloat> raw_type;
    constexpr static size_t count(const value_type&) { return 1; }
    constexpr static size_t size() { return 1; }
    static raw_type encode(const value_type& f) { return raw_type(1, f); }
};

template<> struct algebraic_cvt_<GLint> {
    enum { gl_elem_type = GL_FLOAT }; // HELP: I tried change it to GL_INT but it was just not working
    typedef GLint elem_type;
    typedef GLint value_type;
    typedef std::vector<GLint> raw_type;
    constexpr static size_t count(const value_type&) { return 1; }
    constexpr static size_t size() { return 1; }
    static raw_type encode(const value_type& i) { return raw_type(1, i); }
};

// tuple encoding: wow. such code. so much ellipsis
template<typename Head> size_t tuple_size_()
    { return algebraic_cvt_<Head>::size(); }

template<typename Head, typename ... Tail>
typename std::enable_if<(sizeof...(Tail) > 0), size_t>::type tuple_size_()
    { return algebraic_cvt_<Head>::size() + tuple_size_<Tail ...>(); }

template<size_t I, typename ... TupleArgs>
typename std::enable_if<I == sizeof...(TupleArgs), void>::type
tuple_encode_(std::vector<float>& v, const std::tuple<TupleArgs ...>& t) { }

template<size_t I, typename ... TupleArgs>
typename std::enable_if<I < sizeof...(TupleArgs), void>::type
tuple_encode_(std::vector<float>& v, const std::tuple<TupleArgs ...>& t) {
    std::vector<float> tmp = algebraic_cvt_<typename std::tuple_element<
        I, std::tuple<TupleArgs ...>>::type>::encode(std::get<I>(t));
    v.insert(v.end(), tmp.begin(), tmp.end());
    tuple_encode_<I + 1>(v, t);
}

// NOTE: accept floating point types onlly
template<typename ... Args>
struct algebraic_cvt_<std::tuple<Args ...>> {
    enum { gl_elem_type = GL_FLOAT };
    typedef std::tuple<Args ...> elem_type;
    typedef std::tuple<Args ...> value_type;
    typedef std::vector<float> raw_type;
    static size_t count(const value_type&) { return 1; }
    static size_t size() { return tuple_size_<Args ...>(); }
    static raw_type encode(const value_type& t) {
        raw_type v;
        tuple_encode_<0>(v, t);
        return v;
    }
};

template<typename T>
struct algebraic_cvt_<std::vector<T>> {
    enum { gl_elem_type = algebraic_cvt_<T>::gl_elem_type };
    typedef T elem_type;
    typedef std::vector<T> value_type;
    typedef typename algebraic_cvt_<T>::raw_type raw_type;
    static size_t count(const value_type& v) {
        size_t c = 0;
        for(const T& e : v) c += algebraic_cvt_<T>::count(e);
        return c;
    }
    static size_t size() { return algebraic_cvt_<T>::size(); }
    static raw_type encode(const value_type& v) {
        raw_type raw_v;
        for(const T& e : v) {
            raw_type raw_e = algebraic_cvt_<T>::encode(e);
            raw_v.insert(raw_v.end(), raw_e.begin(), raw_e.end());
        }
        return raw_v;
    }
};

/**********************************************************/

class texture {
public:
    enum component {
        rgba_8888 = 0,
        depth_32 = 1,
    };

protected:
    GLuint texture_id_;
    component comp_;

    static const GLenum tex_param_[2][3];

public:
    texture(GLuint texid) : texture_id_(texid) { }
    texture(const texture& tex) = delete;
    texture(texture&& tex) : texture_id_(tex.texture_id_)
        { tex.texture_id_ = 0; }

    texture(const image& img) {
        comp_ = rgba_8888;

        glGenTextures(1, &texture_id_);
        glBindTexture(GL_TEXTURE_2D, texture_id_);
        glTexImage2D(GL_TEXTURE_2D, 0, tex_param_[comp_][0],
                img.width(), img.height(), 0, tex_param_[comp_][1],
                tex_param_[comp_][2], img.buffer().data());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    texture(int w, int h, component comp = rgba_8888) {
        comp_ = comp;

        glGenTextures(1, &texture_id_);
        glBindTexture(GL_TEXTURE_2D, texture_id_);
        glTexImage2D(GL_TEXTURE_2D, 0, tex_param_[comp][0],
                w, h, 0, tex_param_[comp][1],
                tex_param_[comp][2], 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    texture& operator=(const texture& tex) = delete;
    texture& operator=(texture&& tex) {
        texture_id_ = tex.texture_id_; tex.texture_id_ = 0; return *this; }
    GLuint id() const { return texture_id_; }

    size_t width() const {
        GLint w;
        glBindTexture(GL_TEXTURE_2D, texture_id_);
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
        glBindTexture(GL_TEXTURE_2D, 0);
        return w;
    }

    size_t height() const {
        GLint h;
        glBindTexture(GL_TEXTURE_2D, texture_id_);
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);
        glBindTexture(GL_TEXTURE_2D, 0);
        return h;
    }

    component type() const { return comp_; }

    ~texture() {
        if(texture_id_) glDeleteTextures(0, &texture_id_);
    }
};

const GLenum texture::tex_param_[2][3] = {
    { GL_RGBA8, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, },
    { GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT, },
};

class buffer {
protected:
    GLuint buffer_id_;

public:
    buffer() { glGenBuffers(1, &buffer_id_); }
    buffer(GLuint bid) : buffer_id_(bid) { }
    buffer(const buffer& b) = delete;
    buffer(buffer&& b) : buffer_id_(b.buffer_id_) { b.buffer_id_ = 0; }
    buffer& operator=(const buffer& b) = delete;
    buffer& operator=(buffer&& b) {
        buffer_id_ = b.buffer_id_;
        b.buffer_id_ = 0;
        return *this;
    }
    ~buffer() { if(buffer_id_) glDeleteBuffers(1, &buffer_id_); }
    GLuint id() const { return buffer_id_; }

    template<typename T>
    void assign(const T& v) {
        glBindBuffer(GL_ARRAY_BUFFER, buffer_id_);
        auto raw_v = algebraic_cvt_<T>::encode(v);
        glBufferData(GL_ARRAY_BUFFER,
            raw_v.size() * sizeof(typename decltype(raw_v)::value_type),
            raw_v.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
};

typedef std::unique_ptr<buffer> buffer_ptr;

class frame_buffer {
protected:
    GLuint frame_buffer_id_;

public:
    frame_buffer() { glGenFramebuffers(1, &frame_buffer_id_); }
    frame_buffer(GLuint fbid) : frame_buffer_id_(fbid) { }
    frame_buffer(const frame_buffer& fb) = delete;
    frame_buffer(frame_buffer&& fb) : frame_buffer_id_(fb.frame_buffer_id_)
        { fb.frame_buffer_id_ = 0; }
    frame_buffer& operator=(frame_buffer&& fb) {
        frame_buffer_id_ = fb.frame_buffer_id_;
        fb.frame_buffer_id_ = 0;
        return *this;
    }
    frame_buffer& operator=(const frame_buffer& fb) = delete;
    GLuint id() const { return frame_buffer_id_; }

    const static frame_buffer screen;

    enum buf_type {
        none = 0,
        depth_buffer = 1,
        color_buffer = 2,
        all = 0xffffffff,
    };

    void bind(buf_type bt, texture& tex) {
        glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer_id_);

        if(bt == depth_buffer) {
            if(tex.type() != texture::depth_32)
                throw std::runtime_error("Texture type mismatch.");
            glFramebufferTexture(GL_FRAMEBUFFER,
                    GL_DEPTH_ATTACHMENT, tex.id(), 0);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    ~frame_buffer() {
        if(frame_buffer_id_)
            glDeleteFramebuffers(1, &frame_buffer_id_);
    }
};

const frame_buffer frame_buffer::screen = frame_buffer(0);

class vertex_array {
    friend class program;

protected:
    GLuint vertex_array_id_;

    size_t vertex_count_ = 0;

    std::map<GLuint, buffer_ptr> inputs_;

public:
    vertex_array() { glGenVertexArrays(1, &vertex_array_id_); }
    vertex_array(GLuint vaid) : vertex_array_id_(vaid) { }
    vertex_array(const vertex_array& va) = delete;
    vertex_array(vertex_array&& va) : vertex_array_id_(va.vertex_array_id_)
        { va.vertex_array_id_ = 0; }
    vertex_array& operator=(const vertex_array& va) = delete;
    vertex_array& operator=(vertex_array&& va) {
        vertex_array_id_ = va.vertex_array_id_;
        va.vertex_array_id_ = 0;
        return *this;
    }
    GLuint id() const { return vertex_array_id_; }

    template<typename T>
    void input(GLuint loc, const T& v) {
        buffer_ptr& p = inputs_[loc];
        if(!p) p = buffer_ptr(new buffer());
        p->assign(v);

        glBindVertexArray(vertex_array_id_);
        glBindBuffer(GL_ARRAY_BUFFER, p->id());
        glVertexAttribPointer(loc, algebraic_cvt_<T>::size(),
                algebraic_cvt_<T>::gl_elem_type, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(loc);
        glBindVertexArray(0);

        if(!vertex_count_) vertex_count_ = algebraic_cvt_<T>::count(v);
        else if(vertex_count_ != algebraic_cvt_<T>::count(v))
            throw std::runtime_error("Element count not match.");
    }

    ~vertex_array() {
        if(vertex_array_id_) glDeleteVertexArrays(1, &vertex_array_id_);
    }
};


class program {
protected:
    GLuint program_id_;

    std::map<std::string, buffer_ptr> uniform_blocks_;
    std::vector<std::pair<std::string, texture const *>> texture_bindings_;

public:
    program(GLuint pgid) : program_id_(pgid) { }
    program(const program& pg) : program_id_(pg.program_id_) { }
    program& operator=(program& pg) {
        program_id_ = pg.program_id_; return *this; }
    GLuint id() const { return program_id_; }

    template<typename T>
    void uniform_block(const std::string& name, const T& v) {
        GLuint loc = glGetUniformBlockIndex(program_id_, name.c_str());
        if(loc == GL_INVALID_INDEX)
            throw std::runtime_error("No such uniform block");

        buffer_ptr& p = uniform_blocks_[name];
        if(!p) p = buffer_ptr(new buffer());
        p->assign(v);

        glUseProgram(program_id_);
        glBindBuffer(GL_UNIFORM_BUFFER, p->id());
        glUniformBlockBinding(program_id_, loc, loc); // key
        glBindBufferBase(GL_UNIFORM_BUFFER, loc, p->id());
        glUseProgram(0);
    };

    template<typename T>
    void uniform(const std::string& name, const T& v) {
        GLint loc = glGetUniformLocation(program_id_, name.c_str());
        if(loc < 0) throw std::runtime_error("No such uniform");
        auto stdvec = algebraic_cvt_<T>::encode(v);

        glUseProgram(program_id_);
        uniform_helper_<T>::ufunc(loc,
            algebraic_cvt_<T>::count(v), stdvec.data());
        glUseProgram(0);
    }

    void uniform(const std::string& name, const texture& tex) {
        // store the binding pair. decide mount point on rendering
        texture_bindings_.push_back(make_pair(name, &tex));
    }

    void render(const frame_buffer& fb, const vertex_array& vao,
            frame_buffer::buf_type clear = frame_buffer::all) {
        glBindFramebuffer(GL_FRAMEBUFFER, fb.id());
        glDrawBuffer(GL_BACK);

        if(clear & frame_buffer::color_buffer)
            glClear(GL_COLOR_BUFFER_BIT);
        if(clear & frame_buffer::depth_buffer)
            glClear(GL_DEPTH_BUFFER_BIT);

        glUseProgram(program_id_);

        for(size_t i = 0; i < texture_bindings_.size(); i++) {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, texture_bindings_[i].second->id());
            GLint loc = glGetUniformLocation(program_id_,
                    texture_bindings_[i].first.c_str());
            if(loc < 0) throw std::runtime_error("No such uniform");
            glUniform1i(loc, i);
        }

        glBindVertexArray(vao.id());
        glDrawArrays(GL_TRIANGLES, 0, vao.vertex_count_);
        glBindVertexArray(0);

        glUseProgram(0);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
};

inline program link(const shader& s1, const shader& s2)
{
    GLuint pgid = glCreateProgram();

    glAttachShader(pgid, s1.id());
    glAttachShader(pgid, s2.id());

    glLinkProgram(pgid);

    GLint lnk_param;
    glGetProgramiv(pgid, GL_LINK_STATUS, &lnk_param);
    if(lnk_param != GL_TRUE) {
        glGetProgramiv(pgid, GL_INFO_LOG_LENGTH, &lnk_param);
        std::unique_ptr<char[]> log(new char[lnk_param + 1]);
        GLsizei log_length;
        glGetProgramInfoLog(pgid, lnk_param, &log_length, log.get());
        log[log_length] = '\0';
        std::string log_str(log.get());

        throw std::runtime_error(log_str);
    }

    return program(pgid);
}

}

#endif // SHADER_H_INCLUDED
