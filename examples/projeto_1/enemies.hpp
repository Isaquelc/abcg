#ifndef ENEMIES_HPP_
#define ENEMIES_HPP_

#include <list>
#include <array>
#include <random>

#include "abcg.hpp"
#include "gamedata.hpp"
#include "player.hpp"

class OpenGLWindow;

class Enemies {
  public:
    void initializeGL(GLuint program, int quantity);
    void paintGL();
    void terminateGL();
    void update(const GameData &gameData, float deltaTime);

  private:
    friend OpenGLWindow;

    GLuint m_program{};
    GLint m_colorLoc{};
    GLint m_translationLoc{};

    struct Enemy {
      GLuint m_vao{};
      GLuint m_vbo{};
      GLuint m_ebo{};

      glm::vec4 m_color{1};
      bool m_hit{false};
      glm::vec2 m_translation{glm::vec2(0)};
    };
    
    std::list<Enemy> m_enemies;

    std::default_random_engine m_randomEngine;
    std::uniform_real_distribution<float> m_randomDist{-1.0f, 1.0f};

    Enemies::Enemy createEnemy(glm::vec2 translation = glm::vec2(0));
};
#endif