#include "player.hpp"

#include <fmt/core.h>
#include <tiny_obj_loader.h>

#include <cppitertools/itertools.hpp>
#include <glm/gtx/hash.hpp>
#include <unordered_map>

// Explicit specialization of std::hash for Vertex
namespace std {
template <>
struct hash<Vertex> {
    size_t operator()(Vertex const& vertex) const noexcept {
        const std::size_t h1{std::hash<glm::vec3>()(vertex.position)};
        return h1;
    }
    };
}  // namespace std

void Player::loadObj(std::string_view path, bool standardize) {
    tinyobj::ObjReader reader;

    if (!reader.ParseFromFile(path.data())) {
        if (!reader.Error().empty()) {
        throw abcg::Exception{abcg::Exception::Runtime(
            fmt::format("Failed to load model {} ({})", path, reader.Error()))};
        }
        throw abcg::Exception{
            abcg::Exception::Runtime(fmt::format("Failed to load model {}", path))};
    }

    if (!reader.Warning().empty()) {
        fmt::print("Warning: {}\n", reader.Warning());
    }

    const auto& attrib{reader.GetAttrib()};
    const auto& shapes{reader.GetShapes()};

    m_vertices.clear();
    m_indices.clear();

    // A key:value map with key=Vertex and value=index
    std::unordered_map<Vertex, GLuint> hash{};

    // Loop over shapes
    for (const auto& shape : shapes) {
        // Loop over indices
        for (const auto offset : iter::range(shape.mesh.indices.size())) {
            // Access to vertex
            const tinyobj::index_t index{shape.mesh.indices.at(offset)};

            // Vertex position
            const int startIndex{3 * index.vertex_index};
            const float vx{attrib.vertices.at(startIndex + 0)};
            const float vy{attrib.vertices.at(startIndex + 1)};
            const float vz{attrib.vertices.at(startIndex + 2)};

            Vertex vertex{};
            vertex.position = {vx, vy, vz};

            // If hash doesn't contain this vertex
            if (hash.count(vertex) == 0) {
                // Add this index (size of m_vertices)
                hash[vertex] = m_vertices.size();
                // Add this vertex
                m_vertices.push_back(vertex);
            }

            m_indices.push_back(hash[vertex]);
        }
    }

    if (standardize) {
        this->standardize();
    }

    // createBuffers();
}

void Player::standardize() {
    // Center to origin and normalize largest bound to [-1, 1]

    // Get bounds
    glm::vec3 max(std::numeric_limits<float>::lowest());
    glm::vec3 min(std::numeric_limits<float>::max());
    for (const auto& vertex : m_vertices) {
        max.x = std::max(max.x, vertex.position.x);
        max.y = std::max(max.y, vertex.position.y);
        max.z = std::max(max.z, vertex.position.z);
        min.x = std::min(min.x, vertex.position.x);
        min.y = std::min(min.y, vertex.position.y);
        min.z = std::min(min.z, vertex.position.z);
    }

    // Center and scale
    const auto center{(min + max) / 2.0f};
    const auto scaling{2.0f / glm::length(max - min)};
    for (auto& vertex : m_vertices) {
        vertex.position = (vertex.position - center) * scaling;
    }
}

void Player::initializeGL(GLuint program) {

    m_translation = glm::vec3(0.0f, 0.0f, -5.0f);
    m_angle = 180.0f;

    m_program = program;

    // Generate VBO
    abcg::glGenBuffers(1, &m_VBO);
    abcg::glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    abcg::glBufferData(GL_ARRAY_BUFFER, sizeof(m_vertices[0]) * m_vertices.size(), m_vertices.data(), GL_STATIC_DRAW);
    abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Generate EBO
    abcg::glGenBuffers(1, &m_EBO);
    abcg::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    abcg::glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(m_indices[0]) * m_indices.size(), m_indices.data(), GL_STATIC_DRAW);
    abcg::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // Create VAO
    abcg::glGenVertexArrays(1, &m_VAO);

    // Bind vertex attributes to current VAO
    abcg::glBindVertexArray(m_VAO);

    abcg::glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    const GLint positionAttribute{abcg::glGetAttribLocation(m_program, "inPosition")};
    abcg::glEnableVertexAttribArray(positionAttribute);
    abcg::glVertexAttribPointer(positionAttribute, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), nullptr);
    abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);

    abcg::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);

    // End of binding to current VAO
    abcg::glBindVertexArray(0);
}

void Player::paintGL() {
    abcg::glUseProgram(m_program);
    abcg::glBindVertexArray(m_VAO);

    // Get location of uniform variables (could be precomputed)
    const GLint modelMatrixLoc{abcg::glGetUniformLocation(m_program, "modelMatrix")};
    const GLint colorLoc{abcg::glGetUniformLocation(m_program, "color")};

    m_playerPos = glm::mat4{1.0f};
    m_playerPos = glm::translate(m_playerPos, m_translation); // moves player slightly forward
    m_playerPos = glm::rotate(m_playerPos, glm::radians(m_angle), glm::vec3(0.0f, 1.0f, 0.0f)); // no initial rotation
    m_playerPos = glm::scale(m_playerPos, glm::vec3(1.0f)); // no further scaling

    abcg::glUniformMatrix4fv(modelMatrixLoc, 1, GL_FALSE, &m_playerPos[0][0]);
    abcg::glUniform4f(colorLoc, 1.0f, 1.0f, 1.0f, 1.0f);
    abcg::glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, nullptr);

    abcg::glBindVertexArray(0);
    // abcg::glUseProgram(0);
}

void Player::update(const GameData &gameData, float deltaTime) {
    // Move
    if (gameData.m_input[static_cast<size_t>(Input::Left)] && m_translation.x>-2) {
        m_translation.x -= 5.0 * deltaTime;
        if (m_angle < 180) {
            m_angle = 180;
        }
        else if (m_angle < 200) {
            m_angle += 200 * deltaTime;
        }
        // m_angle = 225;
    }
    if (gameData.m_input[static_cast<size_t>(Input::Right)] && m_translation.x<2) {
        m_translation.x += 5.0 * deltaTime;
        if (m_angle > 180) {
            m_angle = 180;
        }
        else if (m_angle > 160) {
            m_angle -= 200 * deltaTime;
        }
        // m_angle = 135;
    }
    if (! gameData.m_input[static_cast<size_t>(Input::Left)] && 
        ! gameData.m_input[static_cast<size_t>(Input::Right)] &&
        m_angle > 180)
        m_angle -= 200 * deltaTime;
    if (! gameData.m_input[static_cast<size_t>(Input::Left)] && 
        ! gameData.m_input[static_cast<size_t>(Input::Right)] &&
        m_angle < 180)
        m_angle += 200 * deltaTime;
}

void Player::terminateGL() {
    abcg::glDeleteBuffers(1, &m_VBO);
    abcg::glDeleteVertexArrays(1, &m_VAO);
}