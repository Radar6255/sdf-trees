#version 460 core
// In this shader I want to perform native mesh construction of a SDF

layout(local_size_x = 8, local_size_y = 16, local_size_z = 8) in;
int local_x = 8;
int local_y = 16;
int local_z = 8;

// Here I need to get any buffers that I want to read/write to
// Below should be the input for the trees
// Not sure how big to make this...

layout(binding = 4) uniform atomic_uint outVertIndex;
layout(binding = 5) uniform atomic_uint outIndex;
layout(location = 7) uniform int treeOffset;
//layout(std430, binding = 3) buffer outMeta {
//    writeonly restrict uint numVerticiesOut[];
//};
layout(std430, binding = 1) buffer outputIndiciesBuff {
    writeonly restrict uint outIndicies[4000000];
};
layout(std430, binding = 3) buffer outputVerticiesBuff {
    writeonly restrict vec3 outVerts[1000000];
};
layout(std430, binding = 2) buffer inMeta {
    readonly restrict float inData[];
};

//shared vec3 outData[2000];
shared int indicies[9][17][9];
shared float sdfValue[9][17][9];
//shared int indicies[11][11][11];
//shared float sdfValue[11][11][11];

shared uint currIndex;

// SDF from https://iquilezles.org/articles/distfunctions/
float sdVerticalCapsule( vec3 p, float h, float r ) {
  p.y -= clamp( p.y, 0.0, h );
  return length( p ) - r;
}

float sdCapsule( vec3 p, vec3 a, vec3 b, float r ) {
  vec3 pa = p - a, ba = b - a;
  float h = clamp( dot(pa,ba)/dot(ba,ba), 0.0, 1.0 );
  return length( pa - ba*h ) - r;
}

//float def = 0.23;
float def = 0.23;
uint startPos = 0;
uint treeSize;
vec3 offset;

float calcSdf(vec3 p) {
    float r = 0.23;
    //r = 0.1;
    float min = 100000;

    for (uint i = startPos; i < treeSize + startPos; i += 6) {
        vec3 a = vec3(inData[i], inData[i + 1], inData[i + 2]);
        vec3 b = vec3(inData[i + 3], inData[i + 4], inData[i + 5]);

        float t = sdCapsule(p, a, b, r);
        if (t < min) {
            min = t;
        }
    }

    return min;
}

// This is how many work groups tall a tree is
int treeHeight = 10;

void main() {
    if (gl_LocalInvocationID == vec3(0, 0, 0)) {
        //outIndex = 0;
        currIndex = 0;
    }
    // Here we are finding the details about the tree that I am rendering
    int treeNum = int(floor(gl_WorkGroupID.y / treeHeight)) + treeOffset;

    vec3 curIndex = gl_LocalInvocationID + vec3(1, 1, 1);
    vec3 curPos = gl_GlobalInvocationID;
    //vec3 curPos = gl_GlobalInvocationID - vec3(gl_WorkGroupID.x, gl_WorkGroupID.y, gl_WorkGroupID.z);

    curPos.y = mod(curPos.y, treeHeight * local_y);
    curPos -= vec3(gl_WorkGroupID.x, mod(gl_WorkGroupID.y, treeHeight), gl_WorkGroupID.z);

    vec3 pos = {-5, 0, -5};
    //vec3 pos = {0.0, 0.0, 0.0};

    float r = 0.23;

    uint ti = 0;
    while(ti < treeNum) {
        startPos += uint(inData[startPos]);
        ti++;
    }

    // Skipping the header
    treeSize = uint(inData[startPos]) - 4;
    offset = vec3(inData[startPos + 1], inData[startPos + 2], inData[startPos + 3]);
    startPos += 4;

    // Start by calculating all of the sdf values
    vec3 p1 = curPos * def + pos;
    sdfValue[int(curIndex.x)][int(curIndex.y)][int(curIndex.z)] = calcSdf(p1);

    if (gl_LocalInvocationID.x == 0) {
        vec3 p1 = (curPos - vec3(1, 0, 0)) * def + pos;
        sdfValue[0][int(curIndex.y)][int(curIndex.z)] = calcSdf(p1);
    }
    if (gl_LocalInvocationID.y == 0) {
        vec3 p1 = (curPos - vec3(0, 1, 0)) * def + pos;
        sdfValue[int(curIndex.x)][0][int(curIndex.z)] = calcSdf(p1);
        if (gl_LocalInvocationID.x == 0) {
            vec3 p1 = (curPos - vec3(1, 1, 0)) * def + pos;
            sdfValue[0][0][int(curIndex.z)] = calcSdf(p1);
        }
    }
    if (gl_LocalInvocationID.z == 0) {
        vec3 p1 = (curPos - vec3(0, 0, 1)) * def + pos;
        sdfValue[int(curIndex.x)][int(curIndex.y)][0] = calcSdf(p1);
        if (gl_LocalInvocationID.x == 0) {
            vec3 p1 = (curPos - vec3(1, 0, 1)) * def + pos;
            sdfValue[0][int(curIndex.y)][0] = calcSdf(p1);
        }
        if (gl_LocalInvocationID.y == 0) {
            vec3 p1 = (curPos - vec3(0, 1, 1)) * def + pos;
            sdfValue[int(curIndex.x)][0][0] = calcSdf(p1);
        }
        if (gl_LocalInvocationID.x == 0 && gl_LocalInvocationID.y == 0) {
            vec3 p1 = (curPos - vec3(1, 1, 1)) * def + pos;
            sdfValue[0][0][0] = calcSdf(p1);
        }
    }

    // Making sure that all of the SDF values have been calculated first
    barrier();

    vec3 axes[] = {
        {0.0, 0.0, 1.0},
        {1.0, 0.0, 0.0},
        {0.0, 1.0, 0.0},
    };

    vec3 comparisons[] = {
        {0.0, 0.0, 0.0},
        {0.0, 0.0, 1.0},
        {0.0, 1.0, 0.0},
        {0.0, 1.0, 1.0},
        {1.0, 0.0, 0.0},
        {1.0, 0.0, 1.0},
        {1.0, 1.0, 0.0},
        {1.0, 1.0, 1.0},

        {0.0, 0.0, 0.0},
        {1.0, 0.0, 0.0},
        {0.0, 1.0, 0.0},
        {1.0, 1.0, 0.0},
        {0.0, 1.0, 1.0},
        {1.0, 1.0, 1.0},
        {0.0, 0.0, 1.0},
        {1.0, 0.0, 1.0},

        {0.0, 0.0, 0.0},
        {0.0, 1.0, 0.0},
        {1.0, 0.0, 0.0},
        {1.0, 1.0, 0.0},
        {1.0, 0.0, 1.0},
        {1.0, 1.0, 1.0},
        {0.0, 0.0, 1.0},
        {0.0, 1.0, 1.0},
    };

    // This draws the grid of sample points

    //uint tb = atomicCounterIncrement(outIndex);
    //outIndicies[tb] = gl_LocalInvocationID * def + pos;

    int numIntersections = 0;
    vec3 intersectionsSum = vec3(0.0, 0.0, 0.0);
    vec3 normal = vec3(0.0, 0.0, 0.0);
    for (int i = 0; i < 24; i+=2) {
        // Here we are offseting by -1 in all dimensions so we can generate the points at 0, 0, 0
        // The faces connect into the negative direction
        vec3 p1 = (curPos - vec3(1, 1, 1) + comparisons[i]) * def + pos;
        vec3 p2 = (curPos - vec3(1, 1, 1) + comparisons[i + 1]) * def + pos;

        vec3 pa = curIndex + comparisons[i] - vec3(1, 1, 1);
        vec3 pb = curIndex + comparisons[i + 1] - vec3(1, 1, 1);

        float d1 = sdfValue[int(pa.x)][int(pa.y)][int(pa.z)];
        float d2 = sdfValue[int(pb.x)][int(pb.y)][int(pb.z)];

        if ((d1 >= 0 && d2 <= 0) || (d1 <= 0 && d2 >= 0)) {
            float a = (d1 / (d1 - d2));
            vec3 ps = (1 - a) * p1 + a * p2;

            intersectionsSum += ps;
            numIntersections++;
        }

        normal += axes[int(floor(i / 8))] * (d1 - d2);
    }

    if (numIntersections != 0) {
        vec3 avgPoint = intersectionsSum * (1.0f / numIntersections);

        // TODO Need to add this to the output somewhere
        uint outIndexCurr = atomicCounterAdd(outVertIndex, 2);
        outVerts[outIndexCurr] = avgPoint + offset;
        outVerts[outIndexCurr+1] = normal;

        indicies[int(curIndex.x) - 1][int(curIndex.y) - 1][int(curIndex.z) - 1] = int(outIndexCurr);
    } else {
        indicies[int(curIndex.x) - 1][int(curIndex.y) - 1][int(curIndex.z) - 1] = -1;
    }

    // This barrier is to make sure that all threads have finished their completing their indicies
    barrier();

    // Getting rid of the threads at the boundaries, they won't have all of the points they need anyways
    if (gl_LocalInvocationID.x == local_x - 1 || gl_LocalInvocationID.y == local_y - 1 || gl_LocalInvocationID.z == local_z - 1) {
        return;
    }

    if (indicies[int(curIndex.x)][int(curIndex.y)][int(curIndex.z)] != -1) {
        float d1 = sdfValue[int(curIndex.x)][int(curIndex.y)][int(curIndex.z)];

        for (int i = 0; i < 3; i++) {
            vec3 axisA = curIndex + axes[i];
            float d2 = sdfValue[int(axisA.x)][int(axisA.y)][int(axisA.z)];

            if ((d1 >= 0 && d2 < 0) || (d1 < 0 && d2 >= 0)) {
                uint t;

                if (int(axes[i].x) == 1) {
                    int i1 = indicies[int(curIndex.x)][int(curIndex.y)][int(curIndex.z)];
                    int i2 = indicies[int(curIndex.x)][int(curIndex.y) - 1][int(curIndex.z)];
                    int i3 = indicies[int(curIndex.x)][int(curIndex.y)][int(curIndex.z) - 1];
                    int i4 = indicies[int(curIndex.x)][int(curIndex.y) - 1][int(curIndex.z) - 1];
                    if (i2 == -1 || i3 == -1 || i4 == -1) {
                        continue;
                    }

                    t = atomicCounterAdd(outIndex, uint(6));
                    outIndicies[t] = i1;
                    outIndicies[t+1] = i2;
                    outIndicies[t+2] = i3;

                    outIndicies[t+3] = i2;
                    outIndicies[t+4] = i3;
                    outIndicies[t+5] = i4;
                    //outIndicies[t] = outData[i1];
                    //outIndicies[t+1] = outData[i1+1];
                    //outIndicies[t+2] = outData[i2];
                    //outIndicies[t+3] = outData[i2+1];
                    //outIndicies[t+4] = outData[i3];
                    //outIndicies[t+5] = outData[i3+1];

                    //outIndicies[t+6] = outData[i2];
                    //outIndicies[t+7] = outData[i2+1];
                    //outIndicies[t+8] = outData[i3];
                    //outIndicies[t+9] = outData[i3+1];
                    //outIndicies[t+10] = outData[i4];
                    //outIndicies[t+11] = outData[i4+1];

                } else if (int(axes[i].y) == 1) {
                    int i1 = indicies[int(curIndex.x)][int(curIndex.y)][int(curIndex.z)];
                    int i2 = indicies[int(curIndex.x)][int(curIndex.y)][int(curIndex.z) - 1];
                    int i3 = indicies[int(curIndex.x) - 1][int(curIndex.y)][int(curIndex.z)];
                    int i4 = indicies[int(curIndex.x) - 1][int(curIndex.y)][int(curIndex.z) - 1];
                    if (i2 == -1 || i3 == -1 || i4 == -1) {
                        continue;
                    }

                    t = atomicCounterAdd(outIndex, uint(6));
                    outIndicies[t] = i1;
                    outIndicies[t+1] = i2;
                    outIndicies[t+2] = i3;

                    outIndicies[t+3] = i2;
                    outIndicies[t+4] = i3;
                    outIndicies[t+5] = i4;
                    //t = atomicCounterAdd(outIndex, uint(12));
                    //outIndicies[t] = outData[i1];
                    //outIndicies[t+1] = outData[i1+1];
                    //outIndicies[t+2] = outData[i2];
                    //outIndicies[t+3] = outData[i2+1];
                    //outIndicies[t+4] = outData[i3];
                    //outIndicies[t+5] = outData[i3+1];

                    //outIndicies[t+6] = outData[i2];
                    //outIndicies[t+7] = outData[i2+1];
                    //outIndicies[t+8] = outData[i3];
                    //outIndicies[t+9] = outData[i3+1];
                    //outIndicies[t+10] = outData[i4];
                    //outIndicies[t+11] = outData[i4+1];
                } else if (int(axes[i].z) == 1) {
                    int i1 = indicies[int(curIndex.x)][int(curIndex.y)][int(curIndex.z)];
                    int i2 = indicies[int(curIndex.x)][int(curIndex.y) - 1][int(curIndex.z)];
                    int i3 = indicies[int(curIndex.x) - 1][int(curIndex.y)][int(curIndex.z)];
                    int i4 = indicies[int(curIndex.x) - 1][int(curIndex.y) - 1][int(curIndex.z)];
                    if (i2 == -1 || i3 == -1 || i4 == -1) {
                        continue;
                    }

                    t = atomicCounterAdd(outIndex, uint(6));
                    outIndicies[t] = i1;
                    outIndicies[t+1] = i2;
                    outIndicies[t+2] = i3;

                    outIndicies[t+3] = i2;
                    outIndicies[t+4] = i3;
                    outIndicies[t+5] = i4;
                    //t = atomicCounterAdd(outIndex, uint(12));
                    //outIndicies[t] = outData[i1];
                    //outIndicies[t+1] = outData[i1+1];
                    //outIndicies[t+2] = outData[i2];
                    //outIndicies[t+3] = outData[i2+1];
                    //outIndicies[t+4] = outData[i3];
                    //outIndicies[t+5] = outData[i3+1];

                    //outIndicies[t+6] = outData[i2];
                    //outIndicies[t+7] = outData[i2+1];
                    //outIndicies[t+8] = outData[i3];
                    //outIndicies[t+9] = outData[i3+1];
                    //outIndicies[t+10] = outData[i4];
                    //outIndicies[t+11] = outData[i4+1];
                }
            }
        }
    }

}
