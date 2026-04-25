#include "sdl_example.hpp"

#include <stdexcept>
#include <format>
#include <print>

#include <SDL3/SDL.h>
#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_opengl3.h>
#include <hill/graphics_api.hpp>

static hill::configuration::Configuration make_configuration() {
    hill::debug::DebugOutput debug_output;
    debug_output.output_callback = [](std::string message) { std::println(stderr, "{}", message); };

    hill::configuration::Configuration config;
    config.debug_output = debug_output;

    return config;
}

SdlExample::SdlExample()
    : m_renderer(*this, make_configuration()) {
    if (!SDL_InitSubSystem(SDL_INIT_VIDEO)) {
        throw std::runtime_error(std::format("SDL_InitSubSystem: %s", SDL_GetError()));
    }

    if (!SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4)) {
        throw std::runtime_error(std::format("SDL_GL_SetAttribute: %s", SDL_GetError()));
    }

    if (!SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3)) {
        throw std::runtime_error(std::format("SDL_GL_SetAttribute: %s", SDL_GetError()));
    }

    if (!SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE)) {
        throw std::runtime_error(std::format("SDL_GL_SetAttribute: %s", SDL_GetError()));
    }

    if (!SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG)) {
        throw std::runtime_error(std::format("SDL_GL_SetAttribute: %s", SDL_GetError()));
    }

    if (!SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1)) {
        throw std::runtime_error(std::format("SDL_GL_SetAttribute: %s", SDL_GetError()));
    }

    if (!SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24)) {
        throw std::runtime_error(std::format("SDL_GL_SetAttribute: %s", SDL_GetError()));
    }

    if (!SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, 1)) {
        throw std::runtime_error(std::format("SDL_GL_SetAttribute: %s", SDL_GetError()));
    }

    static constexpr unsigned int flags = SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE;

    m_window = SDL_CreateWindow("SDL Example", 1280, 720, flags);

    if (!m_window) {
        throw std::runtime_error(std::format("SDL_CreateWindow: %s", SDL_GetError()));
    }

    m_context = SDL_GL_CreateContext(m_window);

    if (!m_context) {
        throw std::runtime_error(std::format("SDL_GL_CreateContext: %s", SDL_GetError()));
    }

    hill::graphics_api::initialize(SDL_GL_GetProcAddress);

    if (!SDL_GL_SetSwapInterval(1)) {
        std::println(stderr, "SDL_GL_SetSwapInterval: {}", SDL_GetError());
    }

    if (!SDL_SetWindowMinimumSize(m_window, 640, 360)) {
        std::println(stderr, "SDL_SetWindowMinimumSize: {}", SDL_GetError());
    }
}

SdlExample::~SdlExample() {
    SDL_GL_DestroyContext(static_cast<SDL_GLContext>(m_context));
    SDL_DestroyWindow(m_window);
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

void SdlExample::initialize() const {
    ImGui_ImplSDL3_InitForOpenGL(m_window, m_context);
    ImGui_ImplOpenGL3_Init("#version 430 core");
}

void SdlExample::uninitialize() const {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
}

void SdlExample::begin() const {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
}

void SdlExample::end(ImDrawData* draw_data) const {
    ImGui_ImplOpenGL3_RenderDrawData(draw_data);
}

void SdlExample::update() {
    m_editor.update(m_renderer);
    m_editor.update_camera(m_renderer);
}

void SdlExample::run() {
    std::println("{}", hill::graphics_api::version());

    if (!SDL_ShowWindow(m_window)) {
        throw std::runtime_error(std::format("SDL_ShowWindow: %s", SDL_GetError()));
    }

    m_renderer.initialize();

    while (m_running) {
        SDL_Event event;

        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL3_ProcessEvent(&event);

            switch (event.type) {
                case SDL_EVENT_QUIT:
                    m_running = false;
                    break;
                case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
                    m_running = false;
                    break;
                case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
                    m_renderer.window_resize(event.window.data1, event.window.data2);
                    break;
            }
        }

        m_renderer.render();

        if (!SDL_GL_SwapWindow(m_window)) {
            throw std::runtime_error(std::format("SDL_GL_SwapWindow: %s", SDL_GetError()));
        }
    }

    m_renderer.uninitialize();
}
