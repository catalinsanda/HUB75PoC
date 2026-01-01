#include <Arduino.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <Fonts/Org_01.h>

// --- Panel geometry ---
static const uint16_t PANEL_WIDTH = 64;
static const uint16_t PANEL_HEIGHT = 32;
static const uint8_t CHAIN_LENGTH1 = 1;

// --- ESP32_FORUM_PINOUT ---
#define R1_PIN 2
#define G1_PIN 15
#define B1_PIN 4
#define R2_PIN 16
#define G2_PIN 27
#define B2_PIN 17
#define A_PIN 5
#define B_PIN 18
#define C_PIN 19
#define D_PIN 21
#define E_PIN -1
#define LAT_PIN 26
#define OE_PIN 25
#define CLK_PIN 22

MatrixPanel_I2S_DMA *display;

// Frame pacing
static const uint16_t FPS = 120;
static const uint32_t FRAME_TIME_MS = 1000 / FPS;

// HSV animation
float hue = 0.0f;
uint32_t lastFrame = 0;

// Text
const char *row1 = "La Multi Ani";
const char *row2 = "2026";

// Text bounds
int16_t r1_x1, r1_y1, r2_x1, r2_y1;
uint16_t r1_w, r1_h, r2_w, r2_h;

// Scroll state
float r1_x = 0, r1_y = 0;
float r2_x = 0, r2_y = 0;

float r1_dx = 0.10f, r1_dy = 0.05f;
float r2_dx = -0.05f, r2_dy = 0.15f;

// HSV → RGB
static inline void hsvToRgb(float h, float s, float v,
                            uint8_t &r, uint8_t &g, uint8_t &b)
{
  float c = v * s;
  float x = c * (1.0f - fabsf(fmodf(h / 60.0f, 2.0f) - 1.0f));
  float m = v - c;

  float r1, g1, b1;
  if (h < 60)
  {
    r1 = c;
    g1 = x;
    b1 = 0;
  }
  else if (h < 120)
  {
    r1 = x;
    g1 = c;
    b1 = 0;
  }
  else if (h < 180)
  {
    r1 = 0;
    g1 = c;
    b1 = x;
  }
  else if (h < 240)
  {
    r1 = 0;
    g1 = x;
    b1 = c;
  }
  else if (h < 300)
  {
    r1 = x;
    g1 = 0;
    b1 = c;
  }
  else
  {
    r1 = c;
    g1 = 0;
    b1 = x;
  }

  r = (uint8_t)((r1 + m) * 255);
  g = (uint8_t)((g1 + m) * 255);
  b = (uint8_t)((b1 + m) * 255);
}

void setup()
{
  Serial.begin(115200);

  HUB75_I2S_CFG::i2s_pins pins = {
      R1_PIN, G1_PIN, B1_PIN,
      R2_PIN, G2_PIN, B2_PIN,
      A_PIN, B_PIN, C_PIN,
      D_PIN, E_PIN, LAT_PIN,
      OE_PIN, CLK_PIN};

  HUB75_I2S_CFG cfg(
      PANEL_WIDTH,
      PANEL_HEIGHT,
      CHAIN_LENGTH1,
      pins,
      HUB75_I2S_CFG::FM6126A,
      HUB75_I2S_CFG::TYPE138,
      true,
      HUB75_I2S_CFG::HZ_20M,
      3,
      true);

  display = new MatrixPanel_I2S_DMA(cfg);
  display->begin();
  display->setBrightness8(160);

  display->setFont(&Org_01);
  display->setTextWrap(false);

  // Measure text
  display->getTextBounds(row1, 0, 0, &r1_x1, &r1_y1, &r1_w, &r1_h);
  display->getTextBounds(row2, 0, 0, &r2_x1, &r2_y1, &r2_w, &r2_h);

  // Start near center
  r1_x = (PANEL_WIDTH - r1_w) / 2;
  r1_y = 4;
  r2_x = (PANEL_WIDTH - r2_w) / 2;
  r2_y = 20;
}

static inline void bounceText(
    float &x, float &y,
    float &dx, float &dy,
    int16_t x1, int16_t y1,
    uint16_t w, uint16_t h)
{
  const float minX = -x1;
  const float maxX = PANEL_WIDTH - (x1 + w);
  const float minY = -y1;
  const float maxY = PANEL_HEIGHT - (y1 + h);

  if (x <= minX)
  {
    x = minX;
    dx = fabsf(dx);
  }
  if (x >= maxX)
  {
    x = maxX;
    dx = -fabsf(dx);
  }

  if (y <= minY)
  {
    y = minY;
    dy = fabsf(dy);
  }
  if (y >= maxY)
  {
    y = maxY;
    dy = -fabsf(dy);
  }
}

void loop()
{
  uint32_t now = millis();
  if (now - lastFrame < FRAME_TIME_MS)
    return;
  lastFrame = now;

  uint8_t r, g, b;
  hsvToRgb(hue, 1.0f, 1.0f, r, g, b);
  uint16_t color = display->color565(r, g, b);

  // Update scroll
  r1_x += r1_dx;
  r1_y += r1_dy;
  r2_x += r2_dx;
  r2_y += r2_dy;

  bounceText(r1_x, r1_y, r1_dx, r1_dy, r1_x1, r1_y1, r1_w, r1_h);
  bounceText(r2_x, r2_y, r2_dx, r2_dy, r2_x1, r2_y1, r2_w, r2_h);

  display->clearScreen();
  display->setTextColor(color);

  display->setCursor((int)r1_x, (int)r1_y);
  display->print(row1);

  display->setCursor((int)r2_x, (int)r2_y);
  display->print(row2);

  display->flipDMABuffer();

  hue += 0.4f;
  if (hue >= 360.0f)
    hue -= 360.0f;
}
