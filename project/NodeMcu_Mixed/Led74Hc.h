#ifndef LED74HC_H
#define LED74HC_H

#include <Arduino.h>
#include "NodeMcu_Mixed_Defines.h"

/**
 * + > -
 * +: A  B C D E F  G DP
 * -: D1 D2 D3 D4
 * +: 11 7 4 2 1 10 5 3
 * -: 12 9 8 6
 *      A
 *    .____.
 *  F .    . B
 *    .____.
 *  E .  G . C
 *    .____. . DP
 *      D
 */

void Led74Hc_Init(Led74Hc_Type Config);
void Led74Hc_ShowValueAt(uint8_t Value, uint8_t Position, bool DotEnabled);
void Led74Hc_ShowValue(uint32_t Value, uint8_t DigitCount, uint8_t DotPosition);
#endif /* LED74HC_H */
