#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

#include "hill/mesh.hpp"
#include "hill/utility.hpp"
#include "hill/error.hpp"
#include "hill/glm.h++"

struct aiScene;
struct aiNode;
struct aiMesh;

namespace hill::model {
    struct Node {
        std::weak_ptr<Node> parent;
        std::vector<std::shared_ptr<Node>> children;

        std::string name;
        std::vector<std::shared_ptr<mesh::Mesh>> meshes;
        glm::vec3 translation {};
        glm::quat rotation = glm::identity<glm::quat>();
        glm::vec3 scale {1.0f};
    };

    struct TraversalCtx {
        std::weak_ptr<Node> current_node;  // Used for propagation
        std::weak_ptr<Node> parent_node;

        std::unordered_map<const aiMesh*, std::shared_ptr<mesh::Mesh>> processed_meshes;
        std::unordered_map<std::string, std::shared_ptr<image::Image>> processed_textures;
    };

    class Model {
    public:
        explicit Model(const utility::Buffer& buffer);
        explicit Model(const utility::FilePath& file_path);

        const Node* root() const { return m_root.get(); }
    private:
        void process_node(const aiNode* node, const aiScene* scene, TraversalCtx& ctx);
        static mesh::Mesh process_mesh(const aiMesh* mesh, const aiScene* scene, TraversalCtx& ctx);

        std::shared_ptr<Node> m_root;
    };

    struct ModelError : error::Error {
        using Error::Error;
    };
}
