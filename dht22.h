/** @ingroup DHT22
 * @{
 */

#ifndef DHT22_H_
#define DHT22_H_
   
/*******************| Inclusions |*************************************/
#include <PlatformTypes.h>
   
/*******************| Macros |*****************************************/
#if (!defined DHT22_OK)
#define DHT22_OK        0
#endif

#if (!defined DHT22_NOT_OK) 
#define DHT22_NOT_OK    1
#endif

/* number of bits the sensor sends back */
#define DHT22_NUMBEROFBITSFROMSENSOR            40
   
/*******************| Type definitions |*******************************/
typedef enum
{
   DHT22State_Uninit,
   DHT22State_Init,
   DHT22State_ReadInProgress,
   DHT22State_ReadErrorStuckAtVCC,
   DHT22State_ReadErrorStuckAtGND,
   DHT22State_ReadErrorCRCInvalid
} DHT22State_t;

/**
* Structure to directly read out sensor values. To re-use byte order the values are
* read out from byte 3 to byte 0.
*/
typedef union
{
  struct {
    uint8 CRC;
    sint16 Temperatur;            /*!< Operating range temperature -40~80 Celsius, resolution 0.1Celsius */
    uint16 RelativeHumidity;      /*!< Operating range humidity 0-100%RH, resolution 0.1%RH */
  } values;
  uint8 raw[DHT22_NUMBEROFBITSFROMSENSOR / 8];
} DHT22_SensorValue_t;

/*******************| Global variables |*******************************/
extern DHT22_SensorValue_t DHT22_SensorValue;

/*******************| Function prototypes |****************************/
void DHT22_init(void);
DHT22State_t DHT22_readValues(void);

#endif
/** @}*/
