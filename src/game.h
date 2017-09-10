#pragma once

#include <mutex>
#include <set>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "camera.h"
#include "chunk.h"
#include "computejob.h"
#include "frustum.h"
#include "inputmanager.h"
#include "player.h"
#include "renderer.h"
#include "sharedvector.h"
#include "terraingenerator.h"
#include "threadpool.h"
#include "world.h"

class Game
{
public:
    Game(GLFWwindow *window);

    void run();

private:
    int traceRay(glm::vec3 p, glm::vec3 dir, float range, glm::ivec3 &hitNorm, glm::ivec3 &hitIpos);
    bool raycast(glm::vec3 origin, glm::vec3 dir, float range, glm::ivec3 &hit, glm::ivec3 &norm);
    void processInput(float dt);

    void loadNearest(const glm::ivec3 &center, int maxJobs);
    void updateNearest(const glm::ivec3 &center, int maxJobs);
    void updateChunks();
    void dirtyNeighbors(Chunk &chunk, const glm::ivec3 &pos);

    const int m_loadDistance = 1;
    float m_eraseDistance;
    float m_viewDistance;

    World m_world;
    SharedVector<std::unique_ptr<Chunk>> m_processed;
    SharedVector<std::unique_ptr<ComputeJob>> m_updates;

    ThreadPool m_pool;
    TerrainGenerator m_chunkGenerator;
    Renderer m_renderer;
    InputManager m_input;

    GLFWwindow *m_window;
    Camera m_camera;
    Frustum m_frustum;
    Player m_player;
    float m_cooldown;

    int m_width;
    int m_height;
    float m_ratio;
};