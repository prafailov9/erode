#pragma once
#include "Grid.h"
#include <SFML/Graphics.hpp>

class Renderer {
public:
  Renderer(const Grid &grid, int scale);
  void draw(sf::RenderWindow &window);

  static constexpr int BLOOM_RADIUS = 3;
  static constexpr int BLOOM_PASSES = 2;
  static constexpr float BLOOM_INTENSITY = 1.1f;
  static constexpr float BLOOM_SCALE = 1.0f;
  static constexpr float GAMMA = 0.3f;

private:
  const Grid &grid;

  sf::Texture baseTex;
  sf::Sprite baseSprite;
  std::vector<sf::Uint8> pixels; // RGBA — base scene, bloom merged in before upload

  std::vector<float> bloomBuf; // RGB float (ping)
  std::vector<float> blurTmp;  // RGB float (pong)

  sf::Shader compositeShader; // saturation + gamma only (no bloom texture)

  void boxBlurH(int r);
  void boxBlurV(int r);

  static bool isBloomEmitter(const Cell &cell);
  static sf::Color cellColor(CellType type);
};
