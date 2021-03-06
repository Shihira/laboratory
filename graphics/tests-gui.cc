// cflags: -lSDL2 -lGL -lGLU

#include "include/gui.h"
#include "include/model.h"

#include "GL/gl.h"
#include "GL/glu.h"

using namespace std;

int main(int argc, char** argv)
{
    if(argc < 2) {
        cout <<
            "USAGE:\n"
            "    tests_gui <wavefront_obj.obj> [<model_index>]\n";
        return -1;
    }

    windowgl w("Hello");

    size_t model_index = 0;
    if(argc > 2)
        model_index = atoi(argv[2]);
    ifstream fobj(argv[1]);
    wavefront_model m;
    for(size_t i = 0; i < model_index; i++)
        m.read(fobj, true);
    m.read(fobj);

    GLuint dplist = glGenLists(1);
    glNewList(dplist, GL_COMPILE);

    for(wavefront_model::face& f : m.get_meshes()) {
        glBegin(GL_POLYGON);
        for(wavefront_model::face_elem& fe : f) {
            size_t v_i, n_i, t_i;
            std::tie(v_i, n_i, t_i) = fe;

            wavefront_model::normal& n = m.get_normals()[n_i];
            wavefront_model::vertex& v = m.get_vertices()[v_i];

            glNormal3d(n[0], n[1], n[2]);
            glVertex4d(v[0], v[1], v[2], v[3]);
        }
        glEnd();
    }
    glEndList();

    double avr_x = m.statistic(wavefront_model::avr_x);
    double avr_y = m.statistic(wavefront_model::avr_y);
    double avr_z = m.statistic(wavefront_model::avr_z);

    glShadeModel(GL_SMOOTH);
    glEnable(GL_LIGHTING);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_LIGHT0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);
    glEnable(GL_RESCALE_NORMAL);

    float light_position[] = { -300, 0, -300, 1 };
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    float light_ambient[] = { 0.2, 0.2, 0.2, 1 };
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);

    unsigned frame_count = 0;

    app().register_on_paint([&]() {
        glClearColor(0.2, 0.2, 0.2, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        glClear(GL_DEPTH_BUFFER_BIT);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(60, 1.33, 1, 5000);
        gluLookAt(0, 0, -3, 0, 0, 0, 0, 1, 0);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        glColor3d(1, 1, 1);
        glRotated(-30, 1, 0, 0);
        glRotated(frame_count, 0, 1, 0);
        glTranslated(-avr_x, -avr_y, -avr_z);
        glCallList(dplist);

        w.swap_buffer();

        frame_count++;
    });

    app().run();
}
