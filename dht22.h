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

/* all times in ms */
#define DHT22_MCUSENDSTARTSIGNAL_TIME           20
#define DHT22_NUMBEROFBITSFROMSENSOR            48
   
/*******************| Type definitions |*******************************/
typedef enum
{
   DHT22State_Init,
   DHT22State_MCUSendStartSignal,
   DHT22State_MCUWaitForSensorResponse,
   DHT22State_DHTSendResponseLow,
   DHT22State_DHTSendResponseHigh,
   DHT22State_DHTStartTransmission,
   DHT22State_DHTTransmit
} DHT22State_t;

/*******************| Global variables |*******************************/

/*******************| Function prototypes |****************************/
void DHT22_Init(void);
uint8 DHT22_startRead(void);
void DHT22_MainFunction(void);

#endif
/** @}*/
