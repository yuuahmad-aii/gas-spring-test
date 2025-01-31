#include <Arduino.h>
#include <Wire.h>
#include <HX711.h>

#define ARDUINO_NANO 0

// Pin konfigurasi
#if ARDUINO_NANO == 1
#define DT_PIN 2     // Data pin HX711
#define SCK_PIN 3    // Clock pin HX711
#define RELAY_PIN 13 // Pin untuk relay
#else
#define DT_PIN PB11   // Data pin HX711
#define SCK_PIN PB10  // Clock pin HX711
#define RELAY_PIN PB2 // Pin untuk relay
#endif
// Inisialisasi objek HX711
HX711 scale;

void setup()
{
    Serial.begin(9600); // Mulai komunikasi serial
    Serial.println("Inisialisasi HX711...");

    scale.begin(DT_PIN, SCK_PIN); // Mulai komunikasi dengan HX711
    Serial.println("Siap membaca nilai RAW dari Load Cell.");
}

void loop()
{
    // Membaca nilai mentah dari HX711
    long rawValue = scale.get_units();
    Serial.print("Nilai RAW HX711: ");
    Serial.println(rawValue);

    delay(500); // Delay 500ms untuk pembacaan
}
