/*
 * Owen Lyke
 * August 2018
 * 
 * 
 * 
 * Free code! As long as it doesn't hurt anyone you can do with it whatever you like.
 */
#include "SPI.h"

#define BRIGHTNESS 10 /* valid [0, 31] */
#define NUM_LEDS 9
uint8_t data[4*(NUM_LEDS + 2)];

void setup() {
  SPI.begin();
  Serial.begin(9600);
  while(!Serial){};

  Serial.println("BufferSaver Problem Illustration");
  Serial.println();
  
  Serial.println("- Connect an APA102 LED strip to the SPI port on your microcontroller");
  Serial.println("- Use a jumper wire to jiggle the data on the MISO pin");

  Serial.println("You should be able to observe the output values on the LEDs changing without them being told to do so.");
  Serial.println("This is because SPI.transfer() writes over the buffer that you provide.");

  // "Kick" the SPI hardware to start in the correct clock polarity etc
  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE3));
  SPI.transfer(0x00);  // You need to provide extra clock cycles to APA102
  SPI.endTransaction();
  delay(100);
  

  setupData();  // Initialize the data that you want to send out

  // Now set up a few LEDs the way you like using the setLED function. The first led is led0, then led1 and so on. led(NUM_LEDS-1) is the last led
  // setLED(uint16_t led, uint8_t r, uint8_t g, uint8_t b)
  setLED(3, 255, 0, 0);
  
  sendData();

  delay(10000); // Time to read the message and see that your initial data send worked OK
}

void loop() {
  delay(16);    // 60 Hz refresh... (ish - not accounting for other operations)
  Serial.println(millis());
  
  sendData();   // So, when you send the data with SPI.transfer() the data array will be overwritten and you can't be sure what will come out next!
}

void setupData( void )
{
  data[0] = 0x00;                     // 4 bytes of all zeros signals 
  data[1] = 0x00;                     // a new data stream to the leds
  data[2] = 0x00;
  data[3] = 0x00;
  for(uint16_t indi = 0; indi < NUM_LEDS; indi++)
  {
    // Here's where the actual LED data exists - 4 bytes for each led
    data[(4* indi) + 0] = (0xE0 | (0x1F & BRIGHTNESS)); // Brightness has a special format of 0b111BBBBB 
    data[(4* indi) + 1] = 0x00; // Blue
    data[(4* indi) + 2] = 0x00; // Green
    data[(4* indi) + 3] = 0x00; // Red
  }
  data[(4* NUM_LEDS) + 0] = 0xFF;     // The last 4 bytes are all white.
  data[(4* NUM_LEDS) + 1] = 0xFF;     // Not strictly necessary, but it
  data[(4* NUM_LEDS) + 2] = 0xFF;     // helps you know if you have more 
  data[(4* NUM_LEDS) + 3] = 0xFF;     // LEDs available
}

void sendData( void )
{
  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE3));
  SPI.transfer(data, 4*(NUM_LEDS+2));
  for(uint16_t indi = 0; indi < NUM_LEDS/2 + 1; indi++)
  {
    SPI.transfer(0x00);  // You need to provide extra clock cycles to APA102
  }
  SPI.endTransaction();
}

void setLED(uint16_t led, uint8_t r, uint8_t g, uint8_t b)
{
  if(led >= NUM_LEDS)
  {
    return;
  }
  
  data[4*(led + 0) + 0] = (0xE0 | (0x1F & BRIGHTNESS));
  data[4*(led + 0) + 1] = b;
  data[4*(led + 0) + 2] = g;
  data[4*(led + 0) + 3] = r;
}



