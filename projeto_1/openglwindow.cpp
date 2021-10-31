#include "openglwindow.hpp"

#include <imgui.h>

#include "abcg.hpp"
#include "player.hpp"
#include "road.hpp"
#include "enemies.hpp"

void OpenGLWindow::handleEvent(SDL_Event &event) {
  // Keyboard events
  if (event.type == SDL_KEYDOWN) {
    if (event.key.keysym.sym == SDLK_UP || event.key.keysym.sym == SDLK_w)
      m_gameData.m_input.set(static_cast<size_t>(Input::Up));
    if (event.key.keysym.sym == SDLK_DOWN || event.key.keysym.sym == SDLK_s)
      m_gameData.m_input.set(static_cast<size_t>(Input::Down));
    if (event.key.keysym.sym == SDLK_LEFT || event.key.keysym.sym == SDLK_a)
      m_gameData.m_input.set(static_cast<size_t>(Input::Left));
    if (event.key.keysym.sym == SDLK_RIGHT || event.key.keysym.sym == SDLK_d)
      m_gameData.m_input.set(static_cast<size_t>(Input::Right));
  }
  if (event.type == SDL_KEYUP) {
    if (event.key.keysym.sym == SDLK_UP || event.key.keysym.sym == SDLK_w)
      m_gameData.m_input.reset(static_cast<size_t>(Input::Up));
    if (event.key.keysym.sym == SDLK_DOWN || event.key.keysym.sym == SDLK_s)
      m_gameData.m_input.reset(static_cast<size_t>(Input::Down));
    if (event.key.keysym.sym == SDLK_LEFT || event.key.keysym.sym == SDLK_a)
      m_gameData.m_input.reset(static_cast<size_t>(Input::Left));
    if (event.key.keysym.sym == SDLK_RIGHT || event.key.keysym.sym == SDLK_d)
      m_gameData.m_input.reset(static_cast<size_t>(Input::Right));
  }

  // Mouse events
  if (event.type == SDL_MOUSEBUTTONDOWN) {
    if (event.button.button == SDL_BUTTON_RIGHT)
      m_gameData.m_input.set(static_cast<size_t>(Input::Right));
  }
  if (event.type == SDL_MOUSEBUTTONUP) {
    if (event.button.button == SDL_BUTTON_RIGHT)
      m_gameData.m_input.reset(static_cast<size_t>(Input::Left));
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

  // Create program to render the other objects
  m_objectsProgram = createProgramFromFile(getAssetsPath() + "objects.vert", getAssetsPath() + "objects.frag");

  abcg::glClearColor(0, 0, 0, 1);

#if !defined(__EMSCRIPTEN__)
  abcg::glEnable(GL_PROGRAM_POINT_SIZE);
#endif

  // Start pseudo-random number generator
  m_randomEngine.seed(
      std::chrono::steady_clock::now().time_since_epoch().count());

  restart();
}

void OpenGLWindow::restart() {
  m_gameData.m_state = State::Playing;
  // reset score
  m_gameData.gameScore = 0;
  m_gameData.gameSpeed = 1;
  m_road.initializeGL(m_objectsProgram);
  m_enemies.initializeGL(m_objectsProgram, 4);
  m_player.initializeGL(m_objectsProgram);
}

void OpenGLWindow::update() {
  float deltaTime{static_cast<float>(getDeltaTime())};

  // increase score
  if (m_gameData.m_state == State::Playing) { 
    m_gameData.gameSpeed += 0.07 * deltaTime; 
    m_gameData.gameScore += 0.1 * m_gameData.gameSpeed * deltaTime; 
  }
  
  // Wait 5 seconds before restarting
  if (m_gameData.m_state != State::Playing && m_restartWaitTimer.elapsed() > 3) {
    restart();
    return;
  }

  m_road.update(m_gameData, deltaTime);
  m_enemies.update(m_gameData, deltaTime);
  m_player.update(m_gameData, deltaTime);

  if (m_gameData.m_state == State::Playing) {
    checkCollisions();
  }
}

void OpenGLWindow::paintGL() {
  update();
  abcg::glClearColor(0.4, 0.4, 0.4, 1);
  abcg::glClear(GL_COLOR_BUFFER_BIT);
  abcg::glViewport(0, 0, m_viewportWidth, m_viewportHeight);

  m_road.paintGL();
  m_enemies.paintGL();
  m_player.paintGL(m_gameData);
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

  abcg::glClear(GL_COLOR_BUFFER_BIT);
}

void OpenGLWindow::terminateGL() {
  abcg::glDeleteProgram(m_objectsProgram);

  m_road.terminateGL();
  m_enemies.terminateGL();
  m_player.terminateGL();
}

void OpenGLWindow::checkCollisions() {
  // Check collision between Player and enemies
  for (const auto &enemy : m_enemies.m_enemies) {
    const auto &enemyTranslation{enemy.m_translation};
    const auto distance_x{glm::distance(m_player.m_translation.x, enemyTranslation.x)};
    const auto distance_y{glm::distance(m_player.m_translation.y, enemyTranslation.y)};

    if (distance_x < 0.2 && distance_y < 0.4) {
      m_gameData.m_state = State::GameOver;
      m_restartWaitTimer.restart();
    }
  }
}