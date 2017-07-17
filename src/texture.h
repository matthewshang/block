#pragma once

#include <glad/glad.h>

class Texture
{
public:
    Texture(const char *path, GLint format);
    ~Texture();

    void bind(GLenum slot);

private:
    GLuint m_id;
};