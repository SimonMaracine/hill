#pragma once

#include <vector>

#include "hill/mesh.hpp"
#include "hill/utility.hpp"
#include "hill/error.hpp"

struct aiScene;
struct aiNode;
struct aiMesh;

namespace hill::model {
    class Model {
    public:
        explicit Model(const utility::Buffer& buffer);
    private:
        void process_node(const aiNode* node, const aiScene* scene);

        std::vector<mesh::Mesh> m_meshes;
    };

    struct ModelError : error::Error {
        using Error::Error;
    };
}
