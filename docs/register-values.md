# EPD driver values

`SendCommand()` selects an operation; following `SendData()` bytes supply its parameters. Initialization and LUTs match [Waveshare's Arduino driver](https://github.com/waveshareteam/e-Paper/blob/master/Arduino/epd7in5_V2/epd7in5_V2.cpp). Bytes below are hexadecimal, in transmission order.

Register meanings follow the [Waveshare V2 specification](https://files.waveshare.com/upload/6/60/7.5inch_e-Paper_V2_Specification.pdf#page=23).

| Command | Data | Purpose |
| --- | --- | --- |
| `01` | `17 17 3F 3F 11` | Power configuration: internal supplies and border regulator; fast VCOM slew, gate voltages ±20 V; source voltages +15 V/−15 V; auxiliary source +5.8 V. |
| `82` | `24` | Set the common-electrode DC voltage, VCOM, to −1.90 V. |
| `06` | `27 27 2F 17` | Configure booster startup phases, drive strengths and switching timings. |
| `30` | `06` | Select the controller's 50 Hz waveform frame rate. |
| `04` | — | Enable panel power. |
| `00` | `3F` | Select monochrome operation, host-loaded waveforms, normal scan directions and enabled booster. |
| `61` | `03 20 01 E0` | Set width `0320` = 800 and height `01E0` = 480. |
| `15` | `00` | Select single-SPI operation. |
| `50` | `10 00` | Configure border/pixel waveform mapping and VCOM/data interval. |
| `60` | `22` | Set source-to-gate and gate-to-source timing. |
| `65` | `00 00 00 00` | Start source/gate addressing at zero. |
| `13` | Frame bytes | Write the new pixel data. |
| `12` | — | Start the physical display refresh. |
| `71` | — | Select controller status. |
| `02` | — | Switch panel power off. |
| `07` | `A5` | Enter deep sleep using the required check byte. |

Commands `20`–`24` load 42 bytes each from `kLutVcom`, `kLutWw`, `kLutBw`, `kLutWb`, and `kLutBb`: common-electrode and white→white, black→white, white→black, black→black waveforms. Individual fields such as `0F` and `84` are undocumented in the cited specification; preserve the complete [manufacturer waveform profiles](https://github.com/waveshareteam/e-Paper/blob/master/Arduino/epd7in5_V2/epd7in5_V2.cpp).

`kVoltageFrame` index 0 supplies `30`, indices 1–3 and 6 supply `01`, and index 4 supplies `82`. Index 5 (`07`) is unused.

Frames contain 800 × 480 / 8 = 48,000 bytes. The driver inverts framebuffer bits; transmitted `00` represents white pixels. `Displaypart()` still transfers a full frame. The wait loop polls the BUSY GPIO, without reading SPI status data.

`EPD_SPI_CLOCK_HZ` defaults to `4000000` (4 MHz) for commands and data. Override it through build flags, for example `-D EPD_SPI_CLOCK_HZ=2000000`. Transfers remain MSB first, SPI mode 0. This clock controls transfer speed independently of waveform timing. [Waveshare's Arduino interface](https://github.com/waveshareteam/e-Paper/blob/master/Arduino/epd7in5_V2/epdif.cpp) uses 2 MHz; 4 MHz is this driver's default.
