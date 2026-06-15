/*
 * ============================================================================
 * File        : main.c
 * Author      : Educore
 * Target MCU  : PIC16F690
 * Compiler    : XC8
 * IDE         : MPLAB X IDE 5.00
 *
 * Description :
 * Driver test for MCP4821 SPI DAC
 *
 * Tested with:
 * - MPLAB X IDE v5.00
 * - XC8 Compiler
 * - PIC16F690
 * ============================================================================
 */

/*
 * ============================================================================
 * PIC16F690 - DIP20 PINOUT
 * ============================================================================
 *
 *                  +----\/----+
 *   VDD        --- |1      20| --- VSS
 *   RA5/T1CKI  --- |2      19| --- RA0/AN0
 *   RA4/AN3    --- |3      18| --- RA1/AN1
 *   RA3/MCLR   --- |4      17| --- RA2/AN2/VREF
 *   RC5        --- |5      16| --- RC0
 *   RC4        --- |6      15| --- RC1
 *   RC3        --- |7      14| --- RC2
 *   RC6/TX     --- |8      13| --- RB4
 *   RC7/RX     --- |9      12| --- RB5
 *   RB7        --- |10     11| --- RB6/ICSPCLK
 *                  +----------+
 *
 * ============================================================================
 */

// CONFIG
#pragma config FOSC = INTRCIO   // Internal oscillator, I/O function on RA4 and RA5
//#pragma config FOSC = HS        // Oscillator Selection bits (HS oscillator: High-speed crystal/resonator on RA4/OSC2/CLKOUT and RA5/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer disabled
#pragma config PWRTE = OFF      // Power-up Timer disabled
#pragma config MCLRE = ON       // MCLR pin function is MCLR
#pragma config CP = OFF         // Code Protection disabled
#pragma config CPD = OFF        // Data Code Protection disabled
#pragma config BOREN = ON       // Brown-out Reset enabled
#pragma config IESO = OFF       // Internal External Switchover disabled
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor disabled

#define _XTAL_FREQ 4000000UL

#include <xc.h>
#include "SPI.h"
#include "MCP48XX_DAC.h"

// Tetsz?leges lábakat választhatsz a main-ben!
#define DAC_CS_TRIS_BIT   TRISCbits.TRISC6
#define DAC_CS_Lab   PORTCbits.RC6

#define DAC_LDAC_TRIS_BIT TRISCbits.TRISC5
#define DAC_LDAC_Lab PORTCbits.RC5

void main(void) {
    // Lábak konfigurálása kimenetnek
    DAC_CS_TRIS_BIT = 0;
    DAC_LDAC_TRIS_BIT = 0;
    
    // Alaphelyzet (Inaktív szintek)
    DAC_CS_Lab = 1;
    DAC_LDAC_Lab = 1;
    
    SPI_Init(); // SPI hardver indítása
// Vezérlő bit makrók a main.c vagy a header tetejére
//#define DAC_CH_A      0x00  // Bit 7 = 0
//#define DAC_CH_B      0x80  // Bit 7 = 1
//#define DAC_GAIN_1X   0x20  // Bit 5 = 1
//#define DAC_GAIN_2X   0x00  // Bit 5 = 0
//#define DAC_ACTIVE    0x10  // Bit 4 = 1
//#define DAC_SHUTDOWN  0x00  // Bit 4 = 0
//6. bit NEM kell, mert az Don't care bit.
    
while(1) {
        // Példa: Küldjünk jelet a 12-bites MCP4821 'A' csatornájára, 1x-es Gain-nel
        //Az MCP4821-nek csak 1 csatornája van, ezért azt valójában figyelmen kívül hagyja.
        //A driver fel van készítve 2ch-re is. MCP48v2-re, a driver ugyan MCP4821_Dac_Write
        //függvény nevekkel van ellátva, de alkalmas 2CH-re is. A vezérl? bitek változnak.
        DAC_CS_Lab = 0;

        
        //Videóhoz 1365 jobb
        // 1. Létrehozunk egy tiszta bináris változót a vezérlésnek
        unsigned char Dac_Beallitas = 0b00110000; 

        // 2. Meghívjuk a függvényt (1365 az adat, ami kb. a feszültség harmada)
        //1365 binárisan szebben mutat.
        //MCP4821_Dac_Write(1365, Dac_Beallitas);
        MCP4821_Dac_Write_videohoz(1365, Dac_Beallitas);
        //Vezérl? makrókkal:
        //MCP4821_Dac_Write(4095, DAC_CH_A | DAC_GAIN_1X | DAC_ACTIVE);
        
        DAC_CS_Lab = 1;
        
        // Pulzus az LDAC-ra a kimenet frissítéséhez
        DAC_LDAC_Lab = 0;
        __delay_us(2); 
        DAC_LDAC_Lab = 1;

        __delay_us(10);

    }
}
