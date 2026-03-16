#pragma once
#include <cmath>

struct Velocity {
  float x, y;
  float getX() const;
  float getY() const;

  Velocity add(float dx, float dy);
  Velocity scale(float factor);
  Velocity normalize();
  Velocity clamp(float dx, float dy, float max);

  float magnitude();
  float dot(const Velocity &other) const;
  static float clampCoord(float value, float min, float max);
};
