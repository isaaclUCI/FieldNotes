#include "storage.h"
#include "pins.h"
#include <string.h>  // memcpy
// ===================== storage.cpp =====================
// SPI + SdFat setup and simple helpers for WAV + HTTP.

SdFat   sd;
SPIClass bus = SPI;  // use default SPI object; we'll re-pin it below

// Safe clock (8 MHz). You can try 16–25 MHz later if wiring is short/clean.
static SdSpiConfig cfg(SD_CS_PIN, SHARED_SPI, SD_SCK_MHZ(8), &bus);

bool storage_begin(){
  // Attach SPI to the pins we chose in pins.h
  bus.begin(SCK_PIN, MISO_PIN, MOSI_PIN);
  // Mount FAT filesystem via SdFat
  return sd.begin(cfg);
}

// Internal: write a minimal WAV header with placeholder sizes.
// We'll patch dataSize and chunkSize after recording stops.
static void writeWavHeader(FsFile &f, uint32_t sr, uint16_t bits, uint16_t ch, uint32_t N){
  WavHeader h;
  memcpy(h.riff, "RIFF", 4);
  memcpy(h.wave, "WAVE", 4);
  memcpy(h.fmt , "fmt ", 4);
  memcpy(h.data, "data", 4);
  h.subchunk1Size = 16;
  h.audioFormat   = 1;       // PCM
  h.numChannels   = ch;      // mono
  h.sampleRate    = sr;      // 16000
  h.bitsPerSample = bits;    // 16
  h.blockAlign    = (ch * bits) / 8;   // bytes per sample frame
  h.byteRate      = sr * h.blockAlign; // bytes per second
  h.dataSize      = N * h.blockAlign;  // unknown now (0), patched later
  h.chunkSize     = 36 + h.dataSize;
  f.write(&h, sizeof(h));
}

bool storage_openWav(FsFile &f, const char* path, uint32_t sr, uint16_t bits, uint16_t ch){
  if (!f.open(path, O_WRONLY | O_CREAT | O_TRUNC)) return false;
  writeWavHeader(f, sr, bits, ch, 0);
  return true;
}

void storage_patchSizes(FsFile &f, uint32_t samples){
  const uint32_t dataSize  = samples * 2;  // mono 16-bit = 2 bytes/sample
  const uint32_t chunkSize = 36 + dataSize;
  f.seekSet(4);  f.write(&chunkSize, 4);   // RIFF chunk size
  f.seekSet(40); f.write(&dataSize,  4);   // data subchunk size
}

String storage_listHTML(){
  // Build a simple <ul> file list for the web UI
  String h = "<ul>";
  FsFile dir, file; char name[64];
  dir.open("/");
  while (file.openNext(&dir, O_RDONLY)) {
    if (!file.isDir()) {
      file.getName(name, sizeof(name));
      h += String("<li><a href='/dl?f=") + name + "'>" + name + "</a> (" +
           String((unsigned)file.fileSize()) + " bytes)</li>";
    }
    file.close();
  }
  dir.close();
  h += "</ul>";
  return h;
}

bool storage_streamFile(const String& path, WiFiClient& client){
  // Send a file to an HTTP client in 1 KB chunks (keeps RAM small)
  FsFile f; if (!f.open(path.c_str(), O_RDONLY)) return false;
  uint8_t buf[1024]; int n;
  while ((n = f.read(buf, sizeof(buf))) > 0) client.write(buf, n);
  f.close();
  return true;
}

String storage_latestFile(){
  // Find the newest file by FAT write time
  FsFile dir, file; char name[64]; char best[64]={0}; uint32_t bestTime=0;
  dir.open("/");
  while (file.openNext(&dir, O_RDONLY)) {
    if (!file.isDir()) {
      uint16_t date, time;
      file.getModifyDateTime(&date, &time);
      uint32_t wt = ((uint32_t)date << 16) | time;
      if (wt > bestTime) { bestTime = wt; file.getName(best, sizeof(best)); }
    }
    file.close();
  }
  dir.close();
  return (bestTime==0) ? String("") : String("/") + best;
}
