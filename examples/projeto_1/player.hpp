#ifndef PLAYER_HPP_
#define PLAYER_HPP_

#include "abcg.hpp"
#include "gamedata.hpp"

class OpenGLWindow;

class Player {
  public:
    void initializeGL(GLuint program);
    void paintGL(const GameData &gameData);
    void terminateGL();

    void update(const GameData &gameData, float deltaTime);
    void setTranslation(glm::vec2 translation) { m_translation = translation; }

  private:
  
    friend OpenGLWindow;

    GLuint m_program{};
    GLint m_colorLoc{};
    GLint m_translationLoc{};

    GLuint m_vao{};
    GLuint m_vbo{};
    GLuint m_ebo{};

    glm::vec4 m_color{1, 0.3, 0.3, 1};
    glm::vec2 m_translation{glm::vec2(0)};
};
#endif