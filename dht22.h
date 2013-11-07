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
#define DHT22_NUMBEROFBITSFROMSENSOR            48
   
/*******************| Type definitions |*******************************/
typedef enum
{
   DHT22State_Uninit,
   DHT22State_Init,
   DHT22State_ReadInProgress,
   DHT22State_ReadErrorStuckAtVCC,
   DHT22State_ReadErrorStuckAtGND,
} DHT22State_t;

typedef struct
{
  uint16 RelativeHumidity;      /*!< Operating range humidity 0-100%RH, resolution 0.1%RH */
  sint16 Temperatur;            /*!< Operating range temperature -40~80 Celsius, resolution 0.1Celsius */
  uint8 CRC;
} DHT22_SensorValue_t;

/*******************| Global variables |*******************************/

/*******************| Function prototypes |****************************/
void DHT22_init(void);
DHT22State_t DHT22_readValues(void);

#endif
/** @}*/
