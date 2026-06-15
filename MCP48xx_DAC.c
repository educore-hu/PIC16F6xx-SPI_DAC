/*
 * File:        MCP48XX_DAC.c
 * Author:      Educore
 * Target:      PIC16F690 / MCP4801 (8-bit), MCP4811 (10-bit), MCP4821 (12-bit)
 * Description: DAC meghajtó programcsomag Microchip MCP48XX szériához.
 * A hardveres lábak (CS, LDAC) kezelését a main.c végzi!
 * * Created on 2026
 * Note:        Oktatási célra kiegészített, részletesen dokumentált verzió.
 */

#include <xc.h>
#include "SPI.h"
#include "MCP48XX_DAC.h"

/* * Az MCP48XX vezérlő bitek felépítése a felső byte-on:
 * Bit 7 (A/B): 0 = 'A' csatorna, 1 = 'B' csatorna (Az MCP48X1 szériánál mindig 0)
 * Bit 6 (BUF): Don't care -> 0
 * Bit 5 (GA):  1 = 1x Gain (VREF = 2.048V), 0 = 2x Gain (4.096V)
 * Bit 4 (SHDN):1 = Aktív működés, 0 = Leállítás (Shutdown mód)
 * * Mivel a mikrokontrollerünk 3.3V-ról jár, a 2x Gain (4.096V) fizikailag nem érhető el. 
 * Ezért az alapértelmezett konfigurációs maszk binárisan: 0b00110000 -> Hexadecimálisan: 0x30
 */
/*
 ==============================================================================
MCP48XX VEZÉRLŐBÁJT (CONFIG BYTE) SZERKEZETE
==============================================================================
A függvények második paramétere egy 8-bites konfigurációs bájt (Vezérlés), 
amely meghatározza a DAC működési módját. 
A main.c-ben az alábbi maszkok kombinációjával (bitenkénti VAGY kapcsolással) 
lehet összeállítani a kívánt működést:

Bit 7 - Csatorna választás (A/B)
        0 = 'A' Csatorna (Default) -> (Pl. MCP48X2_CH_A)
        1 = 'B' Csatorna           -> (Pl. MCP48X2_CH_B)

Bit 6 - Belső pufferelés (Buffer)
        0 = Nincs pufferelve (Alapértelmezett, stabilabb alacsony tápnál)
        1 = Pufferelt bemenet

Bit 5 - Erősítési tényező (Gain)
        1 = 1x Erősítés (VREF = 2.048V kimeneti maximum) -> (MCP48XX_GAIN_1X)
        0 = 2x Erősítés (VREF = 4.096V kimeneti maximum - csak 5V-os tápnál!)

Bit 4 - Leállítás / Aktív mód (Shutdown)
        1 = Aktív működés (Kimenet engedélyezve) -> (MCP48XX_ACTIVE)
        0 = Shutdown mód (Kimenet nagyimpedanciás, lekapcsolva)

Példa 3.3V-os rendszerhez, 'A' csatornára: (0 << 7) | (0 << 6) | (1 << 5) | (1 << 4) = 0x30
Példa 3.3V-os rendszerhez, 'B' csatornára: (1 << 7) | (0 << 6) | (1 << 5) | (1 << 4) = 0xB0
==============================================================================
 
 */

/**
 * @brief MCP4801 / MCP4802 (8-bites DAC) érték kiküldése tetszőleges konfigurációval.
 * @param Adat:     8-bites analóg érték (0 - 255)
 * @param Vezerles: 8-bites konfigurációs bájt (Csatorna, Gain, Puffer, Shutdown)
 */
void MCP4801_Dac_Write(unsigned char Adat, unsigned char Vezerles)
{
    unsigned int Kuldesre = 0;
    
    // --- ADAT FELKÉSZÍTÉSE ---
    unsigned int IdeiglenesAdat = (unsigned int)Adat << 4;
    
    // --- ÖSSZEILLESZTÉS A MAIN-BŐL ÉRKEZŐ VEZÉRLÉSSEL ---
    // Mivel a 'Vezerles' egy 8-bites paraméter, a C fordító alapból 8 biten tárolja.
    // Ahhoz, hogy a 16-bites csomag felső bájára kerüljön, kötelező az (unsigned int)
    // típuskonverzió (casting), majd a 8 bittel balra tolás (<< 8).
    //
    // Látványterv a bitenkénti VAGY (|) műveletről:
    // ((unsigned int)Vezerles << 8)  -->   [VEZÉRLÉS BÁJT] [0000 0000]
    // | IdeiglenesAdat                -->   [0000 0000]     [8-BIT ADAT + 4x0]
    // -------------------------------------------------------------------------
    // Kuldesre                        =     [VEZÉRLÉS BÁJT] [8-BIT ADAT + 4x0]
    
    Kuldesre = ((unsigned int)Vezerles << 8) | IdeiglenesAdat;
    
    // --- HARDVERES SPI ÁTVITEL ---
    SPI_Transfer((uint8_t)(Kuldesre >> 8)); // Felső byte (D15 - D8)
    SPI_Transfer((uint8_t)(Kuldesre));      // Alsó byte  (D7 - D0)
}

/**
 * @brief MCP4811 / MCP4812 (10-bites DAC) érték kiküldése tetszőleges konfigurációval.
 * @param Adat:     10-bites analóg érték (0 - 1023)
 * @param Vezerles: 8-bites konfigurációs bájt (Csatorna, Gain, Puffer, Shutdown)
 */
void MCP4811_Dac_Write(unsigned int Adat, unsigned char Vezerles)
{
    unsigned int Kuldesre = 0;
    
    // Biztonsági szoftveres maszk (max 10 bit), nehogy felülírja a vezérlést!
    Adat &= 0x03FF; 
    
    // A 10-bites adatot a 12-bites mező felső részére (D11-D2) kell shiftelni.
    unsigned int IdeiglenesAdat = Adat << 2;
    
    // Összeillesztés a 8 bitről 16 bitre bővített és feltolt vezérlőbájttal
    Kuldesre = ((unsigned int)Vezerles << 8) | IdeiglenesAdat;
    
    // Hardveres küldés
    SPI_Transfer((uint8_t)(Kuldesre >> 8));
    SPI_Transfer((uint8_t)(Kuldesre));
}

/**
 * @brief MCP4821 / MCP4822 (12-bites DAC) érték kiküldése tetszőleges konfigurációval.
 * @param Adat:     12-bites analóg érték (0 - 4095)
 * @param Vezerles: 8-bites konfigurációs bájt (Csatorna, Gain, Puffer, Shutdown)
 */
void MCP4821_Dac_Write(unsigned int Adat, unsigned char Vezerles)
{
    unsigned int Kuldesre = 0;
    
    // Biztonsági maszk: Levágjuk a 12 bit feletti esetleges szemetet
    Adat &= 0x0FFF;
    
    // Egy lépésben: A vezérlés bitjeit a helyére toljuk (D15-D12),
    // a 12-bites adatot pedig változtatás nélkül rásimítjuk az alsó bitekre.
    Kuldesre = ((unsigned int)(Vezerles & 0xF0) << 8) | Adat;
    
    // Hardveres küldés egymás után (MSB - felső bájt először)
    SPI_Transfer((uint8_t)(Kuldesre >> 8)); // Felső byte (D15 - D8)
    SPI_Transfer((uint8_t)(Kuldesre));      // Alsó byte  (D7 - D0)
}

/**
 * @brief Didaktikai célú 12-bites DAC írás bájtonkénti bontásban (Videós verzió).
 * @note  FIGYELEM: Ez a függvény feltételezi, hogy a 'Vezerles' paraméter 
 * alsó 4 bitje mindig 0 (pl. 0b00110000), ahogy a videóban meg van határozva!
 * Mivel mindkét bájtdarabka üresen hagyja a másik helyét, a VAGY (|) 
 * kapcsolattal tökéletesen összefésülhetők.
 * @param Adat:     12-bites analóg érték (0 - 4095)
 * @param Vezerles: 8-bites konfigurációs bájt (Csatorna, Gain, Puffer, Shutdown)
 */
void MCP4821_Dac_Write_videohoz(unsigned int Adat, unsigned char Vezerles)
{
    unsigned char kuldes_high = 0;
    unsigned char kuldes_low  = 0;
    
    // Biztonsági maszk (max 12 bit)
    Adat &= 0x0FFF;
    
    // 1. lépés: Beletesszük a vezérlést (A main-ben megadott 0b00110000 már a helyén van)
    kuldes_high = Vezerles;

    // 2. lépés: Mellécsapjuk az adat maradék felső 4 bitjét (D11-D8) az alsó helyekre (0-3 bit)
    kuldes_high |= (unsigned char)(Adat >> 8);
    
    // 3. lépés: Az alsó (Low) bájt elkészítése (Az Adat alsó 8 bitje)
    kuldes_low = (unsigned char)Adat;
    
    // Hardveres küldés egymás után
    SPI_Transfer(kuldes_high);
    SPI_Transfer(kuldes_low);
}
