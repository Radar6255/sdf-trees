#pragma once

#include "CustomModel.h"
#include <noise/module/perlin.h>
#include <vector>
#define TERRAIN_LENGTH 100
#define TERRAIN_WIDTH TERRAIN_LENGTH
#define RENDER_DATA_BUFFERS 1
#define STEP 0.01
#define HEIGHT 20.0f

#define NUM_POINTS TERRAIN_WIDTH * TERRAIN_LENGTH * 2

#include "glad/glad.h"
#include "Vertex.h"
#include "../GameState.h"
#include "../engine/Program.h"
#include <glm/ext/vector_float3.hpp>

class Terrain {
public:
    Terrain(GameState* state);
    void Render(Program* program);
    void Update();
    void UpdateTerrain();
    void setUpdateSize(float alter);
    float alterSize = 0.005f;
    float treeChanceThresh = 4.0f;
    unsigned char treeLife = 16;
private:
    GLuint VAO;
    Vertex terrainHeightMap[NUM_POINTS];
    unsigned char treeHist[NUM_POINTS];
    GLuint *buffers;
    float alter;
    GameState* state;
    CustomModel* tree;

    std::vector<glm::vec3> treeList;

    float generateTree(int x, int curY, int i, float alter, noise::module::Perlin perlinNoise);
};
