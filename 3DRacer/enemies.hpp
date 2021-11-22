#ifndef ENEMY_HPP_
#define ENEMY_HPP_

#include <vector>
#include <random>

#include "abcg.hpp"
#include "gamedata.hpp"

class OpenGLWindow;

class Enemy {
    public:
        void loadObj(std::string_view path, bool standardize = true);
        void initializeGL(GLuint program);
        void paintGL();
        void setupVAO(GLuint program);
        void terminateGL();
        void update(float deltaTime);

        [[nodiscard]] int getNumTriangles() const {
        return static_cast<int>(m_indices.size()) / 3;
        }

    private:

        friend OpenGLWindow;

        static const int m_numCars{5};

        GLuint m_VAO{};
        GLuint m_VBO{};
        GLuint m_EBO{};
        GLuint m_program{};

        std::default_random_engine m_randomEngine;

        std::vector<Vertex> m_vertices;
        std::vector<GLuint> m_indices;

        std::array<glm::vec3, m_numCars> m_enemiesPositions;
        std::array<glm::vec4, m_numCars> m_enemiesColors;

        void standardize();
        void randomizeCar(glm::vec3 &position, glm::vec4 &color);
};

#endif