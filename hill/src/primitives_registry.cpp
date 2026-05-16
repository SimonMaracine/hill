#include "hill/primitives_registry.hpp"

#include <unordered_map>

namespace hill::primitives_registry {
    static std::unordered_map<Primitive, Container> g_primitives;

    void uninitialize() {
        g_primitives.clear();
    }

    const Container& primitives(Primitive primitive) {
        return g_primitives[primitive];
    }

    void add_primitive(Primitive primitive, unsigned int id) {
        g_primitives[primitive].insert(id);
    }

    void remove_primitive(Primitive primitive, unsigned int id) {
        g_primitives[primitive].erase(id);
    }
}
