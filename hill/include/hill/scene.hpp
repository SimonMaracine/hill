#pragma once

#include <vector>

#include "hill/model.hpp"
#include "hill/renderer_common.hpp"

namespace hill::renderer {
    class Renderer;
}

namespace hill::scene {
    class Node {
    public:
        Node() = default;
        virtual ~Node() = default;

        Node(const Node&) = delete;
        Node& operator=(const Node&) = delete;
        Node(Node&&) = delete;
        Node& operator=(Node&&) = delete;
    };

    class ModelNode : public Node {
    public:
        explicit ModelNode(model::Model model)
            : m_model(std::move(model)) {
            m_objects.reserve(m_model.meshes().size());
        }
    private:
        model::Model m_model;
        std::vector<renderer_common::Object> m_objects;
        bool m_configured {};

        friend class renderer::Renderer;
    };
}
