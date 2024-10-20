#include <cmath>
#include <cstdlib>
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/ext/vector_float3.hpp>
#include <iostream>
#include <noise/noise.h>
#include "Terrain.h"
#include "CustomModel.h"


void printPoint(float* array){
    std::cout << "(" << array[0] << ", " << array[1] << ", " << array[2] << ")";
}

noise::module::Perlin treeNoise;
Terrain::Terrain(GameState* state) {
    /*this->tree = new CustomModel("assets/models/tree.obj");*/
    this->tree = new CustomModel("assets/models/tree2.obj");

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
    std::vector<glm::vec3> treeListNew;
    this->alter += this->alterSize;

    // Starting by creating the mesh
    noise::module::Perlin perlinNoise;
    int i = 0;
    int treeDensity = 4;

    for (int x = 0; x < TERRAIN_LENGTH; x++) {
        for (int y = 0; y < TERRAIN_WIDTH; y++) {
            bool genTree = false;
            if (x % treeDensity == 0 && y % treeDensity == 0) {
                genTree = true;
            }

            int curY = x % 2 == 0 ? y : TERRAIN_WIDTH - 1 - y;
            float diffStep = 0.01;
            glm::vec3 a, b, normal;
            double dydx, dydz;

            if ((!(curY == TERRAIN_WIDTH && x % 2 == 1) && !(curY == TERRAIN_WIDTH - 1 && x % 2 == 1) && !(curY == 0 && x % 2 == 0)) || x == 0) {
                // First triangle
                terrainHeightMap[i].Position[0] = x;
                terrainHeightMap[i].Position[1] = HEIGHT * perlinNoise.GetValue(STEP * x, STEP * curY, this->alter);
                terrainHeightMap[i].Position[2] = curY;

                /*std::cout << "Point: (" << x << ", " << terrainHeightMap[i].Position[1] << ", " << curY << std::endl;*/

                // Need to figure out the slope in x
                dydx = terrainHeightMap[i].Position[1] - HEIGHT * perlinNoise.GetValue(STEP * x + diffStep * STEP, STEP * curY, this->alter);
                dydz = terrainHeightMap[i].Position[1] - HEIGHT * perlinNoise.GetValue(STEP * x, STEP * curY + diffStep * STEP, this->alter);

                b = {
                    STEP * diffStep,
                    dydx,
                    0
                };
                a = {
                    0,
                    dydz,
                    STEP * diffStep
                };
                normal = {
                    a[1] * b[2] - a[2] * b[1],
                    a[2] * b[0] - a[0] * b[2],
                    a[0] * b[1] - a[1] * b[0]
                };

                terrainHeightMap[i].Normal = normal;

                /*float treeChance = generateTree(x, curY, i, this->alter, perlinNoise);*/
                /*if (genTree && treeChance > treeChanceThresh) {*/
                /*    terrainHeightMap[i].Normal = {0.0, 1.0, 0.0};*/
                    /*treeListNew.push_back({x, terrainHeightMap[i].Position[1], curY});*/
                /*}*/
                i++;
            }

            // Second triangle
            terrainHeightMap[i].Position[0] = x + 1;
            terrainHeightMap[i].Position[1] = HEIGHT * perlinNoise.GetValue(STEP * (x + 1), STEP * curY, this->alter);
            /*terrainHeightMap[i].Position[1] = 0;*/
            terrainHeightMap[i].Position[2] = curY;
            /*std::cout << "Point: (" << x + 1 << ", " << terrainHeightMap[i].Position[1] << ", " << curY << std::endl;*/

            dydx = terrainHeightMap[i].Position[1] - HEIGHT * perlinNoise.GetValue(STEP * (x + 1) + diffStep * STEP, STEP * curY, this->alter);
            dydz = terrainHeightMap[i].Position[1] - HEIGHT * perlinNoise.GetValue(STEP * (x + 1), STEP * curY + diffStep * STEP, this->alter);

            b = {
                STEP * diffStep,
                dydx,
                0
            };
            a = {
                0,
                dydz,
                STEP * diffStep
            };
            normal = {
                a[1] * b[2] - a[2] * b[1],
                a[2] * b[0] - a[0] * b[2],
                a[0] * b[1] - a[1] * b[0]
            };

            terrainHeightMap[i].Normal = normal;
            /*terrainHeightMap[i].Normal = {1.0, 0.0, 0.0};*/
            if (genTree && generateTree(x + 1, curY, i, this->alter, perlinNoise) > treeChanceThresh) {
                /*terrainHeightMap[i].Normal = {0.0, 1.0, 0.0};*/

                float treeX, treeY;
                float theta = treeNoise.GetValue(50 * STEP * x + 1, 50 * STEP * curY, alter);
                float spiral = 5.0 * treeNoise.GetValue(STEP * x + 1, STEP * curY, alter + STEP * 5) + 0.1 * theta;
                treeX = spiral * cos(theta) + x + 1;
                treeY = spiral * sin(theta) + curY;
                float treeHeight = HEIGHT * perlinNoise.GetValue(STEP * treeX, STEP * treeY, this->alter);

                /*std::cout << "(" << spiral * cos(theta) << ", " << spiral * sin(theta) << ")" <<*/
                /*    ", (" << treeX << ", " << treeY << ")"<< std::endl;*/

                treeListNew.push_back({treeX, treeHeight, treeY});
            }

            i++;
        }
    }

    treeList = treeListNew;
}

float Terrain::generateTree(int x, int curY, int i, float alter, noise::module::Perlin perlinNoise) {
    /*return (-terrainHeightMap[i].Position[1] + HEIGHT * perlinNoise.GetValue(STEP * x, STEP * curY, alter + 5 * STEP))*/
    /*    * treeNoise.GetValue(10 * STEP * x, 10 * STEP * curY, alter);*/
    /*return treeNoise.GetValue(10 * STEP * x, 10 * STEP * curY, alter);*/

    if (terrainHeightMap[i].Position[1] > HEIGHT * perlinNoise.GetValue(STEP * x, STEP * curY, alter + 5 * STEP)) {
        return treeNoise.GetValue(10 * STEP * x, 10 * STEP * curY, alter);
    }
    return 0;
        // If there in 5 steps the height is higher than current color green
        /*terrainHeightMap[i].Normal.y = 1.0;*/
    /*    return true;*/
    /*}*/
    /**/
    /*return false;*/
}

void Terrain::Render(Program* program) {
    glBindVertexArray(this->VAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, NUM_POINTS);

    for (glm::vec3 pos : treeList) {
        // TODO Need to draw a tree here at a specific point
        /*std::cout << "Tree: (" << pos.x << ", " << pos.y << ", " << pos.z << ")" << std::endl;*/
        this->tree->Render(program, pos);
    }
}
