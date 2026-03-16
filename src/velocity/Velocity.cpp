#include "Velocity.h"
#include <cmath>

Velocity Velocity::add(float dx, float dy) { return {x + dx, y + dy}; }
Velocity Velocity::scale(float factor) { return {x * factor, y * factor}; }
float Velocity::dot(const Velocity &other) const {
  return x * other.x + y * other.y;
}
float Velocity::magnitude() {
  float len = std::sqrt(x * x + y * y);
  return len == 0.0f ? 0.0f : len;
}

Velocity Velocity::normalize() {
  float len = magnitude();
  return {x / len, y / len};
}

Velocity Velocity::clamp(float dx, float dy, float max) {
  float len = std::sqrt(dx * dx + dy * dy);

  if (len <= max || len == 0.0f)
    return {dx, dy};

  float scale = max / len;
  return {dx * scale, dy * scale};
}

float Velocity::clampCoord(float value, float min, float max) {
  if (value < min)
    return min;
  if (value > max)
    return max;
  return value;
}