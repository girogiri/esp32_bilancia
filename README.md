# ESP32 Bilancia - Weight Scale Firmware

Firmware per modulo **LilyGo T-Display (ESP32)** per la lettura di celle di carico tramite amplificatore **HX711**.

## Caratteristiche

- Lettura peso in tempo reale a 80 SPS (modalità fast)
- Tara automatica all'avvio
- Visualizzazione su display TFT integrato:
  - Riga superiore: peso attuale in grammi
  - Riga inferiore: peso massimo registrato nella sessione
- Output seriale compatibile con **Arduino Serial Plotter**
- Supporto per celle di carico di diverse portate (configurabile)

## Hardware Richiesto

| Componente | Descrizione |
|------------|-------------|
| LilyGo T-Display | Modulo ESP32 con display TFT ST7789 1.14" |
| HX711 | Amplificatore ADC 24-bit per celle di carico |
| Cella di carico | Default: 10kg (configurabile) |

## Schema di Collegamento

```
HX711          ESP32 T-Display
------         ---------------
VCC     -->    3.3V
GND     -->    GND
DOUT    -->    GPIO 27
SCK     -->    GPIO 26

Cella di carico --> HX711 (E+, E-, A+, A-)
```

### Pinout T-Display utilizzato

| Pin | Funzione |
|-----|----------|
| GPIO 26 | HX711 SCK (Clock) |
| GPIO 27 | HX711 DOUT (Data) |
| GPIO 0 | Pulsante BOOT (Reset Tara) |
| GPIO 35 | Pulsante laterale (Reset Peso Max) |

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
| Pulsante | Funzione |
|----------|----------|
| BOOT (GPIO 0) | Reset Tara (azzera il peso) |
| Laterale (GPIO 35) | Reset Peso Massimo |
| EN/RST | Reset completo del dispositivo |

### Serial Plotter
- Aprire Arduino IDE > Strumenti > Serial Plotter
- Baud rate: **115200**
- Il grafico mostra due linee:
  - **Peso attuale** (g)
  - **Peso massimo** (g)

## Dipendenze (Librerie Arduino)

Installare tramite Library Manager di Arduino IDE:

1. **TFT_eSPI** by Bodmer
   - Dopo l'installazione, configurare per T-Display:
   - Modificare `User_Setup_Select.h` nella cartella della libreria
   - Commentare `#include <User_Setup.h>`
   - Decommentare `#include <User_Setups/Setup25_TTGO_T_Display.h>`

2. **HX711** by bogde
   - Repository: https://github.com/bogde/HX711

## Installazione

1. Clonare questo repository
2. Installare le librerie richieste
3. Aprire `esp32_bilancia.ino` con Arduino IDE
4. Selezionare scheda: **ESP32 Dev Module** o **TTGO T-Display**
5. Configurare il fattore di calibrazione se necessario
6. Caricare il firmware

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
| Display bianco | Verificare configurazione TFT_eSPI |
| Letture instabili | Verificare collegamenti HX711, aumentare `WEIGHT_THRESHOLD` |
| Peso sempre 0 | Controllare cablaggio cella di carico |
| Valori negativi | Eseguire tara con bilancia scarica |

## Licenza

MIT License

## Autore

Progetto generato per uso didattico e hobbistico.
