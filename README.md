# ESP32 Bilancia - Weight Scale Firmware

Firmware per modulo **LilyGo T-Display S3 (ESP32-S3)** per la lettura di celle di carico tramite amplificatore **HX711**.

## Caratteristiche

- Lettura peso in tempo reale a 80 SPS (modalità fast)
- Tara automatica all'avvio
- Visualizzazione su display TFT integrato 1.9":
  - Riga superiore: peso attuale in grammi
  - Riga inferiore: peso massimo registrato nella sessione
- Output seriale compatibile con **Arduino Serial Plotter**
- Supporto per celle di carico di diverse portate (configurabile)

## Hardware Richiesto

| Componente | Descrizione |
|------------|-------------|
| LilyGo T-Display S3 | ESP32-S3 con display TFT ST7789 1.9" (170x320) |
| HX711 | Amplificatore ADC 24-bit per celle di carico |
| Cella di carico | Default: 10kg (configurabile) |

## Schema di Collegamento

```
HX711          T-Display S3
------         -------------
VCC     -->    3.3V
GND     -->    GND
DOUT    -->    GPIO 16
SCK     -->    GPIO 17

Cella di carico --> HX711 (E+, E-, A+, A-)
```

### Pinout T-Display S3 utilizzato

| Pin | Funzione |
|-----|----------|
| GPIO 16 | HX711 DOUT (Data) |
| GPIO 17 | HX711 SCK (Clock) |
| GPIO 0 | Pulsante BOOT (Reset Tara) |
| GPIO 14 | Pulsante KEY (Reset Peso Max) |
| GPIO 15 | Display Power (automatico) |

### Pin disponibili per espansioni

GPIO liberi: 1, 2, 3, 10, 11, 12, 13, 18, 21, 43, 44

**Pin da evitare** (usati dal display):
- GPIO 5, 6, 7, 8, 9 (controllo display)
- GPIO 38 (backlight)
- GPIO 39, 40, 41, 42, 45, 46, 47, 48 (bus dati display)

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
| BOOT (GPIO 0) | Sopra il display | Reset Tara (azzera il peso) |
| KEY (GPIO 14) | Sopra il display | Reset Peso Massimo |
| RST | Laterale | Reset completo del dispositivo |

### Serial Plotter
- Aprire Arduino IDE > Strumenti > Serial Plotter
- Baud rate: **115200**
- Il grafico mostra due linee:
  - **Peso attuale** (g)
  - **Peso massimo** (g)

## Dipendenze (Librerie Arduino)

### 1. TFT_eSPI by Bodmer

**IMPORTANTE per T-Display S3:**

La versione standard di TFT_eSPI richiede configurazione manuale.

**Opzione A - Configurazione manuale:**
1. Installare TFT_eSPI dal Library Manager
2. Aprire il file `User_Setup_Select.h` nella cartella della libreria
3. Commentare: `#include <User_Setup.h>`
4. Decommentare: `#include <User_Setups/Setup206_LilyGo_T_Display_S3.h>`

**Opzione B - Usare la libreria LilyGo (consigliato):**
1. Scaricare la libreria TFT_eSPI dal [repository LilyGo T-Display-S3](https://github.com/Xinyuan-LilyGO/T-Display-S3)
2. Sostituire la cartella TFT_eSPI nelle librerie Arduino
3. NON aggiornare la libreria quando Arduino IDE lo propone

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
4. Cercare "esp32" e installare **versione 2.0.14** o **2.0.17**

   **NOTA:** Versioni 3.x possono causare errori di compilazione con T-Display S3

### Selezione Scheda

- Board: **ESP32S3 Dev Module**
- USB CDC On Boot: **Enabled**
- Flash Size: **16MB**
- Partition Scheme: **Default 4MB with spiffs** (o 16MB se disponibile)
- PSRAM: **OPI PSRAM**

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
| Display nero/bianco | Verificare Setup206 in TFT_eSPI, usare libreria LilyGo |
| Errore GPIO compilazione | Usare ESP32 board package versione 2.0.14-2.0.17 |
| Letture instabili | Verificare collegamenti HX711, aumentare `WEIGHT_THRESHOLD` |
| Peso sempre 0 | Controllare cablaggio cella di carico |
| Valori negativi | Eseguire tara con bilancia scarica |
| Serial non funziona | Abilitare "USB CDC On Boot" nelle impostazioni board |

## Link Utili

- [Repository LilyGo T-Display-S3](https://github.com/Xinyuan-LilyGO/T-Display-S3)
- [Libreria HX711](https://github.com/bogde/HX711)
- [TFT_eSPI Library](https://github.com/Bodmer/TFT_eSPI)

## Licenza

MIT License

## Autore

Progetto generato per uso didattico e hobbistico.
