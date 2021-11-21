#ifndef MODEL_HPP_
#define MODEL_HPP_

#include <vector>

#include "abcg.hpp"
#include "gamedata.hpp"

class OpenGLWindow;

class Player {
    public:
        void loadObj(std::string_view path, bool standardize = true);
        void initializeGL(GLuint program);
        void paintGL();
        void setupVAO(GLuint program);
        void terminateGL();
        void update(const GameData &gameData, float deltaTime);

        [[nodiscard]] int getNumTriangles() const {
        return static_cast<int>(m_indices.size()) / 3;
        }

    private:

        friend OpenGLWindow;

        GLuint m_VAO{};
        GLuint m_VBO{};
        GLuint m_EBO{};
        GLuint m_program{};

        std::vector<Vertex> m_vertices;
        std::vector<GLuint> m_indices;

        glm::vec3 m_translation{glm::vec3(0.0f)};
        float m_angle{};
        glm::mat4 m_playerPos{glm::mat4{0.0f}};

        void createBuffers();
        void standardize();
};

#endif