#include "renderer.h"

#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

Renderer::Renderer(ChunkMap &chunks) :
    m_chunks(chunks), m_chunkShader("../res/shaders/block_vertex.glsl", "../res/shaders/block_fragment.glsl"),
    m_chunkTexture("../res/textures/terrain.png", GL_RGBA)
{
    m_chunkShader.bind();
    m_chunkShader.setInt("texture1", 0);

    m_projection = glm::perspective(glm::radians(45.0f), static_cast<float>(1920) / static_cast<float>(1080), 0.1f, 150.0f);

    glEnable(GL_DEPTH_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    //glEnable(GL_CULL_FACE);
    //glCullFace(GL_FRONT);
    //glFrontFace(GL_CW);
}

void Renderer::render(Camera &cam)
{
    glClearColor(m_skyColor.x, m_skyColor.y, m_skyColor.z, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_chunkShader.bind();
    m_chunkShader.setMat4("transform", m_projection * cam.getView());
    m_chunkShader.setFloat("daylight", m_daylight);

    m_chunkTexture.bind(GL_TEXTURE0);

    for (const auto &it : m_chunks)
    {
        auto &chunk = it.second;
        if (!chunk->isEmpty())
        {
            chunk->bufferData();
            chunk->bind();
            glDrawArrays(GL_TRIANGLES, 0, chunk->getVertexCount());
        }
    }
}