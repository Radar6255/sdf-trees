#include <iostream>
#include <noise/noise.h>
#include "Terrain.h"

#define TERRAIN_LENGTH 10
#define TERRAIN_WIDTH 10
#define RENDER_DATA_BUFFERS 1
#define STEP 0.01f
#define HEIGHT 20.0f

#define NUM_POINTS TERRAIN_WIDTH * TERRAIN_LENGTH * 2 * 3

void printPoint(float* array){
    std::cout << "(" << array[0] << ", " << array[1] << ", " << array[2] << ")";
}

Terrain::Terrain() {
    // Starting by creating the mesh
    noise::module::Perlin perlinNoise;

    // We are making 2 triangles with 3 verticies per 1 unit
    float terrainHeightMap[NUM_POINTS][3];

    for (int x = 0; x < TERRAIN_LENGTH; x++) {
        for (int y = 0; y < TERRAIN_WIDTH; y++) {
            terrainHeightMap[x * TERRAIN_LENGTH * 6 + 6 * y][0] = x;
            terrainHeightMap[x * TERRAIN_LENGTH * 6 + 6 * y][1] = HEIGHT * perlinNoise.GetValue(STEP * x, STEP * y, 0.0);
            terrainHeightMap[x * TERRAIN_LENGTH * 6 + 6 * y][2] = y;

            terrainHeightMap[x * TERRAIN_LENGTH * 6 + 6 * y + 1][0] = x + 1;
            terrainHeightMap[x * TERRAIN_LENGTH * 6 + 6 * y + 1][1] = HEIGHT * perlinNoise.GetValue(STEP * (x + 1), STEP * y, 0.0);
            terrainHeightMap[x * TERRAIN_LENGTH * 6 + 6 * y + 1][2] = y;

            terrainHeightMap[x * TERRAIN_LENGTH * 6 + 6 * y + 2][0] = x + 1;
            terrainHeightMap[x * TERRAIN_LENGTH * 6 + 6 * y + 2][1] = HEIGHT * perlinNoise.GetValue((x + 1) * STEP, (y + 1) * STEP, 0.0);
            terrainHeightMap[x * TERRAIN_LENGTH * 6 + 6 * y + 2][2] = y + 1;

            // Second triangle
            terrainHeightMap[x * TERRAIN_LENGTH * 6 + 6 * y + 3][0] = x;
            terrainHeightMap[x * TERRAIN_LENGTH * 6 + 6 * y + 3][1] = HEIGHT * perlinNoise.GetValue(x * STEP, y * STEP, 0.0);
            terrainHeightMap[x * TERRAIN_LENGTH * 6 + 6 * y + 3][2] = y;

            terrainHeightMap[x * TERRAIN_LENGTH * 6 + 6 * y + 4][0] = x;
            terrainHeightMap[x * TERRAIN_LENGTH * 6 + 6 * y + 4][1] = HEIGHT * perlinNoise.GetValue(x * STEP, (y + 1) * STEP, 0.0);
            terrainHeightMap[x * TERRAIN_LENGTH * 6 + 6 * y + 4][2] = y + 1;

            terrainHeightMap[x * TERRAIN_LENGTH * 6 + 6 * y + 5][0] = x + 1;
            terrainHeightMap[x * TERRAIN_LENGTH * 6 + 6 * y + 5][1] = HEIGHT * perlinNoise.GetValue((x + 1) * STEP, (y + 1) * STEP, 0.0);
            terrainHeightMap[x * TERRAIN_LENGTH * 6 + 6 * y + 5][2] = y + 1;
            std::cout << "p1" << x * TERRAIN_LENGTH * 6 + 6 * y << std::endl;
            std::cout << "p1";
            printPoint(terrainHeightMap[x * TERRAIN_LENGTH * 6 + 6 * y]);
            std::cout << std::endl << "p2";

            printPoint(terrainHeightMap[x * TERRAIN_LENGTH * 6 + 6 * y + 1]);
            std::cout << std::endl << "p3";

            printPoint(terrainHeightMap[x * TERRAIN_LENGTH * 6 + 6 * y + 2]);
            std::cout << std::endl << "p4";

            printPoint(terrainHeightMap[x * TERRAIN_LENGTH * 6 + 6 * y + 3]);
            std::cout << std::endl << "p5";

            printPoint(terrainHeightMap[x * TERRAIN_LENGTH * 6 + 6 * y + 4]);
            std::cout << std::endl << "p6";

            printPoint(terrainHeightMap[x * TERRAIN_LENGTH * 6 + 6 * y + 5]);
            std::cout << std::endl;

            std::cout << "Num points" << NUM_POINTS << std::endl;
        }
    }

    
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
    glBufferData(GL_ARRAY_BUFFER, sizeof(terrainHeightMap), terrainHeightMap, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
    glEnableVertexAttribArray(0);

    // Clearing globals
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    // End VAO creation

    this->VAO = VAO;
};

void Terrain::Render() {
    glBindVertexArray(this->VAO);
    glDrawArrays(GL_TRIANGLES, 0, NUM_POINTS);
}
