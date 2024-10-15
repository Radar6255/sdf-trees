#pragma once

#define TERRAIN_LENGTH 200
#define TERRAIN_WIDTH TERRAIN_LENGTH
#define RENDER_DATA_BUFFERS 1
#define STEP 0.01
#define HEIGHT 20.0f

#define NUM_POINTS TERRAIN_WIDTH * TERRAIN_LENGTH * 2

#include "glad/glad.h"
#include "../GameState.h"
#include <glm/ext/vector_float3.hpp>

class Vertex {
public:
    glm::vec3 Position;
    glm::vec3 Normal;
};

class Terrain {
public:
    Terrain(GameState* state);
    void Render();
    void Update();
    void UpdateTerrain();
    void setUpdateSize(float alter);
    float alterSize = 0.005f;
private:
    GLuint VAO;
    Vertex terrainHeightMap[NUM_POINTS];
    GLuint *buffers;
    float alter;
    GameState* state;
};
