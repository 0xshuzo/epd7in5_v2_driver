#pragma once

// Native panel layout: MSB first, zero = black, one = white.
#define EPD_WIDTH 800
#define EPD_HEIGHT 480
#define EPD_FRAME_BYTES (EPD_WIDTH * EPD_HEIGHT / 8)

// The public interface deliberately matches Waveshare's epd7in5_V2 Epd class.
class Epd {
public:
    Epd();
    ~Epd();
    Epd(const Epd&) = delete;
    Epd& operator=(const Epd&) = delete;

    // Lifecycle calls initialize, synchronize and power down the display.
    int Init(void);
    void WaitUntilIdle(void);
    void Reset(void);
    void SetLut(void);
    void Sleep(void);

    // Display methods consume the native packed framebuffer described above.
    void DisplayFrame(const unsigned char* frame_buffer);
    void Clear(void);
    void Displaypart(const unsigned char* pbuffer, unsigned long Start_X, unsigned long Start_Y,
        unsigned long END_X, unsigned long END_Y);

    // Low-level controller access is available for hardware-specific callers.
    void SendCommand(unsigned char command);
    void SendData(unsigned char data);

private:
    // The selected implementation owns its resources behind this shared API.
    struct Backend;
    Backend* backend_;
};
