# geomancia synth

## initial project 
    https://github.com/schreibfaul1/ESP32-audioI2S

## next iteration will be here
https://github.com/cosmicAquarius/geomancia


![alt text](_doc/asset/wire.jpg)


The ADS1015 ADC Module has 14 pins:

* 5V: Power supply input (5V)
* 3.3V: Power supply input (3.3V)
* GND: Ground reference for the IC
* LRC (Left/Right Clock): Input for left/right channel identification
* DOUT (Data Out): Serial Data Output for transmitting converted analog signals
* BCLK (Bit Clock): Input for synchronizing data transmission
* LOUT (Left Channel Output): Analog audio output for the left channel
* AGND (Analog Ground): Ground for internal analog reference
* ROUT (Right Channel Output): Analog audio output for the right channel
* SCLK (System Clock): Input for system clock synchronization
* SF0 and SF1 (Digital Filter Select): Inputs for selecting digital filter options. SF0 and SF1 are used to set the input data format. By default, both are pulled Low for I2S but you can change them around for alternate formats.
* MUTE: Input for muting the audio output
* PLL (Phase-Locked Loop): Input for external clock synchronization
* DEEM (De-emphasis): Input for de-emphasis control

![alt text](_doc/asset/image.png)

## multiplexer

![alt text](<_doc/asset/image copy.png>)

## esp32 attention spécifique
Attention ce modèle n'est pas standar le brochage n'est pas standar


![alt text](_doc/asset/esp32_non_standard.png)

### 🔌 Câblage 1x 74HC4067 (potentiomètres ou capacitif)

| Fonction      | GPIO   | Détail technique                   | Justification               |
| ------------- | ------ | ---------------------------------- | --------------------------- |
| `S0`          | **12** | Port 0, bit 12                     | Début du bloc sélecteurs    |
| `S1`          | **13** | Port 0, bit 13                     |                             |
| `S2`          | **14** | Port 0, bit 14                     |                             |
| `S3`          | **15** | Port 0, bit 15                     | Bloc complet S0–S3          |
| `EN0`         | **16** | Port 0, bit 16                     | Début bloc enable           |
| `EN1`         | **17** | Port 0, bit 17                     |                             |
| `EN2`         | **21** | Port 0, bit 21                     |                             |
| `EN3`         | **22** | Port 0, bit 22                     | Bloc `ENx`, tous sur port 0 |
| `SIG_IN`      | **36** | ADC1\_CH0 (entrée uniquement)      | Unique entrée analogique    |
| `MAX7219_CLK` | **4**  | disponible (prévu SPI2 / soft SPI) | Réservé                     |
| `MAX7219_CS`  | **2**  | disponible                         | Réservé                     |
| `MAX7219_DIN` | **0**  | disponible                         | Réservé                     |




|74hct4067|  MUX0  |   MUX1   |  MUX2 |  MUX3   |
| ------- | ------ | -------- | ----- | ------- |
|S0──────►| GPIO12 |  shared  |       |         |
|S1──────►| GPIO13 |  shared  |       |         |
|S2──────►| GPIO14 |  shared  |       |         |
|S3──────►| GPIO15 |  shared  |       |         |
|EN──────►| 16     | 17       |  21   |     22  |
|SIG─────►| GPIO36 (ADC)                        |
