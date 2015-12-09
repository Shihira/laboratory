#ifndef GUI_H_INCLUDED
#define GUI_H_INCLUDED

#include <string>
#include <stdexcept>
#include <functional>

#include <SDL2/SDL.h>
#include <GL/glew.h>

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

    int width() const {
        int w, h;
        SDL_GetWindowSize(_window, &w, &h);
        return w;
    }

    int height() const {
        int w, h;
        SDL_GetWindowSize(_window, &w, &h);
        return h;
    }
};

class application {
public:
    enum mouse_button {
        left_button = SDL_BUTTON_LEFT,
        right_button = SDL_BUTTON_RIGHT,
        middle_button = SDL_BUTTON_MIDDLE,
    };

private:
    application() { }
    static application _app;

    std::function<void(void)> _on_paint;
    std::function<void(void)> _on_exit;
    std::function<void(int, int, mouse_button)> _on_mouse_down;
    std::function<void(int, int, mouse_button)> _on_mouse_up;

public:
    void register_on_paint(std::function<void(void)> func) {
        _on_paint = func;
    }

    void register_on_exit(std::function<void(void)> func) {
        _on_exit = func;
    }

    void register_on_mouse_down(decltype(_on_mouse_down) func) {
        _on_mouse_down = func;
    }

    void register_on_mouse_up(decltype(_on_mouse_up) func) {
        _on_mouse_up = func;
    }

    void run() {
        SDL_Event e;
        while(true) {
            SDL_PollEvent(&e);

            if(e.type == SDL_QUIT) {
                if(_on_exit) _on_exit();
                break;
            }

            if(e.type == SDL_MOUSEBUTTONUP) {
                if(_on_mouse_up) _on_mouse_up(
                    e.button.x, e.button.y, (mouse_button)e.button.button);
            }

            if(e.type == SDL_MOUSEBUTTONDOWN) {
                if(_on_mouse_down) _on_mouse_down(
                    e.button.x, e.button.y, (mouse_button)e.button.button);
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
    windowgl(const std::string& title, uint16_t glversion = 0,
            size_t w = 800, size_t h = 600)
            : window() {
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 8);
        if(glversion) {
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, glversion >> 8);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, glversion & 0xff);
        }

        SDL_GL_SetSwapInterval(1);

        _window = SDL_CreateWindow(title.c_str(),
                SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                w, h, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);

        _glcontext = SDL_GL_CreateContext(_window);
    }

    ~windowgl() {
        if(_glcontext) SDL_GL_DeleteContext(_glcontext);
    }

    void swap_buffer() { SDL_GL_SwapWindow(_window); }

    void init_glew() {
        glewExperimental = true;
        if(glewInit() != GLEW_OK)
            throw std::runtime_error("GLEW Initialization failed.");
    }
};

#endif // GUI_H_INCLUDED
