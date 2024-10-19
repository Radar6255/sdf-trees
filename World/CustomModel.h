#pragma once

#include "Mesh.h"
#include "glad/glad.h"
#include <assimp/mesh.h>
#include <assimp/scene.h>
#include <vector>

class CustomModel {
    public:
        CustomModel(const char * location);
        void Render();
    private:
        void processNode(aiNode *node, const aiScene *scene);
        Mesh processMesh(aiMesh *mesh, const aiScene *scene);
        GLuint VAO;
        std::vector<Mesh> meshes;
};