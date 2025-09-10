# Field Notes Mic Module

A compact ESP32-S3-based audio recorder with SD card storage, Wi-Fi connectivity, and a simple web interface for file management. Designed for field recording and note-taking with minimal hardware and robust software.

## Features

- Records high-quality mono WAV files using a MAX9814 AGC microphone.
- Stores recordings on SD card (FAT filesystem).
- Simple button interface for start/stop recording.
- LED indicator for recording status.
- Web server for listing and downloading files over LAN.
- mDNS support for easy device discovery (`http://FieldNotesMicModule.local`).
- NTP time sync for timestamped filenames.
- Optional HTTP upload after recording (configurable).

## Hardware

- ESP32-S3 DevKitC-1
- MAX9814 microphone module
- SD card (wired via SPI)
- Button (GPIO4, INPUT_PULLUP)
- LED (GPIO2, active-HIGH)

See `pins.h` for wiring details.

## Getting Started

1. Clone this repository.
2. Install [PlatformIO](https://platformio.org/).
3. Set your Wi-Fi credentials in `src/config.h`.
4. Connect hardware as described above.
5. Build and upload the firmware:
   ```sh
   pio run --target upload
   ```
6. Open the serial monitor to view logs:
   ```sh
   pio device monitor
   ```
7. Access the web interface via device IP or mDNS.

## File Structure

- `src/` — Main source code (Arduino framework)
- `lib/`, `include/` — For additional libraries and headers
- `test/` — Test files
- `tools/` — Utility scripts

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
