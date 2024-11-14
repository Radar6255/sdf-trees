#include "Camera.h"
#include "Program.h"

#include <GLFW/glfw3.h>
#include <cmath>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/mat4x4.hpp>
#include <glm/trigonometric.hpp>
#include <iostream>
#include <ostream>

Camera::Camera(Shaders* shaders, GameState* state) {
    this->shaders = shaders;
    this->state = state;
    this->modelLoc = glGetUniformLocation(shaders->shaderList[TERRAIN_SHADER]->program, "model");
    this->viewLoc = glGetUniformLocation(shaders->shaderList[TERRAIN_SHADER]->program, "view");
    this->perspectiveLoc = glGetUniformLocation(shaders->shaderList[TERRAIN_SHADER]->program, "projection");

    this->pos = glm::vec3(0.0f, 10.0f, 0.0f);
    this->xrot = 2.0f;
    this->yrot = -55.0f;

    this->cameraFront = glm::vec3(1.0f);
}

// This function is to update the transformation matricies
void Camera::Update(GLFWwindow* window) {
    this->Tick();
    // TODO Some of these matricies don't need to be updated every frame!
    // TODO Here I should handle key stuff
    // Setting up model matrix
    // TODO Don't think the model matrix should be here at all...
    glm::mat4 model = glm::mat4(1.0f);
    /*model = glm::rotate(model, glm::radians(this->yrot), glm::vec3(1.0f, 0.0f, 0.0f));*/
    /*model = glm::rotate(model, glm::radians(this->xrot), glm::vec3(0.0f, 0.0f, 1.0f));*/

    if (yrot > 89.0f) {
        yrot = 89.0f;
    }
    if (yrot < -89.0f) {
        yrot = -89.0f;
    }
    /*std::cout << "xrot: " << this->xrot << ", yrot: " << this->yrot << std::endl;*/

    glm::vec3 direction;
    direction.x = cos(glm::radians(xrot)) * cos(glm::radians(yrot));
    direction.y = sin(glm::radians(yrot));
    direction.z = sin(glm::radians(xrot)) * cos(glm::radians(yrot));
    cameraFront = glm::normalize(direction);
    /*std::cout << "Cam (" << cameraFront.x << ", " << cameraFront.y << ", " << cameraFront.z << ")" << std::endl;*/

    glm::mat4 view = glm::mat4(1.0f);
    // This is the position of the camera
    /*view = glm::translate(view, this->pos);*/
    view = glm::lookAt(this->pos, this->pos + cameraFront, glm::vec3(0.0f, 1.0f, 0.0f));


    // Need to get the current size of the window
    // TODO Refactor so this is grabbed only on update
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    // Setting up perspective matrix
    glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 1000.0f);

    for (Program* p : this->shaders->shaderList) {
        glUseProgram(p->program);
        glUniformMatrix4fv(this->modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(this->viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(this->perspectiveLoc, 1, GL_FALSE, glm::value_ptr(proj));
    }
}

// TODO Eventually this should move out of the camera and into more of a player class
// That is for later though
void Camera::HandleKeypress(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_W) {
        if (action == GLFW_PRESS) {
            w = true;
        } else if (action == GLFW_RELEASE) {
            w = false;
        }
    }

    if (key == GLFW_KEY_A) {
        if (action == GLFW_PRESS) {
            a = true;
        } else if (action == GLFW_RELEASE) {
            a = false;
        }
    }

    if (key == GLFW_KEY_D) {
        if (action == GLFW_PRESS) {
            d = true;
        } else if (action == GLFW_RELEASE) {
            d = false;
        }
    }

    if (key == GLFW_KEY_S) {
        if (action == GLFW_PRESS) {
            s = true;
        } else if (action == GLFW_RELEASE) {
            s = false;
        }
    }
    if (key == GLFW_KEY_SPACE) {
        if (action == GLFW_PRESS) {
            up = true;
        } else if (action == GLFW_RELEASE) {
            up = false;
        }
    }
    if (key == GLFW_KEY_LEFT_SHIFT) {
        if (action == GLFW_PRESS) {
            down = true;
        } else if (action == GLFW_RELEASE) {
            down = false;
        }
    }
    if (key == GLFW_KEY_ESCAPE) {
        if (action == GLFW_PRESS) {
            if (!this->state->paused) {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                this->state->paused = true;
            } else {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                this->state->paused = false;
            }

        }
    }
}

void Camera::Tick() {
    if (w) {
        // TODO Make this only apply in the x, z planes
        this->pos += 0.1f * cameraFront;
    }

    if (a) {
        this->pos -= 0.1f * glm::normalize(glm::cross(cameraFront, glm::vec3(0.0f, 1.0f, 0.0f)));
    }

    if (d) {
        this->pos += 0.1f * glm::normalize(glm::cross(cameraFront, glm::vec3(0.0f, 1.0f, 0.0f)));
    }

    if (s) {
        // TODO Make this only apply in the x, z planes
        this->pos -= 0.1f * cameraFront;
    }

    if (up) {
        this->pos += 0.1f * glm::vec3(0.0f, 1.0f, 0.0f);
    }
    if (down) {
        this->pos -= 0.1f * glm::vec3(0.0f, 1.0f, 0.0f);
    }
}

double lastXPos = 0;
double lastYPos = 0;

void Camera::HandleMouseMovement(GLFWwindow* window, double xpos, double ypos) {
    if (!this->state->paused) {
        this->xrot -= (lastXPos - xpos) / 100.0;
        this->yrot -= -(lastYPos - ypos) / 100.0;
    }

    lastXPos = xpos;
    lastYPos = ypos;

    /*std::cout << "xrot: " << this->xrot << ", yrot: " << this->yrot << std::endl;*/
}
