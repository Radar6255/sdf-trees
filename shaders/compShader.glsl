#version 460 core
// In this shader I want to perform native mesh construction of a SDF

layout(local_size_x = 10, local_size_y = 10, local_size_z = 10) in;

// TODO Here I need to get any buffers that I want to read/write to
//layout(std430, binding = 0) buffer layoutName {
//    readonly vec3 data[2];
//};
layout(std430, binding = 0) buffer outputName {
    restrict vec3 outDataOld[1000];
};
layout(binding = 2) uniform atomic_uint outIndex;
layout(std430, binding = 1) buffer outputIndiciesBuff {
    writeonly restrict vec3 outIndicies[];
};
layout(binding = 2) uniform atomic_uint outIndexOld;

//shared uint outIndex;
shared uint outIndexIndex;
shared vec3 outData[1000];
shared int indicies[11][11][11];

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

shared float sdfValue[11][11][11];
void main() {
//    vec3 curIndex = gl_LocalInvocationID + vec3(1, 1, 1);

    float def = 0.1;
    vec3 pos = {-0.5, 0, -0.5};
    //vec3 pos = {0.0, 0.0, 0.0};


    // TODO Start by calculating all of the sdf values
    vec3 p1 = gl_LocalInvocationID * def + pos;
    sdfValue[gl_LocalInvocationID.x][gl_LocalInvocationID.y][gl_LocalInvocationID.z] = sdVerticalCapsule(p1, 0.7, 0.2);

    barrier();

    // Getting rid of the threads at the boundaries, they won't have all of the points they need anyways
    if (gl_LocalInvocationID.x == 9 || gl_LocalInvocationID.y == 9 || gl_LocalInvocationID.z == 9) {
        return;
    }
    if (gl_LocalInvocationID.x == 9) {
        vec3 p1 = (gl_LocalInvocationID + vec3(1, 0, 0)) * def + pos;
        sdfValue[gl_LocalInvocationID.x + 1][gl_LocalInvocationID.y][gl_LocalInvocationID.z] = sdVerticalCapsule(p1, 1, 0.4);
    }
    if (gl_LocalInvocationID.y == 9) {
        vec3 p1 = (gl_LocalInvocationID + vec3(0, 1, 0)) * def + pos;
        sdfValue[gl_LocalInvocationID.x][gl_LocalInvocationID.y + 1][gl_LocalInvocationID.z] = sdVerticalCapsule(p1, 1, 0.4);
    }
    if (gl_LocalInvocationID.z == 9) {
        vec3 p1 = (gl_LocalInvocationID + vec3(0, 0, 1)) * def + pos;
        sdfValue[gl_LocalInvocationID.x + 1][gl_LocalInvocationID.y][gl_LocalInvocationID.z + 1] = sdVerticalCapsule(p1, 1, 0.4);
    }

    vec3 comparisons[] = {
//        {0.0, 0.0, 0.0},
//        {0.0, 0.0, 1.0},
//        {0.0, 0.0, 0.0},
//        {0.0, 1.0, 0.0},
//        {0.0, 0.0, 0.0},
//        {1.0, 0.0, 0.0},
//        {1.0, 1.0, 1.0},
//        {1.0, 1.0, 0.0},
//        {1.0, 1.0, 1.0},
//        {1.0, 0.0, 1.0},
//        {1.0, 1.0, 1.0},
//        {0.0, 1.0, 1.0},
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
    for (int i = 0; i < 24; i+=2) {
        vec3 p1 = (gl_LocalInvocationID + comparisons[i]) * def + pos;
        vec3 p2 = (gl_LocalInvocationID + comparisons[i + 1]) * def + pos;
//        float d1 = sdVerticalCapsule(p1, 1, 0.5);
//        float d2 = sdVerticalCapsule(p2, 1, 0.5);
        vec3 pa = gl_LocalInvocationID + comparisons[i];
        vec3 pb = gl_LocalInvocationID + comparisons[i + 1];

        float d1 = sdfValue[int(pa.x)][int(pa.y)][int(pa.z)];
        float d2 = sdfValue[int(pb.x)][int(pb.y)][int(pb.z)];

        if ((d1 >= 0 && d2 <= 0) || (d1 <= 0 && d2 >= 0)) {
            float a = (d1 / (d1 - d2));
            vec3 ps = (1 - a) * p1 + a * p2;

            //uint ta = outIndexIndex++;
            //outIndicies[ta] = ps;

            intersectionsSum += ps;
            //intersectionsSum.x =+ ps.x;
            //intersectionsSum.y =+ ps.y;
            //intersectionsSum.z =+ ps.z;
            numIntersections++;

            //uint t = atomicCounterIncrement(outIndex);
            //outData[t] = ps;
            //outIndicies[t] = ps;
        }
    }

    if (numIntersections != 0) {
        //vec3 avgPoint = intersectionsSum / (1.0f * numIntersections);
        vec3 avgPoint = intersectionsSum * (1.0f / numIntersections);

        //outIndicies[ta] = vec3(intersectionsSum.x, 1, avgPoint.x);
        //outIndicies[ta] = vec3(intersectionsSum.x / numIntersections, intersectionsSum.y / numIntersections, intersectionsSum.z / numIntersections);

        //ta = outIndexIndex++;
        //outIndicies[ta] = avgPoint;
        //uint ta = outIndexIndex++;

        // TODO Need to add this to the output somewhere
        uint outIndexCurr = atomicCounterIncrement(outIndex);
        outData[outIndexCurr] = avgPoint;

        uint t = atomicCounterAdd(outIndex, uint(1));
        outDataOld[t] = avgPoint;

        //outIndicies[outIndexCurr] = avgPoint;

        indicies[gl_LocalInvocationID.x][gl_LocalInvocationID.y][gl_LocalInvocationID.z] = int(outIndexCurr);
    } else {
        //uint outIndexCurr = atomicCounterIncrement(outIndex);
        //uint outIndexCurr = outIndex++;
        //outData[outIndexCurr] = vec3(0.0, 0.0, 0.0);
        indicies[gl_LocalInvocationID.x][gl_LocalInvocationID.y][gl_LocalInvocationID.z] = -1;
    }

    //outData[outIndexCurr] = vec3(outIndexCurr, 0.0, 0.0);
    //outData[2] = vec3(outIndexCurr, 1.0, 0.0);

    // This barrier is to make sure that all threads have finished their completing their indicies
    barrier();

    //uint ta = outIndexIndex++;
//    outIndexIndex += 3;
//    outIndicies[ta] = vec3(1, 0, 0);
//    outIndicies[ta] = vec3(0, 0, 0);
//    outIndicies[ta] = vec3(0, 1, 0);
    //outIndicies[ta] = vec3(indicies[gl_LocalInvocationID.x][gl_LocalInvocationID.y][gl_LocalInvocationID.z], 1, 0);

    vec3 axes[] = {
        {1.0, 0.0, 0.0},
        {0.0, 1.0, 0.0},
        {0.0, 0.0, 1.0},
    };

    if (indicies[gl_LocalInvocationID.x][gl_LocalInvocationID.y][gl_LocalInvocationID.z] != -1) {
        //vec3 p1 = gl_LocalInvocationID * def + pos;
        //float d1 = sdVerticalCapsule(p1, 1, 0.4);
        float d1 = sdfValue[gl_LocalInvocationID.x][gl_LocalInvocationID.y][gl_LocalInvocationID.z];

        for (int i = 0; i < 3; i++) {
            //vec3 p2 = (gl_LocalInvocationID + axes[i]) * def + pos;
            vec3 axisA = gl_LocalInvocationID + axes[i];
            float d2 = sdfValue[int(axisA.x)][int(axisA.y)][int(axisA.z)];

            //float d2 = sdVerticalCapsule(p2, 1, 0.4);

            //uint t = atomicCounterAdd(outIndex, uint(3));
            //outIndicies[t] = gl_LocalInvocationID * def + pos;
            //outIndicies[t+1] = (gl_LocalInvocationID - vec3(0, 1, 0)) * def + pos;
            //outIndicies[t+2] = (gl_LocalInvocationID - vec3(0, 0, 1)) * def + pos;

            if ((d1 >= 0 && d2 < 0) || (d1 < 0 && d2 >= 0)) {
                //uint t = atomicCounterAdd(outIndex, uint(3));
                //outIndicies[t] = gl_LocalInvocationID * def + pos;
                //outIndicies[t+1] = (gl_LocalInvocationID - vec3(0, 1, 0)) * def + pos;
                //outIndicies[t+2] = (gl_LocalInvocationID - vec3(0, 0, 1)) * def + pos;
                uint t;

                if (int(axes[i].x) == 1) {
                    int i1 = indicies[gl_LocalInvocationID.x][gl_LocalInvocationID.y][gl_LocalInvocationID.z];
                    int i2 = indicies[gl_LocalInvocationID.x][gl_LocalInvocationID.y - 1][gl_LocalInvocationID.z];
                    int i3 = indicies[gl_LocalInvocationID.x][gl_LocalInvocationID.y][gl_LocalInvocationID.z - 1];
                    int i4 = indicies[gl_LocalInvocationID.x][gl_LocalInvocationID.y - 1][gl_LocalInvocationID.z - 1];
                    if (i2 == -1 || i3 == -1 || i4 == -1) {
                        //t = atomicCounterAdd(outIndex, uint(6));
                        //outIndicies[t] = gl_LocalInvocationID * def + pos;
                        //outIndicies[t+1] = (gl_LocalInvocationID - vec3(0, 1, 0)) * def + pos;
                        //outIndicies[t+2] = (gl_LocalInvocationID - vec3(0, 0, 1)) * def + pos;

                        //outIndicies[t+3] = (gl_LocalInvocationID - vec3(0, 1, 0)) * def + pos;
                        //outIndicies[t+4] = (gl_LocalInvocationID - vec3(0, 0, 1)) * def + pos;
                        //outIndicies[t+5] = (gl_LocalInvocationID - vec3(0, 1, 1)) * def + pos;

                        continue;
                    }

                    t = atomicCounterAdd(outIndex, uint(6));
                    outIndicies[t] = outData[i1];
                    outIndicies[t+1] = outData[i2];
                    outIndicies[t+2] = outData[i3];

                    outIndicies[t+3] = outData[i2];
                    outIndicies[t+4] = outData[i3];
                    outIndicies[t+5] = outData[i4];

                } else if (int(axes[i].y) == 1) {
                    int i1 = indicies[gl_LocalInvocationID.x][gl_LocalInvocationID.y][gl_LocalInvocationID.z];
                    int i2 = indicies[gl_LocalInvocationID.x][gl_LocalInvocationID.y][gl_LocalInvocationID.z - 1];
                    int i3 = indicies[gl_LocalInvocationID.x - 1][gl_LocalInvocationID.y][gl_LocalInvocationID.z];
                    int i4 = indicies[gl_LocalInvocationID.x - 1][gl_LocalInvocationID.y][gl_LocalInvocationID.z - 1];
                    if (i2 == -1 || i3 == -1 || i4 == -1) {
                        continue;
                    }

                    t = atomicCounterAdd(outIndex, uint(6));
                    outIndicies[t] = outData[i1];
                    outIndicies[t+1] = outData[i2];
                    outIndicies[t+2] = outData[i3];

                    outIndicies[t+3] = outData[i2];
                    outIndicies[t+4] = outData[i3];
                    outIndicies[t+5] = outData[i4];
                } else if (int(axes[i].z) == 1) {
                    //t = atomicCounterAdd(outIndex, uint(6));
                    //outIndicies[t] = gl_LocalInvocationID * def + pos;
                    //outIndicies[t+1] = (gl_LocalInvocationID - vec3(0, 1, 0)) * def + pos;
                    //outIndicies[t+2] = (gl_LocalInvocationID - vec3(1, 0, 0)) * def + pos;

                    //outIndicies[t+3] = (gl_LocalInvocationID - vec3(0, 1, 0)) * def + pos;
                    //outIndicies[t+4] = (gl_LocalInvocationID - vec3(1, 0, 0)) * def + pos;
                    //outIndicies[t+5] = (gl_LocalInvocationID - vec3(1, 1, 0)) * def + pos;

                    int i1 = indicies[gl_LocalInvocationID.x][gl_LocalInvocationID.y][gl_LocalInvocationID.z];
                    int i2 = indicies[gl_LocalInvocationID.x][gl_LocalInvocationID.y - 1][gl_LocalInvocationID.z];
                    int i3 = indicies[gl_LocalInvocationID.x - 1][gl_LocalInvocationID.y][gl_LocalInvocationID.z];
                    int i4 = indicies[gl_LocalInvocationID.x - 1][gl_LocalInvocationID.y - 1][gl_LocalInvocationID.z];
                    if (i2 == -1 || i3 == -1 || i4 == -1) {
                        continue;
                    }

                    t = atomicCounterAdd(outIndex, uint(6));
                    outIndicies[t] = outData[i1];
                    outIndicies[t+1] = outData[i2];
                    outIndicies[t+2] = outData[i3];

                    outIndicies[t+3] = outData[i2];
                    outIndicies[t+4] = outData[i3];
                    outIndicies[t+5] = outData[i4];
                }
                /*if (axes[i].x == 1) {
                    outIndicies[t] = outData[indicies[gl_LocalInvocationID.x][gl_LocalInvocationID.y][gl_LocalInvocationID.z]];
                    outIndicies[t+1] = outData[indicies[gl_LocalInvocationID.x][gl_LocalInvocationID.y + 1][gl_LocalInvocationID.z]];
                    outIndicies[t+2] = outData[indicies[gl_LocalInvocationID.x][gl_LocalInvocationID.y][gl_LocalInvocationID.z + 1]];

                    outIndicies[t+3] = outData[indicies[gl_LocalInvocationID.x][gl_LocalInvocationID.y][gl_LocalInvocationID.z + 1]];
                    outIndicies[t+4] = outData[indicies[gl_LocalInvocationID.x][gl_LocalInvocationID.y + 1][gl_LocalInvocationID.z]];
                    outIndicies[t+5] = outData[indicies[gl_LocalInvocationID.x][gl_LocalInvocationID.y + 1][gl_LocalInvocationID.z + 1]];
                } else if (axes[i].y == 1) {
                    outIndicies[t] = outData[indicies[gl_LocalInvocationID.x][gl_LocalInvocationID.y][gl_LocalInvocationID.z]];
                    outIndicies[t+1] = outData[indicies[gl_LocalInvocationID.x][gl_LocalInvocationID.y][gl_LocalInvocationID.z + 1]];
                    outIndicies[t+2] = outData[indicies[gl_LocalInvocationID.x + 1][gl_LocalInvocationID.y][gl_LocalInvocationID.z]];

                    outIndicies[t+3] = outData[indicies[gl_LocalInvocationID.x + 1][gl_LocalInvocationID.y][gl_LocalInvocationID.z]];
                    outIndicies[t+4] = outData[indicies[gl_LocalInvocationID.x][gl_LocalInvocationID.y][gl_LocalInvocationID.z + 1]];
                    outIndicies[t+5] = outData[indicies[gl_LocalInvocationID.x + 1][gl_LocalInvocationID.y][gl_LocalInvocationID.z + 1]];
                } else if (axes[i].z == 1) {
                    outIndicies[t] = outData[indicies[gl_LocalInvocationID.x][gl_LocalInvocationID.y][gl_LocalInvocationID.z]];
                    outIndicies[t+1] = outData[indicies[gl_LocalInvocationID.x][gl_LocalInvocationID.y + 1][gl_LocalInvocationID.z]];
                    outIndicies[t+2] = outData[indicies[gl_LocalInvocationID.x + 1][gl_LocalInvocationID.y][gl_LocalInvocationID.z]];

                    outIndicies[t+3] = outData[indicies[gl_LocalInvocationID.x + 1][gl_LocalInvocationID.y][gl_LocalInvocationID.z]];
                    outIndicies[t+4] = outData[indicies[gl_LocalInvocationID.x][gl_LocalInvocationID.y + 1][gl_LocalInvocationID.z]];
                    outIndicies[t+5] = outData[indicies[gl_LocalInvocationID.x + 1][gl_LocalInvocationID.y + 1][gl_LocalInvocationID.z]];
                }*/
            }
        }
        /*
        // TODO Maybe gather these into an array
        // This is the location of the current point
        vec3 p2 = (gl_LocalInvocationID + vec3(1.0, 0, 0)) * def + pos;

        float d2 = sdVerticalCapsule(p2, 1, 0.4);

        if ((d1 >= 0 && d2 <= 0) || (d1 <= 0 && d2 >= 0)) {
            uint t = atomicCounterAdd(outIndex, uint(3));

            outIndicies[t] = outData[indicies[gl_LocalInvocationID.x][gl_LocalInvocationID.y][gl_LocalInvocationID.z]];
            outIndicies[t+1] = outData[indicies[gl_LocalInvocationID.x][gl_LocalInvocationID.y + 1][gl_LocalInvocationID.z]];
            outIndicies[t+2] = outData[indicies[gl_LocalInvocationID.x][gl_LocalInvocationID.y][gl_LocalInvocationID.z + 1]];
        }
        p2 = (gl_LocalInvocationID + vec3(0, 0, 1)) * def + pos;

        d2 = sdVerticalCapsule(p2, 1, 0.4);

        if ((d1 >= 0 && d2 <= 0) || (d1 <= 0 && d2 >= 0)) {
            uint t = atomicCounterAdd(outIndex, uint(3));

            outIndicies[t] = outData[indicies[gl_LocalInvocationID.x][gl_LocalInvocationID.y][gl_LocalInvocationID.z]];
            outIndicies[t+1] = outData[indicies[gl_LocalInvocationID.x][gl_LocalInvocationID.y + 1][gl_LocalInvocationID.z]];
            outIndicies[t+2] = outData[indicies[gl_LocalInvocationID.x+1][gl_LocalInvocationID.y][gl_LocalInvocationID.z]];
        }
        */
    }
}
