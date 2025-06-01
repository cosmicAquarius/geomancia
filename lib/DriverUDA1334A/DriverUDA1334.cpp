#include "DriverUDA1334A.h"
#include "AudioTools.h"

DriverUDA1334A::DriverUDA1334A() {
    // Constructor implementation
    initialized = false;
}

bool DriverUDA1334A::begin(const AudioInfo &info)
{
    if (initialized)
    {
        return true;
    }



    // I2S configuration for UDA1334A - exactement comme votre code original
    auto config = i2s.defaultConfig(TX_MODE);
    config.copyFrom(info);
    config.pin_bck = I2S_BCLK;
    config.pin_ws = I2S_LRC;
    config.pin_data = I2S_DOUT;
    config.i2s_format = I2S_STD_FORMAT;
    config.bits_per_sample = 16;

    // I2S startup
    if (!i2s.begin(config))
    {
        return false;
    }

    initialized = true;
    return true;
}

void DriverUDA1334A::end()
{
    if (initialized)
    {
        i2s.end();
        initialized = false;
    }
}

I2SStream &DriverUDA1334A::getStream()
{
    return i2s;
}

bool DriverUDA1334A::isInitialized() const
{
    return initialized;
}