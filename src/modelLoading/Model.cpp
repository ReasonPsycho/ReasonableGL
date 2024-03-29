//
// Created by redkc on 02/12/2023.
//

#include "Model.h"

void Model::SimpleDraw(Shader &shader) {
    for (unsigned int i = 0; i < meshes.size(); i++)
        meshes[i].SimpleDraw(shader);
}

// draws the model, and thus all its meshes
void Model::Draw(Shader &shader) {
    for (unsigned int i = 0; i < meshes.size(); i++)
        meshes[i].Draw(shader);
}

//private:
// loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
void Model::loadModel() {
    // read file via ASSIMP
    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(*path,
                                             aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs |
                                             aiProcess_CalcTangentSpace);
    // check for errors
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
    {
        spdlog::error("Assimp error: " + string(importer.GetErrorString()));
        return;
    }
    // retrieve the directory path of the filepath
    directory = path->substr(0, path->find_last_of('/'));

    // process ASSIMP's root node recursively
    processNode(scene->mRootNode, scene);
}

void replaceAll(string &str, const string &from, const string &to) {
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
    }
}

// processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
void Model::processNode(aiNode *node, const aiScene *scene) {
    // process each mesh located at the current node
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        // the node object only contains indices to index the actual objects in the scene. 
        // the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene));
    }
    // after we've processed all of the meshes (if any) we then recursively process each of the children nodes
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        processNode(node->mChildren[i], scene);
    }

}

Mesh Model::processMesh(aiMesh *mesh, const aiScene *scene) {
    // data to fill
    vector<Vertex> vertices;
    vector<unsigned int> indices;
    vector<shared_ptr<Texture>> textures;
    this->futhestLenghtsFromCenter = glm::vec3(0.0f);
    // walk through each of the mesh's vertices
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex;
        glm::vec3 vector; // we declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
        // positions
        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vertex.Position = vector;

        if (vector.x > glm::abs(futhestLenghtsFromCenter.x)) {
            futhestLenghtsFromCenter.x = glm::abs(vector.x);
        }

        if (vector.y > glm::abs(futhestLenghtsFromCenter.y)) {
            futhestLenghtsFromCenter.y = glm::abs(vector.y);
        }

        if (vector.z > glm::abs(futhestLenghtsFromCenter.z)) {
            futhestLenghtsFromCenter.z = glm::abs(vector.z);
        }
        // normals
        if (mesh->HasNormals()) {
            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            vertex.Normal = vector;
        }
        // texture coordinates
        if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
        {
            glm::vec2 vec;
            // a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
            // use models where a vertex can have multiple texture coordinates so we always take the first set (0).
            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.TexCoords = vec;
            // tangent
            vector.x = mesh->mTangents[i].x;
            vector.y = mesh->mTangents[i].y;
            vector.z = mesh->mTangents[i].z;
            vertex.Tangent = vector;
            // bitangent
            vector.x = mesh->mBitangents[i].x;
            vector.y = mesh->mBitangents[i].y;
            vector.z = mesh->mBitangents[i].z;
            vertex.Bitangent = vector;
        } else
            vertex.TexCoords = glm::vec2(0.0f, 0.0f);

        vertices.push_back(vertex);
    }
    // now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        // retrieve all indices of the face and store them in the indices vector
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }
    // process materials
    aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
    // we assume a convention for sampler names in the shaders. Each diffuse texture should be named
    // as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER. 
    // Same applies to other texture as the following list summarizes:
    // diffuse: texture_diffuseN
    // specular: texture_specularN
    // normal: texture_normalN

// 1. albedo maps
    vector<shared_ptr<Texture>> albedoMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_albedo");
    textures.insert(textures.end(), albedoMaps.begin(), albedoMaps.end());
// 2. normal maps
    vector<shared_ptr<Texture>> normalMaps = loadMaterialTextures(material, aiTextureType_NORMALS, "texture_normal");
    textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
// 3. metallic maps
    vector<shared_ptr<Texture>> metallicMaps = loadMaterialTextures(material, aiTextureType_METALNESS,
                                                                    "texture_metallic");
    textures.insert(textures.end(), metallicMaps.begin(), metallicMaps.end());
// 4. roughness maps
    vector<shared_ptr<Texture>> roughnessMaps = loadMaterialTextures(material, aiTextureType_SHININESS,
                                                                     "texture_roughness");
    textures.insert(textures.end(), roughnessMaps.begin(), roughnessMaps.end());
// 5. ambient occlusion maps

    string albedoPath = albedoMaps.back().get()->path;
    string aoPath = albedoPath.substr(0, albedoPath.find_last_of('_')) + "_ao.png";

    vector<shared_ptr<Texture>> aoMaps = forceLoadMaterialTexture(aoPath, aiTextureType_AMBIENT_OCCLUSION,
                                                                  "texture_ao");

    textures.insert(textures.end(), aoMaps.begin(), aoMaps.end());
    // return a mesh object created from the extracted mesh data
    return Mesh(vertices, indices, textures);
}

// checks all material textures of a given type and loads the textures if they're not loaded yet.
// the required info is returned as a Texture struct.
vector<std::shared_ptr<Texture>> Model::loadMaterialTextures(aiMaterial *mat, aiTextureType type, string typeName) {
    vector<std::shared_ptr<Texture>> textures;
    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
        aiString str;

        mat->GetTexture(type, i, &str);
        // check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
        bool skip = false;
        for (unsigned int j = 0; j < textures_loaded.size(); j++) {
            if (std::strcmp(textures_loaded[j]->path.c_str(), str.C_Str()) == 0) {
                textures.push_back(textures_loaded[j]);
                skip = true; // a texture with the same filepath has already been loaded, continue to next one. (optimization)
                break;
            }
        }
        if (!skip) {   // if texture hasn't been loaded already, load it
            directory = path->substr(0, path->find_last_of('/'));
            string texturePath = string("\\" + directory + "\\" + str.C_Str());
            replaceAll(texturePath, "/", "\\");
            shared_ptr<Texture> texture = std::make_shared<Texture>(texturePath, typeName);
            textures.push_back(texture);
            textures_loaded.push_back(
                    texture);  // store it as texture loaded for entire model, to ensure we won't unnecessarily load duplicate textures.
        }
    }
    return textures;
}

vector<std::shared_ptr<Texture>> Model::forceLoadMaterialTexture(string path, aiTextureType type, string typeName) {
    vector<std::shared_ptr<Texture>> textures;
    // check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
    bool skip = false;
    for (unsigned int j = 0; j < textures_loaded.size(); j++) {
        if (std::strcmp(textures_loaded[j]->path.c_str(), path.c_str()) == 0) {
            textures.push_back(textures_loaded[j]);
            skip = true; // a texture with the same filepath has already been loaded, continue to next one. (optimization)
            break;
        }
    }
    if (!skip) {   // if texture hasn't been loaded already, load it
        shared_ptr<Texture> texture = std::make_shared<Texture>(path, typeName);
        textures.push_back(texture);
        textures_loaded.push_back(
                texture);  // store it as texture loaded for entire model, to ensure we won't unnecessarily load duplicate textures.
    }
    return textures;
}

