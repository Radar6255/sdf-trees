#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/ext/vector_float3.hpp>

#include "Program.h"

class Camera {
public:
    Camera(Program* program);

    // This function is to update the transformation matricies
    void Update(GLFWwindow* window);

    void HandleKeypress(GLFWwindow* window, int key, int scancode, int action, int mods);
private:
    int modelLoc, perspectiveLoc, viewLoc;
    glm::vec3 pos;
};
