#include "game.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "chunk.h"
#include "shader.h"
#include "texture.h"

Game::Game(GLFWwindow *window) : m_window(window), m_camera(glm::vec3(0.0f, 0.5f, 2.0f)), 
m_lastX(960), m_lastY(540), m_firstMouse(true)
{
    
}

void Game::run()
{
    Shader shader("../res/shaders/simple_vertex.glsl", "../res/shaders/simple_fragment.glsl");

    Texture texture1("../res/textures/dirt.jpg", GL_RGB);

    Chunk chunk;

    glEnable(GL_DEPTH_TEST);

    shader.bind();
    shader.setInt("texture1", 0);

    glm::mat4 projection;
    projection = glm::perspective(glm::radians(45.0f), static_cast<float>(1920) / static_cast<float>(1080), 0.1f, 100.0f);

    float dt = 0.0f;
    double lastFrame = 0.0f;
    long nFrames = 0;
    double lastTime = 0.0f;

    while (!glfwWindowShouldClose(m_window))
    {
        processInput(dt);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        //glEnable(GL_CULL_FACE);

        shader.bind();

        glm::mat4 model;

        glm::mat4 view = m_camera.getView();

        shader.setMat4("model", model);
        shader.setMat4("view", m_camera.getView());
        shader.setMat4("projection", projection);

        texture1.bind(GL_TEXTURE0);

        chunk.bind();
        glDrawArrays(GL_TRIANGLES, 0, chunk.getVertexCount());

        glfwSwapBuffers(m_window);
        glfwPollEvents();

        double current = glfwGetTime();
        dt = static_cast<float>(current - lastFrame);
        lastFrame = current;

        nFrames++;
        if (current - lastTime > 1.0f)
        {
            char title[256];
            title[255] = '\0';
            snprintf(title, 255, "block - [FPS: %d]", nFrames);
            glfwSetWindowTitle(m_window, title);
            lastTime += 1.0f;
            nFrames = 0;
        }
    }
}

void Game::processInput(float dt)
{
    if (glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(m_window, true);
    }

    float camSpeed = 2.5f * dt;
    if (glfwGetKey(m_window, GLFW_KEY_W) == GLFW_PRESS)
    {
        m_camera.processKeyboard(Camera::Movement::FOWARD, dt);
    }
    if (glfwGetKey(m_window, GLFW_KEY_S) == GLFW_PRESS)
    {
        m_camera.processKeyboard(Camera::Movement::BACKWARD, dt);
    }
    if (glfwGetKey(m_window, GLFW_KEY_A) == GLFW_PRESS)
    {
        m_camera.processKeyboard(Camera::Movement::LEFT, dt);
    }
    if (glfwGetKey(m_window, GLFW_KEY_D) == GLFW_PRESS)
    {
        m_camera.processKeyboard(Camera::Movement::RIGHT, dt);
    }

    double xpos, ypos;
    glfwGetCursorPos(m_window, &xpos, &ypos);
    if (m_firstMouse)
    {
        m_lastX = xpos;
        m_lastY = ypos;
        m_firstMouse = false;
    }
    float dx = xpos - m_lastX;
    float dy = m_lastY - ypos;
    m_lastX = xpos;
    m_lastY = ypos;

    m_camera.processMouse(dx, dy);
}