#pragma once

#include "CustomModel.h"
#include <mutex>
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
    Terrain(GameState* state, Shaders* shaders, int startX, int startY, int width, int length);
    void Render(Shaders* shaders);
    void Update();
    void UpdateTerrain();
    void setUpdateSize(float alter);
    void TreeGeneration(Shaders* shaders, int iter, int iterMax);

    float alterSize = 0.005f;
    float treeChanceThresh = 0.4f;
private:
    Shaders* shaders;

    unsigned long terrainHeightMapSize;
    unsigned long arrayLength;

    int startX, startY, width, length;
    GLuint VAO;
    GLuint EBO;

    std::mutex terrainTreeSync;
    bool treeUpdate;


    Vertex *terrainHeightMap;
    /*Vertex terrainHeightMap[NUM_POINTS];*/
    GLuint *buffers;
    float alter;
    GameState* state;

    short currRenderBuffer;
    short treeGenStep;

    // Tree stuff
    GLuint treeDataBuff;
    GLuint treeIndiciesBuff[2];
    GLuint treeVerticiesBuff[2];
    GLuint treeIndiciesCounterBuff;
    GLuint treeVerticiesCounterBuff;

    GLuint currIndiciesCount;

    GLuint compVao[2];
    GLuint treeVao[2];

    CustomModel* tree;
    std::vector<CustomModel*> trees;

    std::vector<TreeDetails> treeList;
    std::vector<float> treeDetails;
    uint treeCount;

    float generateTree(int x, int curY, int i, float alter, noise::module::Perlin perlinNoise);
    void TreeInit();
};
