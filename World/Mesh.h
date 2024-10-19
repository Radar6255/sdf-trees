#include <vector>
#include "Vertex.h"
#include "glad/glad.h"

#define MESH_BUFFERS 2

class Mesh {
    public:
        Mesh(std::vector<Vertex> verticies, std::vector<unsigned int> indicies);
        void Draw();
    private:
        std::vector<Vertex> verticies;
        std::vector<unsigned int> indicies;

        GLuint VAO;
        GLuint VBO;
        GLuint EBO;
};
