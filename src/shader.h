#pragma once

#include <glad\glad.h>
#include <glm\glm.hpp>

#include <string>

class Shader
{
public:
    Shader(const char *vertexPath, const char *fragmentPath);
    ~Shader();

    void bind();

    void setBool(const std::string &name, bool value) const;
    void setInt(const std::string &name, int value) const;
    void setFloat(const std::string &name, float value) const;
    void setMat4(const std::string &name, glm::mat4 &value) const;
    void setVec3(const std::string &name, glm::vec3 &value) const;

    GLuint getProgram() const;

private:
    GLuint m_program;
};