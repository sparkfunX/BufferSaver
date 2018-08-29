/*
 * Owen Lyke
 * August 2018
 * 
 * Of course you don't always want MOSI connected to MISO - that would prevent you from talking to other sensors!
 * That's why I included a CS pin for the buffer saver. When the CS pin is high the tri-state buffer is disabled.
 * This keeps the SPI bus available for other sensors! (But you can't add more LED strips cause they would need 
 * their own CS pin to receive unique data. Hey... there's an idea!
 * 
 * Free code! As long as it doesn't hurt anyone you can do with it whatever you like.
 */
#include "SPI.h"

#define BRIGHTNESS 10 /* valid [0, 31] */
#define NUM_LEDS 9
uint8_t data[4*(NUM_LEDS + 2)];

#define BS_CS_PIN 0

uint16_t count = 0;
uint8_t ledCount = NUM_LEDS - 1;

SPISettings mySPIsettings(10000000, MSBFIRST, SPI_MODE3);
//SPISettings mySPIsettings(5000000, MSBFIRST, SPI_MODE3);      // Slower test for my Logic4

void setup() {
  SPI.begin();
  kickSPI();
  Serial.begin(9600);
  while(!Serial){};

  pinMode(BS_CS_PIN, OUTPUT);
  digitalWrite(BS_CS_PIN, LOW);

  Serial.println("BufferSaver Problem Illustration");
  Serial.println();
  
  Serial.println("- Connect an APA102 LED strip to the SPI port on your microcontroller");
  Serial.println("- Use a jumper wire to jiggle the data on the MISO pin");

  Serial.println();
  Serial.println("You should be able to observe the output values on the LEDs changing without them being told to do so.");
  Serial.println("This is because SPI.transfer() writes over the buffer that you provide.");

  Serial.println();
  Serial.println("If you connect the buffer saver in between the controller and the strip ");
  Serial.println("and solder the 'always on' jumper) then the outgoing data will be sent ");
  Serial.println("back into the controller, thereby preserving your buffer");


  setupData();  // Initialize the data that you want to send out

  // Now set up a few LEDs the way you like using the setLED function. The first led is led0, then led1 and so on. led(NUM_LEDS-1) is the last led
  // setLED(uint16_t led, uint8_t r, uint8_t g, uint8_t b)
  setLED(3, 255, 0, 0);
  setLED(4, 0, 255, 0);
  setLED(5, 0, 0, 255);

  // Then we send out the data once and enjoy the masterpeice
  sendData();
  delay(1000); // Time to read the message and see that your initial data send worked OK
}

void loop() {
  // Now we will want to change the LED data  to make an animation of sorts

  delay(16);    // 60 Hz refresh... (ish - not accounting for other operations)
  Serial.println(millis());

  // Once per second advance the LED
  if(++count > 60)
  {
    count = 0;
    setLED(ledCount, 0, 0, 0);      // Reset the current LED
    if(++ledCount > NUM_LEDS - 1)   // Increment the current LED
    {
      ledCount = 0; 
    }
    setLED(ledCount, 255, 0, 0);    // Set the current LED (after increment)
  }
  
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
//  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE3));

  SPI.beginTransaction(mySPIsettings);

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

void kickSPI( void )
{
  // "Kick" the SPI hardware to start in the correct clock polarity etc
  SPI.beginTransaction(mySPIsettings);
  for(uint8_t indi = 0; indi < 4; indi++)
  {
    SPI.transfer(0x00);  // You need to provide extra clock cycles to APA102
  }
  SPI.endTransaction();
}


