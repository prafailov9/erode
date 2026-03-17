#pragma once
#include "Grid.h"
#include <SFML/Graphics.hpp>
#include <optional>
#include <vector>

class Panel {
public:
  static constexpr int WIDTH = 160;

  Panel(int simWidth, int windowHeight);

  void draw(sf::RenderWindow &window, CellType activeTool);

  // Returns true and updates activeTool if a panel button was clicked
  bool handleClick(sf::Vector2i mousePos, CellType &activeTool);

  // True if the screen position is inside the panel
  bool contains(sf::Vector2i pos) const;

  // Get the current brush size
  int getBrushSize() const;

private:
  int panelX;
  int panelHeight;
  int brushSize = 5;
  bool isDraggingSlider = false;
  const float SWATCH_W = 28.f;

  std::optional<sf::Font> font;

  sf::RectangleShape bg;
  std::optional<sf::Text> header;

  struct Button {
    CellType type;
    sf::Color color;
    sf::FloatRect rect;
    sf::RectangleShape outline;
    sf::RectangleShape swatch;
    sf::RectangleShape body;
    std::optional<sf::Text> label;
  };
  std::vector<Button> buttons;

  struct Slider {
    sf::FloatRect track;
    sf::FloatRect handle;
    int minValue = 1;
    int maxValue = 50;
  };
  Slider sizeSlider;
  sf::RectangleShape sliderTrack;
  sf::RectangleShape sliderHandle;
  std::optional<sf::Text> sizeLabel;

  static sf::Color colorFor(CellType t);
  void updateSliderHandle();
};
