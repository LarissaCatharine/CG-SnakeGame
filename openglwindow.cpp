#include "openglwindow.hpp"

#include <imgui.h>

#include "abcg.hpp"

void OpenGLWindow::handleEvent(SDL_Event &event) {
  // Keyboard events
  if (event.type == SDL_KEYDOWN) {
    if (event.key.keysym.sym == SDLK_UP || event.key.keysym.sym == SDLK_w) {
      m_gameData.m_input.set(static_cast<size_t>(Input::Up));
      m_gameData.m_input.reset(static_cast<size_t>(Input::Down));
      m_gameData.m_input.reset(static_cast<size_t>(Input::Right));
      m_gameData.m_input.reset(static_cast<size_t>(Input::Left));
      m_gameData.m_state = State::Playing;
    }
    if (event.key.keysym.sym == SDLK_DOWN || event.key.keysym.sym == SDLK_s) {
      m_gameData.m_input.set(static_cast<size_t>(Input::Down));
      m_gameData.m_input.reset(static_cast<size_t>(Input::Up));
      m_gameData.m_input.reset(static_cast<size_t>(Input::Right));
      m_gameData.m_input.reset(static_cast<size_t>(Input::Left));
      m_gameData.m_state = State::Playing;
    }
    if (event.key.keysym.sym == SDLK_LEFT || event.key.keysym.sym == SDLK_a) {
      m_gameData.m_input.set(static_cast<size_t>(Input::Left));
      m_gameData.m_input.reset(static_cast<size_t>(Input::Down));
      m_gameData.m_input.reset(static_cast<size_t>(Input::Right));
      m_gameData.m_input.reset(static_cast<size_t>(Input::Up));
      m_gameData.m_state = State::Playing;
    }
    if (event.key.keysym.sym == SDLK_RIGHT || event.key.keysym.sym == SDLK_d) {
      m_gameData.m_input.set(static_cast<size_t>(Input::Right));
      m_gameData.m_input.reset(static_cast<size_t>(Input::Down));
      m_gameData.m_input.reset(static_cast<size_t>(Input::Up));
      m_gameData.m_input.reset(static_cast<size_t>(Input::Left));
      m_gameData.m_state = State::Playing;
    }
  }
}

void OpenGLWindow::initializeGL() {
  // Load a new font
  ImGuiIO &io{ImGui::GetIO()};
  auto filename{getAssetsPath() + "Inconsolata-Medium.ttf"};
  m_font = io.Fonts->AddFontFromFileTTF(filename.c_str(), 60.0f);
  if (m_font == nullptr) {
    throw abcg::Exception{abcg::Exception::Runtime("Cannot load font file")};
  }

  // Create program to render the other objects
  m_objectsProgram = createProgramFromFile(getAssetsPath() + "objects.vert",
                                           getAssetsPath() + "objects.frag");

  glClearColor(0, 0, 0, 1);

#if !defined(__EMSCRIPTEN__)
  glEnable(GL_PROGRAM_POINT_SIZE);
#endif

  // Start pseudo-random number generator
  auto seed{std::chrono::steady_clock::now().time_since_epoch().count()};
  m_randomEngine.seed(seed);

  restart();
}

void OpenGLWindow::restart() {
  m_gameData.m_state = State::Stopped;

  m_snake.initializeGL(m_objectsProgram, 5);
}

void OpenGLWindow::update() {
  float deltaTime{static_cast<float>(getDeltaTime())};

  // Wait 5 seconds before restarting
  if (m_gameData.m_state != State::Playing &&
      m_gameData.m_state != State::Stopped &&
      m_restartWaitTimer.elapsed() > 5) {
    restart();
    return;
  }

  m_snake.update(m_gameData, deltaTime);

  if (m_gameData.m_state == State::Playing) {
    checkCollisions();
    checkWinCondition();
  }
}

void OpenGLWindow::paintGL() {
  update();

  glClear(GL_COLOR_BUFFER_BIT);
  glViewport(0, 0, m_viewportWidth, m_viewportHeight);

  m_snake.paintGL();
}

void OpenGLWindow::paintUI() {
  abcg::OpenGLWindow::paintUI();

  {
    auto size{ImVec2(300, 85)};
    auto position{ImVec2((m_viewportWidth - size.x) / 2.0f,
                         (m_viewportHeight - size.y) / 2.0f)};
    ImGui::SetNextWindowPos(position);
    ImGui::SetNextWindowSize(size);
    ImGuiWindowFlags flags{ImGuiWindowFlags_NoBackground |
                           ImGuiWindowFlags_NoTitleBar |
                           ImGuiWindowFlags_NoInputs};
    ImGui::Begin(" ", nullptr, flags);
    ImGui::PushFont(m_font);

    if (m_gameData.m_state == State::Win) {
      ImGui::Text("*You Win!*");
    }

    ImGui::PopFont();
    ImGui::End();
  }
}

void OpenGLWindow::resizeGL(int width, int height) {
  m_viewportWidth = width;
  m_viewportHeight = height;

  glClear(GL_COLOR_BUFFER_BIT);
}

void OpenGLWindow::terminateGL() {
  glDeleteProgram(m_objectsProgram);

  m_snake.terminateGL();
}

void OpenGLWindow::checkCollisions() {
    for (auto &apple : m_snake.m_apples) {
      for (auto i : {-2, 0, 2}) {
        for (auto j : {-2, 0, 2}) {
          auto appleTranslation{apple.m_translation + glm::vec2(i, j)};
          auto distance{
              glm::distance(m_snake.m_snake.front().m_translation, appleTranslation)};

          if (distance < m_snake.m_snake.front().m_scale + apple.m_scale * 0.85f) {
            apple.m_hit = true;
          }
        }
      }
    }

    m_snake.m_apples.remove_if([](const Elements::Element &a) { return a.m_hit; });
}

void OpenGLWindow::checkWinCondition() {
  if (m_snake.m_apples.empty()) {
    m_gameData.m_state = State::Win;
    m_restartWaitTimer.restart();
  }
}
