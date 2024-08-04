#include "Cube.h"
#include <GL/gl.h>
#include <cstdlib>
#include <iostream>

#define RENDER_DATA_BUFFERS 1
#define NUM_POINTS 24

static float rectPoints[24][3] = {
    // Front face
    {-0.5f, -0.5f, 0.5f},
    {0.5f, -0.5f, 0.5f},
    {-0.5f, 0.5f, 0.5f},
    {0.5f, -0.5f, 0.5f},
    {0.5f, 0.5f, 0.5f},
    {-0.5f, 0.5f, 0.5f},
    // Back face
    {-0.5f, -0.5f, -0.5f},
    {0.5f, -0.5f, -0.5f},
    {-0.5f, 0.5f, -0.5f},
    {0.5f, -0.5f, -0.5f},
    {0.5f, 0.5f, -0.5f},
    {-0.5f, 0.5f, -0.5f},
    // Top face
    {0.5f, 0.5f, 0.5f},
    {0.5f, 0.5f, -0.5f},
    {-0.5f, 0.5f, 0.5f},
    {-0.5f, 0.5f, -0.5f},
    {0.5f, 0.5f, -0.5f},
    {-0.5f, 0.5f, 0.5f},
    // Bottom face
    {0.5f, -0.5f, 0.5f},
    {0.5f, -0.5f, -0.5f},
    {-0.5f, -0.5f, 0.5f},
    {-0.5f, -0.5f, -0.5f},
    {0.5f, -0.5f, -0.5f},
    {-0.5f, -0.5f, 0.5f},

    // TODO Do the other sides here
};

Cube::Cube() {
    // Start VAO creation, this should happen in the object that we are trying to render
    // TODO replace with CPP methods
    GLuint *buffers = (GLuint *) malloc(sizeof(GLuint *) * RENDER_DATA_BUFFERS);
    /*glCreateBuffers(RENDER_DATA_BUFFERS, buffers);*/
    GLuint VAO;

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(RENDER_DATA_BUFFERS, buffers);

    // Dealing with verticies
    glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(rectPoints), rectPoints, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
    glEnableVertexAttribArray(0);

    // Clearing globals
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    // End VAO creation

    this->VAO = VAO;
}

void Cube::Render() {
    glBindVertexArray(this->VAO);
    glDrawArrays(GL_TRIANGLES, 0, NUM_POINTS);
}
