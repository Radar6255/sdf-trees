#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Program.h"

class Camera {
public:
    Camera(Program* program);

    // This function is to update the transformation matricies
    void Update(GLFWwindow* window);
private:
    int modelLoc, perspectiveLoc, viewLoc;
};
