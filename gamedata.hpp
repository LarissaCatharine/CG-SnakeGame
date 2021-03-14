#ifndef GAMEDATA_HPP_
#define GAMEDATA_HPP_

#include <bitset>

enum class Input { Right, Left, Down, Up };
enum class State { Stopped, Playing, Win }; // Stopped = começo do jogo; Win = comer todas as maçãs

struct GameData {
  State m_state{State::Stopped};
  std::bitset<4> m_input;  // [up, down, left, right]
};

#endif
