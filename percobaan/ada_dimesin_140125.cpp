#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <HX711.h>

// Pin konfigurasi
#define DT_PIN A1   // Data pin HX711
#define SCK_PIN A0  // Clock pin HX711
#define RELAY_PIN 7 // Pin untuk relay

// Objek HX711 dan LCD
HX711 scale;
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Parameter default
unsigned long pressDuration = 2000;   // Lama penekanan (ms)
unsigned long releaseDuration = 2000; // Lama pelepasan (ms)
float calibrationFactor = 2280.0;     // Faktor kalibrasi HX711
bool isTesting = false;               // Status pengujian
int pressCount = 0;                   // Jumlah penekanan

void setup()
{
    Serial.begin(115200);

    // Inisialisasi HX711
    scale.begin(DT_PIN, SCK_PIN);
    scale.set_scale(calibrationFactor);
    scale.tare(); // Tare awal

    // Inisialisasi relay
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, LOW);

    // Inisialisasi LCD
    lcd.init();
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("Stress Test");
    lcd.setCursor(0, 1);
    lcd.print("Ready...");
}

void loop()
{
    if (isTesting)
    {
        // Aktifkan relay untuk penekanan
        digitalWrite(RELAY_PIN, HIGH);
        delay(pressDuration);

        // Baca nilai beban
        float weight = scale.get_units();
        pressCount++;

        // Tampilkan nilai beban dan jumlah penekanan
        Serial.print("Press Count: ");
        Serial.print(pressCount);
        Serial.print(", Weight: ");
        Serial.println(weight, 2);

        lcd.setCursor(0, 0);
        lcd.print("Press: ");
        lcd.print(pressCount);
        lcd.setCursor(0, 1);
        lcd.print("Weight: ");
        lcd.print(weight, 2);

        // Lepaskan relay
        digitalWrite(RELAY_PIN, LOW);
        delay(releaseDuration);
    }

    // Periksa input dari serial untuk mengubah parameter
    if (Serial.available() > 0)
    {
        handleSerialInput();
    }
}

void handleSerialInput()
{
    String input = Serial.readStringUntil('\n');
    input.trim();

    if (input.startsWith("press:"))
    {
        pressDuration = input.substring(6).toInt();
        Serial.print("Press duration set to: ");
        Serial.println(pressDuration);
    }
    else if (input.startsWith("release:"))
    {
        releaseDuration = input.substring(8).toInt();
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
        Serial.print("Calibration factor set to: ");
        Serial.println(calibrationFactor);
    }
    else
    {
        Serial.println("Invalid command. Available commands:");
        Serial.println("press:<ms>     - Set press duration");
        Serial.println("release:<ms>   - Set release duration");
        Serial.println("start          - Start testing");
        Serial.println("stop           - Stop testing");
        Serial.println("tare           - Tare the load cell");
        Serial.println("calibrate:<f>  - Set calibration factor");
    }
}
