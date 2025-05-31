#include "Driver74HCT4067.h"
#include "driver/gpio.h"
#include "soc/gpio_struct.h"

#define MUX_SETTLING_TIME_US 500

Driver74HCT4067::Driver74HCT4067(uint8_t s0Pin, uint8_t s1Pin, uint8_t s2Pin, uint8_t s3Pin,
                                 uint8_t enPin, uint8_t sigPin, bool capacitive)
{
  s0 = s0Pin;
  s1 = s1Pin;
  s2 = s2Pin;
  s3 = s3Pin;
  en = enPin;
  sig = sigPin;
  useTouch = capacitive;
  currentIndex = 0;

  pinMode(s0, OUTPUT);
  pinMode(s1, OUTPUT);
  pinMode(s2, OUTPUT);
  pinMode(s3, OUTPUT);
  pinMode(en, OUTPUT);

  // Disable MUX at start (EN high)
  GPIO.out_w1ts = (1 << en);

  memset(values, 0, sizeof(values));
}

void Driver74HCT4067::setChannel(uint8_t channel)
{
  // Clear all selector pins at once
  GPIO.out_w1tc = (1 << s0) | (1 << s1) | (1 << s2) | (1 << s3);

  // Set each bit individually if needed
  if (channel & 0x01) GPIO.out_w1ts = (1 << s0);
  if (channel & 0x02) GPIO.out_w1ts = (1 << s1);
  if (channel & 0x04) GPIO.out_w1ts = (1 << s2);
  if (channel & 0x08) GPIO.out_w1ts = (1 << s3);
}

void Driver74HCT4067::readNext()
{
  setChannel(currentIndex);

  // Enable MUX (EN low)
  GPIO.out_w1tc = (1 << en);
  delayMicroseconds(MUX_SETTLING_TIME_US);

  int raw = useTouch ? touchRead(sig) : analogRead(sig);
  values[currentIndex] = raw;

  // Disable MUX (EN high)
  GPIO.out_w1ts = (1 << en);

  currentIndex = (currentIndex + 1) % 16;
}

uint16_t Driver74HCT4067::get(uint8_t index)
{
  return values[index];
}

uint8_t Driver74HCT4067::getCurrentIndex()
{
  return currentIndex;
}
