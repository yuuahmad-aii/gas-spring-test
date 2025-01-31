#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>

void setup()
{
    Serial.begin(9600);
    Serial.println("Menghapus semua data di EEPROM...");

    // Loop melalui semua alamat EEPROM
    for (int i = 0; i < EEPROM.length(); i++)
    {
        EEPROM.write(i, 0xFF); // Menulis nilai default (kosong) ke setiap alamat
    }

    Serial.println("EEPROM berhasil dikosongkan!");
}

void loop()
{
    // Tidak ada fungsi di loop karena hanya perlu dijalankan sekali
}
