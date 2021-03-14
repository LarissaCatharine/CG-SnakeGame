#ifndef ELEMENTS_HPP_
#define ELEMENTS_HPP_

#include <list>
#include <random>

#include "abcg.hpp"
#include "gamedata.hpp"

class OpenGLWindow;

class Elements {
 public:
  void initializeGL(GLuint program, int quantity);
  void paintGL();
  void terminateGL();
  void update(const GameData &gameData, float deltaTime);

 private:
  friend OpenGLWindow;

  GLuint m_program{};
  GLint m_colorLoc{};
  GLint m_rotationLoc{};
  GLint m_translationLoc{};
  GLint m_scaleLoc{};

  struct Element {
    GLuint m_vao{};
    GLuint m_vbo{};

    float m_angularVelocity{};
    glm::vec4 m_color{1};
    bool m_hit{false};
    int m_polygonSides{};
    float m_rotation{};
    float m_scale{};
    Input lastDirection{Input::Up};
    glm::vec2 m_translation{glm::vec2(0)};
    glm::vec2 m_velocity{glm::vec2(0)};
  };

  std::list<Element> m_snake;
  std::list<Element> m_apples;

  std::default_random_engine m_randomEngine;
  std::uniform_real_distribution<float> m_randomDist{-1.0f, 1.0f};

  Elements::Element createSnake();
  Elements::Element createApple();
};

#endif
