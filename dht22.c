/*******************| Inclusions |*************************************/
#include "dht22.h"
#include <dht22_cfg.h>

/*******************| Macros |*****************************************/

/*******************| Type definitions |*******************************/

/*******************| Global variables |*******************************/
static DHT22State_t DHT22State = DHT22State_Uninit;
static DHT22_SensorValue_t DHT22_SensorValue;
#ifdef DHT22_DEBUG
/**
 * Copy of the wait counters for the communication between the sensor and
 * the MCU. These values can be used to match your MCU to the sensor and helps
 * to configure the values in dht22_cfg.h
*/
volatile uint16 DHT22_sensorWaitCounter[2];
/**
 * Copy of the bit wait counters for the communication between the sensor and
 * the MCU. These values can be used to match your MCU to the sensor  and helps
 * to configure the values in dht22_cfg.h
*/
volatile uint16 DHT22_sensorBitWaitCounter[DHT22_NUMBEROFBITSFROMSENSOR];
#endif

/*******************| Function definition |****************************/
void DHT22_init(void)
{
  DHT22_SetDataLineOutput();
  DHT22_WriteDataBitHigh();
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
    waitCounter = DHT22_MCUWaitForSensorResponse;
    while (waitCounter)
    {
      if (DHT22_ReadDataBit() == DHT22_DATALINE_LOW) break;
      waitCounter--;
    }
    DHT22_sensorWaitCounter[0] = waitCounter;
    if (waitCounter == 0) {
      DHT22State = DHT22State_ReadErrorStuckAtVCC;
      return DHT22State;
    }
    waitCounter = DHT22_MCUWaitForSensorResponse;
    while (waitCounter)
    {
      if (DHT22_ReadDataBit() == DHT22_DATALINE_HIGH) break;
      waitCounter--;
    }
    DHT22_sensorWaitCounter[1] = waitCounter;
    if (waitCounter == 0) {
      DHT22State = DHT22State_ReadErrorStuckAtGND;
      return DHT22State;
    }
    /* step 3: wait for start of transmission and receive all bits */
    /* to preserve byte order in union/struct we readout in oposite order from bit 39 -> 0 */
    bitCounter = DHT22_NUMBEROFBITSFROMSENSOR;
    while (bitCounter--)
    {
      waitCounter = 0;
      /* wait for the line to go low again */
      while (DHT22_ReadDataBit() != DHT22_DATALINE_LOW) ;
      /* wait for the line to go high */
      while (DHT22_ReadDataBit() != DHT22_DATALINE_HIGH) ;
      /* now count high time */
      while (DHT22_ReadDataBit() == DHT22_DATALINE_HIGH)
      {
        waitCounter++;
      }
#ifdef DHT22_DEBUG
      DHT22_sensorBitWaitCounter[bitCounter] = waitCounter;
#endif
      DHT22_SensorValue.raw[bitCounter / 8] = DHT22_SensorValue.raw[bitCounter / 8] << 1;
      if (waitCounter < DHT22_MCUWaitForSensorSendZero)
      {
        /* zero detected */
        DHT22_SensorValue.raw[bitCounter / 8] &= 0xfe;
      } else {
        /* one detected */
        DHT22_SensorValue.raw[bitCounter / 8] |= 0x01;
      }
    }
    /* check CRC */
    if (DHT22_SensorValue.raw[0] != (uint8)(DHT22_SensorValue.raw[1] + DHT22_SensorValue.raw[2] + DHT22_SensorValue.raw[3] + DHT22_SensorValue.raw[4]))
    {
      DHT22_SensorValue.values.Temperatur = DHT22_TemperaturInvalidValue;
      DHT22_SensorValue.values.RelativeHumidity = DHT22_RelativeHumidityInvalidValue;
      DHT22State = DHT22State_ReadErrorCRCInvalid;
      return DHT22State;
    }
    /* correct negative values for temperatur */
    if (DHT22_SensorValue.raw[4] & 0x80) DHT22_SensorValue.values.Temperatur *= -1;
    DHT22State = DHT22State_Init;
  }
  return DHT22State;
}
