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

#define declare_umat_func_(glfunc) \
    static void ufunc (GLint loc, GLsizei c, GLboolean t, const GLfloat* v) \
    { glfunc(loc, c, t, v); }
#define declare_uvec_func_(glfunc) \
    static void ufunc (GLint loc, GLsizei c, const GLfloat* v) \
    { glfunc(loc, c, v); }

template<> struct uniform_helper_<matrix<4, 4>>
    { declare_umat_func_(glUniformMatrix4fv); };
template<> struct uniform_helper_<col<4>>
    { declare_uvec_func_(glUniform4fv); };

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
    texture(const texture& tex) :
        texture_id_(tex.texture_id_) { }

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

    texture& operator=(texture& tex) {
        texture_id_ = tex.texture_id_; return *this; }
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
};

const GLenum texture::tex_param_[2][3] = {
    { GL_RGBA8, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, },
    { GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT, },
};

class frame_buffer {
    GLuint frame_buffer_id_;

public:
    frame_buffer() { glGenFramebuffers(1, &frame_buffer_id_); }
    frame_buffer(GLuint fbid) : frame_buffer_id_(fbid) { }
    frame_buffer(const frame_buffer& fb)
        : frame_buffer_id_(fb.frame_buffer_id_) { }
    frame_buffer& operator=(frame_buffer& fb) {
        frame_buffer_id_ = fb.frame_buffer_id_; return *this; }
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
};

const frame_buffer frame_buffer::screen = frame_buffer(0);

class vertex_array {
    friend class program;

protected:
    GLuint vertex_array_id_;

    size_t vertex_count_ = 0;

public:
    vertex_array() { glGenVertexArrays(1, &vertex_array_id_); }
    vertex_array(GLuint vaid) : vertex_array_id_(vaid) { }
    vertex_array(const vertex_array& va) :
        vertex_array_id_(va.vertex_array_id_) { }
    vertex_array& operator=(vertex_array& va) {
        vertex_array_id_ = va.vertex_array_id_; return *this; }
    GLuint id() const { return vertex_array_id_; }

    void input(GLuint loc, size_t group, const std::vector<float> v) {
        glBindVertexArray(vertex_array_id_);
        
        GLuint vbo; glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, v.size() * sizeof(float),
                v.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(loc, group, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(loc);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glBindVertexArray(0);

        if(!vertex_count_) vertex_count_ = v.size() / group;
        if(vertex_count_ != v.size() / group)
            throw std::runtime_error("Element count not match.");
    }

    template<size_t M_>
    void input(GLuint loc, const std::vector<col<M_>>& v) {
        std::vector<float> buf;
        for(const col<M_>& c : v)
            for(auto elem : c)
                buf.push_back(elem);
        input(loc, M_, buf);
    }
};


class program {
protected:
    GLuint program_id_;

public:
    program(GLuint pgid) : program_id_(pgid) { }
    program(const program& pg) : program_id_(pg.program_id_) { }
    program& operator=(program& pg) {
        program_id_ = pg.program_id_; return *this; }
    GLuint id() const { return program_id_; }

    std::vector<std::pair<std::string, texture const *>> texture_bindings;

    template<size_t M_, size_t N_>
    void uniform(const std::string& name, const matrix<M_, N_>& m) {
        GLuint loc = glGetUniformLocation(program_id_, name.c_str());

        std::vector<float> buf;
        for(size_t j = 0; j < N_; j++)
            for(size_t i = 0; i < M_; i++)
                buf.push_back(m(i, j));

        glUseProgram(program_id_);
        uniform_helper_<matrix<M_, N_>>::ufunc(loc, 1, false, buf.data());
        glUseProgram(0);
    }

    template<size_t M_>
    void uniform(const std::string& name, const col<M_>& v) {
        GLuint loc = glGetUniformLocation(program_id_, name.c_str());

        std::vector<float> buf;
        for(auto elem : v) buf.push_back(elem);

        glUseProgram(program_id_);
        uniform_helper_<col<M_>>::ufunc(loc, 1, buf.data());
        glUseProgram(0);
    }

    void uniform(const std::string& name, const texture& tex) {
        // store the binding pair. decide mount point on rendering
        texture_bindings.push_back(make_pair(name, &tex));
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

        for(size_t i = 0; i < texture_bindings.size(); i++) {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, texture_bindings[i].second->id());
            GLuint loc = glGetUniformLocation(program_id_,
                    texture_bindings[i].first.c_str());
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
