#ifndef GAMEDATA_HPP_
#define GAMEDATA_HPP_

#include <bitset>
#include <glm/gtc/matrix_inverse.hpp>
#include "camera.hpp"

enum class Input { Right, Left };
enum class State { Playing, GameOver };

struct Vertex {
    glm::vec3 position{};
    glm::vec3 normal{};
    glm::vec2 texCoord{};

    bool operator==(const Vertex& other) const noexcept {
        static const auto epsilon{std::numeric_limits<float>::epsilon()};
        return glm::all(glm::epsilonEqual(position, other.position, epsilon)) &&
            glm::all(glm::epsilonEqual(normal, other.normal, epsilon)) &&
            glm::all(glm::epsilonEqual(texCoord, other.texCoord, epsilon));
    }
};

struct GameData {
    State m_state{State::Playing};
    std::bitset<5> m_input; // [left, right]
    float gameScore = 0;
    float gameSpeed = 1;
};

#endif