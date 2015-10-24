// cflags: -lSDL2 -lGL -lGLEW

#include <iostream>
#include <memory>
#include <GL/glew.h>

#include "../include/matrix.h"
#include "../include/gui.h"

using namespace std;

const char* vs_src = R"EOF(
#version 330 core

layout(location = 0) in vec4 position;
layout(location = 1) in vec3 normal;

out vec4 color;

uniform mat4 mmat;
uniform mat4 pmat;

void main(void)
{
    mat4 nmmat = transpose(inverse(mmat));
    vec3 nml = (nmmat * vec4(normal, 0)).xyz;

    vec4 light = vec4(2, 3, 4, 1);
    float diffuse = clamp(
        dot(-nml, normalize((position - light).xyz)),
        0, 1);

    color = vec4(diffuse, diffuse, diffuse, 1);
    gl_Position = pmat * mmat * position;
}
)EOF";

const char* fs_src = R"EOF(
#version 330 core

in vec4 color;

void main(void)
{
    gl_FragColor = color;
}
)EOF";

/*
 *      A y
 *      |
 *    5----1
 *   /|   /|
 *  4-+--0 |
 *  | 7--|-3
 *  |/   |/---> x
 *  6----2
 *   /
 *  L  z
 *
 */

const GLfloat vertices[] = {
     1,  1,  1, 1,
     1,  1, -1, 1,
     1, -1,  1, 1,
     1, -1, -1, 1,
    -1,  1,  1, 1,
    -1,  1, -1, 1,
    -1, -1,  1, 1,
    -1, -1, -1, 1,
};

const GLfloat normals[] = {
    1, 0, 0,
    -1, 0, 0,
    0, 1, 0,
    0, -1, 0,
    0, 0, 1,
    0, 0, -1,
};

// 0 1 3 2
// 4 5 7 6
const GLuint vindices[] = {
    2, 3, 1,    1, 0, 2,
    4, 5, 7,    7, 6, 4,
    1, 5, 4,    4, 0, 1,
    2, 6, 7,    7, 3, 2,
    0, 4, 6,    6, 2, 0,
    3, 7, 5,    5, 1, 3,
};

const GLuint nindices[] = {
    0, 0, 0,    0, 0, 0,
    1, 1, 1,    1, 1, 1,
    2, 2, 2,    2, 2, 2,
    3, 3, 3,    3, 3, 3,
    4, 4, 4,    4, 4, 4,
    5, 5, 5,    5, 5, 5,
};

template<size_t M_, size_t N_>
unique_ptr<float[]> mat2arr(const matrix<M_, N_>& m)
{
    unique_ptr<float[]> arr(new float[M_ * N_]);
    for(size_t j = 0; j < N_; j++)
        for(size_t i = 0; i < M_; i++)
            arr[j * N_ + i] = m[j][i];
    return arr;
}

unique_ptr<float[]> extend(const GLfloat* vectors, size_t vec_size,
        const GLuint* indices, size_t ind_count)
{
    unique_ptr<float[]> arr(new float[ind_count * vec_size]);
    for(size_t i = 0; i < ind_count; i++) {
        size_t ind = indices[i];
        for(size_t off = 0; off < vec_size; off++)
            arr[i * vec_size + off] = vectors[ind * vec_size + off];
    }

    return arr;
}

int main()
{
    windowgl w("Hello");

    glewExperimental = true;
    if(glewInit() != GLEW_OK)
        throw std::runtime_error("GLEWInitFailed");

    glClearColor(0.2, 0.2, 0.2, 1);
    glClearDepth(0);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_GEQUAL);

    ////////////////////////////////////////////////////////////////////////////
    // Program Part

    GLuint prog = glCreateProgram();
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);

    char log[256]; int log_len;

    glShaderSource(vs, 1, &vs_src, GL_NONE);
    glShaderSource(fs, 1, &fs_src, GL_NONE);
    glCompileShader(vs);
    glCompileShader(fs);

    glGetShaderInfoLog(vs, sizeof(log), &log_len, log);
    log[size_t(log_len) >= sizeof(log) ? 255 : log_len] = 0;
    cout << log << endl;
    glGetShaderInfoLog(fs, sizeof(log), &log_len, log);
    log[size_t(log_len) >= sizeof(log) ? 255 : log_len] = 0;
    cout << log << endl;

    glAttachShader(prog, vs);
    glAttachShader(prog, fs);

    glLinkProgram(prog);

    glGetProgramInfoLog(prog, sizeof(log), &log_len, log);
    log[size_t(log_len) >= sizeof(log) ? 255 : log_len] = 0;
    cout << log << endl;

    glUseProgram(prog);

    ////////////////////////////////////////////////////////////////////////////
    // Buffer Part
    GLuint vao; glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // vertex
    GLuint pos_vbo; glGenBuffers(1, &pos_vbo);
    auto indexed_pos = extend(vertices, 4, vindices,
        sizeof(vindices) / sizeof(GLuint));
    glBindBuffer(GL_ARRAY_BUFFER, pos_vbo);
    glBufferData(GL_ARRAY_BUFFER,
        sizeof(float) * 4 * sizeof(vindices) / sizeof(GLuint),
        indexed_pos.get(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    // normal
    GLuint nml_vbo; glGenBuffers(1, &nml_vbo);
    auto indexed_nml = extend(normals, 3, nindices,
        sizeof(nindices) / sizeof(GLuint));
    glBindBuffer(GL_ARRAY_BUFFER, nml_vbo);
    glBufferData(GL_ARRAY_BUFFER,
        sizeof(float) * 3 * sizeof(nindices) / sizeof(GLuint),
        indexed_nml.get(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // transformation
    GLuint pmat_loc = glGetUniformLocation(prog, "pmat");
    GLuint mmat_loc = glGetUniformLocation(prog, "mmat");
    matrix<4, 4> pmat = transform::perspective(M_PI / 4, 4. / 3, -1, 1);
    matrix<4, 4> mmat = transform::identity();
    mmat *= transform::translate(col<4>{ 0, 0, -5, 1 });
    mmat *= transform::rotate(-M_PI / 6, transform::yOz);

    app().register_on_paint([&]() {
        mmat *= rotate(M_PI / 360, transform::zOx);
        auto mmat_arr = mat2arr(mmat);
        auto pmat_arr = mat2arr(pmat);
        glUniformMatrix4fv(mmat_loc, 1, false, mmat_arr.get());
        glUniformMatrix4fv(pmat_loc, 1, false, pmat_arr.get());

        glClear(GL_COLOR_BUFFER_BIT);
        glClear(GL_DEPTH_BUFFER_BIT);
        glBindVertexArray(vao);
        //glDrawElements(GL_TRIANGLES, sizeof(vindices) / sizeof(GLuint),
        //        GL_UNSIGNED_INT, NULL);
        glDrawArrays(GL_TRIANGLES, 0, sizeof(vindices) / sizeof(GLuint));

        w.swap_buffer();
    });

    app().run();

    return 0;
}
