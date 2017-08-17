#include "geometry.h"

#include <algorithm>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "blocks.h"

void Geometry::makeCube(std::vector<float> &vertices, float x, float y, float z, bool faces[6], int type, float light[6][4])
{
    static const glm::vec3 positions[6][4] = {
        { glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec3(-0.5f,  0.5f,  0.5f), glm::vec3(0.5f,  0.5f,  0.5f), glm::vec3(0.5f, -0.5f,  0.5f) },
        { glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(0.5f,  0.5f, -0.5f), glm::vec3(-0.5f,  0.5f, -0.5f), glm::vec3(-0.5f, -0.5f, -0.5f) },
        { glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(-0.5f,  0.5f, -0.5f), glm::vec3(-0.5f,  0.5f,  0.5f), glm::vec3(-0.5f, -0.5f,  0.5f) },
        { glm::vec3(0.5f, -0.5f,  0.5f), glm::vec3(0.5f,  0.5f,  0.5f), glm::vec3(0.5f,  0.5f, -0.5f), glm::vec3(0.5f, -0.5f, -0.5f) },
        { glm::vec3(-0.5f,  0.5f,  0.5f), glm::vec3(-0.5f,  0.5f, -0.5f), glm::vec3(0.5f,  0.5f, -0.5f), glm::vec3(0.5f,  0.5f,  0.5f) },
        { glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec3(0.5f, -0.5f,  0.5f), glm::vec3(0.5f, -0.5f, -0.5f) }
    };

    static const glm::vec2 texcoords[6] = {
        glm::vec2(0.0f, 0.0f), glm::vec2(0.0f, 1.0f), glm::vec2(1.0f, 1.0f), glm::vec2(1.0f, 0.0f)
    };

    static const int indices[6] = {
        0, 1, 2, 0, 2, 3
    };

    static const int flipped[6] = {
        0, 1, 3, 3, 1, 2
    };

    float s = 1.0f / 16.0f;

    for (int i = 0; i < 6; i++)
    {
        if (!faces[i]) continue;

        int idx = Blocks::faces[type][i];
        float tu = s * static_cast<float>(idx % 16);
        float tv = 1.0f - s * static_cast<float>(idx / 16) - s;

        // https://0fps.net/2013/07/03/ambient-occlusion-for-minecraft-like-worlds/
        bool flip = light[i][0] + light[i][2] > light[i][1] + light[i][3];
        for (int v = 0; v < 6; v++)
        {
            int j = flip ? flipped[v] : indices[v];
            vertices.push_back(positions[i][j].x + x);
            vertices.push_back(positions[i][j].y + y);
            vertices.push_back(positions[i][j].z + z);
            vertices.push_back(tu + texcoords[j].x * s);
            vertices.push_back(tv + texcoords[j].y * s);
            float lightVal = (static_cast<float>(light[i][j]) + 1.0f) / 16.0f;
            lightVal = (std::max)(lightVal, 0.1f);
            vertices.push_back(lightVal);
        }
    }
}

void Geometry::makeSelectCube(std::vector<float> &vertices, float size)
{
    static const glm::vec3 positions[8] = {
        glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(-0.5f, 0.5f, 0.5f), 
        glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.5f, -0.5f, 0.5f),
        glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(-0.5f, 0.5f, -0.5f), 
        glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(0.5f, -0.5f, -0.5f)
    };

    static const int indices[12][2] = {
        {0, 1}, {1, 2}, {2, 3}, {3, 0}, {4, 5}, {5, 6},
        {6, 7}, {7, 4}, {0, 4}, {1, 5}, {2, 6}, {3, 7}
    };

    for (int i = 0; i < 12; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            int idx = indices[i][j];
            vertices.push_back(positions[idx].x * size);
            vertices.push_back(positions[idx].y * size);
            vertices.push_back(positions[idx].z * size);
        }
    }
}

void Geometry::makePlant(std::vector<float> &vertices, float x, float y, float z, int type, int light)
{
    static const glm::vec3 positions[2][4] = {
        { glm::vec3(0.5f,  0.5f,  0.0f), glm::vec3(0.5f, -0.5f,  0.0f), glm::vec3(-0.5f, -0.5f,  0.0f), glm::vec3(-0.5f,  0.5f,  0.0f) },
        { glm::vec3(0.0f,  0.5f,  0.5f), glm::vec3(0.0f, -0.5f,  0.5f), glm::vec3(0.0f, -0.5f, -0.5f), glm::vec3(0.0f,  0.5f, -0.5f) }
    };

    static const glm::vec2 texcoords[4] = {
        glm::vec2(1.0f, 1.0f), glm::vec2(1.0f, 0.0f), glm::vec2(0.0f, 0.0f), glm::vec2(0.0f, 1.0f)
    };

    static const int indices[6] = {
        0, 1, 2, 0, 2, 3
    };

    glm::mat4 model;
    model = glm::rotate(model, 45.0f, glm::vec3(0, 1, 0));

    float s = 1.0f / 16.0f;
    for (int i = 0; i < 2; i++)
    {
        int idx = Blocks::faces[type][i];
        float tu = s * static_cast<float>(idx % 16);
        float tv = 1.0f - s * static_cast<float>(idx / 16) - s;
        for (int v = 0; v < 6; v++)
        {
            int j = indices[v];
            glm::vec4 pos = model * glm::vec4(positions[i][j], 1.0f);
            vertices.push_back(pos.x + x);
            vertices.push_back(pos.y + y);
            vertices.push_back(pos.z + z);
            vertices.push_back(tu + texcoords[j].x * s);
            vertices.push_back(tv + texcoords[j].y * s);
            vertices.push_back((static_cast<float>(light) + 1.0f) / 16.0f);
        }
    }
}