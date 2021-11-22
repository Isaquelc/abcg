#include "openglwindow.hpp"
#include "gamedata.hpp"

#include <fmt/core.h>
#include <imgui.h>
#include <tiny_obj_loader.h>

#include <cppitertools/itertools.hpp>
#include <glm/gtx/fast_trigonometry.hpp>
#include <glm/gtx/hash.hpp>
#include <unordered_map>

// Explicit specialization of std::hash for Vertex
namespace std {
template <>
struct hash<Vertex> {
    size_t operator()(Vertex const& vertex) const noexcept {
        const std::size_t h1{std::hash<glm::vec3>()(vertex.position)};
        return h1;
    }
};
}  // namespace std

void OpenGLWindow::handleEvent(SDL_Event& ev) {
if (ev.type == SDL_KEYDOWN) {
    if (ev.key.keysym.sym == SDLK_LEFT || ev.key.keysym.sym == SDLK_a)
        m_gameData.m_input.set(static_cast<size_t>(Input::Left));
    if (ev.key.keysym.sym == SDLK_RIGHT || ev.key.keysym.sym == SDLK_d)
        m_gameData.m_input.set(static_cast<size_t>(Input::Right));
}
if (ev.type == SDL_KEYUP) {
    if (ev.key.keysym.sym == SDLK_LEFT || ev.key.keysym.sym == SDLK_a)
        m_gameData.m_input.reset(static_cast<size_t>(Input::Left));
    if (ev.key.keysym.sym == SDLK_RIGHT || ev.key.keysym.sym == SDLK_d)
        m_gameData.m_input.reset(static_cast<size_t>(Input::Right));
}
}

void OpenGLWindow::initializeGL() {
    abcg::glClearColor(0, 0, 0, 1);

    // Enable depth buffering
    abcg::glEnable(GL_DEPTH_TEST);

    // Create program
    m_program = createProgramFromFile(getAssetsPath() + "depth.vert",
                                        getAssetsPath() + "depth.frag");

    // initialize the ground
    m_ground.initializeGL(m_program);

    // Load model
    m_player.loadObj(getAssetsPath() + "DeLorean_DMC-12_V2.obj");
    m_enemies.loadObj(getAssetsPath() + "DeLorean_DMC-12_V2.obj");
    
    m_player.initializeGL(m_program);
    m_enemies.initializeGL(m_program);

    resizeGL(getWindowSettings().width, getWindowSettings().height);
}

void OpenGLWindow::paintGL() {
    update();

    // Clear color buffer and depth buffer
    abcg::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    abcg::glViewport(0, 0, m_viewportWidth, m_viewportHeight);

    abcg::glUseProgram(m_program);

    // Get location of uniform variables (could be precomputed)
    const GLint viewMatrixLoc{abcg::glGetUniformLocation(m_program, "viewMatrix")};
    const GLint projMatrixLoc{abcg::glGetUniformLocation(m_program, "projMatrix")};

    // Set uniform variables for viewMatrix and projMatrix
    // These matrices are used for every scene object
    abcg::glUniformMatrix4fv(viewMatrixLoc, 1, GL_FALSE, &m_camera.m_viewMatrix[0][0]);
    abcg::glUniformMatrix4fv(projMatrixLoc, 1, GL_FALSE, &m_camera.m_projMatrix[0][0]);

    m_ground.paintGL();
    m_player.paintGL();
    m_enemies.paintGL();
    abcg::glUseProgram(0);
}

void OpenGLWindow::paintUI() {
    // ImGui::PopFont();
    // ImGui::End();
}

void OpenGLWindow::resizeGL(int width, int height) {
    m_viewportWidth = width;
    m_viewportHeight = height;

    m_camera.computeProjectionMatrix(width, height);
}

void OpenGLWindow::terminateGL() {
    m_ground.terminateGL();
    m_player.terminateGL();
    m_enemies.terminateGL();

    abcg::glDeleteProgram(m_program);
    abcg::glDeleteBuffers(1, &m_EBO);
    abcg::glDeleteBuffers(1, &m_VBO);
    abcg::glDeleteVertexArrays(1, &m_VAO);
}

void OpenGLWindow::update() {
    const float deltaTime{static_cast<float>(getDeltaTime())};
    m_ground.update(deltaTime);
    m_player.update(m_gameData, deltaTime);
    m_enemies.update(deltaTime);
    m_camera.computeViewMatrix();
}