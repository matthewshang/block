#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "camera.h"
#include "chunk.h"
#include "frustum.h"
#include "shader.h"
#include "texture.h"

class Renderer
{
public:
    Renderer(ChunkMap &chunks);

    void render(Camera &cam, Frustum &f);
    void setSkyColor(glm::vec3 c) { m_skyColor = c; };
    void setDaylight(float daylight) { m_daylight = daylight; };
    void setSelected(const glm::vec3 &pos);
    void unselect() { m_showSelect = false; };

private:
    ChunkMap &m_chunks;

    glm::mat4 m_projection;

    Shader m_chunkShader;
    Texture m_chunkTexture;
    glm::vec3 m_skyColor;
    float m_daylight;

    bool m_showSelect;
    glm::vec3 m_selected;
    Shader m_selectShader;
    GLuint m_selectVbo;
    GLuint m_selectVao;
};