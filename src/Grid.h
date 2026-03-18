#pragma once
#include "velocity/Velocity.h"
#include <cstdint>
#include <vector>
// AIR means empty
enum class CellType {
  AIR,
  SAND,
  WATER,
  DIRT,
  FIRE,
  SMOKE,
  GUNPOWDER, // add BOMB -> create a chain eplosion
  WOOD,
  ACID,
  OIL,
  STONE
};

// TODO: finish SoA implementation
struct Cells {
  std::vector<CellType> particles;
  std::vector<bool> updated;
  std::vector<uint8_t> lifetime;
  std::vector<uint8_t> vx;
  std::vector<int8_t> vy;
  std::vector<uint8_t> remVy;
};

const float GRAVITY_VALUE = 20.f;
const float MIN_VEL_CLAMP = -10.f;
const float MAX_VEL_CLAMP = 10.f;
const float DISSOLVE_CHANCE = 0.12f;
const float WOOD_IGNITE_CHANCE = 0.07f;
const float OIL_IGNITE_CHANCE = 0.77f;
const float GUN_IGNITE_CHANCE = 1.0f;
const int BLAST_RADIUS = 25;
const int FUME_TTL = 90;
struct Cell {
  CellType type = CellType::AIR;
  bool updated = false;
  Velocity velocity;
  Velocity remVel = {0.0f, 0.0f}; // remeinder velocity from last tick
  uint8_t r, g, b;
  uint8_t lifetime = 0;
};

class Grid {
public:
  Grid(int width, int height);

  Cell &get(int x, int y);
  const Cell &get(int x, int y) const;
  void set(int x, int y, CellType type);
  bool inBounds(int x, int y) const;
  int idx(int x, int y);
  void swap(int i, int j);
  void update(float dt);

  const int width, height;

private:
  std::vector<Cell> cells; // flat: index = y * width + x
  static constexpr int density[] = {
      0, // AIR
      4, // SAND
      3, // WATER
      5, // DIRT
      1, // FIRE
      1, // SMOKE
      5, // GUNPOWDER
      5, // WOOD
      2, // ACID
      3, // OIL
      5, // STONE
  };

  // all 8 neighbors helper index array
  static constexpr int N = 8;
  static constexpr int neighX[] = {0, 1, 1, 1, 0, -1, -1, -1};
  static constexpr int neighY[] = {-1, -1, 0, 1, 1, 1, 0, -1};

  void applyGravityAcceleration(int i, int x, int y, float dt);
  int gravity(int i, int x, int y, float dt);
  int moveHorizontally(int i, int x, int y, float dt);
  int moveFluid(int i, int x, int y, float dt);
  void rise(int i, int x, int y, float dt);
  void sandRules(int i, int x, int y, float dt);
  void waterRules(int i, int x, int y, float dt);
  void dirtRules(int i, int x, int y, float dt);
  void fireRules(int i, int x, int y, float dt);
  void woodRules(int i, int x, int y, float dt);
  void smokeRules(int i, int x, int y, float dt);
  void gunpowderRules(int i, int x, int y, float dt);
  void applyRules(int i, int x, int y, float dt);
  // TODO: add in cellType
  void oilRules(int i, int x, int y, float dt);

  // acid specific work
  void acidRules(int i, int x, int y, float dt);
  void dissolve(int i, int x, int y);
  bool isDissolvable(int x, int y);
  bool canDissolve(int x, int y);

  // fire work
  bool isFlammable(int j);
  bool canIgnite(int j);
  void ignite(int i, int x, int y, float dt);
  void spreadFire(int i, int x, int y);

  // bomb
  void explode(int i, int x, int y);

  // vector helper
  float computeDisplacement(int i, int x, int y, float dt);
};
