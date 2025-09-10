#pragma once
// ===================== config.h =====================
// One place for settings you might tweak.
// Edit this file to set your Wi-Fi credentials and other parameters.

// --- Network ---
#define WIFI_SSID   ""      // <-- your home Wi-Fi name
#define WIFI_PASS   ""  // <-- your home Wi-Fi password
#define HOSTNAME    "FieldNotesMicModule"          // mDNS name → http://FieldNotesMicModule.local

// --- Audio / DSP ---
// Optimized for MAX9814 AGC mic: proper sample rates to avoid chipmunk effect
#define ADC_SAMPLE_RATE 16000 // internal ADC rate (no oversampling needed with MAX9814)
#define WAV_SAMPLE_RATE 16000 // file sample rate (standard speech rate)
#define HPF_CUTOFF_HZ 80.0f // optimal for MAX9814 AGC mic
#define GATE_THRESH 500 // slightly stronger gate for less static (was 400)
#define CHUNK_SAMPLES 320 // 20 ms blocks at 16 kHz

// Reduce RF hash during recording by sleeping Wi‑Fi modem.
#define WIFI_SLEEP_DURING_RECORD 1 // set 0 to keep Wi‑Fi fully on
