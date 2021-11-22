#ifndef GAMEDATA_HPP_
#define GAMEDATA_HPP_

#include <bitset>

enum class Input { Right, Left };
enum class State { Playing, GameOver };

struct Vertex {
    glm::vec3 position;

    bool operator==(const Vertex& other) const {
        return position == other.position;
    }
};

struct GameData {
    State m_state{State::Playing};
    std::bitset<5> m_input; // [left, right]
    float gameScore = 0;
};

#endif