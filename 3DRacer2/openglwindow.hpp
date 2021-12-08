#ifndef OPENGLWINDOW_HPP_
#define OPENGLWINDOW_HPP_

#include <imgui.h>

#include <vector>

#include "abcg.hpp"
#include "player.hpp"
#include "gamedata.hpp"
#include "camera.hpp"
#include "ground.hpp"
#include "enemies.hpp"

class OpenGLWindow : public abcg::OpenGLWindow {
    protected:
        void handleEvent(SDL_Event& ev) override;
        void initializeGL() override;
        void paintGL() override;
        void paintUI() override;
        void resizeGL(int width, int height) override;
        void terminateGL() override;
        void checkCollisions();

    private:
        GLuint m_VAO{};
        GLuint m_VBO{};
        GLuint m_EBO{};
        GLuint m_program{};

        GameData m_gameData;
        Player m_player;
        Camera m_camera;
        Ground m_ground;
        Enemy m_enemies;

        abcg::ElapsedTimer m_restartWaitTimer;
        ImFont* m_font{};

        int m_viewportWidth{};
        int m_viewportHeight{};

        std::vector<Vertex> m_vertices;
        std::vector<GLuint> m_indices;
        
        // Light and material properties
        glm::vec4 m_lightDir{-0.25f, -1.0f, 0.25f, 0.0f};
        glm::vec4 m_Ia{1.0f, 1.0f, 1.0f, 1.0f};
        glm::vec4 m_Id{1.0f, 1.0f, 1.0f, 1.0f};
        glm::vec4 m_Is{1.0f, 1.0f, 1.0f, 1.0f};

        std::array<float, 4> m_clearColor{0.5f, 0.7f, 1.0f, 1.0f};

        void update();
        void restart();
};

#endif