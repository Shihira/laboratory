#ifndef UTIL_H_DEFINED
#define UTIL_H_DEFINED

#include <utility>
#include <vector>
#include <tuple>
#include <cmath>
#include <iostream>
#include <sstream>
#include <algorithm>
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
    point operator+(const point& other) const
        { return { x + other.x, y + other.y }; }
    point operator-(const point& other) const
        { return { x - other.x, y - other.y }; }
    point operator+=(const point& other)
        { x += other.x; y += other.y; return *this; } 

    double operator|(const point& other) const { // NORM
        double dx = x - other.x;
        double dy = y - other.y;

        return std::sqrt(dx * dx + dy * dy);
    }

    point operator*(double coefficient) const
        { return { coefficient * x, coefficient * y }; }
};

inline point operator*(double coefficient, const point& p)
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

#define EST(code, repeat) \
        { \
                long est_t = clock(); \
                for(int lp = 0; lp < repeat; lp++) { code; } \
                printf("ln.%d used: %ld us\n", __LINE__, clock() - est_t); \
        }
#define EST_10K(code) EST(code, 10000)

#endif
