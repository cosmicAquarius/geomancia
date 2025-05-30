#ifndef DRIVER_74HCT4067_H
#define DRIVER_74HCT4067_H

#include <Arduino.h>

class Driver74HCT4067 {
public:
  Driver74HCT4067(uint8_t s0Pin, uint8_t s1Pin, uint8_t s2Pin, uint8_t s3Pin,
                  uint8_t enPin, uint8_t sigPin, bool capacitive = false);

  void readNext();
  uint16_t get(uint8_t index);
  uint8_t getCurrentIndex();

private:
  void setChannel(uint8_t channel);

  uint8_t s0, s1, s2, s3;
  uint8_t en;
  uint8_t sig;
  bool useTouch;
  uint8_t currentIndex;
  uint16_t values[16];
};

#endif
