/*******************| Inclusions |*************************************/
#include "ppd42ns.h"
#include <ioCC2530.h>
#include <CC253x.h>
#include <Timer1.h>
/**
 * @brief Module for Shinyei PPD42NS sensor
 * Measurement procedure:
 * - Timer one will run until a given number of overflows are counted
 * - During this time positive and negative input capture interrupts are
 *   used to sum up low occupancy time
 * Connecting the sensor: P1 outpur correspnd to 1um particles and P2 output 
 * corresponds to 2,5um particles. P1 shoud be connector to P0.2 (input capture
 * unit 0, P2 should be conncected to P0.3(input capture unit 1).
*/

/*******************| Macros |*****************************************/

/*******************| Type definitions |*******************************/

/*******************| Global variables |*******************************/
/** @todo add more sensors here */
static PPD42DN_sensorValues_t PPD42NS_sensor0;

/**
 * Number of timer1 overflows during this measurement period
 */
static uint32_t PPD42NS_counterValueTotal = 0;

/*******************| Function definition |****************************/
void PPD42NS_init(PPD42NS_Config_t *config)
{
  /* Two Timer1 input capture units will be used per Shinyei PPD43NS sensor 
   * Channel0 -> P0.2, Channel1 -> P0.3, Channel2 -> P0.4, Channel3 -> P0.5, Channel4 -> P0.6
   * Each Shinyei senor is using two channel.
   */
  PERCFG = PERCFG_T1CFG_ALT1;
  
  /* Enable capture for channel 0 and 1 for sensor 1 including interrupt */
  Timer1_captureCompareChannel0(T1CCTL0_IM | T1CCTL0_MODE_CAPTUREMODE | T1CCTL0_CAP_CAPTUREONALL);
  Timer1_captureCompareChannel1(T1CCTL1_IM | T1CCTL1_MODE_CAPTUREMODE | T1CCTL1_CAP_CAPTUREONALL);
  
  /* Enable timer 1 overflow interrupt */
  enableInterrupt(TIMIF, TIMIF_OVFIM);
  enableInterrupt(IEN1, IEN1_T1IE);
  
  /* Start timer1 with 1MHz */
  Timer1_startSynchronous(T1CTL_DIV_DIV32, 0x0000);  
}

/**
 * Interrupt service routing for Timer 1 Capture/Compare/Overflow interrupt
 * One interrupt vector is assigned to the timer. An interrupt request is generated when one of the following
 * timer events occurs:
 * - Counter reaches terminal count value (overflow, or turns around zero).
 * - Input capture event
 * - Output compare event
 * This functions servers two purposes
 * 1. In case of input capture interrupt the function will sum-up the low occupance time
 * 2. In case of overflow interrupt the function will add and if a given
 * number is reached will calculate the ration between low occupancy and total time.
*/
#pragma vector = T1_VECTOR
__near_func __interrupt void PPD42NS_inputCaptureISR(void)
{
  uint32_t currentTimerValue;
  Timer1_t counterValue;
  
  /* Check T1STAT for interrupt source
  The status register, T1STAT, contains the source interrupt flags for the terminal-count value event and the
  five channel compare/capture events. A source interrupt flag is set when the corresponding event occurs,
  regardless of interrupt mask bits. */
  /* Channel0 -> P0.2 */
  if (checkInterruptFlag(T1STAT, T1STAT_CH0IF) )
  {
    Timer1_readCaptureCompareChannel0(&counterValue);
    currentTimerValue = counterValue.value + PPD42NS_counterValueTotal;
    /* Pin change from LOW -> HIGH? Sum up low occupany time */
    if (P0_2 == Px_HIGH)
    {
      /* @todo: According to datasheet max. low pulse is 90ms. Thus, we should ignore everything above? */
      PPD42NS_sensor0.P1.counterValueLowPulseOccupancy += (currentTimerValue - PPD42NS_sensor0.P1.counterValueLowPulseOccupancystart);
    }
    /* Pin change from HIGH -> LOW? Remember counter value for later low occupancy time calculation */
    if (P0_2 == Px_LOW)
    {
      PPD42NS_sensor0.P1.counterValueLowPulseOccupancystart = currentTimerValue;
    }
    /* clear source flag */
    clearInterruptFlag(T1STAT, T1STAT_CH0IF);
  }
  /* Overflow must be checked last in order to avoid problems when overflow and input capture happens at the same time */
  if (checkInterruptFlag(T1STAT, T1STAT_OVFIF) )
  {
    PPD42NS_counterValueTotal += 0xffff;
    if (PPD42NS_counterValueTotal > PPD42NS_TIMER1_MAX)
    {
      
    }
    clearInterruptFlag(T1STAT, T1STAT_OVFIF);
  } 
  /* Clear interrupt flag, will be directly set again if not all source flags were cleared */
  T1IF = 0;
}
