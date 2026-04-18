#include <print>
#include <cstdlib>

#include <SDL3/SDL.h>

#include "sdl_example.hpp"

int main() {
    std::atexit(SDL_Quit);
    std::println("Hello, world!");

    try {
        SdlExample example;
        example.run();
    } catch (const std::runtime_error& e) {
        std::println(stderr, "Critical error: {}", e.what());
        return 1;
    }

    return 0;
}
