#ifndef SECTION
#define SECTION 2
#endif

#include <vector>
#include <map>
#include <set>
#include <cmath>
#include <ctime>
#include <iostream>
#include <fstream>
#include <functional>

using namespace std;

#include "include/image.h"
#include "include/util.h"

#include "bresenham.cc"

template <typename Callable>
void scanline_fill(const point_list& vertex, Callable cb)
{
        map<int, map<int, int>> edges;

        for(size_t i = 0; i < vertex.size(); i++) {
                point g/*roup*/[3] = {
                        vertex[i],
                        vertex[(i + 1) % vertex.size()],
                        vertex[(i + 2) % vertex.size()],
                };

                if(g[1].y == g[2].y) continue;
                if(g[1].y >  g[2].y) {
                        swap(g[1], g[2]);
                        g[0] = vertex[(i + 3) % vertex.size()];
                }

                //// MARK OUT EDGES: from top to bottom
                // use map to restrict one mark at most per line
                set<int> ln_used;
                bresenham_line(g[1], g[2], [&] (const point& p) {
                        if(p.y == g[2].y) return;
                        if(ln_used.find(p.y) != ln_used.end()) return;
                        else ln_used.insert(p.y);

                        auto& line = edges[p.y];
                        if(line.find(p.x) == line.end())
                                line[p.x] = 1;
                        else line[p.x] ++;
                });

                // add another edge in acute angle
                if((g[0].y - g[1].y) * (g[1].y - g[2].y) < 0)
                        edges[g[1].y][g[1].x] ++;
        }

        for(auto e : edges) {
                int state = 0;
                map<int, int> cur_ln = e.second;

                if(cur_ln.empty()) continue;

                int min = cur_ln.begin()->first,
                    max = cur_ln.rbegin()->first;

                for(int j = min; j <= max; j++) {
                        if(state % 2) cb(point(j, e.first));
                        if(cur_ln.find(j) != cur_ln.end()) {
                                state += cur_ln[j];
                        }
                }
        }
}

#if SECTION == 2

int main()
{
        srand(time(0));

        point_list vertex;
        for(int i = 0; i < 50; i++) {
                point newp = { rand() % 800, rand() % 600 };
                vertex.push_back(newp);
        }

        image img(800, 600);
        scanline_fill(vertex, [&img] (const point& p) { img[p] = 0UL; });

        ofstream file("assets/scanline-fill.ppm");
        file << img;
}

#endif
