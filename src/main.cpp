//**********************************************************************************************************
//*    audioI2S-- I2S audiodecoder for ESP32,                                                              *
//**********************************************************************************************************
//
// first release on 11/2018
// Version 3  , Jul.02/2020
//
//
// THE SOFTWARE IS PROVIDED "AS IS" FOR PRIVATE USE ONLY, IT IS NOT FOR COMMERCIAL USE IN WHOLE OR PART OR CONCEPT.
// FOR PERSONAL USE IT IS SUPPLIED WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHOR
// OR COPYRIGHT HOLDER BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
// OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE
//

#include "Arduino.h"
#include "WiFiMulti.h"

#define AUDIO_INFO(x)
#define AUDIO_ERROR(x)
#define AUDIO_DEBUG(x)
#define AUDIO_WARNING(x)

#include "Audio.h"
#include "SPI.h"
#include "SD.h"
#include "FS.h"
#include <MuxController.h>

// Digital I/O used
#define SD_CS 5
#define SPI_MOSI 23
#define SPI_MISO 19
#define SPI_SCK 18
#define I2S_DOUT 25
#define I2S_BCLK 27
#define I2S_LRC 26

Audio audio;
WiFiMulti wifiMulti;
String ssid = "yourssid";
String password = "yourpass";

// Inclusion des classes
MuxController muxController;
// Driver74HCT4067 *mux;

void setup()
{
  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
  SPI.setFrequency(1000000);
  Serial.begin(115200);
  SD.begin(SD_CS);
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);

  /*
  WiFi.mode(WIFI_STA);
  wifiMulti.addAP(ssid.c_str(), password.c_str());
  wifiMulti.run();
  if (WiFi.status() != WL_CONNECTED)
  {
    WiFi.disconnect(true);
    wifiMulti.run();
  }
  Serial.print("info        ");
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setVolume(12); // 0...21
  */

  // Drone Zone
  // audio.connecttohost("http://ice1.somafm.com/dronezone-128-mp3");
  // Groove Salad
  // audio.connecttohost("http://ice1.somafm.com/groovesalad-128-mp3");
  // Secret Agent
  // audio.connecttohost("http://ice1.somafm.com/secretagent-128-mp3");
  // Space Station
 // audio.connecttohost("http://ice1.somafm.com/spacestation-128-mp3");

  Serial.print("info        ");
  analogSetWidth(12);
  analogSetAttenuation(ADC_ATTENDB_MAX);
  //analogSetClockDiv(2);
  
  // mux = new Driver74HCT4067(12, 13, 14, 15, 16, 36, false);
}
void plotValues(uint8_t id, uint16_t value)
{
  Serial.print('>');
  Serial.print("");
  Serial.print(id);
  Serial.print(':');
  Serial.print(value);
  Serial.println();
  Serial.flush(); 

}

float filtered = 0.0f;
void loop()
{

 // audio.loop();
  /*
    uint16_t raw = analogRead(36);
    filtered = 0.9f * filtered + 0.1f * (float)raw;
    plotValues(0, (uint16_t)filtered);
    delay(10);
  */
  // Exemple : afficher la valeur normalisée du canal 3 du MUX 0
  /**/
  uint8_t id = 0;
 
    for (uint8_t i = 0; i < 16; ++i)
    {
      muxController.readNext();
   
      float raw = muxController.get(0, 1);
      filtered = 0.9f * filtered + 0.1f * (float)raw;
      plotValues(1, (uint16_t)raw);
      // Lecture cadencée par tour
      delay(5); 
    }

}