#include <cmath>
#include <cstdlib>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <noise/noise.h>
#include <sys/types.h>
#include "Terrain.h"
#include "CustomModel.h"
#include "Tree.h"
#include "Vertex.h"


const char *treeModels[] = {
    "assets/models/tree_a.obj",
    "assets/models/tree_b.obj",
    "assets/models/tree_c.obj"
};

void printPoint(float* array){
    std::cout << "(" << array[0] << ", " << array[1] << ", " << array[2] << ")";
}

uint* generateIndicies(uint *out, int width, int length) {
    uint i = 0;
    for (int y = 0; y < width - 1; y++) {
        uint cur = width * (y + 1);
        uint last = width * y;

        for (int x = 0; x < length; x++) {
            out[i] = cur;
            cur++;
            i++;

            out[i] = last;
            last++;
            i++;
        }

        // This is the primitive reset
        out[i] = 65535;
        i++;
    }

    return out;
}

noise::module::Perlin treeNoise;
Terrain::Terrain(GameState* state, Shaders* shaders, int startX, int startY, int width, int length) {
    this->shaders = shaders;
    this->startX = startX;
    this->startY = startY;
    this->width = width + 1;
    this->length = length + 1;

    long numPoints = this->width * this->length;
    this->terrainHeightMap = new Vertex[numPoints];
    this->terrainHeightMapSize = sizeof(Vertex) * numPoints;

    /*this->tree = new CustomModel("assets/models/tree.obj");*/
    /*this->tree = new CustomModel("assets/models/tree2.obj");*/
    this->tree = new CustomModel("assets/models/tree_c.obj");
    for (const char* model : treeModels){
        this->trees.push_back(new CustomModel(model));
    }
    /*this->tree = new CustomModel("assets/models/tree.stl");*/

    this->state =state;
    this->alter = 0.0f;

    this->UpdateTerrain();
    
    // Start VAO creation, this should happen in the object that we are trying to render
    // TODO replace with CPP methods
    this->buffers = (GLuint *) malloc(sizeof(GLuint *) * RENDER_DATA_BUFFERS);
    /*glCreateBuffers(RENDER_DATA_BUFFERS, buffers);*/

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // Doing primitive restart
    glEnable(GL_PRIMITIVE_RESTART);
    glPrimitiveRestartIndex(65535);

    glGenBuffers(RENDER_DATA_BUFFERS, buffers);
    glGenBuffers(1, &this->EBO);

    this->arrayLength = 2 * this->width * this->length;

    uint *elementArray = new uint[arrayLength];

    generateIndicies(elementArray, this->width, this->length);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, arrayLength * sizeof(uint), elementArray, GL_STATIC_DRAW);

    // Dealing with verticies
    glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
    glBufferData(GL_ARRAY_BUFFER, this->terrainHeightMapSize, this->terrainHeightMap, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Clearing globals
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    // End VAO creation

    // Initializing all of the stuff for the trees
    this->TreeInit();
}

void Terrain::TreeInit() {
    uint maxIndicies = 4000000;
    uint maxVerticies = 1000000;
    currRenderBuffer = 0;
    treeGenStep = 0;
    treeUpdate = false;

    glUseProgram(shaders->shaderList[COMP_SHADER]->program);

    glGenBuffers(1, &treeDataBuff);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, treeDataBuff);

    // TODO Need to see how big I want to make this
    // Should be able to calculate roughly based off of the number of trees
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float) * 200000 * 2, NULL, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    glGenBuffers(2, treeIndiciesBuff);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, treeIndiciesBuff[0]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint) * maxIndicies, NULL, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, treeIndiciesBuff[1]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint) * maxIndicies, NULL, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    glGenBuffers(2, treeVerticiesBuff);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, treeVerticiesBuff[0]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec3) * maxVerticies, NULL, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, treeVerticiesBuff[1]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec3) * maxVerticies, NULL, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);


    // Setting up counters
    GLuint zero = 0;

    glGenBuffers(1, &this->treeVerticiesCounterBuff);
    std::cout << "Tree verts " << treeVerticiesCounterBuff << std::endl;
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, treeVerticiesCounterBuff);
    glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), &zero, GL_DYNAMIC_COPY);
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);

    glGenBuffers(1, &this->treeIndiciesCounterBuff);
    std::cout << "Tree indicies " << treeIndiciesCounterBuff << std::endl;
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, treeIndiciesCounterBuff);
    glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), &zero, GL_DYNAMIC_COPY);
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);

    // Here I am setting up VAO stuff
    glGenVertexArrays(2, compVao);

    /*glBindVertexArray(compVao[0]);*/
    /*glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 4, treeVerticiesCounterBuff);*/
    /*glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 5, treeIndiciesCounterBuff);*/
    /*glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, treeIndiciesBuff[0]);*/
    /*glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, treeVerticiesBuff[0]);*/
    /*glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, treeDataBuff);*/

    /*glBindVertexArray(compVao[1]);*/
    /*glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 4, treeVerticiesCounterBuff);*/
    /*glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 5, treeIndiciesCounterBuff);*/
    /*glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, treeIndiciesBuff[1]);*/
    /*glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, treeVerticiesBuff[1]);*/
    /*glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, treeDataBuff);*/

    // Setting up the tree rendering
    glUseProgram(shaders->shaderList[TEST_SHADER]->program);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    glBindVertexArray(0);
}

void Terrain::Update() {
    glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, this->terrainHeightMapSize, this->terrainHeightMap);
}

void Terrain::setUpdateSize(float alter) {
    this->alter = alter;
}

void Terrain::UpdateTerrain() {
    uint treeCountCurr = 0;
    std::vector<float> treeDetailsNew;
    this->alter += this->alterSize;

    // Starting by creating the mesh
    noise::module::Perlin perlinNoise;
    noise::module::Perlin biomeNoise;
    int i = 0;
    int treeDensity = 4;

    for (int x = 0; x < this->length; x++) {
        for (int y = 0; y < this->width; y++) {
            int curX = x + this->startX;
            int curY = y + this->startY;

            bool genTree = false;
            if (curX % treeDensity == 0 && curY % treeDensity == 0) {
                genTree = true;
            }

            float diffStep = 0.01;
            glm::vec3 a, b, normal;
            double dydx, dydz;

            /*float height = HEIGHT * biomeNoise.GetValue(STEP * curX, STEP * curY, this->alter);*/
            float height = HEIGHT;

            // First triangle
            terrainHeightMap[i].Position[0] = curX;
            terrainHeightMap[i].Position[1] = HEIGHT * perlinNoise.GetValue(STEP * curX, STEP * curY, this->alter);
            terrainHeightMap[i].Position[2] = curY;

            /*std::cout << "Point: (" << x << ", " << terrainHeightMap[i].Position[1] << ", " << curY << std::endl;*/
            /*float heightDydx = HEIGHT * biomeNoise.GetValue(STEP * curX + diffStep, STEP * curY, this->alter);*/
            /*float heightDydz = HEIGHT * biomeNoise.GetValue(STEP * curX, STEP * curY + diffStep, this->alter);*/
            float heightDydx = HEIGHT;
            float heightDydz = HEIGHT;

            // Need to figure out the slope in x
            dydx = terrainHeightMap[i].Position[1] - heightDydx * perlinNoise.GetValue(STEP * curX + diffStep * STEP, STEP * curY, this->alter);
            dydz = terrainHeightMap[i].Position[1] - heightDydz * perlinNoise.GetValue(STEP * curX, STEP * curY + diffStep * STEP, this->alter);

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
            if (genTree && generateTree(curX, curY, i, this->alter, perlinNoise) > treeChanceThresh) {
                /*terrainHeightMap[i].Normal = {0.0, 1.0, 0.0};*/

                float treeX, treeY;
                float theta = treeNoise.GetValue(50 * STEP * (curX), 50 * STEP * curY, alter);
                float spiral = 5.0 * treeNoise.GetValue(STEP * (curX), STEP * curY, alter + STEP * 5) + 0.1 * theta;
                treeX = spiral * cos(theta) + curX;
                treeY = spiral * sin(theta) + curY;
                float treeHeight = HEIGHT * perlinNoise.GetValue(STEP * treeX, STEP * treeY, this->alter);

                /*std::cout << "(" << spiral * cos(theta) << ", " << spiral * sin(theta) << ")" <<*/
                /*    ", (" << treeX << ", " << treeY << ")"<< std::endl;*/

                TreeDetails td;
                td.pos = {treeX, treeHeight, treeY};
                td.xrot = 0.1 * treeNoise.GetValue(STEP * (curX), STEP * curY, 0.1 * alter);
                td.yrot = 0;
                td.variation = (int) round(10 * generateTree(curX, curY, i, this->alter, perlinNoise)) % this->trees.size();

                Tree t(50);
                std::vector<glm::vec3>* branches = t.GetBranches();
                /*std::cout << "Num branches: " << branches->size() << std::endl;*/

                treeDetailsNew.push_back(branches->size() * 3 + 4);
                treeDetailsNew.push_back(treeX);
                treeDetailsNew.push_back(treeHeight);
                treeDetailsNew.push_back(treeY);

                for (glm::vec3 bPos : *branches) {
                    treeDetailsNew.push_back(bPos.x);
                    treeDetailsNew.push_back(bPos.y);
                    treeDetailsNew.push_back(bPos.z);
                }
                treeCountCurr++;
            }
            /*terrainHeightMap[i].Normal.x = 1;*/
            i++;
        }
    }

    /*treeList = treeListNew;*/
    // TODO This may need to change
    // Also this is doing a copy that I should probably avoid
    /*if (this->treeDetails.size() == 0) {*/
        this->treeDetails = treeDetailsNew;
        this->treeCount = treeCountCurr;
    /*}*/

    if (this->treeCount > 0) {
        treeUpdate = true;

        // Here we are waiting for the trees to be updated
        terrainTreeSync.lock();
    }
}

void Terrain::TreeGeneration(Shaders* shaders, int iter, int iterMax) {
    glUseProgram(shaders->shaderList[COMP_SHADER]->program);
    GLuint start = iter * floor(treeCount / iterMax);
    glUniform1i(7, start);

    /*glBindVertexArray(compVao[(currRenderBuffer + 1) % 2]);*/
    glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 4, treeVerticiesCounterBuff);
    glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 5, treeIndiciesCounterBuff);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, treeIndiciesBuff[(currRenderBuffer + 1) % 2]);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, treeVerticiesBuff[(currRenderBuffer + 1) % 2]);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, treeDataBuff);
    std::cout << "Using buffer for comp: " << (currRenderBuffer + 1) % 2 << std::endl;

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, treeDataBuff);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(float) * treeDetails.size(), treeDetails.data());
    /*std::cout << "Tree details size " << treeDetails.size()  << std::endl;*/

    if (iter == 0) {
        nextIndiciesCount = 0;
        nextVerticiesCount = 0;

        glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, treeIndiciesCounterBuff);
        glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &nextIndiciesCount);

        glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, treeVerticiesCounterBuff);
        glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &nextVerticiesCount);
    }

    /*glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, treeIndiciesCounterBuff);*/
    /*glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &nextIndiciesCount);*/
    /**/
    /*glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, treeVerticiesCounterBuff);*/
    /*glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &nextVerticiesCount);*/


    GLuint treeUpdateCount = floor(treeCount / iterMax);
    if (iter == iterMax - 1) {
        treeUpdateCount = treeCount - start;
    }
    std::cout << "Updating " << treeUpdateCount << " trees..." << std::endl;

    /*std::chrono::time_point<std::chrono::high_resolution_clock> startFrame = std::chrono::high_resolution_clock::now();*/

    // Actually dispatching the compute shader
    glDispatchCompute(5, 10 * treeUpdateCount, 5);
    /*glMemoryBarrier(GL_ATOMIC_COUNTER_BARRIER_BIT);*/
    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    /*glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, treeIndiciesCounterBuff);*/
    /*glGetBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &currIndiciesCount);*/
    /*std::cout << "Got " << nextIndiciesCount << " indicies" << std::endl;*/
    /*uint t = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - startFrame).count();*/
    /*std::cout << "Took " << t << " microseconds" << std::endl;*/
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

void Terrain::Render(Shaders* shaders) {
    glm::mat4 model = glm::mat4(1);

    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "T");

    glUseProgram(shaders->shaderList[TERRAIN_SHADER]->program);
    GLuint modelLoc = glGetUniformLocation(shaders->shaderList[TERRAIN_SHADER]->program, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glBindVertexArray(this->VAO);
    /*glDrawArrays(GL_TRIANGLE_STRIP, 0, this->terrainHeightMapSize / sizeof(Vertex));*/
    glDrawElements(GL_TRIANGLE_STRIP, this->arrayLength - this->length, GL_UNSIGNED_INT, nullptr);

    //for (TreeDetails td : treeList) {
    //    // TODO Need to draw a tree here at a specific point
    //    /*std::cout << "Tree: (" << pos.x << ", " << pos.y << ", " << pos.z << ")" << std::endl;*/
    //    /*this->trees[td.variation]->Render(program, td.pos, td.xrot, td.yrot);*/
    //    this->trees[0]->Render(shaders->shaderList[TREE_SHADER], td.pos, td.xrot, td.yrot);
    //}

    // TODO Here I need to render out the trees
    if (treeUpdate && treeCount > 0) {
        glUseProgram(shaders->shaderList[COMP_SHADER]->program);
        if (treeCount < 5) {
            this->TreeGeneration(shaders, 0, 1);
            /*glMemoryBarrier(GL_ALL_BARRIER_BITS);*/

            /*GLuint verts;*/
            /**/
            /*glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, treeVerticiesCounterBuff);*/
            /*glGetBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &verts);*/

            glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, treeIndiciesCounterBuff);
            glGetBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &currIndiciesCount);

            currRenderBuffer = ++currRenderBuffer % 2;
            /*std::cout << "Indicies <5 " << currIndiciesCount << "==" << verts << std::endl;*/

            treeUpdate = false;
            // Freeing the terrain generation thread since it is done this frame
            terrainTreeSync.unlock();
            treeGenStep = 0;
        } else {
            /*std::cout << treeGenStep << " gen step" << std::endl;*/
            this->TreeGeneration(shaders, treeGenStep, 5);

            /*glMemoryBarrier(GL_ALL_BARRIER_BITS);*/
            /*glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, treeIndiciesCounterBuff);*/
            /*glGetBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &nextIndiciesCount);*/
            /**/
            /*glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, treeVerticiesCounterBuff);*/
            /*glGetBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &nextVerticiesCount);*/
            /**/
            /*std::cout << startX << ", " << startY << ": Next: " << nextIndiciesCount << std::endl;*/
            /*std::cout << startX << ", " << startY << ": Next: " << nextVerticiesCount << std::endl;*/

            /*if (nextVerticiesCount > 65535) {*/
            /*    std::cout << "VERTICIES LARGE!" << std::endl;*/
            /*}*/

            if (treeGenStep == 4) {
                /*glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, treeIndiciesCounterBuff);*/
                /*glGetBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &currIndiciesCount);*/
                glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, treeIndiciesCounterBuff);
                glGetBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &currIndiciesCount);
                /*currIndiciesCount = nextIndiciesCount;*/
                /*std::cout << "Curr indicies " << currIndiciesCount  << std::endl;*/
                currRenderBuffer = ++currRenderBuffer % 2;

                treeUpdate = false;
                // Freeing the terrain generation thread since it is done this frame
                terrainTreeSync.unlock();
            }

            treeGenStep = ++treeGenStep % 5;
        }
    }
    /*glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);*/

    /*std::cout << startX << ", " << startY << ": " << treeCount << std::endl;*/

    if (treeCount) {
        glUseProgram(shaders->shaderList[TEST_SHADER]->program);
        /*model = glm::translate(model, {startX - 100, 0, startY - 100});*/

        GLuint modelLoc = glGetUniformLocation(shaders->shaderList[TEST_SHADER]->program, "model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

        std::cout << currIndiciesCount << std::endl;
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, treeVerticiesBuff[currRenderBuffer]);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, treeIndiciesBuff[currRenderBuffer]);

        /*std::cout << "Using vao for draw: " << currRenderBuffer << std::endl;*/
        glDrawArrays(GL_TRIANGLES, 0, currIndiciesCount);
        /*std::cout << "End of terrain!" << std::endl;*/
    }
    glPopDebugGroup();
}
