#include "CustomModel.h"
#include "Vertex.h"
#include <assimp/Importer.hpp>
#include <assimp/mesh.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/ext/vector_float3.hpp>
#include <iostream>
#include <ostream>
#include <vector>

CustomModel::CustomModel(const char* location) {
    // First step is loading the model
    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(location, aiProcess_Triangulate);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cout << "Failed to load model using Assimp!" << std::endl;
        return;
    }

    processNode(scene->mRootNode, scene);
}

void CustomModel::processNode(aiNode *node, const aiScene *scene) {
    std::cout << "Num meshes: " << node->mNumMeshes << std::endl;
    // process all the node's meshes (if any)
    for(unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]]; 
        meshes.push_back(processMesh(mesh, scene));			
    }
    // then do the same for each of its children
    for(unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNode(node->mChildren[i], scene);
    }
}

Mesh CustomModel::processMesh(aiMesh *mesh, const aiScene *scene) {
    std::vector<Vertex> verticies;

    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex v;
        v.Position.x = mesh->mVertices[i].x;
        v.Position.y = mesh->mVertices[i].y;
        v.Position.z = mesh->mVertices[i].z;

        v.Normal.x = mesh->mNormals[i].x;
        v.Normal.y = mesh->mNormals[i].y;
        v.Normal.z = mesh->mNormals[i].z;

        verticies.push_back(v);
    }

    /*for (Vertex v: verticies) {*/
    /*    std::cout << "(" << v.Position.x << ", " << v.Position.y << ", " << v.Position.z << ")" << std::endl;*/
    /*}*/

    std::vector<unsigned int> indicies;

    std::cout << "Num faces: " << mesh->mNumFaces << std::endl;
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];

        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            indicies.push_back(face.mIndices[j]);
        }
    }
    /*for (unsigned int i: indicies) {*/
    /*    std::cout << "Index: " << i << std::endl;*/
    /*}*/

    Mesh out = Mesh(verticies, indicies);

    return out;
}

void CustomModel::Render(Program* program, glm::vec3 pos, float xrot, float yrot) {
    for (Mesh t : meshes) {
        t.Draw(program, pos, xrot, yrot);
    }
}

void CustomModel::Render() {
    for (Mesh t : meshes) {
        t.Draw();
    }
}
