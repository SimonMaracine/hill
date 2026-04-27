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
                return mesh::TextureType::Specular;
            case aiTextureType_NORMALS:
                return mesh::TextureType::Normal;
            default:
                break;
        }

        std::unreachable();
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

    static mesh::Mesh process_mesh(const aiMesh* mesh, const aiScene* scene) {
        mesh::Mesh result_mesh;

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

        result_mesh.textures.append_range(load_material_textures(material, aiTextureType_DIFFUSE));
        result_mesh.textures.append_range(load_material_textures(material, aiTextureType_SPECULAR));
        result_mesh.textures.append_range(load_material_textures(material, aiTextureType_NORMALS));

        return result_mesh;
    }

    Model::Model(const utility::Buffer& buffer) {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFileFromMemory(buffer.data(), buffer.size(), aiProcess_Triangulate);

        if (!scene) {
            throw ModelError(std::format("Could not load model: {}", importer.GetErrorString()));
        }

        if (!scene->mRootNode) {
            throw ModelError("No root node in scene");
        }

        if (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) {
            throw ModelError("Model is incomplete");
        }

        process_node(scene->mRootNode, scene);
    }

    void Model::process_node(const aiNode* node, const aiScene* scene) {
        for (unsigned int i {}; i < node->mNumMeshes; i++) {
            const aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            m_meshes.push_back(process_mesh(mesh, scene));
        }

        for (unsigned int i {}; i < node->mNumChildren; i++) {
            process_node(node->mChildren[i], scene);
        }
    }
}
