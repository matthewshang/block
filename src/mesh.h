#pragma once

#include <vector>

#include <glad/glad.h>

class Mesh
{
public:
    Mesh();
    Mesh(const std::vector<float> &data, const std::vector<int> &vertexAttribs, 
         bool tris, bool staticDraw = true);
    ~Mesh();

    void draw();
    void updateData(const std::vector<float> &data);

private:
    GLuint m_vao;
    GLuint m_vbo;
    int m_vertexCount;
    int m_vertexSize;
    GLenum m_shapeMode;
    GLenum m_dataType;
}; 