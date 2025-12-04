/*
 * ESP32 Bilancia - Weight Scale Firmware
 *
 * Firmware per LilyGo T-Display S3 (ESP32-S3) con cella di carico HX711
 *
 * Hardware: T-Display S3 ESP32-S3 1.9" ST7789 170x320
 *
 * Caratteristiche:
 * - Lettura peso in tempo reale a 80 SPS
 * - Tara automatica all'avvio
 * - Display: peso attuale e peso massimo
 * - Output seriale per Arduino Serial Plotter
 *
 * Librerie richieste:
 * - TFT_eSPI (configurata per T-Display S3 - Setup206)
 * - HX711 Arduino Library by Bogdan Necula
 */

#include <TFT_eSPI.h>
#include <HX711.h>

// =============================================================================
// CONFIGURAZIONE PIN HX711
// =============================================================================
// Pin disponibili su T-Display S3 (non usati da display/pulsanti):
// GPIO 1, 2, 3, 10, 11, 12, 13, 16, 17, 18, 21
#define HX711_DOUT_PIN  16    // Data pin
#define HX711_SCK_PIN   17    // Clock pin

// =============================================================================
// CONFIGURAZIONE PULSANTI T-DISPLAY S3
// =============================================================================
// I due pulsanti programmabili sulla T-Display S3
#define BUTTON_TARE     0     // GPIO 0 - Pulsante BOOT (reset tara)
#define BUTTON_RESET    14    // GPIO 14 - Pulsante KEY (reset max)

// =============================================================================
// CONFIGURAZIONE CELLA DI CARICO
// =============================================================================
// Portata massima della cella in grammi
// Valori comuni: 1000 (1kg), 5000 (5kg), 10000 (10kg), 20000 (20kg)
#define LOAD_CELL_CAPACITY  10000   // 10 kg

// Fattore di calibrazione - DA CALIBRARE CON PESO NOTO
// Procedura: posizionare peso noto, calcolare:
// nuovo_fattore = vecchio_fattore * (peso_letto / peso_reale)
#define CALIBRATION_FACTOR  420.0

// Soglia minima per considerare un peso valido (evita rumore)
#define WEIGHT_THRESHOLD    5.0     // grammi

// =============================================================================
// CONFIGURAZIONE DISPLAY T-DISPLAY S3
// =============================================================================
// Display: 1.9" ST7789 170x320 pixel (8-bit parallel interface)
#define SCREEN_WIDTH    320
#define SCREEN_HEIGHT   170

#define TFT_BG_COLOR    TFT_BLACK
#define TFT_TEXT_COLOR  TFT_WHITE
#define TFT_MAX_COLOR   TFT_YELLOW
#define TFT_LABEL_COLOR TFT_CYAN

// Pin power display (necessario per alimentazione da batteria)
#define PIN_POWER_ON    15

// =============================================================================
// CONFIGURAZIONE SERIALE
// =============================================================================
#define SERIAL_BAUD     115200

// =============================================================================
// VARIABILI GLOBALI
// =============================================================================
TFT_eSPI tft = TFT_eSPI();
HX711 scale;

float currentWeight = 0.0;      // Peso attuale in grammi
float maxWeight = 0.0;          // Peso massimo registrato
float lastDisplayedWeight = -1; // Per evitare refresh inutili
float lastDisplayedMax = -1;

unsigned long lastReadTime = 0;
const unsigned long READ_INTERVAL = 12;  // ~80 SPS (1000ms / 80 = 12.5ms)

// Debounce pulsanti
unsigned long lastButtonTare = 0;
unsigned long lastButtonReset = 0;
const unsigned long DEBOUNCE_DELAY = 250;

// =============================================================================
// PROTOTIPI FUNZIONI
// =============================================================================
void initDisplay();
void initScale();
void performTare();
void readWeight();
void updateDisplay();
void sendSerialData();
void handleButtons();
void resetMaxWeight();

// =============================================================================
// SETUP
// =============================================================================
void setup() {
  // Abilita alimentazione display (importante per uso con batteria)
  pinMode(PIN_POWER_ON, OUTPUT);
  digitalWrite(PIN_POWER_ON, HIGH);

  // Inizializza seriale
  Serial.begin(SERIAL_BAUD);
  delay(100);

  Serial.println("ESP32-S3 Bilancia - Avvio...");
  Serial.println("Peso(g)\tMax(g)");  // Header per Serial Plotter

  // Configura pulsanti (active LOW con pull-up interno)
  pinMode(BUTTON_TARE, INPUT_PULLUP);
  pinMode(BUTTON_RESET, INPUT_PULLUP);

  // Inizializza display
  initDisplay();

  // Mostra messaggio di avvio
  tft.fillScreen(TFT_BG_COLOR);
  tft.setTextColor(TFT_LABEL_COLOR, TFT_BG_COLOR);
  tft.setTextSize(2);
  tft.setCursor(60, 60);
  tft.println("Inizializzazione...");

  // Inizializza bilancia
  initScale();

  // Esegui tara automatica
  tft.setCursor(60, 90);
  tft.println("Tara in corso...");
  performTare();

  // Pulisci display per la visualizzazione principale
  tft.fillScreen(TFT_BG_COLOR);

  Serial.println("Sistema pronto!");
}

// =============================================================================
// LOOP PRINCIPALE
// =============================================================================
void loop() {
  // Gestisci pulsanti
  handleButtons();

  // Leggi peso a intervalli regolari (80 SPS)
  unsigned long currentTime = millis();
  if (currentTime - lastReadTime >= READ_INTERVAL) {
    lastReadTime = currentTime;

    // Leggi peso
    readWeight();

    // Aggiorna peso massimo
    if (currentWeight > maxWeight && currentWeight > WEIGHT_THRESHOLD) {
      maxWeight = currentWeight;
    }

    // Aggiorna display
    updateDisplay();

    // Invia dati seriali per plotter
    sendSerialData();
  }
}

// =============================================================================
// INIZIALIZZAZIONE DISPLAY
// =============================================================================
void initDisplay() {
  tft.init();
  tft.setRotation(1);  // Landscape (320x170)
  tft.fillScreen(TFT_BG_COLOR);
  tft.setTextColor(TFT_TEXT_COLOR, TFT_BG_COLOR);

  // Imposta luminosità backlight (opzionale)
  // ledcSetup(0, 10000, 8);
  // ledcAttachPin(TFT_BL, 0);
  // ledcWrite(0, 255);  // Luminosità massima
}

// =============================================================================
// INIZIALIZZAZIONE BILANCIA HX711
// =============================================================================
void initScale() {
  scale.begin(HX711_DOUT_PIN, HX711_SCK_PIN);

  // Attendi che HX711 sia pronto
  while (!scale.is_ready()) {
    Serial.println("Attesa HX711...");
    delay(100);
  }

  // Imposta fattore di calibrazione
  scale.set_scale(CALIBRATION_FACTOR);

  // Configura per 80 SPS (canale A, gain 128)
  // HX711 supporta 10 SPS o 80 SPS basato sul pin RATE
  // Con la libreria, usiamo letture veloci
  Serial.println("HX711 inizializzato");
}

// =============================================================================
// ESEGUI TARA
// =============================================================================
void performTare() {
  Serial.println("Esecuzione tara...");

  // Esegui media di 20 letture per tara stabile
  scale.tare(20);

  // Reset peso massimo dopo tara
  maxWeight = 0.0;
  currentWeight = 0.0;
  lastDisplayedWeight = -1;
  lastDisplayedMax = -1;

  Serial.println("Tara completata");
}

// =============================================================================
// LETTURA PESO
// =============================================================================
void readWeight() {
  if (scale.is_ready()) {
    // Lettura singola per velocità massima (80 SPS)
    currentWeight = scale.get_units(1);

    // Applica soglia minima
    if (abs(currentWeight) < WEIGHT_THRESHOLD) {
      currentWeight = 0.0;
    }

    // Limita a valori positivi (opzionale)
    if (currentWeight < 0) {
      currentWeight = 0.0;
    }
  }
}

// =============================================================================
// AGGIORNAMENTO DISPLAY
// =============================================================================
void updateDisplay() {
  // Aggiorna solo se i valori sono cambiati significativamente
  // per evitare flickering
  bool weightChanged = abs(currentWeight - lastDisplayedWeight) >= 0.5;
  bool maxChanged = abs(maxWeight - lastDisplayedMax) >= 0.5;

  if (weightChanged || maxChanged) {
    // ---- RIGA SUPERIORE: PESO ATTUALE ----
    if (weightChanged) {
      // Pulisci area peso attuale
      tft.fillRect(0, 0, SCREEN_WIDTH, 85, TFT_BG_COLOR);

      // Label
      tft.setTextColor(TFT_LABEL_COLOR, TFT_BG_COLOR);
      tft.setTextSize(2);
      tft.setCursor(15, 10);
      tft.print("PESO:");

      // Valore peso attuale (font grande)
      tft.setTextColor(TFT_TEXT_COLOR, TFT_BG_COLOR);
      tft.setTextSize(5);
      tft.setCursor(15, 40);

      // Formatta con precisione appropriata
      if (currentWeight >= 1000) {
        tft.printf("%6.0f g", currentWeight);
      } else {
        tft.printf("%6.1f g", currentWeight);
      }

      lastDisplayedWeight = currentWeight;
    }

    // ---- RIGA INFERIORE: PESO MASSIMO ----
    if (maxChanged) {
      // Pulisci area peso massimo
      tft.fillRect(0, 86, SCREEN_WIDTH, 84, TFT_BG_COLOR);

      // Label
      tft.setTextColor(TFT_MAX_COLOR, TFT_BG_COLOR);
      tft.setTextSize(2);
      tft.setCursor(15, 95);
      tft.print("MAX:");

      // Valore peso massimo (font grande)
      tft.setTextColor(TFT_MAX_COLOR, TFT_BG_COLOR);
      tft.setTextSize(5);
      tft.setCursor(15, 125);

      if (maxWeight >= 1000) {
        tft.printf("%6.0f g", maxWeight);
      } else {
        tft.printf("%6.1f g", maxWeight);
      }

      lastDisplayedMax = maxWeight;
    }
  }
}

// =============================================================================
// INVIO DATI SERIALI (formato Serial Plotter)
// =============================================================================
void sendSerialData() {
  // Formato per Arduino Serial Plotter:
  // valore1 TAB valore2
  // Il plotter riconosce automaticamente le colonne

  Serial.print(currentWeight, 1);
  Serial.print("\t");
  Serial.println(maxWeight, 1);
}

// =============================================================================
// GESTIONE PULSANTI
// =============================================================================
void handleButtons() {
  unsigned long currentTime = millis();

  // Pulsante TARE (GPIO 0 - BOOT) - Active LOW
  if (digitalRead(BUTTON_TARE) == LOW) {
    if (currentTime - lastButtonTare > DEBOUNCE_DELAY) {
      lastButtonTare = currentTime;

      // Feedback visivo
      tft.fillScreen(TFT_BLUE);
      tft.setTextColor(TFT_WHITE, TFT_BLUE);
      tft.setTextSize(4);
      tft.setCursor(100, 65);
      tft.println("TARA...");

      performTare();

      // Ripristina display
      tft.fillScreen(TFT_BG_COLOR);
      lastDisplayedWeight = -1;
      lastDisplayedMax = -1;
    }
  }

  // Pulsante RESET MAX (GPIO 14 - KEY) - Active LOW
  if (digitalRead(BUTTON_RESET) == LOW) {
    if (currentTime - lastButtonReset > DEBOUNCE_DELAY) {
      lastButtonReset = currentTime;
      resetMaxWeight();
    }
  }
}

// =============================================================================
// RESET PESO MASSIMO
// =============================================================================
void resetMaxWeight() {
  // Feedback visivo
  tft.fillRect(0, 86, SCREEN_WIDTH, 84, TFT_GREEN);
  tft.setTextColor(TFT_BLACK, TFT_GREEN);
  tft.setTextSize(3);
  tft.setCursor(70, 115);
  tft.println("MAX RESET!");

  delay(300);

  // Reset valore
  maxWeight = 0.0;
  lastDisplayedMax = -1;

  // Ripristina display
  tft.fillRect(0, 86, SCREEN_WIDTH, 84, TFT_BG_COLOR);

  Serial.println("# Peso massimo resettato");
}
