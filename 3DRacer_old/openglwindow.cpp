#include "openglwindow.hpp"

#include <imgui.h>

#include <cppitertools/itertools.hpp>
#include <glm/gtx/fast_trigonometry.hpp>

void OpenGLWindow::handleEvent(SDL_Event& event) {
  // Keyboard events
  if (event.type == SDL_KEYDOWN) {
    if (event.key.keysym.sym == SDLK_LEFT || event.key.keysym.sym == SDLK_a)
      m_gameData.m_input.set(static_cast<size_t>(Input::Left));
  }
}

void OpenGLWindow::initializeGL() {
  abcg::glClearColor(0, 0, 0, 1);

  // Enable depth buffering
  abcg::glEnable(GL_DEPTH_TEST);

  // Create program
  m_program = createProgramFromFile(getAssetsPath() + "depth.vert",
                                    getAssetsPath() + "depth.frag");

  // Load model
  m_enemies.loadObj(getAssetsPath() + "DeLorean_DMC-12_V2.obj");
  m_enemies.setupVAO(m_program);

  // Camera at (0,0,0) and looking towards the negative z
  m_viewMatrix =
      glm::lookAt(glm::vec3(0.0f, 0.5f, 0.0f), glm::vec3(0.0f, 0.5f, -1.0f),
                  glm::vec3(0.0f, 1.0f, 0.0f));

  // Setup cars
  for (const auto index : iter::range(m_numCars)) {
    auto &position{m_carPositions.at(index)};

    randomizeCar(position);
  }
  m_player.initializeGL(m_program);
}

void OpenGLWindow::randomizeCar(glm::vec3 &position) {
  // Get random position
  // x and y coordinates in the range [-5, 5]
  // z coordinates in the range [-100, 0]
  std::uniform_real_distribution<float> distPosXY(-5.0f, 5.0f);
  std::uniform_real_distribution<float> distPosZ(-100.0f, 0.0f);

  position = glm::vec3(distPosXY(m_randomEngine), 0,
                        distPosZ(m_randomEngine));

}

void OpenGLWindow::paintGL() {
  update();

  // Clear color buffer and depth buffer
  abcg::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  abcg::glViewport(0, 0, m_viewportWidth, m_viewportHeight);

  abcg::glUseProgram(m_program);

  // Get location of uniform variables (could be precomputed)
  const GLint viewMatrixLoc{
      abcg::glGetUniformLocation(m_program, "viewMatrix")};
  const GLint projMatrixLoc{
      abcg::glGetUniformLocation(m_program, "projMatrix")};
  const GLint modelMatrixLoc{
      abcg::glGetUniformLocation(m_program, "modelMatrix")};
  const GLint colorLoc{abcg::glGetUniformLocation(m_program, "color")};

  // Set uniform variables used by every scene object
  abcg::glUniformMatrix4fv(viewMatrixLoc, 1, GL_FALSE, &m_viewMatrix[0][0]);
  abcg::glUniformMatrix4fv(projMatrixLoc, 1, GL_FALSE, &m_projMatrix[0][0]);
  abcg::glUniform4f(colorLoc, 1.0f, 1.0f, 1.0f, 1.0f);  // White

  // Render each car
  for (const auto index : iter::range(m_numCars)) {
    const auto &position{m_carPositions.at(index)};

    // Compute model matrix of the current car
    glm::mat4 modelMatrix{1.0f};
    modelMatrix = glm::translate(modelMatrix, position);
    modelMatrix = glm::scale(modelMatrix, glm::vec3(1.0f));

    // Set uniform variable
    abcg::glUniformMatrix4fv(modelMatrixLoc, 1, GL_FALSE, &modelMatrix[0][0]);

    m_enemies.render();
  }
  
  // render player car

  // Compute model matrix of the current car
  glm::mat4 playerMatrix{1.0f};
  playerMatrix = glm::translate(playerMatrix, glm::vec3(0.0f, 0.0f, -5.0f));
  playerMatrix = glm::scale(playerMatrix, glm::vec3(1.0f));

  // Set uniform variables
  abcg::glUniformMatrix4fv(modelMatrixLoc, 1, GL_FALSE, &playerMatrix[0][0]);
  m_player.render();

  abcg::glUseProgram(0);
}

void OpenGLWindow::paintUI() {
  abcg::OpenGLWindow::paintUI();

  {
    const auto widgetSize{ImVec2(218, 62)};
    ImGui::SetNextWindowPos(ImVec2(m_viewportWidth - widgetSize.x - 5, 5));
    ImGui::SetNextWindowSize(widgetSize);
    ImGui::Begin("Widget window", nullptr, ImGuiWindowFlags_NoDecoration);

    {
      ImGui::PushItemWidth(120);
      static std::size_t currentIndex{};
      const std::vector<std::string> comboItems{"Perspective", "Orthographic"};

      if (ImGui::BeginCombo("Projection",
                            comboItems.at(currentIndex).c_str())) {
        for (const auto index : iter::range(comboItems.size())) {
          const bool isSelected{currentIndex == index};
          if (ImGui::Selectable(comboItems.at(index).c_str(), isSelected))
            currentIndex = index;
          if (isSelected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
      }
      ImGui::PopItemWidth();

      ImGui::PushItemWidth(170);
      const auto aspect{static_cast<float>(m_viewportWidth) /
                        static_cast<float>(m_viewportHeight)};
      if (currentIndex == 0) {
        m_projMatrix =
            glm::perspective(glm::radians(m_FOV), aspect, 0.01f, 100.0f);

        ImGui::SliderFloat("FOV", &m_FOV, 5.0f, 179.0f, "%.0f degrees");
      } else {
        m_projMatrix = glm::ortho(-20.0f * aspect, 20.0f * aspect, -20.0f,
                                  20.0f, 0.01f, 100.0f);
      }
      ImGui::PopItemWidth();
    }

    ImGui::End();
  }
}

void OpenGLWindow::resizeGL(int width, int height) {
  m_viewportWidth = width;
  m_viewportHeight = height;
}

void OpenGLWindow::terminateGL() {
  m_enemies.terminateGL();
  abcg::glDeleteProgram(m_program);
}

void OpenGLWindow::update() {
  // Animate angle by 90 degrees per second
  const float deltaTime{static_cast<float>(getDeltaTime())};
  m_angle = glm::wrapAngle(m_angle + glm::radians(90.0f) * deltaTime);

  // Update cars
  for (const auto index : iter::range(m_numCars)) {
    auto &position{m_carPositions.at(index)};

    // Z coordinate increases by 50 units per second
    position.z += deltaTime * 50.0f;

    // If this star is behind the camera, select a new random position and
    // orientation, and move it back to -100
    if (position.z > 0.1f) {
      randomizeCar(position);
      std::uniform_real_distribution<float> distPosZ(-100.0f, -150.0f);
      position.z = distPosZ(m_randomEngine);  // Back to at least -100
    }
  }
  m_player.update(m_gameData, deltaTime);
}