#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/ext/vector_float3.hpp>

#include "Program.h"
#include "../GameState.h"

class Camera {
public:
    Camera(Shaders* shaders, GameState* state);

    // This function is to update the transformation matricies
    void Update(GLFWwindow* window);
    void Tick();

    void HandleKeypress(GLFWwindow* window, int key, int scancode, int action, int mods);
    void HandleMouseMovement(GLFWwindow* window, double xpos, double ypos);
private:
    int modelLoc, perspectiveLoc, viewLoc;
    glm::vec3 pos;
    glm::vec3 cameraFront;

    float xrot;
    float yrot;

    bool w = false;
    bool a = false;
    bool s = false;
    bool d = false;

    bool up = false;
    bool down = false;

    GameState* state;
};
