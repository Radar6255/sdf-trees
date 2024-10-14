#include <glm/ext/quaternion_geometric.hpp>
#include <glm/ext/vector_float3.hpp>
#include <iostream>
#include <noise/noise.h>
#include "Terrain.h"


void printPoint(float* array){
    std::cout << "(" << array[0] << ", " << array[1] << ", " << array[2] << ")";
}

Terrain::Terrain(GameState* state) {
    this->state =state;
    this->alter = 0.0f;

    this->UpdateTerrain();
    
    // Start VAO creation, this should happen in the object that we are trying to render
    // TODO replace with CPP methods
    this->buffers = (GLuint *) malloc(sizeof(GLuint *) * RENDER_DATA_BUFFERS);
    /*glCreateBuffers(RENDER_DATA_BUFFERS, buffers);*/
    GLuint VAO;

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(RENDER_DATA_BUFFERS, buffers);

    // Dealing with verticies
    glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(this->terrainHeightMap), this->terrainHeightMap, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Clearing globals
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    // End VAO creation

    this->VAO = VAO;
};

void Terrain::Update() {
    glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(terrainHeightMap), this->terrainHeightMap);
}

void Terrain::setUpdateSize(float alter) {
    this->alter = alter;
}

void Terrain::UpdateTerrain() {
    this->alter+= this->alterSize;

    // Starting by creating the mesh
    noise::module::Perlin perlinNoise;

    for (int x = 0; x < TERRAIN_LENGTH; x++) {
        for (int y = 0; y < TERRAIN_WIDTH; y++) {
            int startIndex = x * TERRAIN_LENGTH * 2 + 2 * y;
            int curY = x % 2 == 0 ? y : TERRAIN_WIDTH - y;

            // First triangle
            terrainHeightMap[startIndex].Position[0] = x;
            terrainHeightMap[startIndex].Position[1] = HEIGHT * perlinNoise.GetValue(STEP * x, STEP * curY, this->alter);
            terrainHeightMap[startIndex].Position[2] = curY;

            float diffStep = 0.01;

            // Need to figure out the slope in x
            double dydx = terrainHeightMap[startIndex].Position[1] - HEIGHT * perlinNoise.GetValue(STEP * x + diffStep * STEP, STEP * curY, this->alter);
            double dydz = terrainHeightMap[startIndex].Position[1] - HEIGHT * perlinNoise.GetValue(STEP * x, STEP * curY + diffStep * STEP, this->alter);

            glm::vec3 b = {
                STEP * diffStep,
                dydx,
                0
            };
            glm::vec3 a = {
                0,
                dydz,
                STEP * diffStep
            };
            glm::vec3 normal = {
                a[1] * b[2] - a[2] * b[1],
                a[2] * b[0] - a[0] * b[2],
                a[0] * b[1] - a[1] * b[0]
            };

            terrainHeightMap[startIndex].Normal = normal;

            // Second triangle
            terrainHeightMap[startIndex + 1].Position[0] = x + 1;
            terrainHeightMap[startIndex + 1].Position[1] = HEIGHT * perlinNoise.GetValue(STEP * (x + 1), STEP * (curY + 1), this->alter);
            terrainHeightMap[startIndex + 1].Position[2] = curY;

            dydx = terrainHeightMap[startIndex + 1].Position[1] - HEIGHT * perlinNoise.GetValue(STEP * (x + 1) + diffStep * STEP, STEP * curY, this->alter);
            dydz = terrainHeightMap[startIndex + 1].Position[1] - HEIGHT * perlinNoise.GetValue(STEP * (x + 1), STEP * curY + diffStep * STEP, this->alter);

            b = {
                STEP * diffStep,
                dydx,
                0
            };
            a = {
                0,
                dydx,
                STEP * diffStep
            };
            normal = {
                a[1] * b[2] - a[2] * b[1],
                a[2] * b[0] - a[0] * b[2],
                a[0] * b[1] - a[1] * b[0]
            };

            terrainHeightMap[startIndex + 1].Normal = normal;
        }
    }
}

void Terrain::Render() {
    glBindVertexArray(this->VAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, NUM_POINTS);
}
