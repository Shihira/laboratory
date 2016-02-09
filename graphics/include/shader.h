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
        geometry = GL_GEOMETRY_SHADER,
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

inline shader compile(shader::shader_type_e type, std::istream& src)
{
    return compile(type, std::string(std::istreambuf_iterator<char>(src),
                std::istreambuf_iterator<char>()));
}

inline shader compile(shader::shader_type_e type, std::istream&& src)
{
    return compile(type, src);
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
template<> struct uniform_helper_<col<2>>
    { declare_uvec_func_(glUniform2fv, GLfloat); };
template<> struct uniform_helper_<col<1>>
    { declare_uvec_func_(glUniform1fv, GLfloat); };
template<> struct uniform_helper_<float>
    { declare_uvec_func_(glUniform1fv, GLfloat); };
template<> struct uniform_helper_<GLint>
    { declare_uvec_func_(glUniform1iv, GLint); };
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
template<typename Head> constexpr size_t tuple_size_()
    { return algebraic_cvt_<Head>::size(); }

template<typename Head, typename ... Tail> constexpr
typename std::enable_if<(sizeof...(Tail) > 0), size_t>::type tuple_size_()
    { return algebraic_cvt_<Head>::size() + tuple_size_<Tail ...>(); }

template<size_t I, typename ... TupleArgs>
typename std::enable_if<I == sizeof...(TupleArgs), void>::type
tuple_encode_(std::vector<float>& v, const std::tuple<TupleArgs ...>& t) { }

template<size_t I, typename ... TupleArgs>
typename std::enable_if<(I < sizeof...(TupleArgs)), void>::type
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
    constexpr static size_t count(const value_type&) { return 1; }
    constexpr static size_t size() { return tuple_size_<Args ...>(); }
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
    constexpr static size_t size() { return algebraic_cvt_<T>::size(); }
    static raw_type encode(const value_type& v) {
        raw_type raw_v;
        for(const T& e : v) {
            raw_type raw_e = algebraic_cvt_<T>::encode(e);
            raw_v.insert(raw_v.end(), raw_e.begin(), raw_e.end());
        }
        return raw_v;
    }
};

template<> struct algebraic_cvt_<color> {
    enum { gl_elem_type = GL_UNSIGNED_BYTE };
    typedef color elem_type;
    typedef color value_type;
    typedef std::vector<uint8_t> raw_type;
    constexpr static size_t count(const value_type& c) { return 1; }
    constexpr static size_t size() { return 4; }
    static raw_type encode(const value_type& c) {
        raw_type raw_v;
        raw_v.push_back(c.r);
        raw_v.push_back(c.g);
        raw_v.push_back(c.b);
        raw_v.push_back(c.a);
        return raw_v;
    }
};

/**********************************************************/

template<size_t Size> struct texture_format_ { };
template<> struct texture_format_<1> { enum { format = GL_RED }; };
template<> struct texture_format_<2> { enum { format = GL_RG }; };
template<> struct texture_format_<3> { enum { format = GL_RGB }; };
template<> struct texture_format_<4> { enum { format = GL_RGBA }; };

class texture {
public:
    enum component {
        rgba_8888 = GL_RGBA8,
        rgb_888   = GL_RGB8,
        rg_88     = GL_RG8,
        r_8       = GL_R8,
        rgba_ffff = GL_RGBA32F,
        rgb_fff   = GL_RGB32F,
        rg_ff     = GL_RG32F,
        r_f       = GL_R32F,
        depth_f = GL_DEPTH_COMPONENT32F,
    };

protected:
    GLuint texture_id_;
    component comp_;

public:
    texture(GLuint texid) : texture_id_(texid) { }
    texture(const texture& tex) = delete;
    texture(texture&& tex) : texture_id_(tex.texture_id_)
        { tex.texture_id_ = 0; /*prevent deletion*/ }

    template<typename T>
    texture(const T& img, component comp = rgba_8888) {
        comp_ = comp;

        glGenTextures(1, &texture_id_);
        bitblt(img);
    }

    template<typename T>
    void bitblt(const T& img, size_t width) {
        if(algebraic_cvt_<T>::count(img) % width)
            throw std::runtime_error("Image is not a rectangle.");
        size_t height = algebraic_cvt_<T>::count(img) / width;
        auto data = algebraic_cvt_<T>::encode(img);
        glBindTexture(GL_TEXTURE_2D, texture_id_);
        glTexImage2D(GL_TEXTURE_2D, 0, GLenum(comp_), width, height, 0,
                texture_format_<algebraic_cvt_<T>::size()>::format,
                algebraic_cvt_<T>::gl_elem_type, data.data());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void bitblt(const image& img) {
        bitblt(img.buffer(), img.width());
    }

    texture(int w, int h, component comp = rgba_8888) {
        comp_ = comp;

        glGenTextures(1, &texture_id_);
        glBindTexture(GL_TEXTURE_2D, texture_id_);
        glTexImage2D(GL_TEXTURE_2D, 0, comp_, w, h, 0,
                // Since there is no data transfer, besides depth component
                // restriction, it is free to specify any format and type. I
                comp == depth_f ? GL_DEPTH_COMPONENT : GL_RGBA, // use GL_RGBA
                GL_UNSIGNED_BYTE, 0);
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
        color_buffer_0 = 1,
        color_buffer_1 = 2,
        color_buffer_2 = 4,
        depth_buffer = 1U << 31,
        all = 0xffffffff,
    };

    void bind(buf_type bt, texture& tex) {
        glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer_id_);

        if(bt == depth_buffer) {
            if(tex.type() != texture::depth_f)
                throw std::runtime_error("Texture type mismatch.");
            glFramebufferTexture(GL_FRAMEBUFFER,
                    GL_DEPTH_ATTACHMENT, tex.id(), 0);
        }
        if(bt <= 1 << 30 /*color_buffer_n*/) {
            /*if(tex.type() != texture::rgba_8888)
                throw std::runtime_error("Texture type mismatch.");*/
            glFramebufferTexture(GL_FRAMEBUFFER,
                    GL_COLOR_ATTACHMENT0 + int(log2(bt)), tex.id(), 0);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    ~frame_buffer() {
        if(frame_buffer_id_)
            glDeleteFramebuffers(1, &frame_buffer_id_);
    }

    void clear(color c = 0) {
        glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer_id_);

        glClearColor(c.r / 255., c.g / 255., c.b / 255., c.a / 255.);
        glClear(GL_COLOR_BUFFER_BIT);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
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
    std::map<std::string, texture const *> texture_bindings_;

public:
    program(GLuint pgid) : program_id_(pgid) { }
    program(const program& pg) : program_id_(pg.program_id_) { }
    program& operator=(program& pg) {
        program_id_ = pg.program_id_; return *this; }
    GLuint id() const { return program_id_; }

    template<typename T>
    void uniform_block(const std::string& name, const T& v) {
        GLuint loc = glGetUniformBlockIndex(program_id_, name.c_str());
        if(loc == GL_INVALID_INDEX) {
            std::cerr << "No such uniform block: " << name << std::endl;
            return;
        }

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
        if(loc < 0) {
            std::cerr << "No such uniform: " << name << std::endl;
            return;
        }
        auto stdvec = algebraic_cvt_<T>::encode(v);

        glUseProgram(program_id_);
        uniform_helper_<T>::ufunc(loc,
            algebraic_cvt_<T>::count(v), stdvec.data());
        glUseProgram(0);
    }

    void uniform(const std::string& name, const texture& tex) {
        // store the binding pair. decide mount point on rendering
        texture_bindings_[name] = &tex;
    }

    void render(const frame_buffer& fb, const vertex_array& vao,
            frame_buffer::buf_type clear = frame_buffer::all) {
        glBindFramebuffer(GL_FRAMEBUFFER, fb.id());
        glDrawBuffer(GL_BACK);

        if(clear & ~frame_buffer::depth_buffer) // buffers other than depth
            glClear(GL_COLOR_BUFFER_BIT);
        if(clear & frame_buffer::depth_buffer)
            glClear(GL_DEPTH_BUFFER_BIT);

        glUseProgram(program_id_);

        size_t i = 0;
        for(auto p : texture_bindings_) {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, p.second->id());
            GLint loc = glGetUniformLocation(program_id_, p.first.c_str());
            if(loc < 0) {
                std::cerr << "No such uniform: " << p.first << std::endl;
                continue;
            }
            glUniform1i(loc, i);

            i++;
        }

        glBindVertexArray(vao.id());
        glDrawArrays(GL_TRIANGLES, 0, vao.vertex_count_);
        glBindVertexArray(0);

        glUseProgram(0);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
};

void link_to_program_(GLuint pgid, const shader& s)
{ glAttachShader(pgid, s.id()); }

template<typename ... ShaderType> inline
typename std::enable_if<(sizeof...(ShaderType) > 0), void>::type
link_to_program_(GLuint pgid, const shader& s, ShaderType ... other)
{ link_to_program_(pgid, s); link_to_program_(pgid, other ...); }

template<typename ... ShaderType>
inline program link(ShaderType ... ss)
{
    GLuint pgid = glCreateProgram();

    link_to_program_(pgid, ss ...);

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
