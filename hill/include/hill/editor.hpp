#pragma once

namespace hill::renderer {
    class Renderer;
}

namespace hill::editor {
    class Editor {
    public:
        void update(renderer::Renderer& renderer);
    private:
        void performance(renderer::Renderer& renderer);
        void primitives_registry(renderer::Renderer&);
        void primitives_object(const char* label, const auto& objects);
    };
}
