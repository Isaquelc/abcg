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
        void terminateGL();
        void update(const GameData &gameData, float deltaTime);

        [[nodiscard]] int getNumTriangles() const {
        return static_cast<int>(m_indices.size()) / 3;
        }

    private:
        friend OpenGLWindow;

        static const int m_numCars{5};

        Camera m_camera;

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
        void randomizeCar(glm::vec3 &position, glm::vec4 &m_Kd);
        void computeNormals();
        // Mapping mode
        // 0: triplanar; 1: cylindrical; 2: spherical; 3: from mesh; 4: standard blinn-phong
        int m_mappingMode{4}; 

        // Light and material properties
        glm::vec4 m_Ka{0.05f, 0.07f, 0.1f, 1.0f};
        glm::vec4 m_Ks{0.3f, 0.3f, 0.3f, 1.0f};
        float m_shininess{5.0f};

        bool m_hasNormals{false};
};

#endif