#ifndef DRIVER_UDA1334A_H
#define DRIVER_UDA1334A_H
#include "AudioTools.h"

#define SD_CS 5
#define SPI_MOSI 23
#define SPI_MISO 19
#define SPI_SCK 18
#define I2S_DOUT 25
#define I2S_BCLK 27
#define I2S_LRC 26


class DriverUDA1334A {
private:
    I2SStream i2s;
    bool initialized = false;

public:
    DriverUDA1334A();
    bool begin(const AudioInfo& info);
    void end();
    I2SStream& getStream();
    bool isInitialized() const;
};

#endif