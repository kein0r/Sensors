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
  uint8 waitCounter = 0;
  /* Only if the sensor was correctly initialized a reading can be done */
  if (DHT22State == DHT22State_Init)
  {
    DHT22State = DHT22State_ReadInProgress;
    /* step 1: MCU sends start signal */
    DHT22_SetDataLineOutput();
    DHT22_WriteDataBitLow();
    delay_us(DHT22_MCUSendStartSignalTime);
    /* step 2: Wait for sensor response */
    DHT22_SetDataLineInput()
    for (waitCounter = 0; waitCounter <  4; waitCounter++) ;
  }
  return DHT22State;
}
