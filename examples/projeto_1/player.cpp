#include "player.hpp"

#include <glm/gtx/fast_trigonometry.hpp>

void Player::initializeGL(GLuint program) {
  terminateGL();

  // Define program
  m_program = program;
  m_colorLoc = abcg::glGetUniformLocation(m_program, "color");
  m_translationLoc = abcg::glGetUniformLocation(m_program, "translation");

  m_translation = glm::vec2(0);

  // Array of vertices
  std::array<glm::vec2, 12> positions{
      // Player body
      glm::vec2{-0.10f, +0.15f}, glm::vec2{+0.10f, +0.15f},
      glm::vec2{-0.10f, -0.15f}, glm::vec2{+0.10f, -0.15f}, 

      glm::vec2{-0.07f, +0.22f}, glm::vec2{+0.07f, +0.22f}, 
      glm::vec2{-0.07f, -0.20f}, glm::vec2{+0.07f, -0.20f},
      };

  const std::array<int, 6 * 3> indices{0, 1, 2,
                                       1, 2, 3, 
                                       0, 4, 1,
                                       1, 4, 5,
                                       2, 6, 7, 
                                       7, 2, 3};                         

  // Generate VBOs
  abcg::glGenBuffers(1, &m_vbo);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  abcg::glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions.data(),
                     GL_STATIC_DRAW);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Generate EBO
  abcg::glGenBuffers(1, &m_ebo);
  abcg::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
  abcg::glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices.data(),
                     GL_STATIC_DRAW);
  abcg::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  // Get location of attributes in the program
  GLint positionAttribute{abcg::glGetAttribLocation(m_program, "inPosition")};

  // Create VAO
  abcg::glGenVertexArrays(1, &m_vao);

  // Bind vertex attributes to current VAO
  abcg::glBindVertexArray(m_vao);

  abcg::glEnableVertexAttribArray(positionAttribute);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  abcg::glVertexAttribPointer(positionAttribute, 2, GL_FLOAT, GL_FALSE, 0,
                              nullptr);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);

  abcg::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);

  // End of binding to current VAO
  abcg::glBindVertexArray(0);
}

void Player::paintGL(const GameData &gameData) {
  if (gameData.m_state != State::Playing) return;

  abcg::glUseProgram(m_program);

  abcg::glBindVertexArray(m_vao);

  abcg::glUniform2fv(m_translationLoc, 1, &m_translation.x);

  abcg::glUniform4fv(m_colorLoc, 1, &m_color.r);
  abcg::glDrawElements(GL_TRIANGLES, 3 * 6, GL_UNSIGNED_INT, nullptr);

  abcg::glBindVertexArray(0);

  abcg::glUseProgram(0);
}

void Player::terminateGL() {
  abcg::glDeleteBuffers(1, &m_vbo);
  abcg::glDeleteBuffers(1, &m_ebo);
  abcg::glDeleteVertexArrays(1, &m_vao);
}

void Player::update(const GameData &gameData, float deltaTime) {
  // Move
  if (gameData.m_input[static_cast<size_t>(Input::Left)] && m_translation.x > -0.8)
    m_translation.x -= 2.0 * deltaTime;
  if (gameData.m_input[static_cast<size_t>(Input::Right)] && m_translation.x < 0.8)
    m_translation.x += 2.0 * deltaTime;
  if (gameData.m_input[static_cast<size_t>(Input::Up)] && m_translation.y < 0.7)
    m_translation.y += 2.0 * deltaTime;
  if (gameData.m_input[static_cast<size_t>(Input::Down)] && m_translation.y > -0.7)
    m_translation.y -= 2.0 * deltaTime;
}