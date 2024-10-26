#include <glm/ext/vector_float3.hpp>
#include <vector>
#include "Vertex.h"
#include "glad/glad.h"
#include "../engine/Program.h"

#define MESH_BUFFERS 2

class Mesh {
    public:
        Mesh(std::vector<Vertex> verticies, std::vector<unsigned int> indicies);
        void Draw();
        void Draw(Program* program, glm::vec3 pos, float xrot, float yrot);
    private:
        std::vector<Vertex> verticies;
        std::vector<unsigned int> indicies;

        GLuint VAO;
        GLuint VBO;
        GLuint EBO;
};
