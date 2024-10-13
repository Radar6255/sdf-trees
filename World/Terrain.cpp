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
            int startIndex = x * TERRAIN_LENGTH * 6 + 6 * y;

            // First triangle
            terrainHeightMap[startIndex].Position[0] = x;
            terrainHeightMap[startIndex].Position[1] = HEIGHT * perlinNoise.GetValue(STEP * x, STEP * y, this->alter);
            terrainHeightMap[startIndex].Position[2] = y;

            terrainHeightMap[startIndex + 1].Position[0] = x + 1;
            terrainHeightMap[startIndex + 1].Position[1] = HEIGHT * perlinNoise.GetValue(STEP * (x + 1), STEP * y, this->alter);
            terrainHeightMap[startIndex + 1].Position[2] = y;

            terrainHeightMap[startIndex + 2].Position[0] = x + 1;
            terrainHeightMap[startIndex + 2].Position[1] = HEIGHT * perlinNoise.GetValue((x + 1) * STEP, (y + 1) * STEP, this->alter);
            terrainHeightMap[startIndex + 2].Position[2] = y + 1;

            glm::vec3 b = {
                1,
                -terrainHeightMap[startIndex].Position[1] + terrainHeightMap[startIndex + 1].Position[1],
                0
            };
            glm::vec3 a = {
                1,
                -terrainHeightMap[startIndex + 1].Position[1] + terrainHeightMap[startIndex + 2].Position[1],
                1
            };
            glm::vec3 normal = {
                a[1] * b[2] - a[2] * b[1],
                a[2] * b[0] - a[0] * b[2],
                a[0] * b[1] - a[1] * b[0]
            };

            terrainHeightMap[startIndex].Normal = normal;
            terrainHeightMap[startIndex + 1].Normal = normal;
            terrainHeightMap[startIndex + 2].Normal = normal;

            // Second triangle
            terrainHeightMap[startIndex + 3].Position[0] = x;
            terrainHeightMap[startIndex + 3].Position[1] = HEIGHT * perlinNoise.GetValue(x * STEP, y * STEP, this->alter);
            terrainHeightMap[startIndex + 3].Position[2] = y;

            terrainHeightMap[startIndex + 4].Position[0] = x;
            terrainHeightMap[startIndex + 4].Position[1] = HEIGHT * perlinNoise.GetValue(x * STEP, (y + 1) * STEP, this->alter);
            terrainHeightMap[startIndex + 4].Position[2] = y + 1;

            terrainHeightMap[startIndex + 5].Position[0] = x + 1;
            terrainHeightMap[startIndex + 5].Position[1] = HEIGHT * perlinNoise.GetValue((x + 1) * STEP, (y + 1) * STEP, this->alter);
            terrainHeightMap[startIndex + 5].Position[2] = y + 1;

            a = {
                0,
                -terrainHeightMap[startIndex + 3].Position[1] + terrainHeightMap[startIndex + 4].Position[1],
                1
            };
            b = {
                1,
                -terrainHeightMap[startIndex + 4].Position[1] + terrainHeightMap[startIndex + 5].Position[1],
                1
            };

            normal = {
                (a[1] * b[2] - a[2] * b[1]),
                (a[2] * b[0] - a[0] * b[2]),
                (a[0] * b[1] - a[1] * b[0])
            };

            terrainHeightMap[startIndex + 3].Normal = normal;
            terrainHeightMap[startIndex + 4].Normal = normal;
            terrainHeightMap[startIndex + 5].Normal = normal;
        }
    }
}

void Terrain::Render() {
    glBindVertexArray(this->VAO);
    glDrawArrays(GL_TRIANGLES, 0, NUM_POINTS);
}
