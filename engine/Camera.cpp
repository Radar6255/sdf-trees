#include "Camera.h"

#include <GLFW/glfw3.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/mat4x4.hpp>
#include <glm/trigonometric.hpp>

Camera::Camera(Program* program) {
    this->modelLoc = glGetUniformLocation(program->program, "model");
    this->viewLoc = glGetUniformLocation(program->program, "view");
    this->perspectiveLoc = glGetUniformLocation(program->program, "projection");
}

// This function is to update the transformation matricies
void Camera::Update(GLFWwindow* window) {
    // Setting up model matrix
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::rotate(model, glm::radians(-55.0f), glm::vec3(1.0f, 0.0f, 0.0f));

    glm::mat4 view = glm::mat4(1.0f);
    view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));

    // Need to get the current size of the window
    // TODO Refactor so this is grabbed only on update
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    // Setting up perspective matrix
    glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);

    glUniformMatrix4fv(this->modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(this->viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(this->perspectiveLoc, 1, GL_FALSE, glm::value_ptr(proj));
}
