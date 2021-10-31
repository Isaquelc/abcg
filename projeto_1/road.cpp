#include "road.hpp"

#include <glm/gtx/fast_trigonometry.hpp>
#include <cppitertools/itertools.hpp>

void Road::initializeGL(GLuint program) {
  terminateGL();
  
  m_program = program;
  m_colorLoc = abcg::glGetUniformLocation(m_program, "color");
  m_translationLoc = abcg::glGetUniformLocation(m_program, "translation");

  m_translation = glm::vec2(0);

  // Array of vertices
  std::array<glm::vec2, 4> positions{
      // Road body
      glm::vec2{-0.02f, +0.06f}, glm::vec2{+0.02f, +0.06f},
      glm::vec2{-0.02, -0.06f}, glm::vec2{+0.02f, -0.06f}
      };

  const std::array<int, 6> indices{0, 1, 2,
                           1, 2, 3};

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

void Road::paintGL() {
  abcg::glUseProgram(m_program);

  abcg::glBindVertexArray(m_vao);

  abcg::glUniform4fv(m_colorLoc, 1, &m_color.r);

  abcg::glUniform2fv(m_translationLoc, 1, &m_translation.x);
  for (const auto i: {-4, -3, -2, -1, 0, 1, 2, 2, 3}) {
    abcg::glUniform2f(m_translationLoc, m_translation.x, m_translation.y+(0.5 * i));
    abcg::glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
  }

  abcg::glBindVertexArray(0);

  abcg::glUseProgram(0);
}

void Road::terminateGL() {
  abcg::glDeleteBuffers(1, &m_vbo);
  abcg::glDeleteBuffers(1, &m_ebo);
  abcg::glDeleteVertexArrays(1, &m_vao);
}

void Road::update(const GameData &gameData, float deltaTime) {
  // Move road back
  m_translation.y -= deltaTime * gameData.gameSpeed;

  // Wrap-around
  if (m_translation.y < -1.0f) m_translation.y += 2.0f;
  if (m_translation.y > +1.0f) m_translation.y -= 2.0f;
}