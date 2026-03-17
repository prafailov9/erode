#pragma once
#include "Grid.h"
#include "Panel.h"
#include <SFML/Graphics.hpp>

class InputHandler {
public:
  InputHandler(Grid &grid, Panel &panel, int scale = 1);
  void handle(sf::RenderWindow &window);

  CellType activeTool = CellType::SAND; // cycle with number keys
  int brushSize = 8;                    // radius in cells
  float spawnChance = 0.08f; // fraction of brush cells filled per frame

private:
  Grid &grid;
  Panel &panel;
  int scale;
  void paint(int cx, int cy);
};
