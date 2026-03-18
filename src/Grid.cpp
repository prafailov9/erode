#include "Grid.h"
#include "velocity/Velocity.h"
#include <algorithm>
#include <cstdlib>

Grid::Grid(int width, int height)
    : width(width), height(height), cells(width * height) {}

void Grid::update(float dt) {
  // reset updated flags
  for (auto &cell : cells)
    cell.updated = false;

  for (int y = height - 1; y >= 0; y--) {
    // make sure no horizontal bias
    bool leftToRight = rand() % 2;
    for (int xi = 0; xi < width; xi++) {
      // alternate direction
      int x = leftToRight ? xi : (width - 1 - xi);
      int i = idx(x, y);
      if (!cells[i].updated) {
        applyRules(i, x, y, dt);
      }
    }
  }
}

void Grid::applyRules(int i, int x, int y, float dt) {
  switch (cells[i].type) {
  case CellType::SAND: {
    sandRules(i, x, y, dt);
    break;
  }
  case CellType::DIRT: {
    dirtRules(i, x, y, dt);
    break;
  }
  case CellType::WATER: {
    waterRules(i, x, y, dt);
    break;
  }
  case CellType::FIRE: {
    fireRules(i, x, y, dt);
    break;
  }
  case CellType::SMOKE: {
    smokeRules(i, x, y, dt);
    break;
  }
  case CellType::GUNPOWDER: {
    gunpowderRules(i, x, y, dt);
    break;
  }
  case CellType::ACID: {
    acidRules(i, x, y, dt);
    break;
  }
    // TODO: implement oil rules
  case CellType::OIL: {
    oilRules(i, x, y, dt);
    break;
  }
  case CellType::STONE: {
    cells[i].updated = true;
    break;
  }
  case CellType::WOOD: {
    woodRules(i, x, y, dt);
    break;
  }
  case CellType::AIR: {
    return;
  }
  }
}

void Grid::applyGravityAcceleration(int i, int x, int y, float dt) {
  cells[i].velocity.y = Velocity::clampCoord(
      cells[i].velocity.y += GRAVITY_VALUE * dt, MIN_VEL_CLAMP, MAX_VEL_CLAMP);
}
float Grid::computeDisplacement(int i, int x, int y, float dt) {
  applyGravityAcceleration(i, x, y, dt);
  // combine velocity + previous remainder
  float distance = cells[i].velocity.y + cells[i].remVel.y;
  // integer movement this frame
  int steps = (int)distance;
  // store fractional remainder for next frame
  cells[i].remVel.y = distance - steps;
  return steps;
}

/// Cell rules
void Grid::sandRules(int i, int x, int y, float dt) { gravity(i, x, y, dt); }

void Grid::dirtRules(int i, int x, int y, float dt) { gravity(i, x, y, dt); }

void Grid::waterRules(int i, int x, int y, float dt) {
  cells[i].r = 45 + rand() % 25;
  cells[i].g = 190 + rand() % 30;
  cells[i].b = 245 + rand() % 10;
  moveFluid(i, x, y, dt);
}
void Grid::fireRules(int i, int x, int y, float dt) {
  // decrement lifetime; die into smoke
  if (cells[i].lifetime > 0)
    cells[i].lifetime--;
  if (cells[i].lifetime == 0) {
    set(x, y, CellType::SMOKE);
    cells[i].updated = true;
    return;
  }
  // flicker: hot white core → orange → deep red as it ages
  cells[i].r = 220 + rand() % 35;
  cells[i].g = 80 + rand() % 130;
  cells[i].b = rand() % 40;
  rise(i, x, y, dt);
  spreadFire(i, x, y);
}

void Grid::smokeRules(int i, int x, int y, float dt) {
  // decrement lifetime; vanish when expired
  if (cells[i].lifetime > 0)
    cells[i].lifetime--;
  if (cells[i].lifetime == 0) {
    cells[i].type = CellType::AIR;
    cells[i].r = cells[i].g = cells[i].b = 0;
    cells[i].updated = true;
    return;
  }
  // fade grey as lifetime shrinks — start bright, fade to dark
  uint8_t grey = (uint8_t)(165 + (int)cells[i].lifetime * 60 / 80);
  cells[i].r = grey;
  cells[i].g = grey;
  cells[i].b = (uint8_t)std::clamp((int)grey + 10, 0, 255);
  rise(i, x, y, dt);
  if (cells[i].type == CellType::SMOKE) {
    moveHorizontally(i, x, y, dt);
  }
}

void Grid::gunpowderRules(int i, int x, int y, float dt) {
  gravity(i, x, y, dt); // bombs fall like sand

  for (auto [dx, dy] : std::initializer_list<std::pair<int, int>>{
           {0, 1}, {0, -1}, {1, 0}, {-1, 0}}) {
    if (inBounds(x + dx, y + dy) &&
        get(x + dx, y + dy).type == CellType::FIRE) {
      explode(x, y, BLAST_RADIUS);
      break;
    }
  }
}

void Grid::acidRules(int i, int x, int y, float dt) {
  cells[i].r = 108 + (rand() % 20 - 10);
  cells[i].g = 248 + (rand() % 14 - 7);
  cells[i].b = 108 + (rand() % 20 - 10);

  int ai = moveFluid(i, x, y, dt);

  if (cells[ai].type != CellType::ACID)
    return;

  dissolve(ai, x, y);
}

void Grid::oilRules(int i, int x, int y, float dt) {
  cells[i].r = 188 + (rand() % 16 - 8);
  cells[i].g = 148 + (rand() % 16 - 8);
  cells[i].b = 88 + (rand() % 16 - 8);
  int fi = moveFluid(i, x, y, dt);

  if (cells[fi].type != CellType::OIL)
    return;

  for (int k = 0; k < 8; k++) {
    int nx = x + rand() % 2; // favor horizontal
    int ny = y + rand() % 3 - 1;
    if (inBounds(nx, ny) && get(nx, ny).type == CellType::FIRE &&
        canIgnite(i)) {
      set(x, y, CellType::FIRE);
      cells[i].lifetime = 30 + rand() % 60;
      cells[i].updated = true;
      break;
    }
  }
}

void Grid::woodRules(int i, int x, int y, float dt) {
  if (cells[i].remVel.x > 0)
    return; // charring phase — fire in rise() handles damage via remVel.x--

  // burning phase
  if (cells[i].lifetime == 0) {
    set(x, y, CellType::AIR);
    cells[i].updated = true;
    return;
  }
  cells[i].lifetime--;
  spreadFire(i, x, y);

  // glow: fresh = bright orange-hot, dying = dark red charcoal
  // values are high to survive the invGamma darkening in the shader
  float t = cells[i].lifetime / 25.0f;  // 1.0 fresh → 0.0 dying
  cells[i].r = (uint8_t)(185 + t * 70); // 255 → 185
  cells[i].g = (uint8_t)(90 + t * 80);  // 170 → 90
  cells[i].b = (uint8_t)(50 - t * 20);  //  30 → 50

  // occasionally emit smoke above
  if ((rand() / (float)RAND_MAX) < 0.12f) {
    int uy = y - 1;
    if (!inBounds(x, uy))
      return;
    if (cells[idx(x, uy)].type == CellType::AIR) {
      set(x, uy, CellType::SMOKE);
      cells[idx(x, uy)].updated = true;
      return;
    }
    int dir = (rand() % 2 == 0) ? -1 : 1;
    for (int d : {dir, -dir}) {
      int nx = x + d;
      if (inBounds(nx, uy) && cells[idx(nx, uy)].type == CellType::AIR) {
        set(nx, uy, CellType::SMOKE);
        cells[idx(nx, uy)].updated = true;
        return;
      }
    }
  }
  cells[i].updated = true;
}

/// behavior
int Grid::gravity(int i, int x, int y, float dt) {
  int steps = std::min((int)computeDisplacement(i, x, y, dt), 4);
  int ci = i;
  int cx = x;
  int cy = y;

  // keep going straight down or down diagonal until
  //  steps are done, no valid neighbors downward
  // or if no move is possible
  for (int k = 0; k < steps; k++) {
    int cdens = density[(int)cells[ci].type];
    int ny = cy + 1;
    // try going down
    if (!inBounds(cx, ny))
      break;

    // swap with down neigh if current density is less
    int j = idx(cx, ny);
    if (cdens > density[(int)cells[j].type]) {
      swap(ci, j);
      cy = ny;
      ci = j;
    } else {
      int dir = (rand() % 2 == 0) ? -1 : 1;
      bool moved = false;

      for (int d : {dir, -dir}) {
        int nx = cx + d;
        if (inBounds(nx, ny)) {
          int nj = idx(nx, ny);
          if (cdens > density[(int)cells[nj].type]) {
            swap(ci, nj);
            cx = nx;
            cy = ny;
            ci = nj;
            moved = true;
            break;
          }
        }
      }

      if (!moved)
        break;
    }
  }

  cells[ci].updated = true;
  return ci;
}

int Grid::moveFluid(int i, int x, int y, float dt) {
  int finalI = gravity(i, x, y, dt);

  // for more sparse cell movement remove this if
  if (finalI != i) {
    return finalI;
  }
  int fx = finalI % width;
  int fy = finalI / width;

  return moveHorizontally(finalI, fx, fy, dt);
}
int Grid::moveHorizontally(int i, int x, int y, float dt) {
  // reduced horizontal movement
  int steps = std::min((int)computeDisplacement(i, x, y, dt), 5);
  int ci = i;
  int cx = x;
  int cy = y;

  int dir = (rand() % 2 == 0) ? -1 : 1;

  for (int k = 0; k < steps; k++) {
    int dens = density[(int)cells[ci].type];
    bool moved = false;

    for (int d : {dir, -dir}) {
      int dx = cx + d;
      if (!inBounds(dx, cy))
        continue;

      int j = idx(dx, cy);
      if (dens > density[(int)cells[j].type]) {
        swap(ci, j);
        cx = dx;
        ci = j; // critical fix
        moved = true;
        break;
      }
    }

    if (!moved)
      break;
  }

  cells[ci].updated = true;
  return ci;
}

void Grid::rise(int i, int x, int y, float dt) {
  int uy = y - 1;
  if (!inBounds(x, uy))
    return;
  int j = idx(x, uy);
  if (cells[j].type == CellType::AIR) {
    swap(i, j);
    cells[i].updated = true;
    cells[j].updated = true;
    return;
  }
  int steps = computeDisplacement(i, x, y, dt);
  for (int k = 0; k < steps; k++) {
    int dir = (rand() % 2 == 0) ? -1 : 1;
    for (int d : {dir, -dir}) {
      int dx = x + d;
      if (inBounds(dx, uy)) {
        int nj = idx(dx, uy);
        if (cells[nj].type == CellType::AIR) {
          swap(i, nj);
          cells[i].updated = true;
          cells[nj].updated = true;
          return;
        }
      }
    }
  }
}

void Grid::spreadFire(int i, int x, int y) {
  for (auto [dx, dy] : std::initializer_list<std::pair<int, int>>{
           {0, 1}, {0, -1}, {1, 0}, {-1, 0}}) {
    int nx = x + dx, ny = y + dy;
    if (!inBounds(nx, ny))
      continue;
    int j = idx(nx, ny);
    if (!canIgnite(j))
      continue;
    if (cells[j].type == CellType::WOOD) {
      if (cells[j].remVel.x > 0)
        cells[j].remVel.x--;
    } else {
      // oil / gunpowder: damage lifetime until it ignites
      if (cells[j].lifetime > 0)
        cells[j].lifetime -= 10;
    }
  }
}

void Grid::ignite(int i, int x, int y, float dt) {
  int ax = i % width; // column (x)
  int ay = i / width; // row (y)
  // explode when touching fire
  Cell *fire = &cells[i];
  int j = idx(x, y);
  if (cells[j].lifetime <= 0) {
    fire->updated = true;
    cells[j] = *fire;
    set(ax, ay, CellType::AIR);
  } else {
    if (cells[j].type == CellType::WOOD) {
      if (cells[j].remVel.x > 0) {
        cells[j].remVel.x--;
      }
    } else {
      cells[j].lifetime -= 5;
    }
  }
}

bool Grid::canIgnite(int j) {
  switch (cells[j].type) {
  case CellType::OIL:
    return (rand() / (float)RAND_MAX) < OIL_IGNITE_CHANCE;
  case CellType::WOOD:
    return (rand() / (float)RAND_MAX) < WOOD_IGNITE_CHANCE;
  case CellType::GUNPOWDER:
    return (rand() / (float)RAND_MAX) < GUN_IGNITE_CHANCE;
  default:
    return false;
  }
}
bool Grid::isFlammable(int j) {
  return cells[j].type == CellType::WOOD || cells[j].type == CellType::OIL;
}

void Grid::dissolve(int i, int x, int y) {
  int ax = i % width; // column (x)
  int ay = i / width; // row (y)

  for (int k = 0; k < N; k++) {
    // try random neighbor coords for realism
    // preffer downward dissolving
    int dx = rand() % 3 - 1;
    int dy = rand() % 2;

    // skip self
    if (dx == 0 && dy == 0) {
      continue;
    }

    // get neighbor coords
    int nx = ax + dx;
    int ny = ay + dy;
    if (!inBounds(nx, ny)) {
      continue;
    }

    if (!canDissolve(nx, ny)) {
      continue;
    }
    // do dissolve
    Cell *acid = &cells[i];
    int j = idx(nx, ny);
    // kill dissolvable cell only when its lifetime reaches the death
    // threshold
    if (cells[j].lifetime <= 0) {
      // move acid into dissolved cell pos
      acid->updated = true;
      cells[j] = *acid;
      set(ax, ay, CellType::AIR);
    } else {
      cells[j].lifetime -= 5;
    }
    // acid weakens when dissolving; use signed compare to avoid uint8
    // underflow
    if (acid->lifetime <= 0) {
      // acid gets consumed
      set(ax, ay, CellType::AIR);
      acid->updated = true;
    } else {
      acid->lifetime -= 10;
    }
    cells[i].updated = true;
    // spawn fumes during dissolve
    // TODO: push fumes emmission to the outer layer of acid
    // iterate upwards, while every upwared cell is acid, until air cell is hit.
    // Then, emit fumes.
    int fumeY = ay - 1;
    if (inBounds(ax, fumeY) && cells[idx(ax, fumeY)].type == CellType::AIR) {
      set(ax, fumeY, CellType::SMOKE);
      cells[idx(ax, fumeY)].lifetime = FUME_TTL;
      cells[idx(ax, fumeY)].updated = true;
    }
    return;
  }
}

void Grid::explode(int x, int y, int radius) {
  for (int dy = -radius; dy <= radius; dy++) {
    for (int dx = -radius; dx <= radius; dx++) {
      int explosionRadius = radius * radius;
      int cellsInRadius = dx * dx + dy * dy;
      if (cellsInRadius <= explosionRadius) {
        int nx = x + dx;
        int ny = y + dy;
        if (inBounds(nx, ny)) {
          set(nx, ny, CellType::FIRE);
        }
      } else if (cellsInRadius <= (explosionRadius / 10)) {
        int nx = dx + rand() % 3 - 1;
        int ny = dy + rand() % 2;
        for (int k = 0; k < N; k++) {
          if (inBounds(nx, ny)) {
            set(nx, ny, CellType::FIRE);
          }
        }
      }
    }
  }
}

bool Grid::canDissolve(int x, int y) {
  return isDissolvable(x, y) && (rand() / (float)RAND_MAX) < DISSOLVE_CHANCE;
}

bool Grid::isDissolvable(int x, int y) {
  return cells[idx(x, y)].type == CellType::WOOD;
}

// Helpers
void Grid::swap(int i, int j) {
  Cell t = cells.at(i);
  cells[i] = cells.at(j);
  cells[j] = t;
}

Cell &Grid::get(int x, int y) { return cells[y * width + x]; }

const Cell &Grid::get(int x, int y) const { return cells[y * width + x]; }

void Grid::set(int x, int y, CellType type) {
  if (!inBounds(x, y))
    return;
  cells[y * width + x].type = type;
  // jitter the base color slightly
  auto jitter = [](uint8_t base, int range) -> uint8_t {
    int v = base + (rand() % (range * 2 + 1)) - range;
    return (uint8_t)std::clamp(v, 0, 255);
  };

  switch (type) {
  case CellType::SAND:
    cells[y * width + x].r = jitter(255, 8);
    cells[y * width + x].g = jitter(232, 10);
    cells[y * width + x].b = jitter(148, 10);
    cells[y * width + x].velocity = {3.0f, 3.0f};
    cells[y * width + x].lifetime = 0;
    break;
  case CellType::WATER:
    cells[y * width + x].r = jitter(55, 10);
    cells[y * width + x].g = jitter(195, 10);
    cells[y * width + x].b = jitter(255, 0);
    cells[y * width + x].velocity = {3.0f, 3.0f};
    cells[y * width + x].lifetime = 0;
    break;
  case CellType::DIRT:
    cells[y * width + x].r = jitter(240, 12);
    cells[y * width + x].g = jitter(190, 10);
    cells[y * width + x].b = jitter(125, 8);
    cells[y * width + x].velocity = {3.0f, 3.0f};
    cells[y * width + x].lifetime = 0;
    break;
  case CellType::FIRE:
    cells[y * width + x].r = jitter(255, 0);
    cells[y * width + x].g = jitter(160, 40);
    cells[y * width + x].b = jitter(30, 20);
    cells[y * width + x].velocity = {3.0f, 3.0f};
    cells[y * width + x].lifetime = 90 + rand() % 60;
    break;
  case CellType::SMOKE:
    cells[y * width + x].r = 205;
    cells[y * width + x].g = 205;
    cells[y * width + x].b = 215;
    cells[y * width + x].velocity = {3.0f, 3.0f};
    cells[y * width + x].lifetime = 40 + rand() % 40;
    break;
  case CellType::GUNPOWDER:
    cells[y * width + x].r = 155;
    cells[y * width + x].g = 148;
    cells[y * width + x].b = 162;
    cells[y * width + x].velocity = {3.0f, 3.0f};
    cells[y * width + x].lifetime = 0;
    break;
  case CellType::WOOD:
    cells[y * width + x].r = jitter(150, 8);
    cells[y * width + x].g = jitter(111, 10);
    cells[y * width + x].b = jitter(51, 8);
    cells[y * width + x].velocity = {0.0f, 0.0f};
    cells[y * width + x].remVel = {3.0f, 0.0f}; // charring counter
    cells[y * width + x].lifetime =
        25; // burn duration — tune to control chain length
    break;
  case CellType::ACID:
    cells[y * width + x].r = jitter(105, 10);
    cells[y * width + x].g = jitter(248, 7);
    cells[y * width + x].b = jitter(108, 10);
    cells[y * width + x].velocity = {3.0f, 3.0f};
    cells[y * width + x].lifetime = 65 + rand() % 50;
    break;
  case CellType::OIL:
    cells[y * width + x].r = jitter(185, 10);
    cells[y * width + x].g = jitter(148, 8);
    cells[y * width + x].b = jitter(88, 8);
    cells[y * width + x].velocity = {3.0f, 3.0f};
    cells[y * width + x].lifetime = 0;
    break;
  case CellType::STONE:
    cells[y * width + x].r = jitter(222, 12);
    cells[y * width + x].g = jitter(216, 12);
    cells[y * width + x].b = jitter(210, 12);
    cells[y * width + x].velocity = {0.0f, 0.0f};
    cells[y * width + x].lifetime = 0;
    break;
  default:
    cells[y * width + x].r = cells[y * width + x].g = cells[y * width + x].b =
        0;
    cells[y * width + x].lifetime = 0;
    cells[y * width + x].velocity = {0.0f, 0.0f};
    break;
  }
}

bool Grid::inBounds(int x, int y) const {
  return x >= 0 && x < width && y >= 0 && y < height;
}

int Grid::idx(int x, int y) { return y * width + x; }