#pragma once
#include <SdFat.h>
#include "dsp.h"
#include "config.h"
// ===================== recorder.h =====================
// Encapsulates the recording lifecycle:
//   start()  → open new WAV and reset counters
//   feed()   → append filtered samples in blocks
//   stop()   → patch header sizes, close (and maybe upload)

class Recorder {
public:
  bool begin();                               // init (DSP now in main.cpp)
  bool start();                               // open WAV with timestamped name
  void feed(int16_t* block, size_t nSamples); // append nSamples mono int16
  void stop();                                // finalize and optional upload
  bool isRecording() const { return recording; }
private:
  FsFile   wav;         // open WAV while recording
  uint32_t samples=0;   // total mono samples written
  bool     recording=false;
};
