#ifndef GAMEDATA_HPP_
#define GAMEDATA_HPP_

#include <bitset>

enum class Input { Right, Left, Down, Up };
enum class State { Playing, GameOver };

struct GameData {
  State m_state{State::Playing};
  std::bitset<5> m_input;  // [up, down, left, right]
  float gameScore = 0;
  float gameSpeed = 1;
};

#endif