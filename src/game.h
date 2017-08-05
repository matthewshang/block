#pragma once

#include <mutex>
#include <set>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "camera.h"
#include "chunk.h"
#include "chunkcompare.h"
#include "common.h"
#include "computejob.h"
#include "sharedvector.h"
#include "terraingenerator.h"
#include "threadpool.h"

class Game
{
public:
    Game(GLFWwindow *window);

    void run();

private:
    void processInput(float dt);
    void updateChunk(Chunk *chunk);
    void dirtyChunks(glm::ivec3 center);
    void loadChunks();
    void initChunks();
    Chunk *chunkFromWorld(const glm::vec3 &pos);

    const int m_renderDistance = 1;
    float m_eraseDistance;
    ChunkMap m_chunks;
    std::set<glm::ivec3, ChunkCompare> m_loadedChunks;
    SharedVector<std::unique_ptr<Chunk>> m_processed;
    SharedVector<std::unique_ptr<ComputeJob>> m_updates;
    std::vector<glm::ivec3> m_toErase;
    ThreadPool m_pool;
    TerrainGenerator m_chunkGenerator;

    GLFWwindow *m_window;
    Camera m_camera;

    double m_lastX;
    double m_lastY;
    bool m_firstMouse;
};