/*
 * Copyright (C) Shihira Fung <fengzhiping@hotmail.com>
 *
 * Linear algebra library header. By default matrices are an array of column
 * vectors. Because the name `vector` has been used by STL and I do not want to
 * wrap my classes with a namespace so I substitute `vector` with `col`.
 *
 */

#ifndef MATRIX_INC_H
#define MATRIX_INC_H

#include <array>
#include <cmath>
#include <algorithm>
#include <iostream>

template <size_t M_, size_t N_> class matrix;

template <size_t M_>
class col {
private:
        std::array<double, M_> data_;

public:
        double& operator[] (size_t pos) { return data_[pos]; }
        double operator[] (size_t pos) const { return data_[pos]; }
        inline operator matrix<M_, 1> () const;

        col () { for(double& e : data_) e = 0.; }
        col (const matrix<M_, 1>& m) { (*this) = m; }
        col (const col<M_>& other) { *this = other; }
        col (std::initializer_list<double> l)
                { std::copy(l.begin(), l.end(), data_.begin()); }

        inline col<M_>& operator= (const matrix<M_, 1>& m);
        col<M_>& operator= (const col<M_>& other)
                { data_ = other.data_; return *this; }

        auto begin() -> decltype(data_.begin()) { return data_.begin(); }
        auto end() -> decltype(data_.end()) { return data_.end(); }

        matrix<M_, 1> to_mat() const { return matrix<M_, 1>(*this); }
        double abs() const {
                return sqrt((*this) * (*this));
        }

        template<size_t N_>
        col<N_> to_vec() const {
            col<N_> c;
            for(size_t i = 0; i < N_; i++) {
                if(i < M_) c[i] = (*this)[i];
                else c[i] = 0;
            }
            return c;
        }

        col<M_-1> reduce() const {
                col<M_-1> result;
                for(size_t i = 0; i < M_ - 1; i++)
                        result[i] = (*this)[i] / (*this)[M_ - 1];
                return result;
        }

        col<M_+1> homo() const {
                col<M_+1> result;
                for(size_t i = 0; i < M_; i++)
                        result[i] = (*this)[i];
                result[M_] = 1.0;
                return result;
        }

        double operator* (const col<M_>& other) const {
                double result = 0.;
                for(size_t i = 0; i < M_; i++)
                        result += (*this)[i] * other[i];
                return result;
        }

        col<M_> operator* (double coef) const {
                col<M_> result;
                for(size_t i = 0; i < M_; i++)
                        result[i] += (*this)[i] * coef;
                return result;
        }

        col<M_> operator+ (const col<M_>& other) const {
                col<M_> result;
                for(size_t i = 0; i < M_; i++)
                        result[i] = (*this)[i] + other[i];
                return result;
        }

        col<M_> operator- (const col<M_>& other) const {
                col<M_> result;
                for(size_t i = 0; i < M_; i++)
                        result[i] = (*this)[i] - other[i];
                return result;
        }

        col<M_> operator- () const {
                col<M_> result;
                for(size_t i = 0; i < M_; i++)
                        result[i] = -(*this)[i];
                return result;
        }

        col<M_> operator+ () const {
                col<M_> result;
                for(size_t i = 0; i < M_; i++)
                        result[i] = -(*this)[i];
                return result;
        }
};

template<size_t M_>
std::ostream& operator<< (std::ostream& os, const col<M_>& c)
{
    os << '[';
    for(size_t i = 0; i < M_; i++)
        os << c[i] << (i == M_ - 1 ? "" : ", ");
    os << ']';

    return os;
}

template <size_t M_>
inline double det(const matrix<M_, M_>& mat);

template <size_t M_, size_t N_>
class matrix {
        friend class col<M_>;
protected:
        std::array<col<M_>, N_> cols_;

public:
        col<M_>& operator[] (size_t n) { return cols_[n]; }
        const col<M_> operator[] (size_t n) const { return cols_[n]; }
        double& operator() (size_t m, size_t n) { return cols_[n][m]; }
        double operator() (size_t m, size_t n) const { return cols_[n][m]; }

        matrix<M_, N_>& operator= (const matrix<M_, N_>& other)
                { cols_ = other.cols_; return (*this); }

        template <size_t L_>
        matrix<M_, L_> operator* (const matrix<N_, L_>& other) const {
                matrix<M_, L_> new_mat;
                for(size_t i = 0; i < M_; ++i)
                for(size_t j = 0; j < L_; ++j)
                for(size_t k = 0; k < N_; ++k)
                        new_mat(i, j) += (*this)(i, k) * other(k, j);

                return new_mat;
        }

        template <size_t L_>
        matrix<M_, L_> operator*= (const matrix<N_, L_>& other) {
            (*this) = operator* (other);
            return *this;
        }

        matrix() { }
        matrix(std::initializer_list<double> l) {
                size_t cur_n = 0;
                size_t cur_m = 0;
                for(double elem : l) {
                        if(cur_m >= M_) break;
                        if(cur_n >= N_) { ++cur_m; cur_n = 0; }

                        (*this)(cur_m, cur_n) = elem;
                        ++cur_n;
                }
        }
        matrix(std::initializer_list<col<M_>> l) {
                std::copy(l.begin(), l.end(), cols_.begin());
        }

        auto begin() -> decltype(cols_.begin()) { return cols_.begin(); }
        auto end() -> decltype(cols_.end()) { return cols_.end(); }

        double cofactor(size_t m, size_t n) const {
                matrix<M_-1, N_-1> new_mat;
                for(size_t i = 0; i < N_; i++) {
                        if(i == n) continue;
                        size_t new_i = i > n ? i - 1 : i;
                        for(size_t j = 0; j < M_; j++) {
                                if(j == m) continue;
                                size_t new_j = j > m ? j - 1 : j;
                                new_mat[new_i][new_j] = (*this)[i][j];
                        }
                }

                return det(new_mat);
        }

        matrix inverse() const {
            double D = det(*this);

            matrix new_mat;
            for(size_t i = 0; i < M_; ++i)
            for(size_t j = 0; j < N_; ++j)
                new_mat(j, i) = ((i + j) % 2 ? -1 : 1) * cofactor(i, j) / D;

            return new_mat;
        }

        matrix<N_, M_> t() const {
            matrix<N_, M_> new_mat;
            for(size_t i = 0; i < M_; ++i)
            for(size_t j = 0; j < N_; ++j)
                new_mat(j, i) = (*this)(i, j);

            return new_mat;
        }

};

template <size_t M_>
double det(const matrix<M_, M_>& mat) {
        double result = 0;
        for(size_t i = 0; i < M_; i++)
                result += (i % 2 ? -1 : 1) *
                        mat(i, 0) * mat.cofactor(i, 0);
        return result;
}
template <>
double det(const matrix<1, 1>& mat) {
        return mat(0, 0);
}

template <size_t M_>
col<M_>::operator matrix<M_, 1> () const {
        matrix<M_, 1> new_mat;
        new_mat.cols_[0] = *this;
        return new_mat;
}

col<3> operator%(const col<3>& l, const col<3>& r) {
        matrix<3, 3> mat = { col<3>{1, 1, 1}, l, r };
        return col<3>{
                mat.cofactor(0, 0),
                - mat.cofactor(1, 0),
                mat.cofactor(2, 0),
        };
}

template <size_t M_>
col<M_>& col<M_>::operator= (const matrix<M_, 1>& m)
        { data_ = m[0].data_; return *this; }

template <size_t M_, size_t N_>
std::ostream& operator<< (std::ostream& s, const matrix<M_, N_>& mat) {
        for(size_t i = 0; i < M_; i++) {
                if(i) s << "  "; else s << "[ ";
                for(size_t j = 0; j < N_; j++) {
                        s << mat(i, j) << (
                                (j != N_ - 1) ? ", " :
                                (i != M_ - 1) ? "; " : " ]"
                        );
                }
                s << std::endl;
        }

        return s;
}

namespace transform {

typedef enum { xOy, yOz, zOx } plane;

inline matrix<4, 4> diagonal(col<4> diag)
{
    return matrix<4, 4>{
        diag[0], 0, 0, 0,
        0, diag[1], 0, 0,
        0, 0, diag[2], 0,
        0, 0, 0, diag[3],
    };
}

inline matrix<4, 4> rotate(double a, plane p)
{
    if(p == xOy)
        return matrix<4, 4>{
            cos(a),-sin(a), 0, 0,
            sin(a), cos(a), 0, 0,
            0,      0,      1, 0,
            0,      0,      0, 1,
        };
    else if(p == yOz)
        return matrix<4, 4>{
            1, 0,      0,      0,
            0, cos(a), sin(a), 0,
            0,-sin(a), cos(a), 0,
            0, 0,      0,      1,
        };
    else
        return matrix<4, 4>{
            cos(a), 0,-sin(a), 0,
            0,      1, 0,      0,
            sin(a), 0, cos(a), 0,
            0,      0, 0,      1,
        };
}

inline matrix<4, 4> translate(col<4> t)
{
    return matrix<4, 4>{
        t[3], 0,    0,    t[0],
        0,    t[3], 0,    t[1],
        0,    0,    t[3], t[2],
        0,    0,    0,    t[3],
    };
}

inline matrix<4, 4> scale(double x, double y, double z)
    { return diagonal({x, y, z, 1}); }

inline matrix<4, 4> identity()
    { return diagonal({1, 1, 1, 1}); }

/*
inline matrix<4, 4> perspective(double z_retina)
{

}
*/

}

#endif
