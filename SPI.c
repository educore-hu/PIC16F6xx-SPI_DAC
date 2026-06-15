/*
 * File:        SPI.c
 * Author:      Educore
 * Target:      PIC16F690
 * Compiler:    XC8
 * Description: SPI (Serial Peripheral Interface) driver master módban.
 *              A chip select (CS) lábak kezelése kiszervezésre került a 
 *              felhasználói rétegbe (main.c) a rugalmasság érdekében.
 * 
 * Created on 2026
 * Revision history: 2.0 (Clean hardware layer without fixed CS)
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
#include <xc.h>
#include "SPI.h"

/**
 * @brief Inicializálja az MSSP modult SPI Master módba.
 */
void SPI_Init(void)
{
        
    // 1. Teljes leállítás és konfigurációs regiszterek lenullázása
    SSPCON = 0x00;   // SSPEN = 0, és az összes SSPM bit kényszerített NULLA!
    SSPSTAT = 0x00;

    // 2. Analóg funkciók letiltása (Hogy az RB6 és RB4 digitális legyen)
    ANSELHbits.ANS10 = 0; // RB4 digitális bemenet (MISO / SDI)
    ANSELHbits.ANS11 = 0; // RB6 digitális kimenet (SCK)

    // 3. I/O irányok beállítása a fix hardveres SPI lábakhoz
    TRISBbits.TRISB6 = 0; // SCK (RB6) -> Kimenet (Órajel generátor)
    TRISCbits.TRISC7 = 0; // SDO (RC7) -> Kimenet (MOSI)
    TRISBbits.TRISB4 = 1; // SDI (RB4) -> Bemenet (MISO)

    // 4. SPI Id?zítések (Mode 0: Közép-mintavétel, aktívból passzívba váltó él)
    SSPSTATbits.SMP = 0;  // Mintavétel az adat-id?tartam közepén
    SSPSTATbits.CKE = 1;  // Aktívból passzívba váltáskor ad le jelet
    
    // 5. Órajel és Mód beállítása (Master mód, Fosc/4)
    SSPCONbits.SSPM = 0b0000; // Master mode, Fosc/4
    SSPCONbits.SSPEN = 1;     // MSSP modul engedélyezése SPI módban
}

/**
 * @brief Egyetlen bájt küldése és fogadása az SPI buszon.
 * @note A hívás el?tt a CS lábat a main-ben alacsonyra kell húzni!
 * @param data: A kiküldend? 8-bites adat.
 * @return uint8_t: Az átvitel során fogadott 8-bites adat.
 */
uint8_t SPI_Transfer(uint8_t data)
{
    // Hibajelek törlése (túlcsordulás és ütközés védelem)
    SSPCONbits.SSPOV = 0;
    SSPCONbits.WCOL  = 0;

    // Adat beírása a pufferbe -> elindítja a hardveres órajelet és küldést
    SSPBUF = data;        

    // Várakozás, amíg az adatátvitel teljesen befejez?dik (BF flag 1 lesz)
    while(!SSPSTATbits.BF);  

    // A fogadott bájt visszaadása (Kiolvasása kötelez?, ez törli a BF flag-et)
    return SSPBUF;                  
}
