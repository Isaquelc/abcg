#ifndef OPENGLWINDOW_HPP_
#define OPENGLWINDOW_HPP_

#include <vector>

#include "abcg.hpp"
#include "player.hpp"
#include "gamedata.hpp"
#include "camera.hpp"

class OpenGLWindow : public abcg::OpenGLWindow {
    protected:
        void handleEvent(SDL_Event& ev) override;
        void initializeGL() override;
        void paintGL() override;
        void paintUI() override;
        void resizeGL(int width, int height) override;
        void terminateGL() override;

    private:
        GLuint m_VAO{};
        GLuint m_VBO{};
        GLuint m_EBO{};
        GLuint m_program{};

        GameData m_gameData;
        Player m_player;
        Camera m_camera;

        int m_viewportWidth{};
        int m_viewportHeight{};

        std::vector<Vertex> m_vertices;
        std::vector<GLuint> m_indices;

        void update();
};

#endif