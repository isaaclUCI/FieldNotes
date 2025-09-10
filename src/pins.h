#pragma once
// ===================== pins.h =====================
// All hardware pins in one place so rewiring is easy.

// Mic + UI
#define MIC_ADC_PIN  1   // MAX9814 OUT → GPIO1 (ADC1_CH0)
#define BUTTON_PIN   4   // button between GPIO4 and GND (INPUT_PULLUP)
#define LED_PIN      2   // GPIO2 → 330Ω → LED → GND (active-HIGH)

// SD/SPI
#define SCK_PIN     12   // CLK  → GPIO12
#define MOSI_PIN    11   // DI   → GPIO11 (ESP32 MOSI)
#define MISO_PIN    13   // DO   → GPIO13 (ESP32 MISO)
#define SD_CS_PIN   10   // CS   → GPIO10
