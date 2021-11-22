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
    // Load a new font
    ImGuiIO &io{ImGui::GetIO()};
    auto filename{getAssetsPath() + "Inconsolata-Medium.ttf"};
    m_font = io.Fonts->AddFontFromFileTTF(filename.c_str(), 60.0f);
    if (m_font == nullptr) {
        throw abcg::Exception{abcg::Exception::Runtime("Cannot load font file")};
    }

    abcg::glClearColor(0, 0, 0, 1);

    // Enable depth buffering
    abcg::glEnable(GL_DEPTH_TEST);

    // Create program
    m_program = createProgramFromFile(getAssetsPath() + "depth.vert",
                                        getAssetsPath() + "depth.frag");

    // Load model
    m_player.loadObj(getAssetsPath() + "DeLorean_DMC-12_V2.obj");
    m_enemies.loadObj(getAssetsPath() + "DeLorean_DMC-12_V2.obj");

    m_player.initializeGL(m_program);
    m_enemies.initializeGL(m_program);
    m_ground.initializeGL(m_program);

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
    abcg::OpenGLWindow::paintUI();

    {
        if (m_gameData.m_state == State::GameOver) {
            auto size{ImVec2(400, 400)};
            auto position{ImVec2((m_viewportWidth - size.x) / 2.0f, (m_viewportHeight - size.y) / 2.0f)};
            ImGui::SetNextWindowPos(position);
            ImGui::SetNextWindowSize(size);
            ImGuiWindowFlags flags{ImGuiWindowFlags_NoBackground |
                                ImGuiWindowFlags_NoTitleBar |
                                ImGuiWindowFlags_NoInputs};
            ImGui::Begin(" ", nullptr, flags);
            ImGui::PushFont(m_font);
            ImGui::Text("Game Over\nYour Score:\n%.2f Km", m_gameData.gameScore);
    }

    if (m_gameData.m_state == State::Playing) {
        auto size{ImVec2(600, 600)};
        auto position{ImVec2((m_viewportWidth - size.x) / 2.0f, (m_viewportHeight - size.y) / 2.0f)};
        ImGui::SetNextWindowPos(position);
        ImGui::SetNextWindowSize(size);
        ImGuiWindowFlags flags{ImGuiWindowFlags_NoBackground |
                            ImGuiWindowFlags_NoTitleBar |
                            ImGuiWindowFlags_NoInputs};
        ImGui::Begin(" ", nullptr, flags);
        ImGui::PushFont(m_font);
        ImGui::Text("Score:%.2f Km", m_gameData.gameScore);
    }

    ImGui::PopFont();
    ImGui::End();
    }
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

void OpenGLWindow::restart() {
    m_gameData.m_state = State::Playing;
    // reset score
    m_gameData.gameScore = 0;

    m_ground.initializeGL(m_program);
    m_player.initializeGL(m_program);
    m_enemies.initializeGL(m_program);
}

void OpenGLWindow::update() {
    const float deltaTime{static_cast<float>(getDeltaTime())};

    // increase score
    if (m_gameData.m_state == State::Playing) { 
        m_gameData.gameScore += 0.1 * deltaTime; 
        m_player.update(m_gameData, deltaTime);
        m_enemies.update(deltaTime);
        m_ground.update(deltaTime);
        checkCollisions();
    }
    
    // Wait 5 seconds before restarting
    if (m_gameData.m_state != State::Playing && m_restartWaitTimer.elapsed() > 3) {
        restart();
        return;
    }

    m_camera.computeViewMatrix();
}

void OpenGLWindow::checkCollisions() {
    // Check collision between Player and enemies
    for (const auto index : iter::range(m_enemies.m_numCars)) {
        auto &position{m_enemies.m_enemiesPositions.at(index)};
        const auto distance_x{std::abs(glm::distance(m_player.m_translation.x, position.x))};
        const auto distance_z{std::abs(glm::distance(m_player.m_translation.z, position.z))};

        if (distance_x < 0.7 && distance_z < 2.25) {
            m_gameData.m_state = State::GameOver;
            m_restartWaitTimer.restart();
        }
    }
}