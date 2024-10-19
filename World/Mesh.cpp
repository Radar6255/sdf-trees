#include <iostream>
#include <ostream>
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
    /*std::cout << "Index count " << indicies.size() << std::endl;*/
    /*std::cout << "Vertex count " << verticies.size() << std::endl;*/
    glBindVertexArray(this->VAO);
    /*glDrawElements(GL_TRIANGLES, indicies.size(), GL_UNSIGNED_INT, nullptr);*/
    glDrawElements(GL_TRIANGLES, indicies.size(), GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}
