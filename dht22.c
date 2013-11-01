/*******************| Inclusions |*************************************/
#include "dht22.h"
#include <dht22_cfg.h>

/*******************| Macros |*****************************************/

/*******************| Type definitions |*******************************/

/*******************| Global variables |*******************************/
static DHT22State_t DHT22State = DHT22State_Uninit;

/*******************| Function definition |****************************/
void DHT22_init(void)
{
  DHT22State = DHT22State_Init;
}

/* 
 * Do read-out from the sensor. Read-out will only be started if no other 
 * read-out is done right now.
 * @return if read could be started funciton will return DHT22_OK, if not
 * DHT22_NOT_OK
 */
DHT22State_t DHT22_readValues(void)
{
  uint16 waitCounter = 0;
  uint8 bitCounter = 0;
  /* Only if the sensor was correctly initialized a reading can be done */
  if (DHT22State == DHT22State_Init)
  {
    DHT22State = DHT22State_ReadInProgress;
    /* step 1: MCU sends start signal */
    DHT22_SetDataLineOutput();
    DHT22_WriteDataBitLow();
    delay_us(DHT22_MCUSendStartSignalTime);
    DHT22_WriteDataBitHigh();
    DHT22_SetDataLineInput();
    /* step 2: Wait for sensor response with low pulse and high pulse */
    waitCounter = BOARD_TICKSPERMICROSECOND * DHT22_MCUWaitForSensorResponse;
    while (waitCounter)
    {
      if (DHT22_ReadDataBit() == DHT22_DATALINE_LOW) break;
      waitCounter--;
    }
    if (waitCounter == 0) {
      DHT22State = DHT22State_ReadErrorStuckAtVCC;
      return DHT22State;
    }
    waitCounter = BOARD_TICKSPERMICROSECOND * DHT22_MCUWaitForSensorResponse;
    while (waitCounter)
    {
      if (DHT22_ReadDataBit() == DHT22_DATALINE_HIGH) break;
      waitCounter--;
    }
    if (waitCounter == 0) {
      DHT22State = DHT22State_ReadErrorStuckAtGND;
      return DHT22State;
    }
    /* step 3: wait for start of transmission and receive all bits */
    for (bitCounter = 0; bitCounter < DHT22_NUMBEROFBITSFROMSENSOR; bitCounter++)
    {
      waitCounter = BOARD_TICKSPERMICROSECOND * DHT22_MCUWaitForSensorResponse;
      /* wait for the line to go low again */
      while (waitCounter)
      {
        if (DHT22_ReadDataBit() == DHT22_DATALINE_LOW) break;
        waitCounter--;
      }
      if (waitCounter == 0) {
        DHT22State = DHT22State_ReadErrorStuckAtVCC;
        return DHT22State;
      }
      
    }
  }
  return DHT22State;
}
