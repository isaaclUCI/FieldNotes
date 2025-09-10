#pragma once
#include <Arduino.h>
#include <SdFat.h>
#include <WiFiClient.h>
// ===================== storage.h =====================
// Owns the SPI bus + SdFat filesystem, and gives helper
// functions for WAV headers and HTTP streaming.

extern SdFat   sd;   // defined in storage.cpp
extern SPIClass bus; // defined in storage.cpp

bool storage_begin();  // init SPI pins + mount SdFat

// Basic PCM WAV header structure (16-bit mono)
struct WavHeader {
  char     riff[4];
  uint32_t chunkSize;
  char     wave[4];
  char     fmt[4];
  uint32_t subchunk1Size;
  uint16_t audioFormat;
  uint16_t numChannels;
  uint32_t sampleRate;
  uint32_t byteRate;
  uint16_t blockAlign;
  uint16_t bitsPerSample;
  char     data[4];
  uint32_t dataSize;
} __attribute__((packed));

bool   storage_openWav(FsFile &f, const char* path, uint32_t sr, uint16_t bits, uint16_t ch);
void   storage_patchSizes(FsFile &f, uint32_t samples); // mono 16-bit → bytes = samples*2
String storage_listHTML();                               // build <ul> of files
bool   storage_streamFile(const String& path, WiFiClient& client);
String storage_latestFile();                             // newest file path or ""
