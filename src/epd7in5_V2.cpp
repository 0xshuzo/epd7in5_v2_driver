#include "epd7in5_V2.h"

#include <Arduino.h>
#include <SPI.h>

#ifndef EPD_SPI_CLOCK_HZ
#define EPD_SPI_CLOCK_HZ 4000000
#endif

#ifndef EPD_SCK_PIN
#define EPD_SCK_PIN 18
#endif

#ifndef EPD_MOSI_PIN
#define EPD_MOSI_PIN 23
#endif

#ifndef EPD_CS_PIN
#define EPD_CS_PIN 5
#endif

#ifndef EPD_DC_PIN
#define EPD_DC_PIN 17
#endif

#ifndef EPD_RST_PIN
#define EPD_RST_PIN 16
#endif

#ifndef EPD_BUSY_PIN
#define EPD_BUSY_PIN 4
#endif

namespace {

// Panel-specific voltage and waveform tables used during initialization.
constexpr unsigned char kVoltageFrame[] = {0x06, 0x3F, 0x3F, 0x11, 0x24, 0x07, 0x17};
constexpr unsigned char kLutVcom[] = {0x00, 0x0F, 0x0F, 0x00, 0x00, 0x01, 0x00, 0x0F, 0x01, 0x0F,
    0x01, 0x02, 0x00, 0x0F, 0x0F, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
constexpr unsigned char kLutWw[] = {0x10, 0x0F, 0x0F, 0x00, 0x00, 0x01, 0x84, 0x0F, 0x01, 0x0F,
    0x01, 0x02, 0x20, 0x0F, 0x0F, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
constexpr unsigned char kLutBw[] = {0x10, 0x0F, 0x0F, 0x00, 0x00, 0x01, 0x84, 0x0F, 0x01, 0x0F,
    0x01, 0x02, 0x20, 0x0F, 0x0F, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
constexpr unsigned char kLutWb[] = {0x80, 0x0F, 0x0F, 0x00, 0x00, 0x01, 0x84, 0x0F, 0x01, 0x0F,
    0x01, 0x02, 0x40, 0x0F, 0x0F, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
constexpr unsigned char kLutBb[] = {0x80, 0x0F, 0x0F, 0x00, 0x00, 0x01, 0x84, 0x0F, 0x01, 0x0F,
    0x01, 0x02, 0x40, 0x0F, 0x0F, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

} // namespace

struct Epd::Backend {
    unsigned int reset_pin = EPD_RST_PIN;
    unsigned int dc_pin = EPD_DC_PIN;
    unsigned int cs_pin = EPD_CS_PIN;
    unsigned int busy_pin = EPD_BUSY_PIN;
    unsigned long width = EPD_WIDTH;
    unsigned long height = EPD_HEIGHT;
};

Epd::Epd() : backend_(new Backend()) {
}
Epd::~Epd() {
    delete backend_;
}

int Epd::Init(void) {
    // Configure the bus before releasing the panel from reset.
    pinMode(backend_->reset_pin, OUTPUT);
    pinMode(backend_->dc_pin, OUTPUT);
    pinMode(backend_->cs_pin, OUTPUT);
    pinMode(backend_->busy_pin, INPUT);
    digitalWrite(backend_->cs_pin, HIGH);
    SPI.begin(EPD_SCK_PIN, -1, EPD_MOSI_PIN, backend_->cs_pin);

    Reset();

    // Apply the panel voltage and drive settings.
    SendCommand(0x01);
    SendData(0x17);
    SendData(kVoltageFrame[6]);
    SendData(kVoltageFrame[1]);
    SendData(kVoltageFrame[2]);
    SendData(kVoltageFrame[3]);
    SendCommand(0x82);
    SendData(kVoltageFrame[4]);
    SendCommand(0x06);
    SendData(0x27);
    SendData(0x27);
    SendData(0x2F);
    SendData(0x17);
    SendCommand(0x30);
    SendData(kVoltageFrame[0]);

    // Wait for power-up before configuring the frame.
    SendCommand(0x04);
    delay(100);
    WaitUntilIdle();

    // Select the native 800 x 480 frame size.
    SendCommand(0x00);
    SendData(0x3F);
    SendCommand(0x61);
    SendData(0x03);
    SendData(0x20);
    SendData(0x01);
    SendData(0xE0);

    // Complete controller setup before loading the refresh waveforms.
    SendCommand(0x15);
    SendData(0x00);
    SendCommand(0x50);
    SendData(0x10);
    SendData(0x00);
    SendCommand(0x60);
    SendData(0x22);
    SendCommand(0x65);
    SendData(0x00);
    SendData(0x00);
    SendData(0x00);
    SendData(0x00);

    SetLut();
    return 0;
}

void Epd::WaitUntilIdle(void) {
    // BUSY is active low; let the controller finish before another command.
    while (digitalRead(backend_->busy_pin) == LOW) {
        SendCommand(0x71);
        delay(20);
    }
    delay(20);
}

void Epd::Reset(void) {
    // Preserve the reset pulse and settling times required by the panel.
    digitalWrite(backend_->reset_pin, HIGH);
    delay(20);
    digitalWrite(backend_->reset_pin, LOW);
    delay(4);
    digitalWrite(backend_->reset_pin, HIGH);
    delay(20);
}

static void SetLutByHost(Epd& epd, const unsigned char* lut_vcom, const unsigned char* lut_ww,
    const unsigned char* lut_bw, const unsigned char* lut_wb, const unsigned char* lut_bb) {
    // Consecutive controller registers hold the five pixel-transition tables.
    const unsigned char* luts[] = {lut_vcom, lut_ww, lut_bw, lut_wb, lut_bb};
    for (unsigned char lut = 0; lut < 5; ++lut) {
        epd.SendCommand(0x20 + lut);
        for (unsigned char count = 0; count < 42; ++count) {
            epd.SendData(pgm_read_byte(luts[lut] + count));
        }
    }
}

void Epd::SetLut(void) {
    SetLutByHost(*this, kLutVcom, kLutWw, kLutBw, kLutWb, kLutBb);
}

void Epd::DisplayFrame(const unsigned char* frame_buffer) {
    if (frame_buffer == nullptr) {
        return;
    }

    // Transfer a full frame before triggering the physical refresh.
    SendCommand(0x13);
    for (unsigned long i = 0; i < EPD_FRAME_BYTES; ++i) {
        // Waveshare's API uses zero-bit = black. The controller transfer is inverted.
        SendData(~pgm_read_byte(frame_buffer + i));
    }

    SendCommand(0x12);
    delay(100);
    WaitUntilIdle();
}

void Epd::Displaypart(const unsigned char* pbuffer, unsigned long x_start, unsigned long y_start,
    unsigned long picture_width, unsigned long picture_height) {
    if (pbuffer == nullptr || x_start >= backend_->width || y_start >= backend_->height ||
        picture_width == 0 || picture_height == 0 || x_start % 8 != 0 || picture_width % 8 != 0) {
        return;
    }

    // Packed rows must stay byte-aligned and inside the panel.
    if (picture_width > backend_->width - x_start) {
        picture_width = backend_->width - x_start;
    }
    if (picture_height > backend_->height - y_start) {
        picture_height = backend_->height - y_start;
    }

    SendCommand(0x13);
    // The driver sends white pixels outside the supplied image rectangle.
    const unsigned long bytes_per_row = picture_width / 8;
    for (unsigned long y = 0; y < backend_->height; ++y) {
        for (unsigned long x_byte = 0; x_byte < backend_->width / 8; ++x_byte) {
            const bool within_picture = y >= y_start && y < y_start + picture_height &&
                x_byte * 8 >= x_start && x_byte * 8 < x_start + picture_width;
            if (within_picture) {
                const unsigned long source = (y - y_start) * bytes_per_row + x_byte - x_start / 8;
                SendData(~pgm_read_byte(pbuffer + source));
            } else {
                SendData(0x00);
            }
        }
    }

    SendCommand(0x12);
    delay(100);
    WaitUntilIdle();
}

void Epd::SendCommand(unsigned char command) {
    // DC distinguishes command bytes from framebuffer and register data.
    digitalWrite(backend_->dc_pin, LOW);
    digitalWrite(backend_->cs_pin, LOW);

    SPI.beginTransaction(SPISettings(EPD_SPI_CLOCK_HZ, MSBFIRST, SPI_MODE0));
    SPI.transfer(command);
    SPI.endTransaction();

    digitalWrite(backend_->cs_pin, HIGH);
}

void Epd::SendData(unsigned char data) {
    digitalWrite(backend_->dc_pin, HIGH);
    digitalWrite(backend_->cs_pin, LOW);

    SPI.beginTransaction(SPISettings(EPD_SPI_CLOCK_HZ, MSBFIRST, SPI_MODE0));
    SPI.transfer(data);
    SPI.endTransaction();

    digitalWrite(backend_->cs_pin, HIGH);
}

void Epd::Sleep(void) {
    // Finish power-down before entering deep sleep; the image is retained.
    SendCommand(0x02);
    WaitUntilIdle();

    SendCommand(0x07);
    SendData(0xA5);
}

void Epd::Clear(void) {
    // The transfer uses inverted bits, so zero bytes produce a white panel.
    SendCommand(0x13);
    for (unsigned long i = 0; i < EPD_FRAME_BYTES; ++i) {
        SendData(0x00);
    }

    SendCommand(0x12);
    delay(100);
    WaitUntilIdle();
}
