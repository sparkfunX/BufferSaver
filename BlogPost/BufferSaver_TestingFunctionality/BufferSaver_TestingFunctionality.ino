
#include "SPI.h"

#define BRIGHTNESS 31 /* valid [0, 31] */
#define NUM_LEDS 9
uint8_t data[4*(NUM_LEDS + 2)];

void setup() {
  // put your setup code here, to run once:
  SPI.begin();
  Serial.begin(9600);
  while(!Serial){};

  Serial.println("Welcome!");

  setupData();
  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE3));
  SPI.transfer(data, 4*(NUM_LEDS+2));
  SPI.endTransaction();
  
}

void loop() {
  // put your main code here, to run repeatedly:

  delay(16); // 60 Hz refresh... (ish - not accounting for other operations)
  Serial.println("x");

  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
  SPI.transfer(data, 4*(NUM_LEDS+2));
  SPI.endTransaction();

}

void setupData( void )
{
  data[0] = 0x00;
  data[1] = 0x00;
  data[2] = 0x00;
  data[3] = 0x00;
  for(uint16_t indi = 0; indi < NUM_LEDS; indi++)
  {
    data[(4* indi) + 0] = (0xE0 | (0x1F & BRIGHTNESS));
    data[(4* indi) + 1] = 0x00;
    data[(4* indi) + 2] = 0x00;
    data[(4* indi) + 3] = 0x00;
  }
  data[(4* NUM_LEDS) + 0] = 0xFF;
  data[(4* NUM_LEDS) + 1] = 0xFF;
  data[(4* NUM_LEDS) + 2] = 0xFF;
  data[(4* NUM_LEDS) + 3] = 0xFF;
}



