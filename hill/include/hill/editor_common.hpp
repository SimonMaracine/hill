#pragma once

namespace hill::editor {
    class Editor;
}

namespace hill::editor_common {
    struct Inspectable {
        Inspectable() = default;
        virtual ~Inspectable() = default;

        Inspectable(const Inspectable&) = default;
        Inspectable& operator=(const Inspectable&) = default;
        Inspectable(Inspectable&&) = default;
        Inspectable& operator=(Inspectable&&) = default;

        virtual void editor_inspect(editor::Editor& editor) = 0;
    };
}
