#ifndef ROAD_HPP_
#define ROAD_HPP_

#include <array>
#include <random>

#include "abcg.hpp"
#include "gamedata.hpp"

class OpenGLWindow;

class Road {
  public:
    void initializeGL(GLuint program);
    void paintGL();
    void terminateGL();

    void update(const GameData &gameData, float deltaTime);

  private:
    GLuint m_program{};
    GLint m_colorLoc{};
    GLint m_translationLoc{};

    GLuint m_vao{};
    GLuint m_vbo{};
    GLuint m_ebo{};

    glm::vec4 m_color{1, 1, 0, 1};
    glm::vec2 m_translation{glm::vec2(0)};
};
#endif