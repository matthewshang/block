#include "mesh.h"

#include <numeric>

Mesh::Mesh() : m_vertexCount(0)
{

}

Mesh::Mesh(const std::vector<float> &data, const std::vector<int> &vertexAttribs, 
    bool tris, bool staticDraw)
{
    m_vertexSize = std::accumulate(vertexAttribs.begin(), vertexAttribs.end(), 0);
    m_vertexCount = data.size() / m_vertexSize;
    m_shapeMode = tris ? GL_TRIANGLES : GL_LINES;
    m_dataType = staticDraw ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW;

    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    glGenBuffers(1, &m_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), data.data(), m_dataType);
    GLsizei stride = m_vertexSize * sizeof(float);
    int ptr = 0;
    for (int i = 0; i < vertexAttribs.size(); i++)
    {
        glVertexAttribPointer(i, vertexAttribs[i], GL_FLOAT, GL_FALSE, stride, (void*)(ptr * sizeof(float)));
        glEnableVertexAttribArray(i);
        ptr += vertexAttribs[i];
    }

    glBindVertexArray(0);
}

Mesh::~Mesh()
{
    glDeleteVertexArrays(1, &m_vao);
    glDeleteBuffers(1, &m_vbo);
}

void Mesh::draw()
{
    glBindVertexArray(m_vao);
    glDrawArrays(m_shapeMode, 0, m_vertexCount);
}

void Mesh::updateData(const std::vector<float> &data)
{
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), data.data(), m_dataType);
    m_vertexCount = data.size() / m_vertexSize;
}