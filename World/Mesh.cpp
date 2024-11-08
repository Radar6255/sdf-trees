#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/trigonometric.hpp>
#include <vector>
#include "Mesh.h"
#include "Vertex.h"

Mesh::Mesh(std::vector<Vertex> verticies, std::vector<unsigned int> indicies) {
    this->verticies = verticies;
    this->indicies = indicies;

    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    GLuint VAO;

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    /*glGenBuffers(MESH_BUFFERS, buffers);*/

    // Dealing with element indicies
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicies.size() * sizeof(unsigned int), indicies.data(), GL_STATIC_DRAW);

    // Dealing with verticies
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, verticies.size() * sizeof(Vertex), verticies.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, Normal));
    glEnableVertexAttribArray(1);

    // Clearing globals
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    // End VAO creation

    this->VAO = VAO;
}

void Mesh::Draw() {
    glBindVertexArray(this->VAO);
    glDrawElements(GL_TRIANGLES, indicies.size(), GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

/**
 * @brief Function to draw the mesh
 *
 * @param program The program that will be drawing the mesh, used to get uniform locations
 * @param pos The position offset of the object
 * @param xrot The x rotation of the object in degrees
 * @param yrot The y rotation of the object in degrees
 */
void Mesh::Draw(Program* program, glm::vec3 pos, float xrot, float yrot) {
    glm::mat4 model = glm::translate(glm::mat4(1), pos);
    /*model = glm::scale(model, {0.5, 0.5, 0.5});*/

    model = glm::rotate(model, glm::degrees(xrot), {0.0, 1.0, 0.0});
    model = glm::rotate(model, glm::degrees(yrot), {0.0, 0.0, 1.0});

    glUseProgram(program->program);

    GLuint modelLoc = glGetUniformLocation(program->program, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    glBindVertexArray(this->VAO);
    glDrawElements(GL_TRIANGLES, indicies.size(), GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}
