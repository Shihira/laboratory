#ifndef SECTION
#define SECTION 1
#endif

#include <iostream>
#include <fstream>
#include <cmath>

#include "include/util.h"

using namespace std;

template <typename Callable>
void bresenham_line(const point& b/*egin*/, const point& e/*nd*/, Callable cb)
{
        // difference of two points
        int Dx = e.x - b.x;
        int Dy = e.y - b.y;
        // distance of two points
        int dx = abs(Dx);
        int dy = abs(Dy);
        // unit orthogonal base
        int ex = Dx > 0 ? 1 : -1;
        int ey = Dy > 0 ? 1 : -1;

        /*
         * LINE EQUATION:
         *     Dy / Dx = (y - yf) / (x - xf)
         *           0 = Dy (x - xf) - Dx (y - yf)
         *
         * f(x, y) = Dy x - Dy xf - Dx y + Dx yf
         *
         *     f_x = f(x + 1, y) - f(x, y) = Dy
         *     f_y = f(x, y + 1) - f(x, y) = - Dx
         */

        for(int x = b.x, y = b.y, f = 0; dy >= dx ? y != e.y : x != e.x; ) {
                cb(point(x, y));
                int f_1 = f   + (dy >= dx ? ey * -Dx : ex *  Dy); // axis
                int f_2 = f_1 + (dy >= dx ? ex *  Dy : ey * -Dx); // identity

                if(abs(f_1) < abs(f_2)) {
                        x += dy >= dx ? 0 : ex;
                        y += dy >= dx ? ey : 0;
                        f = f_1;
                } else {
                        x += ex;
                        y += ey;
                        f = f_2;
                }
        }
}

#if SECTION == 1

#include <cstdlib>
#include <ctime>

#include "include/image.h"

int main()
{
        srand(time(0));
        image img(800, 600);

        // completed line algorithm
        bresenham_line(point(200, 300), point(400, 200),
                [&img] (const point& p) { img[p] = 0UL; });
        for(int i = 0; i < 50; i++) {
                bresenham_line(
                        point(rand() % 800, rand() % 600),
                        point(rand() % 800, rand() % 600),
                        [&img] (const point& p) { img[p] = color(0UL); });
        }

        ofstream f("assets/bresenham-line.ppm");
        f << img;
}

#endif
