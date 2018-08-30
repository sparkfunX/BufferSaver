/*
 * This is a sort of simple benchmark program to see how much the BufferSaver saves you in terms of execution time
 * 
 * Owen Lyke
 * August 2018
 * 
 * Free code! As long as it doesn't hurt anyone you can do with it what you please.
 */

#include "SPI.h"

typedef struct{
  uint16_t numTests;
  uint32_t * p_sizes;
  uint32_t * presults_individual;
  uint32_t * presults_enforce;
  uint32_t * presults_buffer;
}TestParams_TypeDef;


#define NUM_TESTS 10
#define CS 0

SPISettings mySettings(10000000, MSBFIRST, SPI_MODE3);

uint32_t sizes[NUM_TESTS] = {1,5,10,15,25,50,100,250,500,1000};
uint32_t resInd[NUM_TESTS];
uint32_t resEnf[NUM_TESTS];
uint32_t resBuf[NUM_TESTS];


TestParams_TypeDef testSpec;



void setup() {
  // put your setup code here, to run once:
  pinMode(CS, OUTPUT);
  digitalWrite(CS, HIGH);


  testSpec.numTests = NUM_TESTS;
  testSpec.p_sizes = sizes;
  testSpec.presults_individual = resInd;
  testSpec.presults_enforce = resEnf;
  testSpec.presults_buffer = resBuf;

  SPI.begin();

  Serial.begin(9600);
  while(!Serial){};

  Serial.println("Buffer Saver - Testing Efficiency");
  Serial.println("---------------------------------");
  Serial.println("                                 ");
  printTestParameters(&testSpec);

  runFullTest(&testSpec);
  printTestResults(&testSpec);

}

void loop() {
  // put your main code here, to run repeatedly:

}

void runFullTest(TestParams_TypeDef * ptestParams)
{
  for(uint16_t indi = 0; indi < ptestParams->numTests; indi++)
  {
    Serial.print("testing case: "); Serial.println(indi);
    testCase(indi, ptestParams);
  }
  Serial.println("\n");
}

void testCase(uint16_t caseIndex, TestParams_TypeDef * ptestParams)
{
// Allocate the memory to send out
  uint32_t numBytes = *(ptestParams->p_sizes + caseIndex);
  uint8_t * pdata = NULL;
  pdata = (uint8_t *)malloc(numBytes*sizeof(uint8_t));
  if(pdata == NULL)
  {
    abortCase(caseIndex, ptestParams);
    free(pdata);
    return;
  }

  *(ptestParams->presults_individual + caseIndex) =  testIndividualWrites(pdata, numBytes);
  *(ptestParams->presults_enforce + caseIndex) =  testEnforceBuffer(pdata, numBytes);
  *(ptestParams->presults_buffer + caseIndex) =  testBufferSaverMethod(pdata, numBytes);

  // deallocate the memory
  free(pdata);
}

void abortCase(uint16_t caseIndex, TestParams_TypeDef * ptestParams)
{
  // Set results to zero here
  *(ptestParams->presults_individual + caseIndex) = 0;
  *(ptestParams->presults_enforce + caseIndex) = 0;
  *(ptestParams->presults_buffer + caseIndex) = 0;
}

uint32_t testIndividualWrites(uint8_t * pdata, uint32_t size)
{
  // In this test the cost of setting the data is ignored because it is protected by just transferring one byte at a time
  SPI.beginTransaction(mySettings);
  uint32_t tn = micros();
  for(uint32_t indi = 0; indi < size; indi++)
  {
    SPI.transfer(*(pdata + indi)); // Directly transfer from the data buffer one byte at a time. 
  }
  uint32_t tt = micros();
  SPI.endTransaction();
  return (tt-tn);
}

uint32_t testEnforceBuffer(uint8_t * pdata, uint32_t size)
{
  // In this test the time to reset the whole buffer is included because it happens for every output
  SPI.beginTransaction(mySettings);
  volatile uint8_t volvar = 0;
  uint32_t tn = micros();
  for(uint32_t indi = 0; indi < size; indi++)
  {
    *(pdata + indi) = volvar; // Volatile variable used to prevent compiler optimization
  }
  SPI.transfer(pdata, size);
  uint32_t tt = micros();
  SPI.endTransaction();
  return (tt-tn);
}

uint32_t testBufferSaverMethod(uint8_t * pdata, uint32_t size)
{
  SPI.beginTransaction(mySettings);
  uint32_t tn = micros();
  digitalWrite(CS, LOW);      // Worst case scenario you will need to toggle a pin
  SPI.transfer(pdata, size);  // If the buffer saver is used you can just send the data, no worries
  digitalWrite(CS, HIGH);     // And detoggle the pin when you're done
  uint32_t tt = micros();
  SPI.endTransaction();
  return (tt-tn);  
}

void printTestResults(TestParams_TypeDef * ptestParams)
{
  Serial.println("Test Results:");
  Serial.println("-------------");
  Serial.println("");

  Serial.println("Number of bytes, Individual Writes (uS), Pre-enforce (uS), BufferSaver (uS)");
  Serial.println("---------------------------------------------------------------------------");
  for(uint32_t indi = 0; indi < ptestParams->numTests; indi++)
  {
    Serial.print(*(ptestParams->p_sizes + indi)); Serial.print(", ");
    Serial.print(*(ptestParams->presults_individual + indi)); Serial.print(", ");
    Serial.print(*(ptestParams->presults_enforce + indi)); Serial.print(", ");
    Serial.print(*(ptestParams->presults_buffer + indi)); Serial.println("");
  }
}

void printTestParameters(TestParams_TypeDef * ptestParams)
{
  Serial.println("Test Settings (change in setup):");
  Serial.print("Number of cases to run: "); Serial.println(ptestParams->numTests);

  Serial.print("Number of bytes to write in each test: ");
  for(uint32_t indi = 0; indi < ptestParams->numTests; indi++)
  {
    Serial.print(*(ptestParams->p_sizes + indi)); 
    
    if(indi != (ptestParams->numTests - 1))
    {
      Serial.print(", ");
    }
  }
  Serial.println();
}

