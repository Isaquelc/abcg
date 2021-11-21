#ifndef OPENGLWINDOW_HPP_
#define OPENGLWINDOW_HPP_

#include <random>

#include "abcg.hpp"
#include "enemies.hpp"
#include "player.hpp"

class OpenGLWindow : public abcg::OpenGLWindow {
  protected:
    void handleEvent(SDL_Event& ev) override;
    void initializeGL() override;
    void paintGL() override;
    void paintUI() override;
    void resizeGL(int width, int height) override;
    void terminateGL() override;

  private:
    static const int m_numCars{5};

    GLuint m_program{};

    int m_viewportWidth{};
    int m_viewportHeight{};

    std::default_random_engine m_randomEngine;

    GameData m_gameData;
    Enemy m_enemies;
    Player m_player;

    std::array<glm::vec3, m_numCars> m_carPositions;
    glm::vec3 m_playerPosition;
    float m_angle{};

    glm::mat4 m_viewMatrix{1.0f};
    glm::mat4 m_projMatrix{1.0f};
    float m_FOV{50.0f};

    void randomizeCar(glm::vec3 &position);
    void update();
};

#endif