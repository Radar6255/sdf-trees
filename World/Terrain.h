#pragma once

#include "CustomModel.h"
#include <noise/module/perlin.h>
#include <vector>
#define TERRAIN_LENGTH 150
#define TERRAIN_WIDTH TERRAIN_LENGTH
#define RENDER_DATA_BUFFERS 1
#define STEP 0.01
#define HEIGHT 20.0f

#define NUM_POINTS TERRAIN_WIDTH * TERRAIN_LENGTH * 2 - TERRAIN_LENGTH + 1

#include "glad/glad.h"
#include "Vertex.h"
#include "../GameState.h"
#include "../engine/Program.h"
#include <glm/ext/vector_float3.hpp>

/*= {*/
/*    "assets/models/tree2.obj",*/
/*    "assets/models/tree_b.obj",*/
/*    "assets/models/tree_c.obj"*/
/*};*/

class TreeDetails {
public:
    glm::vec3 pos;
    float xrot;
    float yrot;
    int variation;
};

class Terrain {
public:
    Terrain(GameState* state);
    void Render(Program* program);
    void Update();
    void UpdateTerrain();
    void setUpdateSize(float alter);
    float alterSize = 0.005f;
    float treeChanceThresh = 0.4f;
private:
    GLuint VAO;
    Vertex terrainHeightMap[NUM_POINTS];
    GLuint *buffers;
    float alter;
    GameState* state;
    CustomModel* tree;
    std::vector<CustomModel*> trees;

    std::vector<TreeDetails> treeList;

    float generateTree(int x, int curY, int i, float alter, noise::module::Perlin perlinNoise);
};
