#include "recorder.h"
#include "storage.h"
#include "pins.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <time.h>

// Tiny URL parser for http://host[:port]/path (no https)
static bool parseHttpUrl(const String &url, String &host, uint16_t &port, String &path)
{
  host = "";
  path = "/";
  port = 80;
  String u = url;
  if (u.startsWith("http://"))
    u.remove(0, 7);
  int slash = u.indexOf('/');
  String hp = (slash < 0) ? u : u.substring(0, slash);
  path = (slash < 0) ? "/" : u.substring(slash);
  int colon = hp.indexOf(':');
  if (colon >= 0)
  {
    host = hp.substring(0, colon);
    port = (uint16_t)hp.substring(colon + 1).toInt();
  }
  else
    host = hp;
  return host.length() > 0;
}

static bool uploadFileHTTP(const String &url, const String &absPath)
{
  String host, path;
  uint16_t port;
  if (!parseHttpUrl(url, host, port, path))
    return false;
  FsFile f;
  if (!f.open(absPath.c_str(), O_RDONLY))
    return false;
  size_t size = f.fileSize();
  String name = absPath.substring(1); // strip leading '/'

  WiFiClient client;
  client.setTimeout(5000); // 5 second timeout
  if (!client.connect(host.c_str(), port))
  {
    f.close();
    Serial.println("Upload server not reachable - file saved locally");
    return false;
  }

  // Minimal HTTP/1.1 POST with Content-Length + custom filename header
  client.printf("POST %s HTTP/1.1\r\n", path.c_str());
  client.printf("Host: %s\r\n", host.c_str());
  client.printf("Content-Type: application/octet-stream\r\n");
  client.printf("X-Filename: %s\r\n", name.c_str());
  client.printf("Content-Length: %u\r\n", (unsigned)size);
  client.printf("Connection: close\r\n\r\n");

  uint8_t buf[1024];
  int n;
  while ((n = f.read(buf, sizeof(buf))) > 0)
    client.write(buf, n);
  f.close();
  client.flush();
  
  // Brief wait for response
  uint32_t t0 = millis();
  while (client.connected() && millis() - t0 < 2000)
  {
    while (client.available())
      client.read();
  }
  client.stop();
  Serial.printf("File uploaded: %s\n", name.c_str());
  return true;
}

// Make a timestamped filename (Pacific Time). If no NTP yet, fall back to a counter.
static String timestampName()
{
  time_t t = time(nullptr);
  if (t < 1609459200)
  { // before 2021‑01‑01 → no time yet
    static uint32_t n = 0;
    char tmp[32];
    snprintf(tmp, sizeof(tmp), "REC_%06lu.wav", (unsigned long)++n);
    return String(tmp);
  }
  
  struct tm tm;
  localtime_r(&t, &tm); // Now uses Pacific Time thanks to configTzTime
  
  char name[64];
  // Format: YYYY-MM-DD_HHMMSS.wav (24-hour format, Pacific Time)
  strftime(name, sizeof(name), "%Y-%m-%d_%H%M%S.wav", &tm);
  return String(name);
}

bool Recorder::begin()
{
  // DSP initialization is now handled in main.cpp for better performance
  return true;
}

bool Recorder::start()
{
  String path = String("/") + timestampName();
  if (!storage_openWav(wav, path.c_str(), WAV_SAMPLE_RATE, 16, 1))
    return false;
  samples = 0;
  recording = true;
  return true;
}

void Recorder::feed(int16_t *block, size_t n)
{
  if (!recording || !wav)
    return;
  wav.write(block, n * 2);
  samples += n;
}

void Recorder::stop()
{
  if (!recording)
    return;
  recording = false;
  storage_patchSizes(wav, samples);
  wav.close();

#ifdef UPLOAD_URL
  if (WiFi.status() == WL_CONNECTED)
  {
    String path = storage_latestFile();
    if (path.length())
      uploadFileHTTP(UPLOAD_URL, path);
  }
#endif
}
