#include <Arduino.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <Fonts/Org_01.h>

// --- Panel geometry ---
static const uint16_t PANEL_WIDTH  = 64;
static const uint16_t PANEL_HEIGHT = 32;
static const uint8_t  CHAIN_LENGTH1 = 1;

// --- ESP32_FORUM_PINOUT ---
#define R1_PIN   2
#define G1_PIN  15
#define B1_PIN   4
#define R2_PIN  16
#define G2_PIN  27
#define B2_PIN  17
#define A_PIN    5
#define B_PIN   18
#define C_PIN   19
#define D_PIN   21
#define E_PIN   -1
#define LAT_PIN 26
#define OE_PIN  25
#define CLK_PIN 22

MatrixPanel_I2S_DMA* display;

// Frame pacing
static const uint16_t FPS = 120;
static const uint32_t FRAME_TIME_MS = 1000 / FPS;

// Animation
float hue = 0.0f;
uint32_t lastFrame = 0;

// Text
const char* row1 = "La Multi Ani";
const char* row2 = "2026";

// Cached cursor positions
int16_t row1_x, row1_y;
int16_t row2_x, row2_y;

// HSV → RGB
static inline void hsvToRgb(
  float h, float s, float v,
  uint8_t& r, uint8_t& g, uint8_t& b
) {
  float c = v * s;
  float x = c * (1.0f - fabsf(fmodf(h / 60.0f, 2.0f) - 1.0f));
  float m = v - c;

  float r1, g1, b1;
  if (h < 60)       { r1 = c; g1 = x; b1 = 0; }
  else if (h < 120) { r1 = x; g1 = c; b1 = 0; }
  else if (h < 180) { r1 = 0; g1 = c; b1 = x; }
  else if (h < 240) { r1 = 0; g1 = x; b1 = c; }
  else if (h < 300) { r1 = x; g1 = 0; b1 = c; }
  else              { r1 = c; g1 = 0; b1 = x; }

  r = (uint8_t)((r1 + m) * 255.0f);
  g = (uint8_t)((g1 + m) * 255.0f);
  b = (uint8_t)((b1 + m) * 255.0f);
}

void computeCenteredRow(
  const char* text,
  int16_t topY,
  int16_t& outX,
  int16_t& outY
) {
  int16_t x1, y1;
  uint16_t w, h;
  display->getTextBounds(text, 0, 0, &x1, &y1, &w, &h);

  outX = (PANEL_WIDTH - w) / 2 - x1;
  outY = topY - y1;
}

void setup() {
  Serial.begin(115200);

  HUB75_I2S_CFG::i2s_pins pins = {
    R1_PIN, G1_PIN, B1_PIN,
    R2_PIN, G2_PIN, B2_PIN,
    A_PIN,  B_PIN,  C_PIN,
    D_PIN,  E_PIN,  LAT_PIN,
    OE_PIN, CLK_PIN
  };

  HUB75_I2S_CFG cfg(
    PANEL_WIDTH,
    PANEL_HEIGHT,
    CHAIN_LENGTH1,
    pins,
    HUB75_I2S_CFG::FM6126A,   // driver
    HUB75_I2S_CFG::TYPE138,  // line decoder (default)
    true,                    // double buffering ENABLED
    HUB75_I2S_CFG::HZ_10M,   // i2s speed
    3,                       // latch blanking
    true                     // clkphase
  );

  cfg.driver   = HUB75_I2S_CFG::FM6126A;
  cfg.clkphase = true;
  cfg.i2sspeed = HUB75_I2S_CFG::HZ_20M;

  display = new MatrixPanel_I2S_DMA(cfg);
  display->begin();

  display->clearScreen();
  display->flipDMABuffer();
  display->clearScreen();
  display->flipDMABuffer();

  display->setBrightness8(160);

  display->setFont(&Org_01);
  display->setTextSize(1);
  display->setTextWrap(false);

  // Precompute layout
  computeCenteredRow(row1, 5,  row1_x, row1_y);
  computeCenteredRow(row2, 21, row2_x, row2_y);
}

void loop() {
  uint32_t now = millis();
  if (now - lastFrame < FRAME_TIME_MS) return;
  lastFrame = now;

  uint8_t r, g, b;
  hsvToRgb(hue, 1.0f, 1.0f, r, g, b);
  uint16_t color = display->color565(r, g, b);

  display->clearScreen();
  display->setTextColor(color);

  display->setCursor(row1_x, row1_y);
  display->print(row1);

  display->setCursor(row2_x, row2_y);
  display->print(row2);

  display->flipDMABuffer();

  hue += 0.3f;
  if (hue >= 360.0f) hue -= 360.0f;
}
