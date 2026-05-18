#include "hill/model.hpp"

#include <format>
#include <utility>
#include <cstring>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace hill::model {
    static aabb::Aabb aabb(const aiAABB& aabb) {
        return {
            .min = glm::vec3(aabb.mMin.x, aabb.mMin.y, aabb.mMin.z),
            .max = glm::vec3(aabb.mMax.x, aabb.mMax.y, aabb.mMax.z)
        };
    }

    static std::shared_ptr<image::Image> load_material_texture(const aiMaterial* material, const aiScene* scene, aiTextureType texture_type, TraversalCtx& ctx) {
        if (material->GetTextureCount(texture_type) == 0) {
            return nullptr;
        }

        aiString path;
        material->GetTexture(texture_type, 0, &path);

        if (const auto iter = ctx.processed_textures.find(path.C_Str()); iter != ctx.processed_textures.end()) {
            return iter->second;
        }

        const aiTexture* texture = scene->GetEmbeddedTexture(path.C_Str());

        if (texture) {
            const bool encoded = texture->mHeight == 0;

            if (encoded) {
                utility::Buffer buffer;
                buffer.resize(texture->mWidth);
                std::memcpy(buffer.data(), texture->pcData, texture->mWidth);

                return ctx.processed_textures[path.C_Str()] = std::make_shared<image::Image>(buffer);
            }

            throw ModelError("Raw image loading not implemented");
        }

        utility::Buffer buffer;
        utility::read_file(std::filesystem::path(path.C_Str()), buffer);

        return ctx.processed_textures[path.C_Str()] = std::make_shared<image::Image>(buffer);
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
        static constexpr auto flags = aiProcess_ValidateDataStructure | aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_GenBoundingBoxes;

        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFileFromMemory(buffer.data(), buffer.size(), flags);

        load(importer, scene);
    }

    Model::Model(const utility::FilePath& file_path) {
        static constexpr auto flags = aiProcess_ValidateDataStructure | aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_GenBoundingBoxes;

        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(file_path.string().c_str(), flags);

        load(importer, scene);
    }

    void Model::load(const Assimp::Importer& importer, const aiScene* scene) {
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

        aiVector3D translation, scale;
        aiQuaternion rotation;
        node->mTransformation.Decompose(scale, rotation, translation);

        current_node->translation = glm::vec3(translation.x, translation.y, translation.z);
        current_node->rotation = glm::quat(rotation.w, rotation.x, rotation.y, rotation.z);
        current_node->scale = glm::vec3(scale.x, scale.y, scale.z);

        for (unsigned int i {}; i < node->mNumMeshes; i++) {
            const aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

            if (const auto iter = ctx.processed_meshes.find(mesh); iter != ctx.processed_meshes.end()) {
                current_node->meshes.push_back(iter->second);
                continue;
            }

            current_node->meshes.push_back(std::make_shared<mesh::Mesh>(process_mesh(mesh, scene, ctx)));
        }

        for (unsigned int i {}; i < node->mNumChildren; i++) {
            current_node->children.push_back(std::make_shared<Node>());
            ctx.current_node = current_node->children.back();
            process_node(node->mChildren[i], scene, ctx);
        }
    }

    mesh::Mesh Model::process_mesh(const aiMesh* mesh, const aiScene* scene, TraversalCtx& ctx) {
        mesh::Mesh result_mesh;

        result_mesh.name = mesh->mName.C_Str();
        result_mesh.aabb = aabb(mesh->mAABB);

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
                vertex.texture_coordinate.x = mesh->mTextureCoords[0][i].x;
                vertex.texture_coordinate.y = mesh->mTextureCoords[0][i].y;
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

        // Load the material even if it was already loaded before because of sharing, doesn't hurt much
        result_mesh.material = load_material_properties(material);

        // Handle shared textures, however
        result_mesh.material.texture_diffuse = load_material_texture(material, scene, aiTextureType_DIFFUSE, ctx);
        result_mesh.material.texture_specular = load_material_texture(material, scene, aiTextureType_SPECULAR, ctx);
        // result_mesh.material.texture_diffuse = load_material_texture(material, scene, aiTextureType_NORMALS, ctx);

        return result_mesh;
    }
}
