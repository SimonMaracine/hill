#include "sdl_example.hpp"

#include <stdexcept>
#include <format>
#include <print>

#include <SDL3/SDL.h>
#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_opengl3.h>
#include <hill/graphics_api.hpp>
#include "hill/debug.hpp"

static hill::configuration::Configuration make_configuration() {
    hill::configuration::Configuration config;

    return config;
}

SdlExample::SdlExample() {
    if (!SDL_InitSubSystem(SDL_INIT_VIDEO)) {
        throw std::runtime_error(std::format("SDL_InitSubSystem: {}", SDL_GetError()));
    }

    if (!SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4)) {
        throw std::runtime_error(std::format("SDL_GL_SetAttribute: {}", SDL_GetError()));
    }

    if (!SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3)) {
        throw std::runtime_error(std::format("SDL_GL_SetAttribute: {}", SDL_GetError()));
    }

    if (!SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE)) {
        throw std::runtime_error(std::format("SDL_GL_SetAttribute: {}", SDL_GetError()));
    }

    if (!SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG)) {
        throw std::runtime_error(std::format("SDL_GL_SetAttribute: {}", SDL_GetError()));
    }

    if (!SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1)) {
        throw std::runtime_error(std::format("SDL_GL_SetAttribute: {}", SDL_GetError()));
    }

    if (!SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24)) {
        throw std::runtime_error(std::format("SDL_GL_SetAttribute: {}", SDL_GetError()));
    }

    if (!SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, 1)) {
        throw std::runtime_error(std::format("SDL_GL_SetAttribute: {}", SDL_GetError()));
    }

    static constexpr unsigned int flags = SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE;

    m_window = SDL_CreateWindow("SDL Example", 1280, 720, flags);

    if (!m_window) {
        throw std::runtime_error(std::format("SDL_CreateWindow: {}", SDL_GetError()));
    }

    m_context = SDL_GL_CreateContext(m_window);

    if (!m_context) {
        throw std::runtime_error(std::format("SDL_GL_CreateContext: {}", SDL_GetError()));
    }

    hill::graphics_api::initialize(SDL_GL_GetProcAddress);

    if (!SDL_GL_SetSwapInterval(1)) {
        std::println(stderr, "SDL_GL_SetSwapInterval: {}", SDL_GetError());
    }

    if (!SDL_SetWindowMinimumSize(m_window, 640, 360)) {
        std::println(stderr, "SDL_SetWindowMinimumSize: {}", SDL_GetError());
    }

    hill::debug::DebugOutput debug_output;
    debug_output.output_callback = [](hill::debug::Type, std::string message) {
        std::println(stderr, "{}", message);
    };

    hill::debug::initialize(debug_output);

    m_renderer = std::make_unique<hill::renderer::Renderer>(*this, make_configuration());
    m_editor = std::make_unique<hill::editor::Editor>(*this);
}

SdlExample::~SdlExample() {
    m_editor.reset();
    m_renderer.reset();

    hill::debug::uninitialize();

    SDL_GL_DestroyContext(static_cast<SDL_GLContext>(m_context));
    SDL_DestroyWindow(m_window);
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

void SdlExample::imgui_initialize() const {
    ImGui_ImplSDL3_InitForOpenGL(m_window, m_context);
    ImGui_ImplOpenGL3_Init("#version 430 core");
}

void SdlExample::imgui_uninitialize() const {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
}

void SdlExample::imgui_begin() const {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
}

void SdlExample::imgui_end(ImDrawData* draw_data) const {
    ImGui_ImplOpenGL3_RenderDrawData(draw_data);
}

void SdlExample::imgui_update() {
    m_editor->update(*m_renderer);
    m_editor->update_camera(*m_renderer);
}

void SdlExample::windowing_grab_mouse() const {
    if (!SDL_SetWindowRelativeMouseMode(m_window, true)) {
        std::println(stderr, "SDL_SetWindowRelativeMouseMode: {}", SDL_GetError());
    }
}

void SdlExample::windowing_ungrab_mouse() const {
    if (!SDL_SetWindowRelativeMouseMode(m_window, false)) {
        std::println(stderr, "SDL_SetWindowRelativeMouseMode: {}", SDL_GetError());
    }
}

void SdlExample::run() {
    std::println("{}", hill::graphics_api::version());

    if (!SDL_ShowWindow(m_window)) {
        throw std::runtime_error(std::format("SDL_ShowWindow: {}", SDL_GetError()));
    }

    hill::utility::Buffer buffer;
    hill::utility::read_file("assets/teapot.obj", buffer);

    auto teapot = hill::scene::ModelNode::from_model(hill::model::Model(buffer));
    teapot->translation(glm::vec3(7.0f, 2.0f, -7.0f));
    teapot->rotation(glm::vec3(0.0f, -30.0f, 0.0f));
    m_renderer->root_node()->child(teapot);

    auto heart = hill::scene::ModelNode::from_model(hill::model::Model(hill::utility::FilePath("assets/heart/heart.obj")));
    heart->translation(glm::vec3(-2.0f, 3.0f, 7.0f));
    heart->rotation(glm::vec3(-60.0f, 0.0f, 0.0f));
    heart->scale(glm::vec3(0.3f, 0.3f, 0.3f));
    m_renderer->root_node()->child(heart);

    auto cube = hill::scene::ModelNode::from_model(hill::model::Model(hill::utility::FilePath("assets/cube/cube.glb")));
    m_renderer->root_node()->child(cube);

    auto tree = hill::scene::ModelNode::from_model(hill::model::Model(hill::utility::FilePath("assets/tree.fbx")));
    tree->translation(glm::vec3(-12.0f, 1.0f, -8.0f));
    tree->scale(glm::vec3(0.005f));
    m_renderer->root_node()->child(tree);

    auto backpack = hill::scene::ModelNode::from_model(hill::model::Model(hill::utility::FilePath("assets/backpack/backpack.obj")));
    backpack->translation(glm::vec3(10.0f, 2.0f, 10.0f));
    m_renderer->root_node()->child(backpack);

    auto sun = std::make_shared<hill::scene::DirectionalLightNode>("Sun");
    sun->directional_light.direction = glm::normalize(glm::vec3(0.1f, -1.0f, 0.4f));
    m_renderer->root_node()->child(sun);

    auto light = std::make_shared<hill::scene::PointLightNode>("Light");
    light->point_light.position = glm::vec3(1.0f, 1.0f, 0.0f);
    m_renderer->root_node()->child(light);

    auto light2 = std::make_shared<hill::scene::PointLightNode>("Light2");
    light2->point_light.position = glm::vec3(-5.0f, 1.0f, 0.0f);
    m_renderer->root_node()->child(light2);

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
                    m_renderer->window_resize(event.window.data1, event.window.data2);
                    break;
            }
        }

        m_renderer->render();

        if (!SDL_GL_SwapWindow(m_window)) {
            throw std::runtime_error(std::format("SDL_GL_SwapWindow: {}", SDL_GetError()));
        }
    }
}
