#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <HX711.h>
#include <EEPROM.h>

// konfigurasi setting parameter
#define DEBUG_MODE 0
#define DENGAN_LCD 0
#define ARDUINO_NANO 0

// Pin konfigurasi
#if ARDUINO_NANO ==1
#define DT_PIN 2     // Data pin HX711
#define SCK_PIN 3    // Clock pin HX711
#define RELAY_PIN 13 // Pin untuk relay
#else
#define DT_PIN PB11     // Data pin HX711
#define SCK_PIN PB10    // Clock pin HX711
#define RELAY_PIN PB2 // Pin untuk relay
#endif
// #define RELAY_PIN1 12 // Pin untuk relay
// #define RELAY_PIN2 11 // Pin untuk relay

// Objek HX711 dan LCD
HX711 scale;
#if DENGAN_LCD == 1
LiquidCrystal_I2C lcd(0x27, 16, 2);
#endif

// Parameter default
unsigned int pressDuration = 2000;   // Lama penekanan (ms)
unsigned int releaseDuration = 2000; // Lama pelepasan (ms)
float calibrationFactor = 2280.0;    // Faktor kalibrasi HX711
uint8_t inverseRelay = 1;            // balikkan nilai logic relay(active ketika low)
uint8_t mode_skala = 0;              // mode skala menggunakan kg atau gram
bool isTesting = false;              // Status pengujian
unsigned long pressCount = 0;        // Jumlah penekanan

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
  Serial.begin(115200);

  // Inisialisasi HX711
  scale.begin(DT_PIN, SCK_PIN);
#if DEBUG_MODE == 1
  Serial.println(EEPROM.length()); // lihat panjang eeprom
#endif
  delay(5000);
  loadParametersFromEEPROM();
  scale.set_scale(calibrationFactor);
  scale.tare(); // Tare awal

  // Inisialisasi relay
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, inverseRelay);
  // pinMode(RELAY_PIN1, OUTPUT);
  // digitalWrite(RELAY_PIN1, inverseRelay);
  // pinMode(RELAY_PIN2, OUTPUT);
  // digitalWrite(RELAY_PIN2, inverseRelay);

// Inisialisasi LCD
#if DENGAN_LCD == 1
  lcd.init();
  lcd.noBacklight();
#endif

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
  Serial.println("memulai program");
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
      // digitalWrite(RELAY_PIN1, HIGH);
      // digitalWrite(RELAY_PIN2, HIGH);
      pressCount++;
    }

    // Pelepasan beban
    if (relayState == !inverseRelay && currentMillis - previousMillis >= pressDuration)
    {
      relayState = inverseRelay;
      previousMillis = currentMillis;
      digitalWrite(RELAY_PIN, LOW);
      // digitalWrite(RELAY_PIN1, LOW);
      // digitalWrite(RELAY_PIN2, LOW);
    }

    // Tampilkan nilai beban dan jumlah penekanan
    Serial.print(pressCount);
    Serial.print(", ");
    Serial.print(weight, 2);
    Serial.print(", ");
    Serial.println(relayState);

    // nonaktifkan pesan lcd
#if DENGAN_LCD == 1
    lcd.setCursor(0, 0);
    lcd.print("Cycle: ");
    lcd.print(pressCount);
    lcd.print("         ");
#endif
    // lcd.setCursor(0, 1);
    // lcd.print("Beban: ");
    // lcd.print(weight, 2);
    // mode_skala == 1 ? lcd.print("kg      ") : lcd.print("gr      ");
  }
  else
  {
    // reset nilai relay
    digitalWrite(RELAY_PIN, inverseRelay);
    // digitalWrite(RELAY_PIN1, inverseRelay);
    // digitalWrite(RELAY_PIN2, inverseRelay);

#if DENGAN_LCD == 1
    lcd.setCursor(0, 0);
    lcd.print("btn mode      ");
    lcd.setCursor(0, 1);
    lcd.print("bbn: ");
    lcd.print(weight, 2);
    mode_skala == 1 ? lcd.print("kg      ") : lcd.print("gr      ");
#endif
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
    Serial.print("Press duration: ");
    Serial.println(pressDuration);
  }
  else if (input.startsWith("release:"))
  {
    releaseDuration = input.substring(8).toInt();
    saveParametersToEEPROM();
    Serial.print("Release duration: ");
    Serial.println(releaseDuration);
  }
  else if (input.startsWith("start"))
  {
    isTesting = true;
    Serial.println("Testing started...");
#if DENGAN_LCD == 1
    lcd.clear();
#endif
  }
  else if (input.startsWith("pause"))
  {
    isTesting = false;
    Serial.println("Testing paused...");
#if DENGAN_LCD == 1
    lcd.clear();
#endif
  }
  else if (input.startsWith("stop"))
  {
    isTesting = false;
    pressCount = 0;
    Serial.println("Testing stopped.");
    Serial.print("press count restarted: ");
    Serial.println(pressCount);
#if DENGAN_LCD == 1
    lcd.clear();
#endif
  }
  else if (input.startsWith("tare"))
  {
    scale.tare();
    Serial.println("Load cell tared.");
  }
  else if (input.startsWith("calibrate:"))
  {
    calibrationFactor = input.substring(10).toFloat();
    scale.set_scale(calibrationFactor);
    saveParametersToEEPROM();
    Serial.print("Calibration factor: ");
    Serial.println(calibrationFactor);
  }
  else if (input.startsWith("inv_relay:"))
  {
    inverseRelay = input.substring(10).toInt();
    saveParametersToEEPROM();
    Serial.print("inverse relay: ");
    Serial.println(inverseRelay);
  }
  else if (input.startsWith("mode_skala:"))
  {
    mode_skala = input.substring(11).toInt();
    saveParametersToEEPROM();
    Serial.print("mode skala: ");
    mode_skala == 1 ? Serial.println("kg") : Serial.println("gram");
  }
  else if (input.startsWith("calibration"))
  {
    calibrateLoadCell();
  }
  else if (input.startsWith("lihat_data"))
  {
    Serial.print("Press Duration: ");
    Serial.println(pressDuration);
    Serial.print("Release Duration: ");
    Serial.println(releaseDuration);
    Serial.print("Calibration Factor: ");
    Serial.println(calibrationFactor);
    Serial.print("Inverse Relay: ");
    Serial.println(inverseRelay);
    Serial.print("Mode Skala: ");
    mode_skala == 1 ? Serial.println("kilogram") : Serial.println("gram");
    Serial.print("Press Count: ");
    Serial.println(pressCount);
  }
  else if (input.startsWith("press_count:"))
  {
    pressCount = input.substring(12).toInt();
    // saveParametersToEEPROM(); // jangan simpan parameter press_count ke eeprom
    Serial.print("press count: ");
    Serial.println(pressCount);
  }
  else
  {
    Serial.println("Available commands:");
    Serial.println("press:<ms>");
    Serial.println("release:<ms>");
    Serial.println("start");
    Serial.println("pause");
    Serial.println("stop");
    Serial.println("tare");
    Serial.println("calibrate:<f>");
    Serial.println("calibration");
    Serial.println("inv_relay:<1/0>");
    Serial.println("mode_skala:<1/0> <kg/gr>");
    Serial.println("lihat_data");
    Serial.println("press_count:<int>");
  }
}

// Fungsi kalibrasi load cell
void calibrateLoadCell()
{
  scale.set_scale();
  Serial.println("Tare, remove any weights");
  delay(5000);
  scale.tare();
  Serial.println("Tare done...");
  Serial.println("Place a known weight");
  delay(5000); // Tunggu 5 detik untuk menempatkan beban

  float rawValue = scale.get_units(10);
  Serial.print("Raw value: ");
  Serial.println(rawValue);

  Serial.println("Enter the weight in gr:");
  while (!Serial.available())
  {
    delay(100);
  }

  float knownWeight = Serial.parseFloat();
  calibrationFactor = rawValue / knownWeight;
  scale.set_scale(calibrationFactor);
  saveParametersToEEPROM();

  Serial.print("Completed. New cal factor: ");
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
  Serial.println("saved.");
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

#if DEBUG_MODE == 1
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
#endif
}
