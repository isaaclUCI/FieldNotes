#include "web.h"
#include <WebServer.h>
#include <ESPmDNS.h>
#include <WiFi.h>
#include "storage.h"
#include "config.h"

// ===================== web.cpp =====================
// Minimal LAN web server to list and download files.

static WebServer server(80); // HTTP server

// HTML page generation
static String page(){
  String h = "<html><head><meta name='viewport' content='width=device-width,initial-scale=1'>"
             "<style>body{font-family:system-ui;margin:16px}li{margin:6px 0}</style></head><body>";
  h += "<h2>Field Notes Mic Module</h2>";
  h += storage_listHTML();
  h += String("<p>Open: <code>") + WiFi.localIP().toString() +
       "</code> or <code>http://" + HOSTNAME + ".local</code></p></body></html>";
  return h;
}

// Web server initialization
void web_begin(){
  // Register HTTP handlers

  server.on("/", [](){ server.send(200, "text/html", page()); });
  
  server.on("/dl", [](){
    if (!server.hasArg("f")) { server.send(400, "text/plain", "missing f"); return; } // Check for file argument
    String name = server.arg("f");                                                    // Get file name
    String path  = "/" + name;                                                        // Get file path

    // Open file for reading
    FsFile f;
    if (!f.open(path.c_str(), O_RDONLY)) { server.send(404, "text/plain", "not found"); return; }

    // Stream file to client
    size_t size = f.fileSize();
    WiFiClient client = server.client();                                            // Get client connection

    // Write raw HTTP headers manually
    client.printf("HTTP/1.1 200 OK\r\n");
    client.printf("Content-Type: %s\r\n", name.endsWith(".wav") ? "audio/wav" : "application/octet-stream");
    client.printf("Content-Disposition: attachment; filename=\"%s\"\r\n", name.c_str());
    client.printf("Content-Length: %u\r\n", (unsigned)size);
    client.printf("Connection: close\r\n\r\n");

    // Stream file contents
    uint8_t buf[1024]; // Buffer for file chunks
    int n;
    while ((n = f.read(buf, sizeof(buf))) > 0) {
        client.write(buf, n);
    }
    f.close();
    client.flush();
    client.stop();
    });

  server.begin(); // Start the server
}

void web_handle(){
  server.handleClient(); // Handle incoming HTTP requests
}