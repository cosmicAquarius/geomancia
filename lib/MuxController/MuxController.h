#pragma once
#include <Arduino.h>
#include "Driver74HCT4067.h"

class MuxController {
  public:
    MuxController();
    void readNext();
    uint16_t get(uint8_t muxIndex, uint8_t channelIndex);

  private:
    Driver74HCT4067* mux[2];
    uint8_t activeMux;
};
