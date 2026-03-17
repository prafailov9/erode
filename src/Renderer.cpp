#include "Renderer.h"
#include "Grid.h"
#include <algorithm>
#include <stdexcept>

// Saturation boost + gamma only. Bloom is merged into the texture on CPU
// before upload, so no second texture sample is needed here.
static const char *COMPOSITE_GLSL = R"glsl(
uniform sampler2D tex;
uniform float invGamma;

void main() {
    vec2 uv = gl_TexCoord[0].xy;
    vec3 color = texture(tex, uv).rgb;

    // saturation boost
    float lum = dot(color, vec3(0.2126, 0.7152, 0.0722));
    color = clamp(mix(vec3(lum), color, 1.35), 0.0, 1.0);

    // gamma
    color = pow(color, vec3(invGamma));

    gl_FragColor = vec4(color, 1.0);
}
)glsl";

// ---------------------------------------------------------------------------

Renderer::Renderer(const Grid &grid, int scale)
    : grid(grid), pixels(grid.width * grid.height * 4),
      bloomBuf(grid.width * grid.height * 3, 0.0f),
      blurTmp(grid.width * grid.height * 3, 0.0f),
      baseTex(sf::Vector2u{static_cast<unsigned>(grid.width),
                           static_cast<unsigned>(grid.height)}),
      baseSprite(baseTex) {

  baseSprite.setScale(
      sf::Vector2f{static_cast<float>(scale), static_cast<float>(scale)});

  if (!compositeShader.loadFromMemory(COMPOSITE_GLSL,
                                      sf::Shader::Type::Fragment))
    throw std::runtime_error("Failed to compile composite shader");
  compositeShader.setUniform("tex", sf::Shader::CurrentTextureType{});
  compositeShader.setUniform("invGamma", 1.0f / GAMMA);
}

// ---------------------------------------------------------------------------

void Renderer::draw(sf::RenderWindow &window) {
  // --- base pass + bloom extraction ---
  std::fill(bloomBuf.begin(), bloomBuf.end(), 0.0f);

  for (int y = 0; y < grid.height; ++y) {
    for (int x = 0; x < grid.width; ++x) {
      const Cell &cell = grid.get(x, y);
      int i4 = (y * grid.width + x) * 4;
      int i3 = (y * grid.width + x) * 3;

      pixels[i4 + 0] = cell.r;
      pixels[i4 + 1] = cell.g;
      pixels[i4 + 2] = cell.b;
      pixels[i4 + 3] = cellColor(cell.type).a;

      if (isBloomEmitter(cell)) {
        bloomBuf[i3 + 0] = cell.r * BLOOM_INTENSITY;
        bloomBuf[i3 + 1] = cell.g * BLOOM_INTENSITY;
        bloomBuf[i3 + 2] = cell.b * BLOOM_INTENSITY;
      }
    }
  }

  // // --- bloom blur ---
  // for (int pass = 0; pass < BLOOM_PASSES; ++pass) {
  //   boxBlurH(BLOOM_RADIUS);
  //   boxBlurV(BLOOM_RADIUS);
  // }

  // // --- merge bloom additively into pixels before upload ---
  // // Keeps the shader simple (one texture sample) vs the old two-texture
  // // approach.
  // for (int i = 0; i < grid.width * grid.height; ++i) {
  //   int i3 = i * 3, i4 = i * 4;
  //   float r = pixels[i4] + bloomBuf[i3] * BLOOM_SCALE;
  //   float g = pixels[i4 + 1] + bloomBuf[i3 + 1] * BLOOM_SCALE;
  //   float b = pixels[i4 + 2] + bloomBuf[i3 + 2] * BLOOM_SCALE;
  //   pixels[i4] = static_cast<std::uint8_t>(std::clamp(r, 0.0f, 255.0f));
  //   pixels[i4 + 1] = static_cast<std::uint8_t>(std::clamp(g, 0.0f, 255.0f));
  //   pixels[i4 + 2] = static_cast<std::uint8_t>(std::clamp(b, 0.0f, 255.0f));
  // }

  // --- upload & draw ---
  baseTex.update(pixels.data());
  window.draw(baseSprite, sf::RenderStates(&compositeShader));
}

// ---------------------------------------------------------------------------

void Renderer::boxBlurH(int r) {
  int w = grid.width, h = grid.height;
  float inv = 1.0f / static_cast<float>(2 * r + 1);

  for (int y = 0; y < h; ++y) {
    float sumR = 0.0f, sumG = 0.0f, sumB = 0.0f;

    for (int kx = 0; kx <= r && kx < w; ++kx) {
      int pi = (y * w + kx) * 3;
      sumR += bloomBuf[pi];
      sumG += bloomBuf[pi + 1];
      sumB += bloomBuf[pi + 2];
    }

    for (int x = 0; x < w; ++x) {
      int pi = (y * w + x) * 3;
      blurTmp[pi] = sumR * inv;
      blurTmp[pi + 1] = sumG * inv;
      blurTmp[pi + 2] = sumB * inv;

      int removeX = x - r;
      int addX = x + r + 1;

      if (removeX >= 0) {
        int rpi = (y * w + removeX) * 3;
        sumR -= bloomBuf[rpi];
        sumG -= bloomBuf[rpi + 1];
        sumB -= bloomBuf[rpi + 2];
      }
      if (addX < w) {
        int api = (y * w + addX) * 3;
        sumR += bloomBuf[api];
        sumG += bloomBuf[api + 1];
        sumB += bloomBuf[api + 2];
      }
    }
  }
  std::swap(bloomBuf, blurTmp);
}

void Renderer::boxBlurV(int r) {
  int w = grid.width, h = grid.height;
  float inv = 1.0f / static_cast<float>(2 * r + 1);

  for (int x = 0; x < w; ++x) {
    float sumR = 0.0f, sumG = 0.0f, sumB = 0.0f;

    for (int ky = 0; ky <= r && ky < h; ++ky) {
      int pi = (ky * w + x) * 3;
      sumR += bloomBuf[pi];
      sumG += bloomBuf[pi + 1];
      sumB += bloomBuf[pi + 2];
    }

    for (int y = 0; y < h; ++y) {
      int pi = (y * w + x) * 3;
      blurTmp[pi] = sumR * inv;
      blurTmp[pi + 1] = sumG * inv;
      blurTmp[pi + 2] = sumB * inv;

      int removeY = y - r;
      int addY = y + r + 1;

      if (removeY >= 0) {
        int rpi = (removeY * w + x) * 3;
        sumR -= bloomBuf[rpi];
        sumG -= bloomBuf[rpi + 1];
        sumB -= bloomBuf[rpi + 2];
      }
      if (addY < h) {
        int api = (addY * w + x) * 3;
        sumR += bloomBuf[api];
        sumG += bloomBuf[api + 1];
        sumB += bloomBuf[api + 2];
      }
    }
  }
  std::swap(bloomBuf, blurTmp);
}

// ---------------------------------------------------------------------------

bool Renderer::isBloomEmitter(const Cell &cell) {
  switch (cell.type) {
  case CellType::FIRE:
  case CellType::ACID:
    return true;
  case CellType::WOOD:
    return cell.remVel.x == 0;
  default:
    return false;
  }
}

sf::Color Renderer::cellColor(CellType type) {
  switch (type) {
  case CellType::AIR:
    return {15, 15, 20, 255};
  case CellType::SAND:
    return {212, 154, 74, 255};
  case CellType::WATER:
    return {64, 164, 223, 200};
  case CellType::DIRT:
    return {101, 67, 33, 255};
  case CellType::FIRE:
    return {255, 80, 20, 255};
  case CellType::SMOKE:
    return {180, 180, 180, 120};
  case CellType::GUNPOWDER:
    return {30, 30, 30, 255};
  case CellType::STONE:
    return {120, 120, 120, 200};
  case CellType::ACID:
    return {178, 255, 50};
  default:
    return {255, 0, 255, 255};
  }
}
