#pragma once

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

private:
    ChunkMap &m_chunks;

    glm::mat4 m_projection;

    Shader m_chunkShader;
    Texture m_chunkTexture;
    glm::vec3 m_skyColor;
    float m_daylight;
};