#include "Panel.h"
#include "Grid.h"
#include <algorithm>

// Common font paths to try (Linux / WSL)
static const char *FONT_PATHS[] = {
    "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
    "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
    "/usr/share/fonts/truetype/ubuntu/Ubuntu-R.ttf",
    "/usr/share/fonts/TTF/DejaVuSans.ttf", nullptr};

static const struct {
  CellType type;
  const char *label;
} CELL_DEFS[] = {{CellType::SAND, "Sand"},   {CellType::WATER, "Water"},
                 {CellType::DIRT, "Dirt"},   {CellType::FIRE, "Fire"},
                 {CellType::SMOKE, "Smoke"}, {CellType::GUNPOWDER, "Bomb"},
                 {CellType::WOOD, "Wood"},   {CellType::ACID, "Acid"},
                 {CellType::OIL, "Oil"},     {CellType::AIR, "Erase"},
                 {CellType::STONE, "Stone"}};

sf::Color Panel::colorFor(CellType t) {
  switch (t) {
  case CellType::SAND:
    return {212, 154, 74, 255};
  case CellType::WATER:
    return {64, 164, 223, 255};
  case CellType::DIRT:
    return {101, 67, 33, 255};
  case CellType::FIRE:
    return {255, 80, 20, 255};
  case CellType::SMOKE:
    return {160, 160, 160, 255};
  case CellType::GUNPOWDER:
    return {50, 50, 50, 255};
  case CellType::WOOD:
    return {139, 90, 43, 255};
  case CellType::ACID:
    return {80, 220, 60, 255};
  case CellType::OIL:
    return {100, 80, 30, 255};
  case CellType::STONE:
    return {120, 120, 120};
  case CellType::AIR:
  default:
    return {40, 40, 50, 255};
  }
}

Panel::Panel(int simWidth, int windowHeight)
    : panelX(simWidth), panelHeight(windowHeight) {
  // Load font FIRST — sf::Text must be constructed after the font is ready
  for (const char **p = FONT_PATHS; *p; ++p) {
    if (font.loadFromFile(*p)) {
      fontLoaded = true;
      break;
    }
  }

  bg = sf::RectangleShape(
      {static_cast<float>(WIDTH), static_cast<float>(panelHeight)});
  bg.setPosition(static_cast<float>(panelX), 0.f);
  bg.setFillColor({22, 22, 30, 255});

  if (fontLoaded) {
    header = sf::Text("Brush", font, 16);
    header.setFillColor({200, 200, 200, 255});
    header.setPosition(static_cast<float>(panelX + 10), 16.f);
  }

  const int BTN_W = WIDTH - 20;
  const int BTN_H = 42;
  const int BTN_X = panelX + 10;
  const int PADDING = 8;
  const int START_Y = 50;

  int n = static_cast<int>(sizeof(CELL_DEFS) / sizeof(CELL_DEFS[0]));
  buttons.reserve(n);
  for (int i = 0; i < n; ++i) {
    Button btn;
    btn.type = CELL_DEFS[i].type;
    btn.color = colorFor(btn.type);
    btn.rect = {static_cast<float>(BTN_X),
                static_cast<float>(START_Y + i * (BTN_H + PADDING)),
                static_cast<float>(BTN_W), static_cast<float>(BTN_H)};

    btn.outline = sf::RectangleShape({btn.rect.width + 4, btn.rect.height + 4});
    btn.outline.setPosition(btn.rect.left - 2, btn.rect.top - 2);
    btn.outline.setFillColor({60, 60, 70, 255});

    btn.swatch = sf::RectangleShape({SWATCH_W, btn.rect.height});
    btn.swatch.setPosition(btn.rect.left, btn.rect.top);
    btn.swatch.setFillColor(btn.color);

    btn.body = sf::RectangleShape({btn.rect.width - SWATCH_W, btn.rect.height});
    btn.body.setPosition(btn.rect.left + SWATCH_W, btn.rect.top);
    btn.body.setFillColor({35, 35, 45, 255});

    if (fontLoaded) {
      btn.label = sf::Text(CELL_DEFS[i].label, font, 14);
      btn.label.setFillColor({180, 180, 190, 255});
      float ty = btn.rect.top +
                 (btn.rect.height - btn.label.getLocalBounds().height) / 2.f -
                 2.f;
      btn.label.setPosition(btn.rect.left + SWATCH_W + 8.f, ty);
    }

    buttons.push_back(std::move(btn));
  }

  // Initialize slider track rect before building slider shapes
  const int SLIDER_Y = START_Y + n * (BTN_H + PADDING) + 15;
  sizeSlider.track = {static_cast<float>(BTN_X), static_cast<float>(SLIDER_Y),
                      static_cast<float>(BTN_W), 8.f};

  sliderTrack = sf::RectangleShape({sizeSlider.track.width, sizeSlider.track.height});
  sliderTrack.setPosition(sizeSlider.track.left, sizeSlider.track.top);
  sliderTrack.setFillColor({60, 60, 70, 255});

  // Build handle with placeholder size; updateSliderHandle sets position
  sliderHandle = sf::RectangleShape({12.f, 16.f});
  sliderHandle.setFillColor({100, 150, 200, 255});

  if (fontLoaded) {
    sizeLabel = sf::Text("Size: " + std::to_string(brushSize), font, 12);
    sizeLabel.setFillColor({180, 180, 190, 255});
    sizeLabel.setPosition(static_cast<float>(BTN_X),
                          sizeSlider.track.top - 20.f);
  }

  updateSliderHandle();
}

void Panel::updateSliderHandle() {
  if (sizeSlider.maxValue - sizeSlider.minValue > 0) {
    float ratio = static_cast<float>(brushSize - sizeSlider.minValue) /
                  (sizeSlider.maxValue - sizeSlider.minValue);
    float handleX = sizeSlider.track.left + ratio * sizeSlider.track.width - 6;
    sizeSlider.handle = {handleX, sizeSlider.track.top - 4.f, 12.f, 16.f};
    sliderHandle.setPosition(sizeSlider.handle.left, sizeSlider.handle.top);
    if (fontLoaded)
      sizeLabel.setString("Size: " + std::to_string(brushSize));
  }
}

void Panel::draw(sf::RenderWindow &window, CellType activeTool) {
  window.draw(bg);
  if (fontLoaded)
    window.draw(header);

  for (auto &btn : buttons) {
    bool active = (btn.type == activeTool);
    btn.outline.setFillColor(active ? sf::Color{255, 255, 255, 220}
                                    : sf::Color{60, 60, 70, 255});
    btn.body.setFillColor(active ? sf::Color{60, 60, 80, 255}
                                 : sf::Color{35, 35, 45, 255});
    if (fontLoaded)
      btn.label.setFillColor(active ? sf::Color{255, 255, 255, 255}
                                    : sf::Color{180, 180, 190, 255});
    window.draw(btn.outline);
    window.draw(btn.swatch);
    window.draw(btn.body);
    if (fontLoaded)
      window.draw(btn.label);
  }

  window.draw(sliderTrack);
  window.draw(sliderHandle);
  if (fontLoaded)
    window.draw(sizeLabel);
}

bool Panel::handleClick(sf::Vector2i mousePos, CellType &activeTool) {
  if (!contains(mousePos))
    return false;
  sf::Vector2f mp(static_cast<float>(mousePos.x),
                  static_cast<float>(mousePos.y));

  sf::FloatRect sliderClickArea = sizeSlider.track;
  sliderClickArea.top -= 5.f;
  sliderClickArea.height += 10.f;

  if (sliderClickArea.contains(mp)) {
    float relativePos = (mp.x - sizeSlider.track.left) / sizeSlider.track.width;
    relativePos = std::max(0.f, std::min(1.f, relativePos));
    brushSize = sizeSlider.minValue +
                static_cast<int>(relativePos *
                                 (sizeSlider.maxValue - sizeSlider.minValue));
    updateSliderHandle();
    return true;
  }

  for (const auto &btn : buttons) {
    if (btn.rect.contains(mp)) {
      activeTool = btn.type;
      return true;
    }
  }
  return true;
}

bool Panel::contains(sf::Vector2i pos) const {
  return pos.x >= panelX && pos.x < panelX + WIDTH && pos.y >= 0 &&
         pos.y < panelHeight;
}

int Panel::getBrushSize() const { return brushSize; }
