#ifndef GUI_H_INCLUDED
#define GUI_H_INCLUDED

#include <string>
#include <stdexcept>
#include <functional>

#include <SDL2/SDL.h>

#include "image.h"

class window {
protected:
    SDL_Window* _window;
    SDL_Renderer* _renderer;

    window() {
        static bool sdl_has_init = false;
        if(!sdl_has_init) {
            if(SDL_Init(SDL_INIT_VIDEO) < 0)
                throw std::runtime_error("Failed to initialize SDL.");
            atexit(SDL_Quit);
        }

    }

public:
    window(const std::string& title, size_t w = 800, size_t h = 600) {
        window();

        _window = SDL_CreateWindow(title.c_str(),
                SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                w, h, SDL_WINDOW_SHOWN);
        if(!_window) throw std::runtime_error("Failed to create window.");

        _renderer = SDL_CreateRenderer(_window, -1, SDL_RENDERER_ACCELERATED);
    }

    ~window() {
        if(_renderer) SDL_DestroyRenderer(_renderer);
        if(_window) SDL_DestroyWindow(_window);
    }

    void draw(const image& img) {
        SDL_Surface* surf = SDL_GetWindowSurface(_window);
        image::buf_type img_buf = img.buffer();

        uint32_t* pixels = (uint32_t*) surf->pixels;
        for(int i = 0; i < surf->w * surf->h; i++)
            pixels[i] = img_buf[i].value();

        SDL_UpdateWindowSurface(_window);
    }
};

class application {
private:
    application() { }
    static application _app;

    std::function<void(void)> _on_paint;
    std::function<void(void)> _on_exit;

public:
    void register_on_paint(std::function<void(void)> func) {
        _on_paint = func;
    }

    void register_on_exit(std::function<void(void)> func) {
        _on_exit = func;
    }

    void run() {
        SDL_Event e;
        while(true) {
            SDL_PollEvent(&e);

            if(e.type == SDL_QUIT) {
                if(_on_exit) _on_exit();
                break;
            }

            if(_on_paint) _on_paint();
            
            SDL_Delay(16);
        }
    }

    static application& instance() {
        return _app;
    }
};

application application::_app;

inline application& app() {
    return application::instance();
}

class windowgl : public window {
protected:
    SDL_GLContext _glcontext;

public:
    windowgl(const std::string& title, size_t w = 800, size_t h = 600)
            : window() {
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 8);

        _window = SDL_CreateWindow(title.c_str(),
                SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                w, h, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);

        _glcontext = SDL_GL_CreateContext(_window);
    }

    ~windowgl() {
        if(_glcontext) SDL_GL_DeleteContext(_glcontext);
    }

    void swap_buffer() { SDL_GL_SwapWindow(_window); }
};

#endif // GUI_H_INCLUDED
