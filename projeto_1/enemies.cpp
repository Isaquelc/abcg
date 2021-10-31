#include "enemies.hpp"

#include <cppitertools/itertools.hpp>
#include <glm/gtx/fast_trigonometry.hpp>

void Enemies::initializeGL(GLuint program, int quantity) {
  terminateGL();
  // Start pseudo-random number generator
  m_randomEngine.seed(
    std::chrono::steady_clock::now().time_since_epoch().count());

  auto &re{m_randomEngine}; // Shortcut

  m_program = program;
  m_colorLoc = abcg::glGetUniformLocation(m_program, "color");
  m_translationLoc = abcg::glGetUniformLocation(m_program, "translation");

  // Create enemy cars
  m_enemies.clear();
  m_enemies.resize(quantity);

  for (auto &enemy : m_enemies) {
    enemy = createEnemy();

    // Make sure enemies spawn far enough from player (offscreen)
    // Choose a starting x position
    std::uniform_real_distribution<float> randomX(-0.8f, 0.8f);
    
    enemy.m_translation.x = randomX(re);

    // Choose a random y length to add
    std::uniform_real_distribution<float> randomY(2.2f, 3.0f);
    enemy.m_translation.y = randomY(re);
  }
 }

void Enemies::paintGL() {
  abcg::glUseProgram(m_program);

  for (const auto &enemy : m_enemies) {
    abcg::glBindVertexArray(enemy.m_vao);

    abcg::glUniform4fv(m_colorLoc, 1, &enemy.m_color.r);

    //for (auto i : {-2, 0, 2}) {
    abcg::glUniform2f(m_translationLoc, enemy.m_translation.x, enemy.m_translation.y);

    abcg::glDrawElements(GL_TRIANGLES, 3 * 6, GL_UNSIGNED_INT, nullptr);
    //}
    abcg::glBindVertexArray(0);
  }
  abcg::glUseProgram(0);
}

void Enemies::terminateGL() {
  for (auto enemy : m_enemies) {
    abcg::glDeleteBuffers(1, &enemy.m_vbo);
    abcg::glDeleteVertexArrays(1, &enemy.m_vao);
  }
}

void Enemies::update(const GameData &gameData, float deltaTime) {
  auto &re{m_randomEngine}; // Shortcut

  for (auto  &enemy : m_enemies) {
    enemy.m_translation.y -= 1.5 * deltaTime * gameData.gameSpeed;
    //enemy.m_translation += glm::vec2({-2.0 * deltaTime, 0});
    //Wrap-around
    if (enemy.m_translation.y < -1.2f) {
      // Choose a starting x position
      std::uniform_real_distribution<float> randomX(-0.8f, 0.8f);
      
      enemy.m_translation.x = randomX(re);

      // Choose a random y length to add
      std::uniform_real_distribution<float> randomY(0.0f, 1.0f);
      enemy.m_translation.y += 2.4f + randomY(re);
    }
  }
}

Enemies::Enemy Enemies::createEnemy(glm::vec2 translation) {

  Enemy enemy;

  auto &re{m_randomEngine}; // Shortcut

  // Choose a random color
  std::uniform_real_distribution<float> randomR(0.0f, 1.0f);
  std::uniform_real_distribution<float> randomG(0.0f, 1.0f);
  std::uniform_real_distribution<float> randomB(0.0f, 1.0f);
  enemy.m_color = glm::vec4(randomR(re), randomG(re), randomB(re), 1);

  // Choose a random starting position
  //std::uniform_real_distribution<float> randomX(-1.0f, +1.0f);
  enemy.m_translation = translation;

  // Create geometry

  // Array of vertices
  std::array<glm::vec2, 12> positions{
      // Player body
      glm::vec2{-0.10f, +0.15f}, glm::vec2{+0.10f, +0.15f},
      glm::vec2{-0.10f, -0.15f}, glm::vec2{+0.10f, -0.15f},

      glm::vec2{-0.07f, +0.20f}, glm::vec2{+0.07f, +0.20f},
      glm::vec2{-0.07f, -0.22f}, glm::vec2{+0.07f, -0.22f},
      };

  const std::array<int, 6 * 3> indices{0, 1, 2,
                                       1, 2, 3,
                                       0, 4, 1,
                                       1, 4, 5,
                                       2, 6, 7,
                                       7, 2, 3};

  // Generate VBOs
  abcg::glGenBuffers(1, &enemy.m_vbo);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, enemy.m_vbo);
  abcg::glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions.data(),
                     GL_STATIC_DRAW);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Generate EBO
  abcg::glGenBuffers(1, &enemy.m_ebo);
  abcg::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, enemy.m_ebo);
  abcg::glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices.data(),
                     GL_STATIC_DRAW);
  abcg::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  // Get location of attributes in the program
  GLint positionAttribute{abcg::glGetAttribLocation(m_program, "inPosition")};

  // Create VAO
  abcg::glGenVertexArrays(1, &enemy.m_vao);

  // Bind vertex attributes to current VAO
  abcg::glBindVertexArray(enemy.m_vao);

  abcg::glEnableVertexAttribArray(positionAttribute);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, enemy.m_vbo);
  abcg::glVertexAttribPointer(positionAttribute, 2, GL_FLOAT, GL_FALSE, 0,
                              nullptr);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);

  abcg::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, enemy.m_ebo);

  // End of binding to current VAO
  abcg::glBindVertexArray(0);

  return enemy;
}