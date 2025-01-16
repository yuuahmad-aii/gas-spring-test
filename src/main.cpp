#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <HX711.h>
#include <EEPROM.h>

// Pin konfigurasi
#define DT_PIN A1    // Data pin HX711
#define SCK_PIN A0   // Clock pin HX711
#define RELAY_PIN 13 // Pin untuk relay

// Objek HX711 dan LCD
HX711 scale;
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Parameter default
unsigned long pressDuration = 2000;   // Lama penekanan (ms)
unsigned long releaseDuration = 2000; // Lama pelepasan (ms)
float calibrationFactor = 2280.0;     // Faktor kalibrasi HX711
uint8_t inverseRelay = 1;             // balikkan nilai logic relay(active ketika low)
uint8_t mode_skala = 0;               // mode skala menggunakan kg atau gram
bool isTesting = false;               // Status pengujian
int pressCount = 0;                   // Jumlah penekanan

// EEPROM alamat penyimpanan
#define EEPROM_PRESS_DURATION_ADDR 0
#define EEPROM_RELEASE_DURATION_ADDR 4
#define EEPROM_CALIBRATION_FACTOR_ADDR 8
#define EEPROM_INVERSE_RELAY_ADDR 12
#define EEPROM_MODE_SKALA_ADDR 16

// Variabel untuk non-blocking timing
unsigned long previousMillis = 0;
uint8_t relayState = 0;

// primitive
void loadParametersFromEEPROM();
void saveParametersToEEPROM();
void calibrateLoadCell();
void handleSerialInput();

// Fungsi setup
void setup()
{
  Serial.begin(9600);

  // Inisialisasi HX711
  scale.begin(DT_PIN, SCK_PIN);
  Serial.println(EEPROM.length()); // lihat panjang eeprom
  delay(5000);
  loadParametersFromEEPROM();
  scale.set_scale(calibrationFactor);
  scale.tare(); // Tare awal

  // Inisialisasi relay
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, inverseRelay);

  // Inisialisasi LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Stress Test    ");
  lcd.setCursor(0, 1);
  lcd.print("Ready...    ");

  // tampilkan perintah yang tersedia diawal mulai
  // just press help
  // Serial.println("Available commands:");
  // Serial.println("press:<ms>       - Set press duration");
  // Serial.println("release:<ms>     - Set release duration");
  // Serial.println("start            - Start testing");
  // Serial.println("stop             - Stop testing");
  // Serial.println("tare             - Tare the load cell");
  // Serial.println("calibrate:<f>    - Set calibration factor");
  // Serial.println("calibration      - Start load cell calibration");
  // Serial.println("inv_relay:<1/0>  - inverse relay value (low active or high active)");
  // Serial.println("mode_skala:<1/0> - pakai kilogram atau gram (1 kg, 0 gram)");
}

void loop()
{
  // Baca nilai beban
  float weight = scale.get_units();
  if (mode_skala == 1)
    weight = weight / 1000;

  // Periksa status pengujian
  if (isTesting)
  {
    unsigned long currentMillis = millis();

    // Penekanan beban
    if (relayState == inverseRelay && currentMillis - previousMillis >= releaseDuration)
    {
      relayState = !inverseRelay;
      previousMillis = currentMillis;
      digitalWrite(RELAY_PIN, HIGH);
      pressCount++;
    }

    // Pelepasan beban
    if (relayState == !inverseRelay && currentMillis - previousMillis >= pressDuration)
    {
      relayState = inverseRelay;
      previousMillis = currentMillis;
      digitalWrite(RELAY_PIN, LOW);
    }

    // Tampilkan nilai beban dan jumlah penekanan
    // Serial.print("Press Count: ");
    Serial.print(pressCount);
    Serial.print(", ");
    Serial.print(weight, 2);
    Serial.print(", ");
    Serial.println(relayState);

    // nonaktifkan pesan lcd
    // lcd.setCursor(0, 0);
    // lcd.print("Prss: ");
    // lcd.print(pressCount);
    // lcd.print("      ");
    // lcd.setCursor(0, 1);
    // lcd.print("Bbn: ");
    // lcd.print(weight, 2);
    // mode_skala == 1 ? lcd.print("kg   ") : lcd.print("gr   ");
  }
  else
  {
    // reset nilai relay
    digitalWrite(RELAY_PIN, inverseRelay);

    lcd.setCursor(0, 0);
    lcd.print("btn mode     ");
    // lcd.print(digitalRead(RELAY_PIN));
    // lcd.print("      ");
    lcd.setCursor(0, 1);
    lcd.print("bbn: ");
    lcd.print(weight, 2);
    mode_skala == 1 ? lcd.print("kg   ") : lcd.print("gr   ");
  }

  // Periksa input dari serial untuk mengubah parameter
  if (Serial.available() > 0)
  {
    handleSerialInput();
  }
}

// Fungsi menangani input serial
void handleSerialInput()
{
  String input = Serial.readStringUntil('\n');
  input.trim();

  if (input.startsWith("press:"))
  {
    pressDuration = input.substring(6).toInt();
    saveParametersToEEPROM();
    Serial.print("Press duration set to: ");
    Serial.println(pressDuration);
  }
  else if (input.startsWith("release:"))
  {
    releaseDuration = input.substring(8).toInt();
    saveParametersToEEPROM();
    Serial.print("Release duration set to: ");
    Serial.println(releaseDuration);
  }
  else if (input.startsWith("start"))
  {
    isTesting = true;
    pressCount = 0;
    Serial.println("Testing started...");
    lcd.clear();
    lcd.print("Testing...");
  }
  else if (input.startsWith("stop"))
  {
    isTesting = false;
    Serial.println("Testing stopped.");
    lcd.clear();
    lcd.print("Test Stopped");
  }
  else if (input.startsWith("tare"))
  {
    scale.tare();
    Serial.println("Load cell tared.");
    lcd.clear();
    lcd.print("Tared");
  }
  else if (input.startsWith("calibrate:"))
  {
    calibrationFactor = input.substring(10).toFloat();
    scale.set_scale(calibrationFactor);
    saveParametersToEEPROM();
    Serial.print("Calibration factor set to: ");
    Serial.println(calibrationFactor);
  }
  else if (input.startsWith("inv_relay:"))
  {
    inverseRelay = input.substring(10).toInt();
    saveParametersToEEPROM();
    Serial.print("inverse relay set to: ");
    Serial.println(inverseRelay);
  }
  else if (input.startsWith("mode_skala:"))
  {
    mode_skala = input.substring(11).toInt();
    saveParametersToEEPROM();
    Serial.print("mode skala : ");
    mode_skala == 1 ? Serial.println("kg") : Serial.println("gram");
  }
  else if (input.startsWith("calibration"))
  {
    calibrateLoadCell();
  }
  else
  {
    Serial.println("Available commands:");
    Serial.println("press:<ms>       - Set press duration");
    Serial.println("release:<ms>     - Set release duration");
    Serial.println("start            - Start testing");
    Serial.println("stop             - Stop testing");
    Serial.println("tare             - Tare the load cell");
    Serial.println("calibrate:<f>    - Set calibration factor");
    Serial.println("calibration      - Start load cell calibration");
    Serial.println("inv_relay:<1/0>  - inverse relay value (low active or high active)");
    Serial.println("mode_skala:<1/0> - pakai kilogram atau gram (1 kg, 0 gram)");
  }
}

// Fungsi kalibrasi load cell
void calibrateLoadCell()
{
  scale.set_scale();
  Serial.println("Tare... remove any weights from the scale.");
  delay(5000);
  scale.tare();
  Serial.println("Tare done...");
  Serial.println("Place a known weight on the load cell.");
  delay(5000); // Tunggu 5 detik untuk menempatkan beban

  float rawValue = scale.get_units(10);
  Serial.print("Raw value: ");
  Serial.println(rawValue);

  Serial.println("Enter the weight in grams:");
  while (!Serial.available())
  {
    delay(100);
  }

  float knownWeight = Serial.parseFloat();
  calibrationFactor = rawValue / knownWeight;
  scale.set_scale(calibrationFactor);
  saveParametersToEEPROM();

  Serial.print("Calibration completed. New calibration factor: ");
  Serial.println(calibrationFactor);
}

// Simpan parameter ke EEPROM
void saveParametersToEEPROM()
{
  EEPROM.put(EEPROM_PRESS_DURATION_ADDR, pressDuration);
  EEPROM.put(EEPROM_RELEASE_DURATION_ADDR, releaseDuration);
  EEPROM.put(EEPROM_CALIBRATION_FACTOR_ADDR, calibrationFactor);
  EEPROM.put(EEPROM_CALIBRATION_FACTOR_ADDR, inverseRelay);
  EEPROM.put(EEPROM_MODE_SKALA_ADDR, mode_skala);
  Serial.println("Parameters saved to EEPROM.");
}

// Muat parameter dari EEPROM
void loadParametersFromEEPROM()
{
  EEPROM.get(EEPROM_PRESS_DURATION_ADDR, pressDuration);
  EEPROM.get(EEPROM_RELEASE_DURATION_ADDR, releaseDuration);
  EEPROM.get(EEPROM_CALIBRATION_FACTOR_ADDR, calibrationFactor);
  EEPROM.get(EEPROM_INVERSE_RELAY_ADDR, inverseRelay);
  EEPROM.get(EEPROM_MODE_SKALA_ADDR, mode_skala);

  if (isnan(calibrationFactor) || calibrationFactor == 0)
  {
    calibrationFactor = 2280.0; // Default nilai kalibrasi
  }

  Serial.println("Parameters loaded from EEPROM.");
  // Serial.print("Press Duration: ");
  Serial.println(pressDuration);
  // Serial.print("Release Duration: ");
  Serial.println(releaseDuration);
  // Serial.print("Calibration Factor: ");
  Serial.println(calibrationFactor);
  // Serial.print("Inverse Relay: ");
  Serial.println(inverseRelay);
  // Serial.print("Mode Skala: ");
  Serial.println(mode_skala);
  // mode_skala == 1 ? Serial.println("kilogram") : Serial.println("gram");
}
