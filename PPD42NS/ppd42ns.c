/*******************| Inclusions |*************************************/
#include "ppd42ns.h"
#include "Config.h"
#include <ioCC2530.h>
#include <stdbool.h>
#include <CC253x.h>
#include <Timer1.h>

/**
 * @brief Module for Shinyei PPD42NS sensor
 * Measurement procedure:
 * - Timer one will run until a given number of overflows is reached
 * - During this time positive and negative input capture interrupts are
 *   used to sum up low occupancy time
 * Connecting the sensor: P1 output corresponds to 1um particles and P2 output 
 * corresponds to 2,5um particles. P1 shoud be connected to P0.2 (input capture
 * unit 0, P2 should be conncected to P0.3(input capture unit 1).
*/

/*******************| Macros |*****************************************/

/*******************| Type definitions |*******************************/

/*******************| Global variables |*******************************/
/** @todo add more sensors here */
static PPD42DN_sensorValues_t PPD42NS_sensor0;
#ifdef PPD42NS_SENSOR1CONNECTED
static PPD42DN_sensorValues_t PPD42NS_sensor1;
#endif

/**
 * Number of timer1 overflows during this measurement period
 */
static uint32_t PPD42NS_counterValueTotal = 0;

static bool PPD42NS_nextSensorValueAvailable = false;

/*******************| Function definition |****************************/
void PPD42NS_init()
{
  /* Initialize values for measurement */
  PPD42NS_sensor0.P1.counterValueLowPulseOccupancy = 0;
  PPD42NS_sensor0.P2.counterValueLowPulseOccupancy = 0;
  
  /* Two Timer1 input capture units will be used per Shinyei PPD43NS sensor 
   * Channel0 -> P0.2, Channel1 -> P0.3, Channel2 -> P0.4, Channel3 -> P0.5, Channel4 -> P0.6
   * Each Shinyei senor is using two channel.
   */
  PERCFG |= PERCFG_T1CFG_ALT1;
  
  /* No need to configure PIN direction because "When a channel is configured as an input 
   * capture channel, the I/O pin associated with that channel is configured as an input."
   * However, "Before an I/O pin can be used by the timer, the required I/O pin must be 
   * configured as a Timer 1 peripheral pin."
   */
  P0SEL |= P0SEL_SELP0_2_PERIPHERALFUNCTION | P0SEL_SELP0_3_PERIPHERALFUNCTION | P0SEL_SELP0_4_PERIPHERALFUNCTION | P0SEL_SELP0_5_PERIPHERALFUNCTION;
  
  /* Set priority of peripherals to 
   * 1st priority: Timer 1 channels 0-1
   * 2nd priority: USART 1
   * 3rd priority: USART 0
   * 4th priority: Timer 1 channels 2-3 */
  P2DIR = P2DIR_PRIP0_TIMER1CH01USART1USART0TIMER1CH23;
  
  /* Enable capture for channel 0 and 1 for sensor 1 including interrupt */
  Timer1_captureCompareChannel0(T1CCTL0_IM | T1CCTL0_MODE_CAPTUREMODE | T1CCTL0_CAP_CAPTUREONALL);
  Timer1_captureCompareChannel1(T1CCTL1_IM | T1CCTL1_MODE_CAPTUREMODE | T1CCTL1_CAP_CAPTUREONALL);
#ifdef PPD42NS_SENSOR1CONNECTED
  Timer1_captureCompareChannel2(T1CCTL2_IM | T1CCTL2_MODE_CAPTUREMODE | T1CCTL2_CAP_CAPTUREONALL);
  Timer1_captureCompareChannel3(T1CCTL3_IM | T1CCTL3_MODE_CAPTUREMODE | T1CCTL3_CAP_CAPTUREONALL);
#endif
  
  /* Enable timer 1 overflow interrupt */
  enableInterrupt(TIMIF, TIMIF_OVFIM);
  enableInterrupt(IEN1, IEN1_T1IE);
  
  /* Start timer1 with 1MHz (Divide by 32) and start timer 1 in free running mode */
  Timer1_startSynchronous(T1CTL_DIV_DIV32 | T1CTL_MODE_FREERUNNING, 0x0000);  
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
 * number is reached will calculate the ratio between low occupancy and total time.
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
  /* Channel1 -> P0.3 */
  if (checkInterruptFlag(T1STAT, T1STAT_CH1IF) )
  {
    Timer1_readCaptureCompareChannel1(&counterValue);
    currentTimerValue = counterValue.value + PPD42NS_counterValueTotal;
    /* Pin change from LOW -> HIGH? Sum up low occupany time */
    if (P0_3 == Px_HIGH)
    {
      /* @todo: According to datasheet max. low pulse is 90ms. Thus, we should ignore everything above? */
      PPD42NS_sensor0.P2.counterValueLowPulseOccupancy += (currentTimerValue - PPD42NS_sensor0.P2.counterValueLowPulseOccupancystart);
    }
    /* Pin change from HIGH -> LOW? Remember counter value for later low occupancy time calculation */
    if (P0_3 == Px_LOW)
    {
      PPD42NS_sensor0.P2.counterValueLowPulseOccupancystart = currentTimerValue;
    }
    /* clear source flag */
    clearInterruptFlag(T1STAT, T1STAT_CH1IF);
  }
#ifdef PPD42NS_SENSOR1CONNECTED
  /* Channel2 -> P0.4 */
  if (checkInterruptFlag(T1STAT, T1STAT_CH2IF) )
  {
    Timer1_readCaptureCompareChannel2(&counterValue);
    currentTimerValue = counterValue.value + PPD42NS_counterValueTotal;
    /* Pin change from LOW -> HIGH? Sum up low occupany time */
    if (P0_4 == Px_HIGH)
    {
      /* @todo: According to datasheet max. low pulse is 90ms. Thus, we should ignore everything above? */
      PPD42NS_sensor1.P1.counterValueLowPulseOccupancy += (currentTimerValue - PPD42NS_sensor1.P1.counterValueLowPulseOccupancystart);
    }
    /* Pin change from HIGH -> LOW? Remember counter value for later low occupancy time calculation */
    if (P0_4 == Px_LOW)
    {
      PPD42NS_sensor1.P1.counterValueLowPulseOccupancystart = currentTimerValue;
    }
    /* clear source flag */
    clearInterruptFlag(T1STAT, T1STAT_CH2IF);
  }
  /* Channel3 -> P0.5 */
  if (checkInterruptFlag(T1STAT, T1STAT_CH3IF) )
  {
    Timer1_readCaptureCompareChannel3(&counterValue);
    currentTimerValue = counterValue.value + PPD42NS_counterValueTotal;
    /* Pin change from LOW -> HIGH? Sum up low occupany time */
    if (P0_5 == Px_HIGH)
    {
      /* @todo: According to datasheet max. low pulse is 90ms. Thus, we should ignore everything above? */
      PPD42NS_sensor1.P2.counterValueLowPulseOccupancy += (currentTimerValue - PPD42NS_sensor1.P2.counterValueLowPulseOccupancystart);
    }
    /* Pin change from HIGH -> LOW? Remember counter value for later low occupancy time calculation */
    if (P0_5 == Px_LOW)
    {
      PPD42NS_sensor1.P2.counterValueLowPulseOccupancystart = currentTimerValue;
    }
    /* clear source flag */
    clearInterruptFlag(T1STAT, T1STAT_CH3IF);
  }
#endif
  /* Overflow must be checked last in order to avoid problems when overflow and input capture happens at the same time */
  if (checkInterruptFlag(T1STAT, T1STAT_OVFIF) )
  {
    PPD42NS_counterValueTotal += 0xffff;
    if (PPD42NS_counterValueTotal > PPD42NS_TIMER1_MAX)
    {
      /* Calculate ratio */
      PPD42NS_sensor0.P1.ratio = (float)PPD42NS_sensor0.P1.counterValueLowPulseOccupancy/PPD42NS_counterValueTotal;
      PPD42NS_sensor0.P2.ratio = (float)PPD42NS_sensor0.P2.counterValueLowPulseOccupancy/PPD42NS_counterValueTotal;
      /* reset values for next measurement */
      PPD42NS_sensor0.P1.counterValueLowPulseOccupancy = 0;
      PPD42NS_sensor0.P2.counterValueLowPulseOccupancy = 0;
      PPD42NS_nextSensorValueAvailable = true;
    }
    clearInterruptFlag(T1STAT, T1STAT_OVFIF);
  } 
  /* Clear interrupt flag, will be directly set again if not all source flags were cleared */
  T1IF = 0;
}

/**
 * Blocking wait until new sensor value is ready.
 */
void PPD42NS_waitForNextSenorValue()
{
  /* Assumption PPD42NS_nextSensorValueAvailable will be writte atomic */
  while(!PPD42NS_nextSensorValueAvailable)
  {
    nop();
  }
  PPD42NS_nextSensorValueAvailable = false;
}

/**
 * Readout function for PPD42NS sensor values. The values are transformed from
 * raw (ration between low occupancy and total length) to physical (particle/m^3).
 * For each sensor (Sensor0, Sensor1) and channel (P1, P2) a separate read-out
 * function is provided.
 * @return particle concentration in particle/m^3
 */
void PPD42NS_readSensor0P1Value(float *value)
{
  disableInterrupt(IEN1, IEN1_T1IE);
  /* @todo transform sensor value to physical value */
  *value = PPD42NS_sensor0.P1.ratio;
  enableInterrupt(IEN1, IEN1_T1IE);
}

/**
 * Readout function for PPD42NS sensor values. The values are transformed from
 * raw (ration between low occupancy and total length) to physical (particle/m^3).
 * For each sensor (Sensor0, Sensor1) and channel (P1, P2) a separate read-out
 * function is provided.
 * @return particle concentration in particle/m^3
 */
void PPD42NS_readSensor0P2Value(float *value)
{
  disableInterrupt(IEN1, IEN1_T1IE);
  /* @todo transform sensor value to physical value */
  *value = PPD42NS_sensor0.P2.ratio;
  enableInterrupt(IEN1, IEN1_T1IE);
}

#ifdef PPD42NS_SENSOR1CONNECTED
/**
 * Readout function for PPD42NS sensor values. The values are transformed from
 * raw (ration between low occupancy and total length) to physical (particle/m^3).
 * For each sensor (Sensor0, Sensor1) and channel (P1, P2) a separate read-out
 * function is provided.
 * @return particle concentration in particle/m^3
 */
float PPD42NS_readSensor1P1Value()
{
  /* @todo transform sensor value to physical value */
  return PPD42NS_sensor1.P1.ratio;
}

/**
 * Readout function for PPD42NS sensor values. The values are transformed from
 * raw (ration between low occupancy and total length) to physical (particle/m^3).
 * For each sensor (Sensor0, Sensor1) and channel (P1, P2) a separate read-out
 * function is provided.
 * @return particle concentration in particle/m^3
 */
float PPD42NS_readSensor1P2Value()
{
  /* @todo transform sensor value to physical value */
  return PPD42NS_sensor1.P2.ratio;
}
#endif