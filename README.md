# ESP32 Bilancia - Weight Scale Firmware

Firmware per modulo **LilyGo T-Display (ESP32)** per la lettura di celle di carico tramite amplificatore **HX711**.

## Caratteristiche

- Lettura peso in tempo reale a 80 SPS (modalità fast)
- Tara automatica all'avvio
- Visualizzazione su display TFT integrato 1.14":
  - Riga superiore: peso attuale in grammi
  - Riga inferiore: peso massimo registrato nella sessione
- Output seriale compatibile con **Arduino Serial Plotter**
- Supporto per celle di carico di diverse portate (configurabile)

## Hardware Richiesto

| Componente | Descrizione |
|------------|-------------|
| LilyGo T-Display | ESP32 con display TFT ST7789 1.14" (135x240) |
| HX711 | Amplificatore ADC 24-bit per celle di carico |
| Cella di carico | Default: 10kg (configurabile) |

## Schema di Collegamento

```
HX711          T-Display
------         ---------
VCC     -->    3.3V
GND     -->    GND
DOUT    -->    GPIO 27
SCK     -->    GPIO 26

Cella di carico --> HX711 (E+, E-, A+, A-)
```

### Pinout T-Display utilizzato

| Pin | Funzione |
|-----|----------|
| GPIO 27 | HX711 DOUT (Data) |
| GPIO 26 | HX711 SCK (Clock) |
| GPIO 0 | Pulsante superiore - BOOT (Reset Tara) |
| GPIO 35 | Pulsante inferiore (Reset Peso Max) |

### Pin disponibili per espansioni

GPIO liberi: 2, 12, 13, 15, 17, 21, 22, 25, 32, 33

**Pin da evitare** (usati dal display SPI):
- GPIO 5 (CS), GPIO 16 (DC), GPIO 23 (RST)
- GPIO 18 (SCLK), GPIO 19 (MOSI)
- GPIO 4 (Backlight)

## Configurazione

Nel file `esp32_bilancia.ino` puoi modificare le seguenti costanti:

```cpp
// Portata cella di carico in grammi
#define LOAD_CELL_CAPACITY 10000  // 10kg = 10000g

// Fattore di calibrazione (da calibrare con peso noto)
#define CALIBRATION_FACTOR 420.0

// Soglia minima per registrare peso (evita rumore)
#define WEIGHT_THRESHOLD 5.0  // grammi
```

## Calibrazione

1. Caricare il firmware con `CALIBRATION_FACTOR` iniziale (es. 420.0)
2. Posizionare un peso noto sulla bilancia (es. 1000g)
3. Leggere il valore visualizzato
4. Calcolare il nuovo fattore: `nuovo_fattore = vecchio_fattore * (peso_letto / peso_reale)`
5. Aggiornare `CALIBRATION_FACTOR` e ricaricare il firmware

## Utilizzo

### Avvio
- All'accensione, il firmware esegue automaticamente la tara
- Assicurarsi che la bilancia sia scarica durante l'avvio

### Pulsanti
| Pulsante | Posizione | Funzione |
|----------|-----------|----------|
| BOOT (GPIO 0) | Superiore | Reset Tara (azzera il peso) |
| GPIO 35 | Inferiore | Reset Peso Massimo |
| EN/RST | Laterale | Reset completo del dispositivo |

### Serial Plotter
- Aprire Arduino IDE > Strumenti > Serial Plotter
- Baud rate: **115200**
- Il grafico mostra due linee:
  - **Peso attuale** (g)
  - **Peso massimo** (g)

## Dipendenze (Librerie Arduino)

### 1. TFT_eSPI by Bodmer

Installare dal Library Manager, poi configurare:

1. Aprire il file `User_Setup_Select.h` nella cartella della libreria:
   ```
   C:\Users\<utente>\Documents\Arduino\libraries\TFT_eSPI\User_Setup_Select.h
   ```

2. Commentare la riga (aggiungere `//` all'inizio):
   ```cpp
   //#include <User_Setup.h>
   ```

3. Decommentare la riga (rimuovere `//`):
   ```cpp
   #include <User_Setups/Setup25_TTGO_T_Display.h>
   ```

4. Salvare e chiudere il file

### 2. HX711 Arduino Library by Bogdan Necula

- Nel Library Manager cerca: `HX711`
- Seleziona: "HX711 Arduino Library" by Bogdan Necula
- Repository: https://github.com/bogde/HX711

## Installazione Arduino IDE

### Configurazione Board Manager

1. Aprire Arduino IDE > File > Preferences
2. Aggiungere URL Board Manager:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
3. Strumenti > Board > Board Manager
4. Cercare "esp32" e installare (versione 2.x consigliata)

### Selezione Scheda

- Board: **ESP32 Dev Module**
- Upload Speed: **921600**
- Flash Frequency: **80MHz**
- Flash Size: **4MB**
- Partition Scheme: **Default 4MB with spiffs**

## Formato Output Seriale

L'output seriale è formattato per Arduino Serial Plotter:

```
peso_attuale    peso_max
123.5           150.2
124.1           150.2
...
```

I valori sono separati da tabulazione, compatibili con il Serial Plotter.

## Troubleshooting

| Problema | Soluzione |
|----------|-----------|
| Display bianco/nero | Verificare Setup25 in TFT_eSPI |
| Letture instabili | Verificare collegamenti HX711, aumentare `WEIGHT_THRESHOLD` |
| Peso sempre 0 | Controllare cablaggio cella di carico |
| Valori negativi | Eseguire tara con bilancia scarica |
| Errore upload | Tenere premuto BOOT durante upload |

## Link Utili

- [Repository LilyGo TTGO-T-Display](https://github.com/Xinyuan-LilyGO/TTGO-T-Display)
- [Libreria HX711](https://github.com/bogde/HX711)
- [TFT_eSPI Library](https://github.com/Bodmer/TFT_eSPI)

## Licenza

MIT License
