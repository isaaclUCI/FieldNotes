#pragma once
#include <Arduino.h>
// ===================== dsp.h =====================
// Minimal DSP: one‑pole high‑pass filter + simple noise gate.
// HPF: y[n] = α * (y[n-1] + x[n] − x[n-1]) with α from cutoff & fs.

struct HPF
{
  float y_prev = 0.0f; // previous output
  int16_t x_prev = 0;  // previous input
  float alpha = 0.94f; // computed in init()

  void init(float cutoff, float fs)
  {
    const float dt = 1.0f / fs;
    const float rc = 1.0f / (2.0f * 3.14159265f * cutoff);
    alpha = rc / (rc + dt);
    y_prev = 0.0f;
    x_prev = 0;
  }
  inline int16_t process(int16_t x)
  {
    float y = alpha * (y_prev + (float)x - (float)x_prev);
    y_prev = y;
    x_prev = x;
    if (y > 32767.0f)
      y = 32767.0f;
    if (y < -32768.0f)
      y = -32768.0f;
    return (int16_t)y;
  }
};

inline int16_t gate(int16_t s, int16_t thr) { return (abs(s) < thr) ? 0 : s; }
