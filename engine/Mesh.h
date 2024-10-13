#include <glm/ext/vector_float3.hpp>
#include <vector>

class Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
};

class Mesh {
    public:
        std::vector<Vertex> verticies;
};
