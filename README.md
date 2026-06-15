# PIC16F690 SPI Master & MCP48XX DAC Driver

Ez a projekt bemutatja, hogyan konfigurálható a **PIC16F690** mikrokontroller beépített MSSP modulja **SPI Master** módba, valamint hogyan vezérelhető vele egy **Microchip MCP48XX** szériás (MCP4801/4811/4821) digitális-analóg átalakító (DAC).

A projekt az **Educore** YouTube csatorna oktatóvideóihoz készült.

---

## 🚀 Főbb jellemzők

* **Tiszta hardveres réteg:** Az SPI driver teljesen független a perifériáktól.

* **Rugalmas Chip Select (CS) kezelés:** A hardveres lábak vezérlése szándékosan a felhasználói rétegbe (`main.c`) került, így a driver több SPI eszköz esetén is könnyen újrahasznosítható.

* **Moduláris felépítés:** Különálló, részletesen kommentált driverek az SPI és a DAC kezeléséhez.

* **Oktatásközpontú kód:** Tartalmaz egy hagyományos, bitmaszkolós függvényt és egy lépésről lépésre felépített, bájtonkénti verziót is a könnyebb megértés érdekében.

---

## 📌 Hardver architektúra & Bekötés

### PIC16F690 DIP20 Pinout

```text
                  +----\/----+
            VDD --- |1      20| --- VSS
      RA5/T1CKI --- |2      19| --- RA0/AN0
        RA4/AN3 --- |3      18| --- RA1/AN1
       RA3/MCLR --- |4      17| --- RA2/AN2/VREF
            RC5 --- |5      16| --- RC0
            RC4 --- |6      15| --- RC1
            RC3 --- |7      14| --- RC2
         RC6/TX --- |8      13| --- RB4
         RC7/RX --- |9      12| --- RB5
            RB7 --- |10     11| --- RB6/ICSPCLK
                  +----------+

[PIC16F690]              [MCP4821 DAC]          [Megjegyzés]
RB6 (SCK)       ---->    SCK                    Fix hardveres SPI órajel
RC7 (SDO)       ---->    SDI                    Fix hardveres adat kimenet
RB4 (SDI)       ---->    (Szabadon marad)       A DAC nem küld vissza adatot
RC6 (I/O)       ---->    CS (Chip Select)       Szoftveres vezérlés (main.c)
RC5 (I/O)       ---->    LDAC (Latch DAC)       Kimenet frissítés ütemezése

💻 Szoftver struktúra
A projekt három fő modulból áll:

  1 SPI.c / SPI.h: Az MSSP modul inicializálása (Mode 0, Fosc/4) és a 8-bites adatcsere (SPI_Transfer).

  2 MCP48XX_DAC.c / MCP48XX_DAC.h: A 8, 10 és 12 bites Microchip DAC-ok meghajtója. Elvégzi a konfigurációs bitek és az adatok 16 bites csomaggá fűzését.

  3 main.c: A hardveres lábak konfigurációja, az inicializálás, és a DAC folyamatos frissítése egy példaértékkel (1365).

⚙️ Vezérlőbájt (Config Byte) felépítése
Az MCP48XX család egy 16 bites SPI keretet vár, melynek felső 4 bitje a működési módot határozza meg:

Bit 7 (A/B): Csatorna választás (0 = 'A' csatorna, 1 = 'B' csatorna).

Bit 6 (BUF): Bemeneti pufferelés (0 = nincs pufferelve).

Bit 5 (GA): Erősítés (1 = 1x Gain [VREF = 2.048V], 0 = 2x Gain [VREF = 4.096V]). Megjegyzés: 3.3V-os tápfeszültség mellett a 2x Gain fizikailag nem érhető el!

Bit 4 (SHDN): Output Shutdown (1 = Aktív mód, 0 = Lekapcsolt kimenet).

⚠️ Fontos a videós függvényhez (MCP4821_Dac_Write_videohoz):
Ez a demonstrációs függvény feltételezi, hogy a main.c-ből érkező Vezerles paraméter alsó 4 bitje szigorúan 0 (pl. 0b00110000), így az adatok bájtonkénti összefésülése során a bitek nem csúsznak egymásra, és a logikai analizátoron is tökéletes jelalak látható.

🛠️ Fejlesztői környezet
IDE: MPLAB X IDE (v5.00 vagy újabb)

Fordító: XC8 (v2.00 vagy újabb)

Tesztelt hardver: PIC16F690 (Internal OSC @ 4MHz), MCP4821 (12-bit DAC)

📺 Kapcsolódó videók
Ha szeretnéd látni a kód részletes magyarázatát és a logikai analizátoros méréseket lépésről lépésre, nézd meg a YouTube csatornánkat:

https://www.youtube.com/@educore_videok

