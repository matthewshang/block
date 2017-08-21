#include "renderer.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "geometry.h"

Renderer::Renderer(ChunkMap &chunks) :
    m_chunks(chunks), m_chunkShader("../res/shaders/block_vertex.glsl", "../res/shaders/block_fragment.glsl"),
    m_chunkTexture("../res/textures/terrain2.png", GL_RGBA),
    m_selectShader("../res/shaders/select_vertex.glsl", "../res/shaders/select_fragment.glsl"),
    m_crosshair("../res/textures/crosshair.png", GL_RGBA),
    m_guiShader("../res/shaders/gui_vertex.glsl", "../res/shaders/gui_fragment.glsl")
{
    m_chunkShader.bind();
    m_chunkShader.setInt("texture1", 0);

    m_guiShader.bind();
    m_guiShader.setInt("tex", 0);

    m_projection = glm::perspective(glm::radians(45.0f), static_cast<float>(1920) / static_cast<float>(1080), 0.1f, 150.0f);

    glGenVertexArrays(1, &m_selectVao);
    glBindVertexArray(m_selectVao);

    glGenBuffers(1, &m_selectVbo);
    std::vector<float> vertices;
    Geometry::makeSelectCube(vertices, 1.05f);
    glBindBuffer(GL_ARRAY_BUFFER, m_selectVbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    glGenVertexArrays(1, &m_crossVao);
    glBindVertexArray(m_crossVao);

    glGenBuffers(1, &m_crossVbo);
    vertices.clear();
    Geometry::makeGuiQuad(vertices, 20.0f, 20.0f);
    glBindBuffer(GL_ARRAY_BUFFER, m_crossVbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    //glEnable(GL_CULL_FACE);
    //glCullFace(GL_FRONT);
    //glFrontFace(GL_CW);
}

void Renderer::render(Camera &cam, Frustum &f)
{
    glClearColor(m_skyColor.x, m_skyColor.y, m_skyColor.z, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);

    m_chunkShader.bind();
    m_chunkShader.setMat4("transform", m_projection * cam.getView());
    m_chunkShader.setFloat("daylight", m_daylight);

    m_chunkTexture.bind(GL_TEXTURE0);

    m_projection = glm::perspective(glm::radians(f.getFov()), f.getRatio(), f.getNear(), f.getFar());

    for (const auto &it : m_chunks)
    {
        auto &chunk = it.second;
        if (!chunk->isEmpty())
        {
            glm::vec3 corner = chunk->getCoords() * 16;
            if (!f.boxInFrustum(corner, glm::vec3(16)))
                continue;

            chunk->bufferData();
            chunk->bind();
            glDrawArrays(GL_TRIANGLES, 0, chunk->getVertexCount());
        }
    }

    if (m_showSelect)
    {
        glLineWidth(1.5f);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        //glDisable(GL_DEPTH_TEST);

        glm::mat4 model;
        model = glm::translate(model, m_selected);
        m_selectShader.bind();
        m_selectShader.setMat4("transform", m_projection * cam.getView() * model);

        glBindVertexArray(m_selectVao);
        glDrawArrays(GL_LINES, 0, 24);
    }

    glDisable(GL_DEPTH_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    m_guiShader.bind();
    glm::mat4 projection = glm::ortho(0.0f, 1440.0f, 0.0f, 800.0f, -1.0f, 1.0f);
    glm::mat4 model;
    model = glm::translate(model, glm::vec3(720, 400, 0));
    m_guiShader.setMat4("transform", projection * model);
    m_crosshair.bind(GL_TEXTURE0);
    glBindVertexArray(m_crossVao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void Renderer::setSelected(const glm::vec3 &pos)
{
    m_selected = pos;
    m_showSelect = true;
}