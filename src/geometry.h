#pragma once

#include <vector>

namespace Geometry
{
    void makeCube(std::vector<float> &vertices, float x, float y, float z, bool faces[6], int type, float light[6][4]);
    void makeSelectCube(std::vector<float> &vertices, float size);
    void makePlant(std::vector<float> &vertices, float x, float y, float z, int type, int light);
}