
/* 
 * File:   SPI
 * Author: Educore
 *
 * Created on 2026
 * Revision history: 1.0
 */
#ifndef SPI_H
#define SPI_H

#include <stdint.h>

void SPI_Init(void);
uint8_t SPI_Transfer(uint8_t data);

#endif
