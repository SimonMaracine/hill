#pragma once

struct SDL_Window;

class SdlExample {
public:
    SdlExample();
    ~SdlExample();

    void run();
private:
    SDL_Window* m_window {};
    void* m_context {};
    bool m_running = true;
};
