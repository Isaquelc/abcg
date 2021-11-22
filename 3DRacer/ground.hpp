#ifndef GROUND_HPP_
#define GROUND_HPP_

#include "abcg.hpp"
#include "gamedata.hpp"

class OpenGLWindow;

class Ground {
    public:
        void initializeGL(GLuint program);
        void paintGL();
        void terminateGL();
        void update(float deltaTime);

    private:

        friend OpenGLWindow;

        GLuint m_VAO{};
        GLuint m_VBO{};
        GLuint m_program{};

        std::array<glm::vec3, 500> m_tilesPositions;
        std::array<float, 500> m_tilesColors;

        GLint m_modelMatrixLoc{};
        GLint m_colorLoc{};
};

#endif