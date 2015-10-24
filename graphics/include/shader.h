#ifndef SHADER_H_INCLUDED
#define SHADER_H_INCLUDED

#include <GL/glew.h>
#include <string>
#include <memory>

#include "matrix.h"

namespace gl_type_helper {

////////////////////////////////////////////////////////////////////////////////
// OpenGL type abstraction

// Uniform part --------
// We use to_uniform_ and uniform_assign_ two part to induce different algebra
// strcture to a small amount of function and induce (or overload) different
// glUniform**** to one interface.

template<size_t M_, size_t N_>
inline std::unique_ptr<float[]> to_uniform_(const matrix<M_, N_>& u)
{
    std::unique_ptr<float[]> unif_arr(new float[M_ * N_]);
    // columns
    for(size_t j = 0; j < N_; j++)
    for(size_t i = 0; i < M_; i++)
        unif_arr[j * N_ + i] = u(i, j);

    return unif_arr;
}

template <typename T, typename Ptr = int*>
inline void uniform_assign_(GLuint loc, Ptr data)
    { throw std::runtime_error("NotImpl"); }

template <>
inline void uniform_assign_<matrix<4, 4>>(
        GLuint loc, const float* data) {
    glUniformMatrix4fv(loc, 16, false, data);
}

template <>
inline void uniform_assign_<matrix<3, 3>>(
        GLuint loc, const float* data) {
    glUniformMatrix3fv(loc, 9, false, data);
}

// Attribute part --------

template <typename T>
struct gl_type_trait { };
template <size_t M_>
struct gl_type_trait<col<M_>> {
    enum {
        channel = M_,
        typeind = GL_FLOAT,
    };

    typedef const float* const_nptr;
    typedef float* nptr;
    typedef std::unique_ptr<float[]> sptr;
};

template <size_t M_>
inline typename gl_type_trait<col<M_>>::sptr to_attribute(const col<M_>& c)
{
    typename gl_type_trait<col<M_>>::sptr attr_arr(new float[M_]);
    for(size_t i = 0; i < M_; i++)
        attr_arr[i] = c[i];
}

template <typename T>
inline void attribute_assign_(GLuint loc,
        typename gl_type_trait<T>::const_nptr* data)
{
    glVertexAttribPointer(loc,
        gl_type_trait<T>::channel,
        gl_type_trait<T>::typeind,
        GL_FALSE, 0, data);
}

}

////////////////////////////////////////////////////////////////////////////////

class program;

class shader {
    friend class program;

protected:
    GLuint shader_id_;

    shader(GLenum sdtype)
        : shader_id_(glCreateShader(sdtype)) { }

public:
    shader(const shader& s) : shader_id_(s.shader_id_) { }

    void source(const std::string& cd) {
        const char* source = cd.data();
        GLsizei source_len = cd.length();

        glShaderSource(shader_id_, 1,
                &source, &source_len);
    }

    GLuint id() const { return shader_id_; }
};

////////////////////////////////////////////////////////////////////////////////

class program {
protected:
    GLuint program_id_;

public:

    program() {
        program_id_ = glCreateProgram();
    }

    GLuint id() const { return program_id_; }

    template <typename T>
    void uniform(const std::string& name, const T& u) {
        using namespace gl_type_helper;
        auto data = to_uniform_(u);
        uniform_assign_<T>(glGetUniformLocation(
            id(), name.c_str()), &*data);
    }

    template <typename T>
    void attribute(const std::string& name, const T& u) {
        using namespace gl_type_helper;
        auto data = to_attribute(u);
        attribute_assign_<T>(glGetAttribLocation(
            id(), name.c_str()), &*data);
    }
};

#endif // SHADER_H_INCLUDED
