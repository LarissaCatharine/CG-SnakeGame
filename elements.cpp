#include "elements.hpp"

#include <cppitertools/itertools.hpp>
#include <glm/gtx/fast_trigonometry.hpp>

void Elements::initializeGL(GLuint program, int quantity) {
  terminateGL();

  // Start pseudo-random number generator
  auto seed{std::chrono::steady_clock::now().time_since_epoch().count()};
  m_randomEngine.seed(seed);

  m_program = program;
  m_colorLoc = glGetUniformLocation(m_program, "color");
  m_rotationLoc = glGetUniformLocation(m_program, "rotation");
  m_scaleLoc = glGetUniformLocation(m_program, "scale");
  m_translationLoc = glGetUniformLocation(m_program, "translation");

  // Create snake
  m_snake.clear();
  m_snake.resize(1);

  // Create apples
  m_apples.clear();
  m_apples.resize(quantity);

  Element head = createSnake();
  head.m_translation = {0.0f, 0.0f}; // para iniciar no centro da tela
  m_snake.push_back(head);

  for (auto &apple : m_apples) {
    apple = createApple();
    do {
      apple.m_translation = {m_randomDist(m_randomEngine),
                                m_randomDist(m_randomEngine)};
    } while (glm::length(apple.m_translation) < 0.5f);
  }
}

void Elements::paintGL() {
	glUseProgram(m_program);

	for (auto &snake : m_snake) {
		glBindVertexArray(snake.m_vao);

		glUniform4fv(m_colorLoc, 1, &snake.m_color.r);
		glUniform1f(m_scaleLoc, snake.m_scale);
		glUniform1f(m_rotationLoc, snake.m_rotation);

		glUniform2f(m_translationLoc, snake.m_translation.x, snake.m_translation.y);
		glDrawArrays(GL_TRIANGLE_FAN, 0, snake.m_polygonSides + 2);

		glBindVertexArray(0);
	}

	for (auto &apple : m_apples) {
		glBindVertexArray(apple.m_vao);

		glUniform4fv(m_colorLoc, 1, &apple.m_color.r);
		glUniform1f(m_scaleLoc, apple.m_scale);
		glUniform1f(m_rotationLoc, apple.m_rotation);

		glUniform2f(m_translationLoc, apple.m_translation.x, apple.m_translation.y);
		glDrawArrays(GL_TRIANGLE_FAN, 0, apple.m_polygonSides + 2);

		glBindVertexArray(0);
	}

	glUseProgram(0);
}

void Elements::terminateGL() {
  for (auto snake : m_snake) {
    glDeleteBuffers(1, &snake.m_vbo);
    glDeleteVertexArrays(1, &snake.m_vao);
  }
}

void Elements::update(const GameData &gameData, float deltaTime) {

 if (gameData.m_state == State::Stopped) {
   return;
 }

 Element old = m_snake.front();
 
 // 0.5 para uma velocidade mediana
 Element *head = &m_snake.front();
 if (gameData.m_input[static_cast<size_t>(Input::Left)]) {
   head->m_translation.x -= 0.5f * deltaTime;
   head->lastDirection = Input::Left;
 } else if (gameData.m_input[static_cast<size_t>(Input::Right)]) {
   head->m_translation.x += 0.5f * deltaTime;
   head->lastDirection = Input::Right;
 } else if (gameData.m_input[static_cast<size_t>(Input::Down)]) {
   head->m_translation.y -= 0.5f * deltaTime;
   head->lastDirection = Input::Down;
 } else if (gameData.m_input[static_cast<size_t>(Input::Up)]) {
   head->m_translation.y += 0.5f * deltaTime;
   head->lastDirection = Input::Up;
 }

 int i = 0;
  for (auto &snake : m_snake) {
	  i++;
	  if (i == 1) {
		continue;
	  }

    if (old.lastDirection == Input::Left) {
      snake.m_translation.x -= 0.5f * deltaTime;
    } else if (old.lastDirection == Input::Right) {
      snake.m_translation.x += 0.5f * deltaTime;
    } else if (old.lastDirection == Input::Down) {
      snake.m_translation.y -= 0.5f * deltaTime;
    } else if (old.lastDirection == Input::Up) {
      snake.m_translation.y += 0.5f * deltaTime;
    }
    snake.lastDirection = old.lastDirection;
    old = snake;
  }
}

Elements::Element Elements::createSnake() {
  Element snake;

  auto &re{m_randomEngine};  // Shortcut

  snake.m_polygonSides = 4; // é um quadrado
  snake.m_color = glm::vec4{0.0f, 1.0f, 0.0f, 1.0f}; // a cobra é verde
  snake.m_color.a = 0.0f;
  snake.m_rotation = 0.8f;
  snake.m_scale = 0.08f;
  snake.m_translation = {1,1};

  // Create geometry
  std::vector<glm::vec2> positions(0);
  positions.emplace_back(0, 0);
  auto step{M_PI * 2 / snake.m_polygonSides};
  std::uniform_real_distribution<float> randomRadius(1.0f, 1.0f);
  for (auto angle : iter::range(0.0, M_PI * 2, step)) {
    auto radius{randomRadius(re)};
    positions.emplace_back(radius * std::cos(angle), radius * std::sin(angle));
  }
  positions.push_back(positions.at(1));

  // Generate VBO
  glGenBuffers(1, &snake.m_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, snake.m_vbo);
  glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec2),
               positions.data(), GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Get location of attributes in the program
  GLint positionAttribute{glGetAttribLocation(m_program, "inPosition")};

  // Create VAO
  glGenVertexArrays(1, &snake.m_vao);

  // Bind vertex attributes to current VAO
  glBindVertexArray(snake.m_vao);

  glBindBuffer(GL_ARRAY_BUFFER, snake.m_vbo);
  glEnableVertexAttribArray(positionAttribute);
  glVertexAttribPointer(positionAttribute, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // End of binding to current VAO
  glBindVertexArray(0);

  return snake;
}

Elements::Element Elements::createApple() {
  Element apple;

  auto &re{m_randomEngine};  // Shortcut

  apple.m_polygonSides = 4; // também é um quadrado
  apple.m_color = glm::vec4{1.0f, 0.0f, 0.0f, 1.0f}; // as maçãs são vermelhas
  apple.m_color.a = 0.0f;
  apple.m_rotation = 0.8f;
  apple.m_scale = 0.08f;
  apple.m_translation = {1,1};

  // Create geometry
  std::vector<glm::vec2> positions(0);
  positions.emplace_back(0, 0);
  auto step{M_PI * 2 / apple.m_polygonSides};
  std::uniform_real_distribution<float> randomRadius(1.0f, 1.0f);
  for (auto angle : iter::range(0.0, M_PI * 2, step)) {
    auto radius{randomRadius(re)};
    positions.emplace_back(radius * std::cos(angle), radius * std::sin(angle));
  }
  positions.push_back(positions.at(1));

  // Generate VBO
  glGenBuffers(1, &apple.m_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, apple.m_vbo);
  glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec2),
               positions.data(), GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Get location of attributes in the program
  GLint positionAttribute{glGetAttribLocation(m_program, "inPosition")};

  // Create VAO
  glGenVertexArrays(1, &apple.m_vao);

  // Bind vertex attributes to current VAO
  glBindVertexArray(apple.m_vao);

  glBindBuffer(GL_ARRAY_BUFFER, apple.m_vbo);
  glEnableVertexAttribArray(positionAttribute);
  glVertexAttribPointer(positionAttribute, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // End of binding to current VAO
  glBindVertexArray(0);

  return apple;
}
