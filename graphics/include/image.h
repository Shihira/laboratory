/*
 * Copyright (C) Shihira Fung <fengzhiping@hotmail.com>
 *
 * This is a Lab for computer graphics learning based on Netbpm format. Nearly
 * all algorithms will output a Netbpm result. The functionalities provided here
 * aim only to hide file processing details.
 *
 */

#ifndef IMAGE_H_INC
#define IMAGE_H_INC

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <stdexcept>
#include <cstdint>
#include <limits>

#include "util.h"

////////////////////////////////////////////////////////////////////////////////
//
// `color` provides 4 components, but only 3 are used while processing files.
// The alpha component, standing for opacity usually, is provided for raw image
// and devide-independent processing
//
// For convenience color is defined as a structure instead of a class, and hence
// you can give a direct assignment with:
//
//     color c = {128, 128, 128}; // watch out overflow warning!
//     c = color(64, 64, 64); // recommended
//     c = 0x77ffffff; // and 4-byte unsigned integer is allowed for sure

struct color {
        uint8_t b;
        uint8_t g;
        uint8_t r;
        uint8_t a;

        color() { operator=(0xffffffff); }
        color(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = 255)
                : b(blue), g(green), r(red), a(alpha) { }
        color(uint32_t pxl) { operator=(pxl); }

        color& operator=(uint32_t pxl) {
            *reinterpret_cast<uint32_t*>(this) = pxl;
            return *this;
        }

        uint32_t value() const {
            return *reinterpret_cast<uint32_t const *>(this);
        }

        color operator+(const color& c) const {
            // our opacity, their opacity and complementary their opacity
            return blend(c, c.a / 255.0);
        }

        color blend(const color& c, double topc) const {
            // Blending is low performance
            return color(
                    r + (c.r - r) * topc,
                    g + (c.g - g) * topc,
                    b + (c.b - b) * topc,
                    a + (c.a - a) * topc
                );
        }

        static color from_arithematic(uint32_t red, uint32_t green,
                uint32_t blue, uint32_t alpha = 255) {

            return color(
                    red > 255 ? 255 : red,
                    green > 255 ? 255 : green,
                    blue > 255 ? 255 : blue,
                    alpha > 255 ? 255 : alpha
                );
        }
};

////////////////////////////////////////////////////////////////////////////////
//
// `image` provides a pixel (aka. color) buffer to store an Netbpm, as well as
// file processing functionality. Regretfully enough, only version 3 and 6 (aka.
// PPM with 24 bit color image of Raw or Ascii format) is implemented. `image`
// is compatible with STL streams, so feel free to dump the file string to and
// read from any streams, using `load` and `dump` function:
//
//     image file_1("image1.ppm");
//     image file_2;
//     std::ifstream fin_2("image2.ppm");
//     file.load(fin_2);
//     fin_2.close();
//
// But you can also use the more pretty `<<` and `>>` operator
//
//     image file_2;
//     std::ifstream fin_2("image2.ppm");
//     fin_2 >> file_2;
//     fin_2.close();
//
//

class image {
public:
        typedef std::vector<color> buf_type;
        typedef std::vector<color>::iterator iter_type;

private:
        size_t _ver;
        size_t _w;
        size_t _h;
        buf_type _buf;

        // read and decode one COMPonent
        void _read_one(std::istream& fin, int comp[3], int max) {
                int scale = 256 / max;

                for(int i = 0; i < 3; i++) {
                        if(_ver == 3)
                                fin >> comp[i];
                        else if(_ver == 6)
                                comp[i] = (uint8_t)fin.get();
                        comp[i] = (comp[i] + 1) * scale - 1;
                }
        }

        // encode and write one COMPonent
        void _write_one(std::ostream& fout, int comp[3]) const {
                for(int i = 0; i < 3; i++) {
                        if(_ver == 3)
                                fout << comp[i] << " ";
                        else if(_ver == 6)
                                fout << (uint8_t)comp[i];
                }
        }

        static void _skip_line(std::istream& os) {
                os.ignore(std::numeric_limits
                        <std::streamsize>::max(), '\n');
        }


public:
        buf_type& buffer() { return _buf; }
        const buf_type& buffer() const { return _buf; }
        size_t width() const { return _w; }
        size_t height() const { return _h; }
        size_t version() const { return _ver; }
        void version(int v) { _ver = v; }

        image() : _ver(6) { }
        image(size_t w, size_t h) : _ver(6) { assign(w, h); }
        image(const std::string& fn) {
                std::ifstream fin(fn);
                load(fin);
                fin.close();
        }

        // **erase** the image and resize the image
        void assign(const image& other) {
                assign(other._w, other._h);
        }
        void assign(size_t w, size_t h) {
                _w = w;
                _h = h;
                _buf.resize(_w * _h);
        }

        color& operator[](const point& p) { return pixel(p); }
        color& operator()(size_t x, size_t y) { return pixel(x, y); }
        color& pixel(const point& p) { return pixel(p.x, p.y); }
        color& pixel(size_t x, size_t y) {
                static color trash;
                if(x >= _w || y >= _h) {
                        x = x >= _w ? _w - 1: x;
                        y = y >= _h ? _h - 1: y;
                        trash = pixel(x, y).value();
                        return trash;
                }
                else return _buf.at(y * _w + x);
        }

        void load(std::istream& fin) {
                // magic number
                if((_ver = fin.get()) != 'P')
                        throw std::logic_error("Corrupted file.");

                size_t max;

                // read until 4 numbers are got
                size_t i = 0, *numlist[] = { &_ver, &_w, &_h, &max };
                while(i < 4 && fin.good()) {
                        char sharp; fin >> sharp;
                        if(sharp == '#') // lines after # are comment
                                _skip_line(fin);
                        else {
                                fin.putback(sharp);
                                fin >> *(numlist[i++]);
                                // i++ at the very last ^_^
                        }
                }

                if(_ver != 3 && _ver != 6)
                        throw std::logic_error("Not a supported version.");
                if(256 % ++max)
                        throw std::logic_error("Not a supported colour scale.");
                assign(_w, _h);
                _skip_line(fin);

                // off we go!
                for(buf_type::iterator i = _buf.begin();
                        i < _buf.end(); i++)
                {
                        if(fin.eof())
                                throw std::logic_error("Size not matched.");

                        int comp[3];
                        _read_one(fin, comp, max);
                        i->r = comp[0];
                        i->g = comp[1];
                        i->b = comp[2];
                }
        }

        void dump(std::ostream& fout) const {
                fout << "P" << _ver << std::endl;
                fout << "# Creator: Shihira Netpbm parser" << std::endl;
                fout << _w << ' ' << _h << ' ' << 255 << std::endl;

                for(buf_type::const_iterator i = _buf.begin();
                        i < _buf.end(); i++)
                {
                        int comp[3] = { i->r, i->g, i->b };
                        _write_one(fout, comp);
                }
        }
};

// STL stream compatible operator overload
std::istream& operator>>(std::istream& si, image& img)
{
        img.load(si);
        return si;
}

std::ostream& operator<<(std::ostream& so, const image& img)
{
        img.dump(so);
        return so;
}

#endif // IMAGE_H_INC

