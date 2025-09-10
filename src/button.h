#pragma once
#include <Arduino.h>
// ===================== button.h =====================
// Debounced, edge-triggered button helper.
// Wiring: GPIO → button → GND, use INPUT_PULLUP in begin().

class DebouncedButton {
  uint8_t  pin;           // which GPIO we read
  bool     last=false;    // last raw state (true=pressed/LOW)
  uint32_t edgeMs=0;      // when the last edge happened
  bool     latched=false; // emit only once per press
  uint16_t debounceMs=30; // debounce window in ms
public:
  explicit DebouncedButton(uint8_t p): pin(p) {}
  void begin(){ pinMode(pin, INPUT_PULLUP); last = (digitalRead(pin)==LOW); }
  inline bool pressedRaw(){ return digitalRead(pin)==LOW; }
  // Returns true exactly once per full press (after debounce).
  bool toggled(){
    bool p = pressedRaw();
    if (p != last) { edgeMs = millis(); last = p; }
    if ((millis() - edgeMs) > debounceMs) {
      if (p && !latched) { latched = true; return true; }
      if (!p) latched = false;
    }
    return false;
  }
};
