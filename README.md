# epd7in5_v2_driver

Arduino/ESP32 SPI driver for the Waveshare 7.5-inch V2, 800 × 480 monochrome
panel. Owns the `Epd` interface, GPIO/SPI transfers, controller commands,
waveforms, refresh and sleep behavior.

`src/epd7in5_V2.h` declares the public display interface. The framebuffer is
48,000 bytes, row-major, MSB first: 0 is black, 1 is white. The public method
names and signatures follow Waveshare's `epd7in5_V2` Arduino API.

```cpp
#include <epd7in5_V2.h>
Epd display;
// Supply a complete 48,000-byte framebuffer.
display.Init();
display.DisplayFrame(frame_buffer);
display.Sleep();
```

Use this repo as a PlatformIO library. It depends on Arduino and SPI only.
The CMake `epd7in5_v2_api` target exports the public header.

| Signal | Default ESP32 GPIO |
| --- | --- |
| MOSI / DIN | 23 |
| SCK / CLK | 18 |
| CS | 5 |
| DC | 17 |
| RST | 16 |
| BUSY | 4 |

Override `EPD_MOSI_PIN`, `EPD_SCK_PIN`, `EPD_CS_PIN`, `EPD_DC_PIN`,
`EPD_RST_PIN` and `EPD_BUSY_PIN` with build flags in the consuming firmware.
The application configures its wiring; driver defaults live here. Use 3.3 V
logic and the appropriate panel power connections.

`EPD_SPI_CLOCK_HZ` sets the SPI clock for commands and data (default: 4,000,000 Hz).
Override it with a build flag such as `-D EPD_SPI_CLOCK_HZ=2000000`.
See [register values](docs/register-values.md) for the controller settings and
waveform tables.

`Displaypart` takes a byte-aligned start X and image width, plus start Y and
image height. It fills the rest of the frame white; it is not a retained-region
partial refresh. `Sleep()` powers down the panel while retaining its image.
