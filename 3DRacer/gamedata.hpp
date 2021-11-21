#ifndef GAMEDATA_HPP_
#define GAMEDATA_HPP_

#include <bitset>

enum class Input { Right, Left };

struct Vertex {
    glm::vec3 position;

    bool operator==(const Vertex& other) const {
        return position == other.position;
    }
};

struct GameData {
    std::bitset<5> m_input; // [left, right]
};

#endif