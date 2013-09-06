/*******************| Inclusions |*************************************/
#include "dht22.h"
#include <dht22_cfg.h>

/*******************| Macros |*****************************************/

/*******************| Type definitions |*******************************/

/*******************| Global variables |*******************************/
static DHT22State_t DHT22State = DHT22State_Init;
static uint8 DHT22WaitCounter = 0;
static uint8 DHT22ReceivedBitsCounter = 0;

/*******************| Function definition |****************************/
void DHT22_Init(void)
{
  
}

/* 
 * Start a read-out from the sensor. Read-out will only be started if no
 * other read-out is done right now.
 * @return if read could be started funciton will return DHT22_OK, if not
 * DHT22_NOT_OK
 */
uint8 DHT22_startRead(void)
{
  if (DHT22State == DHT22State_Init)
  {
    DHT22WaitCounter = 0;
    DHT22State = DHT22State_MCUSendStartSignal;
    return DHT22_OK;
  } 
  else return DHT22_NOT_OK;
}

void DHT22_MainFunction(void)
{
  DHT22State_t nextState;
  
  /*
   * classical state machine approach.
   * First evaluate (next) state according to given state and conditions then
   * do transition to (next) state and perform required actions for the (new) state
   */
  nextState = DHT22State; /* if no change needed we will stay in current state */
  switch (DHT22State)
  {
  case DHT22State_Init:
    break;
  case DHT22State_MCUSendStartSignal:
    if (DHT22WaitCounter >= DHT22_MCUSENDSTARTSIGNAL_TIME)
    {      
      nextState = DHT22State_MCUWaitForSensorResponse; 
    }
    break;
  case DHT22State_MCUWaitForSensorResponse:
    /* directly switch to next state since it's only used to switch port direction */
    nextState = DHT22State_DHTSendResponseLow;
    break;
  case DHT22State_DHTSendResponseLow:
    if (DHT22_ReadDataBit() == DHT22_DATALINE_LOW)
    {
      nextState = DHT22State_DHTSendResponseHigh;
    }
    break;
  case DHT22State_DHTSendResponseHigh:
    if (DHT22_ReadDataBit() == DHT22_DATALINE_HIGH)
    {
      nextState = DHT22State_DHTStartTransmission;
    }
    break;
  case DHT22State_DHTStartTransmission:
    if (DHT22_ReadDataBit() == DHT22_DATALINE_LOW)
    {
      nextState = DHT22State_DHTTransmit;
    }
    break;
  case DHT22State_DHTTransmit:
    if (DHT22ReceivedBitsCounter == DHT22_NUMBEROFBITSFROMSENSOR)
    {
      nextState = DHT22State_Init;
    }
    break;
  }
  
  /* 
   * now perform required actions for state
   */
  switch (nextState)
  {
  case DHT22State_Init:
    DHT22WaitCounter = 0;
    break;
  case DHT22State_MCUSendStartSignal:
    /* put output low for 50ms */
    DHT22_WriteDataBitLow();
    DHT22WaitCounter += DHT22_MAINFUNCTION_CYCLETIME;
    break;
  case DHT22State_MCUWaitForSensorResponse:
     DHT22_SetDataLineInput();
     break;
  case DHT22State_DHTSendResponseLow:
    break;
  case DHT22State_DHTSendResponseHigh:
    break;
  case DHT22State_DHTStartTransmission:
    break;
  case DHT22State_DHTTransmit:
    break;
  }
  DHT22State = nextState;
}
