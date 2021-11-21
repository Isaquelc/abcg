#ifndef ENEMIES_HPP_
#define ENEMIES_HPP_

#include <vector>

#include "abcg.hpp"
#include "gamedata.hpp"

struct Vertex {
    glm::vec3 position{};

    bool operator==(const Vertex& other) const noexcept {
        return position == other.position;
    }
};

class Player {
  public:
    void loadObj(std::string_view path, bool standardize = true);
    void initializeGL(GLuint program);
    void render(int numTriangles = -1) const;
    void setupVAO(GLuint program);
    void terminateGL();
    void update(const GameData m_gameData, float deltaTime);

    [[nodiscard]] int getNumTriangles() const {
      return static_cast<int>(m_indices.size()) / 3;
    }

  private:
    GLuint m_VAO{};
    GLuint m_VBO{};
    GLuint m_EBO{};

    std::vector<Vertex> m_vertices;
    std::vector<GLuint> m_indices;

    glm::vec3 m_translation{glm::vec3(0)};

    void createBuffers();
    void standardize();
};

#endif