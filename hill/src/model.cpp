#include "hill/model.hpp"

#include <format>
#include <utility>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace hill::model {
    static constexpr mesh::TextureType texture_type_enum(aiTextureType texture_type) {
        switch (texture_type) {
            case aiTextureType_DIFFUSE:
                return mesh::TextureType::Albedo;
            case aiTextureType_SPECULAR:
                return mesh::TextureType::Metallic;
            case aiTextureType_NORMALS:
                return mesh::TextureType::Normal;
            default:
                std::unreachable();
        }
    }

    static constexpr glm::mat4 transformation(const aiMatrix4x4& matrix) {
        glm::mat4 result_matrix;

        result_matrix[0][0] = matrix.a1;
        result_matrix[0][1] = matrix.a2;
        result_matrix[0][2] = matrix.a3;
        result_matrix[0][3] = matrix.a4;
        result_matrix[1][0] = matrix.b1;
        result_matrix[1][1] = matrix.b2;
        result_matrix[1][2] = matrix.b3;
        result_matrix[1][3] = matrix.b4;
        result_matrix[2][0] = matrix.c1;
        result_matrix[2][1] = matrix.c2;
        result_matrix[2][2] = matrix.c3;
        result_matrix[2][3] = matrix.c4;
        result_matrix[3][0] = matrix.d1;
        result_matrix[3][1] = matrix.d2;
        result_matrix[3][2] = matrix.d3;
        result_matrix[3][3] = matrix.d4;

        return glm::transpose(result_matrix);
    }

    static std::vector<mesh::Texture> load_material_textures(const aiMaterial* material, aiTextureType texture_type) {
        std::vector<mesh::Texture> textures;

        for (unsigned int i {}; i < material->GetTextureCount(texture_type); i++) {
            aiString path;
            material->GetTexture(texture_type, i, &path);

            mesh::Texture texture;
            texture.path = path.C_Str();
            texture.type = texture_type_enum(texture_type);

            textures.push_back(std::move(texture));
        }

        return textures;
    }

    static mesh::Material load_material_properties(const aiMaterial* material) {
        mesh::Material result_material;
        result_material.name = material->GetName().C_Str();

        if (aiColor4D value; aiGetMaterialColor(material, AI_MATKEY_COLOR_AMBIENT, &value) == aiReturn_SUCCESS) {
            result_material.color_ambient = glm::vec3(value.r, value.g, value.b);
        }

        if (aiColor4D value; aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, &value) == aiReturn_SUCCESS) {
            result_material.color_diffuse = glm::vec3(value.r, value.g, value.b);
        }

        if (aiColor4D value; aiGetMaterialColor(material, AI_MATKEY_COLOR_SPECULAR, &value) == aiReturn_SUCCESS) {
            result_material.color_specular = glm::vec3(value.r, value.g, value.b);
        }

        if (float value; aiGetMaterialFloat(material, AI_MATKEY_SHININESS, &value) == aiReturn_SUCCESS) {
            result_material.shininess = value;
        }

        return result_material;
    }

    Model::Model(const utility::Buffer& buffer) {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFileFromMemory(buffer.data(), buffer.size(), aiProcess_Triangulate | aiProcess_GenNormals);

        if (!scene) {
            throw ModelError(std::format("Could not load model: {}", importer.GetErrorString()));
        }

        if (!scene->mRootNode) {
            throw ModelError("No root node in scene");
        }

        if (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) {
            throw ModelError("Model is incomplete");
        }

        TraversalCtx ctx;

        m_root = std::make_shared<Node>();
        ctx.current_node = m_root;
        process_node(scene->mRootNode, scene, ctx);
    }

    Model::Model(const utility::FilePath& file_path) {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(file_path.string().c_str(), aiProcess_Triangulate | aiProcess_GenNormals);

        if (!scene) {
            throw ModelError(std::format("Could not load model: {}", importer.GetErrorString()));
        }

        if (!scene->mRootNode) {
            throw ModelError("No root node in scene");
        }

        if (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) {
            throw ModelError("Model is incomplete");
        }

        TraversalCtx ctx;

        m_root = std::make_shared<Node>();
        ctx.current_node = m_root;
        process_node(scene->mRootNode, scene, ctx);
    }

    void Model::process_node(const aiNode* node, const aiScene* scene, TraversalCtx& ctx) {
        ctx.current_node.lock()->parent = std::exchange(ctx.parent_node, ctx.current_node.lock());

        const auto current_node = ctx.current_node.lock();

        current_node->name = node->mName.C_Str();
        current_node->transform = transformation(node->mTransformation);

        for (unsigned int i {}; i < node->mNumMeshes; i++) {
            const aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

            if (const auto iter = ctx.processed_meshes.find(mesh); iter != ctx.processed_meshes.end()) {
                current_node->meshes.push_back(iter->second);
                continue;
            }

            current_node->meshes.push_back(std::make_shared<mesh::Mesh>(process_mesh(mesh, scene)));
        }

        for (unsigned int i {}; i < node->mNumChildren; i++) {
            current_node->children.push_back(std::make_shared<Node>());
            ctx.current_node = current_node->children.back();
            process_node(node->mChildren[i], scene, ctx);
        }
    }

    mesh::Mesh Model::process_mesh(const aiMesh* mesh, const aiScene* scene) {
        mesh::Mesh result_mesh;

        result_mesh.name = mesh->mName.C_Str();

        if (!mesh->HasPositions()) {
            throw ModelError("Mesh doesn't have positions");
        }

        if (mesh->HasVertexColors(0)) {
            result_mesh.vertex_attributes |= mesh::Colors;
        }

        if (mesh->HasNormals()) {
            result_mesh.vertex_attributes |= mesh::Normals;
        }

        if (mesh->HasTextureCoords(0)) {
            result_mesh.vertex_attributes |= mesh::TextureCoordinates;
        }

        for (unsigned int i {}; i < mesh->mNumVertices; i++) {
            mesh::Vertex vertex;

            vertex.position.x = mesh->mVertices[i].x;
            vertex.position.y = mesh->mVertices[i].y;
            vertex.position.z = mesh->mVertices[i].z;

            if (mesh->HasVertexColors(0)) {
                vertex.color.r = mesh->mColors[0][i].r;
                vertex.color.g = mesh->mColors[0][i].g;
                vertex.color.b = mesh->mColors[0][i].b;
                vertex.color.a = mesh->mColors[0][i].a;
            }

            if (mesh->HasNormals()) {
                vertex.normal.x = mesh->mNormals[i].x;
                vertex.normal.y = mesh->mNormals[i].y;
                vertex.normal.z = mesh->mNormals[i].z;
            }

            if (mesh->HasTextureCoords(0)) {
                vertex.uv.x = mesh->mTextureCoords[0][i].x;
                vertex.uv.y = mesh->mTextureCoords[0][i].y;
            }

            result_mesh.vertices.push_back(vertex);
        }

        if (!mesh->HasFaces()) {
            throw ModelError("Mesh doesn't have faces");
        }

        for (unsigned int i {}; i < mesh->mNumFaces; i++) {
            const aiFace& face = mesh->mFaces[i];

            for (unsigned int j {}; j < face.mNumIndices; j++) {
                result_mesh.indices.push_back(face.mIndices[j]);
            }
        }

        const aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

        if (!m_materials.contains(mesh->mMaterialIndex)) {
            m_materials[mesh->mMaterialIndex] = load_material_properties(material);
        }

        result_mesh.material_index = mesh->mMaterialIndex;

        result_mesh.textures.append_range(load_material_textures(material, aiTextureType_DIFFUSE));
        result_mesh.textures.append_range(load_material_textures(material, aiTextureType_SPECULAR));
        result_mesh.textures.append_range(load_material_textures(material, aiTextureType_NORMALS));

        return result_mesh;
    }
}
