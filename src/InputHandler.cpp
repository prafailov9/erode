#include "InputHandler.h"
#include "Grid.h"

InputHandler::InputHandler(Grid &grid, Panel &panel, int scale)
    : grid(grid), panel(panel), scale(scale) {}

void InputHandler::handle(sf::RenderWindow &window) {
  // Number keys switch active tool
  if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num1))
    activeTool = CellType::SAND;
  if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num2))
    activeTool = CellType::WATER;
  if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num3))
    activeTool = CellType::DIRT;
  if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num4))
    activeTool = CellType::FIRE;
  if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num5))
    activeTool = CellType::GUNPOWDER;
  if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num9))
    activeTool = CellType::STONE;
  if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num0))
    activeTool = CellType::AIR; // eraser

  auto pos = sf::Mouse::getPosition(window);

  // Update brush size from panel
  brushSize = panel.getBrushSize();

  // Left mouse: panel click takes priority over painting
  if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
    if (!panel.handleClick(pos, activeTool))
      paint(pos.x / scale, pos.y / scale);
  }

  // Right mouse: erase (only on the simulation area)
  if (sf::Mouse::isButtonPressed(sf::Mouse::Right)) {
    if (!panel.contains(pos)) {
      auto saved = activeTool;
      activeTool = CellType::AIR;
      paint(pos.x / scale, pos.y / scale);
      activeTool = saved;
    }
  }
}

void InputHandler::paint(int cx, int cy) {
  float chance = (activeTool == CellType::WOOD || activeTool == CellType::STONE)
                     ? 1.0f
                     : spawnChance;
  for (int dy = -brushSize; dy <= brushSize; ++dy)
    for (int dx = -brushSize; dx <= brushSize; ++dx)
      if (dx * dx + dy * dy <= brushSize * brushSize)
        // if ((rand() / (float)RAND_MAX) < chance)
        grid.set(cx + dx, cy + dy, activeTool);
}
