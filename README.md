# ESP32 HUB75 RGB Matrix Demo

This PlatformIO project drives a 64x32 HUB75 RGB LED matrix using an ESP32 and the ESP32-HUB75-MatrixPanel-I2S-DMA library.

The sketch displays two centered text rows with a smooth HSV color-cycling animation. Rendering uses I2S DMA with double buffering for stable, flicker-free output.

## Hardware
- ESP32
- 64×32 HUB75 RGB LED matrix (FM6126A driver)
- Wiring matches the pin definitions in the sketch

## Build and Flash
1. Open the project in PlatformIO  
2. Select an ESP32 environment  
3. Build and upload to the board  

## Configuration
- Panel size and chain length are defined at the top of the sketch
- HUB75 pin mapping follows the ESP32_FORUM layout
- I2S DMA runs at 20 MHz
- Target frame rate is 120 FPS

## Notes
- Text is centered using precomputed layout bounds
- Brightness is set to a fixed level in `setup()`
