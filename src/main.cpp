#include "Grid.h"
#include "InputHandler.h"
#include "Panel.h"
#include "Renderer.h"
#include <SFML/Graphics.hpp>
#include <cstdio>

struct FPSCounter {
  float elapsed = 0.f;
  int frames = 0;
  float fps = 0.f;
  bool ready = false;

  void update(float dt) {
    elapsed += dt;
    frames++;

    if (elapsed >= 0.5f) {
      fps = frames / elapsed;
      frames = 0;
      elapsed = 0.f;
      ready = true;
    }
  }

  float get() const { return fps; }
};

int main() {
  const int SCALE = 5;

  const int W = 2560 / SCALE, H = 1440 / SCALE;
  const int MAX_FPS = 70;
  const int SIM_W = W * SCALE; // pixel width of the simulation area
  const int SIM_H = H * SCALE;

  sf::Clock clock;
  sf::RenderWindow window(
      sf::VideoMode({static_cast<unsigned>(SIM_W + Panel::WIDTH),
                     static_cast<unsigned>(SIM_H)}),
      "Erode");
  window.setFramerateLimit(MAX_FPS);

  Grid grid(W, H);
  Renderer renderer(grid, SCALE);
  Panel panel(SIM_W, SIM_H);
  InputHandler input(grid, panel, SCALE);
  FPSCounter fpsCounter;
  sf::Clock sectionTimer;
  float tSim = 0, tRender = 0, tDisplay = 0;
  int frameCount = 0;
  const int REPORT_EVERY = 60;

  while (window.isOpen()) {
    float dt = clock.restart().asSeconds();
    fpsCounter.update(dt);
    while (const auto event = window.pollEvent())
      if (event->is<sf::Event::Closed>())
        window.close();

    input.handle(window);

    sectionTimer.restart();
    grid.update(dt);
    tSim += sectionTimer.restart().asSeconds() * 1000.f;

    window.clear();
    renderer.draw(window);
    panel.draw(window, input.activeTool);
    tRender += sectionTimer.restart().asSeconds() * 1000.f;

    window.display();
    tDisplay += sectionTimer.restart().asSeconds() * 1000.f;

    if (++frameCount >= REPORT_EVERY) {
      float fps = frameCount / (tSim + tRender + tDisplay) * 1000.f;
      std::printf("fps=%.0f  sim=%.2fms  render=%.2fms  display=%.2fms\n", fps,
                  tSim / frameCount, tRender / frameCount,
                  tDisplay / frameCount);
      std::fflush(stdout);
      tSim = tRender = tDisplay = 0;
      frameCount = 0;
    }
    // if (fpsCounter.ready && frameCount++ % 60 == 0) {
    // std::printf("FPS: %.1f\n", fpsCounter.get());
    // }
  }

  return 0;
}
