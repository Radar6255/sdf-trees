#include "Tree.h"
#include <cmath>
#include <cstdlib>
#include <glm/ext/vector_float3.hpp>
#include <glm/geometric.hpp>
#include <iostream>
#include <ostream>

Branch* Branch::newChild() {
    Branch* out = new Branch();
    out->root = this->root + this->dir;
    out->dir = glm::normalize(this->forceDirection * (1 / (float) this->directionCount));
    out->parent = this;
    out->forceDirection = this->dir;
    out->directionCount = 0;

    this->children.push_back(out);
    this->reset();

    return out;
}

void Branch::reset() {
    this->directionCount = 0;
    this->forceDirection = this->dir;
}

float minDist = 3;
float maxDist = 10;

Tree::Tree(int numPoints) {
    // This is the root
    Branch *out = new Branch();

    out->root = {0, 0, 0};
    // Starting the direction of the tree upwards
    out->dir = {0, 1, 0};
    out->forceDirection = out->dir;
    out->parent = NULL;
    out->directionCount = 0;

    this->branches.push_back(out);

    // TODO Here I want to generate the random leaves
    for (int i = 0; i < numPoints; i++) {
        Leaf t;
        int size = 10;
        t.pos = {
            size * (static_cast<float>(rand()) / static_cast<float>(RAND_MAX) -0.5), 
            size * (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)),
            size * (static_cast<float>(rand()) / static_cast<float>(RAND_MAX) - 0.5)
        };
        t.reached = false;


        float x = sqrt(pow(t.pos.x, 2) + pow(t.pos.z, 2));
        if (0.2 * pow(x, 1.2) > t.pos.y) {
            i--;
            continue;
        }
        /*std::cout << t.pos.x << ", " << t.pos.z << "| Dist: " << x << std::endl;*/

        this->leaves.push_back(t);
    }

    int i = 0;
    // Growing the whole tree out
    while(this->Grow() && i < 20) {
        i++;
    }
}

std::vector<glm::vec3>* Tree::GetBranches(){
    /*std::vector<glm::vec3> out = new glm::vec3[this->branches.size() * 2];*/
    std::vector<glm::vec3>* out = new std::vector<glm::vec3>(this->branches.size() * 2);

    glm::vec3 offset = {0, 2, 0};

    int i = 0;
    for (Branch* b: this->branches) {
        out->push_back(b->root + offset);
        out->push_back(b->root + b->dir + offset);
        i += 2;
        /*std::cout << "Branch start: (" << b->root.x << ", " << b->root.y << ", " << b->root.z << ")" << std::endl;*/
        /*std::cout << "Branch end: (" << b->root.x - b->dir.x << ", " << b->root.y - b->dir.y << ", " << b->root.z - b->dir.z << ")" << std::endl;*/
    }

    return out;
}

// Returning whether or not any new branches were made
bool Tree::Grow() {
    // Here we are finding the forces on all of the branches
    for (int l = this->leaves.size() - 1; l >= 0; l--) {
        Branch* closest = NULL;
        float currMinDist = 100000;

        for (int b = 0; b < this->branches.size(); b++) {
            float dist = glm::distance(this->branches[b]->root + this->branches[b]->dir, this->leaves[l].pos);

            if (!closest) {
                closest = this->branches[b];
                currMinDist = dist;
            } else if (dist < minDist) {
                /*std::cout << "Removing leaf at (" << this->leaves[l].pos.x << ", " << this->leaves[l].pos.y << ", " << this->leaves[l].pos.z << ")" << std::endl;*/
                /*std::cout << "Branch: (" << this->branches[b]->root.x << ", " << this->branches[b]->root.y << ", " << this->branches[b]->root.z << ")" << std::endl;*/
                /*std::cout << "Dist: " << dist << std::endl;*/

                // Removing the leaf since a branch already reached it
                this->leaves.erase(this->leaves.begin() + l);
                closest = NULL;
                break;
            } else if (dist > maxDist) {
                // Ignoring the point if it is too far away
            } else if (dist < currMinDist) {
                closest = this->branches[b];
                currMinDist = dist;
            }
        }

        if (!closest) {
            continue;
        }

        // Now we know the closest branch
        // For this closest branch we need to add the force to the current leaf
        glm::vec3 dir = this->leaves[l].pos - closest->root;

        closest->forceDirection += dir;
        closest->directionCount++;
    }

    bool out = false;

    // Here we need to apply the forces to all of the branches to make new branches
    for (int b = this->branches.size() - 1; b >= 0; b--) {
        Branch* branch = this->branches[b];

        if (branch->directionCount == 0) {
            continue;
        }

        /*std::cout << "Direction Count: " << branch->directionCount << std::endl;*/
        /*std::cout << "Dir: (" << branch->dir.x << ", " << branch->dir.y << ", " << branch->dir.z << ")" << std::endl;*/
        /*std::cout << "Root: (" << branch->root.x << ", " << branch->root.y << ", " << branch->root.z << ")" << std::endl;*/

        out = true;

        Branch* child = branch->newChild();
        /*std::cout << "Child Dir: (" << child->dir.x << ", " << child->dir.y << ", " << child->dir.z << ")" << std::endl;*/

        // This creates the new child and resets this branch
        this->branches.push_back(child);
        /*std::cout << "Branches size: " << this->branches.size() << std::endl;*/
    }

    for (int l = 0; l < this->leaves.size(); l++) {
        /*std::cout << "Leaf pos: " << this->leaves[l].pos.x << ", " << this->leaves[l].pos.y << ", " << this->leaves[l].pos.z << ")" << std::endl;*/
    }
    return out;
}

void Tree::Render(Shaders* shaders) {

}
