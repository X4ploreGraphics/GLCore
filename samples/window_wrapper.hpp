

#ifndef WINDOW_WRAPPER_HPP
#define WINDOW_WRAPPER_HPP

#include <functional>
#include <chrono>
#include <cstdint>

#if defined(__EMSCRIPTEN__)
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#include <SDL2/SDL.h>
#include <stdio.h>
#else
#include <GLFW/glfw3.h>
#endif

#include "glcore/glcore_log.hpp"
#include "glcore/context.hpp"
#include "shadow/camera.hpp"
#include <spdlog/sinks/stdout_color_sinks.h>

#include <iostream>


#if !defined(__EMSCRIPTEN__)
void key2_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
  if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
      glfwSetWindowShouldClose(window, GL_TRUE);

}
#endif

namespace nitros
{
    class Time2
    {
        public:
        using clock_type = std::chrono::steady_clock;
        using duration_value   = std::chrono::duration<double, std::milli>;
        using time_point_value = std::chrono::time_point<clock_type, duration_value>;

        static void set_time(const time_point_value  &time_point) noexcept;
        [[nodiscard]] static auto get_time() noexcept -> time_point_value;
        [[nodiscard]] static auto delta_time() noexcept -> duration_value;

        private:
        static time_point_value last_time;
        static time_point_value current_time;
    };

    void Time2::set_time(const Time2::time_point_value  &time_point) noexcept{
        last_time = current_time;
        current_time = time_point;
    }

    auto  Time2::get_time() noexcept -> Time2::time_point_value {
        return current_time;
    }

    auto   Time2::delta_time() noexcept -> Time2::duration_value {
        return current_time - last_time;
    }

    Time2::time_point_value Time2::last_time{};
    Time2::time_point_value Time2::current_time{};
}

#if defined(__EMSCRIPTEN__)
void print_gl_version()
{
    int major_version{};
    int minor_version{};
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &major_version);
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &minor_version);

    auto context_handle = emscripten_webgl_get_current_context();

    auto ctx_attrs = EmscriptenWebGLContextAttributes{};
    emscripten_webgl_get_context_attributes(context_handle, &ctx_attrs);

    std::cout<<"WebGL ( "<< ctx_attrs.majorVersion<<'.'<<ctx_attrs.minorVersion<<" )"<<std::endl;
    std::cout<< ctx_attrs.explicitSwapControl <<std::endl;
    std::cout<< ctx_attrs.alpha <<std::endl;
    std::cout<< ctx_attrs.depth <<std::endl;
    std::cout<< ctx_attrs.premultipliedAlpha <<std::endl;

    std::cout<<"Video Driver "<< SDL_GetCurrentVideoDriver()<<std::endl;
    std::cout<<"OpenGL "<< major_version <<'.'<< minor_version <<std::endl;
}

auto loop = std::function<void()>{};
auto input_update = std::function<void()>{};
void main_loop()
{
    std::invoke(loop);
    std::invoke(input_update);
    nitros::Time2::set_time(nitros::Time2::clock_type::now());
}

EM_BOOL click_callback(int eventType, const EmscriptenMouseEvent *e, void *userData)
{
    std::cout<<"Mouse CallBack "<<std::endl;
    return 0;
}

EM_BOOL keydown_callback(int eventType, const EmscriptenKeyboardEvent *keyEvent, void *userData)
{
    std::cout<<"Key Down Callback "<<std::endl;
    return 0;
}

#endif

class Window
{
    public:
    
    Window(std::uint32_t width = 800, std::uint32_t  height = 600)
        :dim{width, height}
        ,camera{}
    {
    
    auto sink = std::make_shared<spdlog::sinks::stdout_color_sink_st>();
    nitros::glcore::setup_logger({sink})->set_level(spdlog::level::debug);

#if defined(__EMSCRIPTEN__)
    if(auto flag = SDL_Init(SDL_INIT_VIDEO); flag == -1)
    {
        std::cout<<"SDL Init Failed "<<std::endl;
        throw std::runtime_error("SDL Init Failed");
    }
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetSwapInterval(0);
    
    //SDL_SetHint(SDL_HINT_EMSCRIPTEN_KEYBOARD_ELEMENT, "#canvas");

    window = SDL_CreateWindow("GL_window",
                              SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              width, height, SDL_WINDOW_OPENGL);

    auto ctx = SDL_GL_CreateContext(window);
 
    int iwidth, iheight;
    SDL_GetWindowSize(window, &iwidth, &iheight);

    std::cout<< '(' << iwidth <<',' << iheight << ')' <<std::endl;
    print_gl_version();

    //emscripten_set_click_callback("canvas", 0, 0, click_callback );
    //emscripten_set_keypress_callback("canvas", 0, 0, keydown_callback );

#else
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);

    auto [version , str] = nitros::glcore::get_version();

    if(version >= 40500) {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    }
    else {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    }
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, "Window", nullptr, nullptr);
    glfwMakeContextCurrent(window);

    glfwSetKeyCallback(window, key2_callback);
#endif

    }

    auto window_dim() const noexcept{
        //return std::make_pair(800u, 600u);
        return dim;
    }

    ~Window()
    {
#if defined(__EMSCRIPTEN__)
        SDL_DestroyWindow(window);
        SDL_Quit();
#else
        glfwDestroyWindow(window);
#endif
    }

    void run(const std::function<void()>  &funct, std::size_t   count = -1)
    {
        auto lastFrame = 0.f;
        
#if defined(__EMSCRIPTEN__)
        loop = [funct, win = window](){
            std::invoke(funct);
            SDL_GL_SwapWindow(win);
        };
        
        //emscripten_SDL_SetEventHandler(EventHandler, 0);
        input_update = [this]()
        {
            SDL_Event  event;
            while(SDL_PollEvent(&event))
            {
                switch(event.type)
                {
                    case SDL_KEYDOWN :
                        switch ( event.key.keysym.sym )
                        {
                            case SDLK_w: mv_front = true; break;
                            case SDLK_s: mv_back  = true; break;
                            case SDLK_a: mv_left  = true; break;
                            case SDLK_d: mv_right = true; break;
                            case SDLK_q: mv_up    = true; break;
                            case SDLK_e: mv_down  = true; break;

                            case SDLK_UP   : cc_up    = true; break;
                            case SDLK_DOWN : cc_down  = true; break;
                            case SDLK_LEFT : cc_left  = true; break;
                            case SDLK_RIGHT: cc_right = true; break;

                        default:
                            break;
                        }
                        break;

                    case SDL_KEYUP :
                        switch ( event.key.keysym.sym )
                        {
                            case SDLK_w: mv_front = false; break;
                            case SDLK_s: mv_back  = false; break;
                            case SDLK_a: mv_left  = false; break;
                            case SDLK_d: mv_right = false; break;
                            case SDLK_q: mv_up    = false; break;
                            case SDLK_e: mv_down  = false; break;

                            case SDLK_UP   : cc_up    = false; break;
                            case SDLK_DOWN : cc_down  = false; break;
                            case SDLK_LEFT : cc_left  = false; break;
                            case SDLK_RIGHT: cc_right = false; break;

                        default:
                            break;
                        }
                        break;

                    default:    break;
                }
            }

            auto deltaTime = nitros::Time2::delta_time().count()/1000.f;

            if(mv_front)
                camera.ProcessKeyboard(FORWARD, deltaTime);
            if (mv_back)
                camera.ProcessKeyboard(BACKWARD, deltaTime);
            if (mv_left)
                camera.ProcessKeyboard(LEFT, deltaTime);
            if (mv_right)
                camera.ProcessKeyboard(RIGHT, deltaTime);
            if (mv_up)
                camera.ProcessKeyboard(UP, deltaTime);
            if (mv_down)
                camera.ProcessKeyboard(DOWN, deltaTime);

            if (cc_up)
                camera.ProcessLook(Camera_Look::Up, deltaTime);
            if (cc_down)
                camera.ProcessLook(Camera_Look::Down, deltaTime);
            if (cc_left)
                camera.ProcessLook(Camera_Look::Left, deltaTime);
            if (cc_right)
                camera.ProcessLook(Camera_Look::Right, deltaTime);
        };
        
        emscripten_set_main_loop(main_loop, 0, true);
#else
        while(!glfwWindowShouldClose(window) && count != 0)
        {
            funct();

            auto current_frame = glfwGetTime();
            auto deltaTime = current_frame - lastFrame;
            lastFrame = current_frame;

            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
                camera.ProcessKeyboard(FORWARD, deltaTime);
            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
                camera.ProcessKeyboard(BACKWARD, deltaTime);
            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
                camera.ProcessKeyboard(LEFT, deltaTime);
            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
                camera.ProcessKeyboard(RIGHT, deltaTime);
            if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
                camera.ProcessKeyboard(UP, deltaTime);
            if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
                camera.ProcessKeyboard(DOWN, deltaTime);

            if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
                camera.ProcessLook(Camera_Look::Up, deltaTime);
            if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
                camera.ProcessLook(Camera_Look::Down, deltaTime);
            if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
                camera.ProcessLook(Camera_Look::Left, deltaTime);
            if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
                camera.ProcessLook(Camera_Look::Right, deltaTime);

            glfwSwapBuffers(window);
            glfwPollEvents();
            nitros::Time2::set_time(nitros::Time2::clock_type::now());

            if(count > 0)
                count--;
        }
#endif
    }

#if !defined(__EMSCRIPTEN__)
    GLFWwindow*  window;
#else
    SDL_Window*  window;
#endif
    Camera  camera;

    private:
    std::pair<std::uint32_t, std::uint32_t>   dim;
    
    bool mv_front = false; 
    bool mv_back  = false;
    bool mv_left  = false;
    bool mv_right = false;
    bool mv_up   = false;
    bool mv_down = false;

    bool cc_up    = false; 
    bool cc_down  = false;
    bool cc_left  = false;
    bool cc_right = false;
};

#endif