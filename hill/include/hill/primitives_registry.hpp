#pragma once

#include <unordered_set>

namespace hill::primitives_registry {
    enum class Primitive {
        VertexBuffer,
        ElementBuffer,
        VertexArray,
        Shader,
        Program,
        Texture2D,
        TextureCubemap
    };

    using Container = std::unordered_set<unsigned int>;

    void uninitialize();
    const Container& primitives(Primitive primitive);
    void add_primitive(Primitive primitive, unsigned int id);
    void remove_primitive(Primitive primitive, unsigned int id);
}
