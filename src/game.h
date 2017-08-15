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
#include "frustum.h"
#include "inputmanager.h"
#include "player.h"
#include "renderer.h"
#include "sharedvector.h"
#include "terraingenerator.h"
#include "threadpool.h"

class Game
{
public:
    Game(GLFWwindow *window);

    void run();

private:
    int getVoxel(const glm::ivec3 &i);
    int traceRay(glm::vec3 p, glm::vec3 dir, float range, glm::ivec3 &hitNorm, glm::ivec3 &hitIpos);
    void raycast(glm::vec3 origin, glm::vec3 dir, float range, int block);
    void processInput(float dt);

    void loadNearest(const glm::ivec3 &center, int maxJobs);
    void updateNearest(const glm::ivec3 &center, int maxJobs);
    void updateChunks();

    void dirtyChunks(glm::ivec3 center);
    Chunk *chunkFromWorld(const glm::vec3 &pos);

    const int m_loadDistance = 2;
    float m_eraseDistance;

    ChunkMap m_chunks;
    std::set<glm::ivec3, ChunkCompare> m_loadedChunks;
    SharedVector<std::unique_ptr<Chunk>> m_processed;
    SharedVector<std::unique_ptr<ComputeJob>> m_updates;
    std::vector<glm::ivec3> m_toErase;

    ThreadPool m_pool;
    TerrainGenerator m_chunkGenerator;

    Renderer m_renderer;
    InputManager m_input;

    GLFWwindow *m_window;
    Camera m_camera;
    Frustum m_frustum;
    Player m_player;
    float m_cooldown;

    double m_lastX;
    double m_lastY;
    bool m_firstMouse;
};