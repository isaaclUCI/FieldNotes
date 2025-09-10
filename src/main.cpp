#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <time.h> // configTzTime
#include "config.h"
#include "pins.h"
#include "storage.h"
#include "recorder.h"
#include "dsp.h"
#include "web.h"
#include "button.h"

// This stitches everything:
// 1) SD + Wi‑Fi (mDNS + NTP time for filenames)
// 2) Button toggles start/stop; LED mirrors state
// 3) 32 kHz ADC → average 2 → 16 kHz → HPF → gate → write blocks

static Recorder rec;
static DebouncedButton btn(BUTTON_PIN);

static void setLed(bool on)
{
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, on ? HIGH : LOW);
}

void setup()
{
    Serial.begin(115200);
    delay(300);
    setLed(false);
    btn.begin();

    // ADC for mic: 12‑bit; MAX9814 has built-in AGC, use moderate attenuation
    analogReadResolution(12);
    analogSetPinAttenuation(MIC_ADC_PIN, ADC_6db); // moderate attenuation for MAX9814

    // SD card
    if (storage_begin())
        Serial.println("SD mounted");
    else
        Serial.println("SD not mounted");

    // Wi‑Fi + mDNS + NTP (Pacific Time with automatic DST)
    WiFi.mode(WIFI_STA);
    WiFi.setHostname(HOSTNAME);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    Serial.printf("Connecting to %s", WIFI_SSID);
    for (int i = 0; i < 60 && WiFi.status() != WL_CONNECTED; ++i)
    {
        delay(250);
        Serial.print(".");
    }
    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.printf("\nIP: %s STA MAC: %s\n", WiFi.localIP().toString().c_str(), WiFi.macAddress().c_str());
        MDNS.begin(HOSTNAME);
        // Set Pacific Time with automatic DST handling
        configTzTime("PST8PDT,M3.2.0,M11.1.0", "pool.ntp.org");
    }
    else
    {
        Serial.println("\nWi‑Fi not connected (still OK)");
    }

    rec.begin();
    web_begin();
}

void loop()
{
    web_handle();

    // Toggle start/stop on one clean press
    if (btn.toggled())
    {
#if WIFI_SLEEP_DURING_RECORD
        if (!rec.isRecording())
            WiFi.setSleep(true); // reduce RF noise while recording
#endif
        if (!rec.isRecording())
        {
            if (rec.start())
                setLed(true);
        }
        else
        {
            rec.stop();
            setLed(false);
#if WIFI_SLEEP_DURING_RECORD
            WiFi.setSleep(false);
#endif
        }
    }

    if (!rec.isRecording())
        return; // idle if not recording

    // === Optimized for MAX9814 AGC microphone (no oversampling needed) ===
    static int16_t block[CHUNK_SAMPLES];
    static HPF hpf;
    static bool inited = false;
    if (!inited)
    {
        hpf.init(HPF_CUTOFF_HZ, WAV_SAMPLE_RATE);
        inited = true;
    }
    static float dc = 2048.0f;     // running DC estimate (ADC mid)
    const float dc_alpha = 0.996f; // slightly faster DC tracking for better static reduction

    uint32_t tNext = micros();
    for (int i = 0; i < CHUNK_SAMPLES; ++i)
    {
        tNext += (1000000UL / ADC_SAMPLE_RATE);
        while ((int32_t)(micros() - tNext) < 0)
        { /* wait for next sample time */
        }
        
        int raw = analogRead(MIC_ADC_PIN);            // 0..4095, ~2048 mid
        dc = dc_alpha * dc + (1.0f - dc_alpha) * raw; // track slow drift
        int16_t x = (int16_t)((raw - dc) * 8);        // conservative scaling for MAX9814
        
        int16_t s = hpf.process(x);
        s = gate(s, GATE_THRESH);
        block[i] = s;
    }

    rec.feed(block, CHUNK_SAMPLES);
}
