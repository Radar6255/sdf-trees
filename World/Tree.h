#pragma once
#include "../engine/Program.h"
#include <glm/ext/vector_float3.hpp>
#include <vector>

class Leaf {
public:
    glm::vec3 pos;
    bool reached = false;
};

class Branch {
public:
    glm::vec3 root;
    glm::vec3 dir;
    glm::vec3 forceDirection;
    Branch* parent;
    std::vector<Branch*> children;
    int directionCount = 0;
    Branch* newChild();
private:
    void reset();
};

class Tree {
public:
    Tree(int numPoints);
    void Render(Shaders* shaders);
    glm::vec3* GetBranches();
private:
    std::vector<Branch*> branches;
    std::vector<Leaf> leaves;
    bool Grow();
    Branch root;
};
