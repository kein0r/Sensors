/** @ingroup PPD42NS
 * @{
 */
#ifndef PPD42NS_H_
#define PPD42NS_H_
/*
  Copyright (c) 2015 Jan Rüdiger.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

  Credits:
*/
/*******************| Inclusions |*************************************/
#include <PlatformTypes.h>
   
/*******************| Macros |*****************************************/

/*******************| Type definitions |*******************************/
typedef struct {
  uint8_t t;
} PPD42NS_Config_t;

typedef struct {
  uint32_t counterValueLowPulseOccupancyP1start;
  uint32_t counterValueLowPulseOccupancyP1;             /**< Low occupancy time for P1 (1um) */        
  uint32_t counterValueLowPulseOccupancyP2start;
  uint32_t counterValueLowPulseOccupancyP2;             /**< Low occupancy time for P2 (2,5um) */
} PPD42DN_CounterValues_t;

/*******************| Type definitions |*******************************/

/*******************| Global variables |*******************************/

/*******************| Function prototypes |****************************/
void PPD42NS_init(PPD42NS_Config_t *config);

#endif
/** @}*/