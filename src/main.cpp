// program untuk test endurance gas spring
// dan dapat digunakan secara manual untuk test produksi

#include <Arduino.h>
#include <HX711.h>
#include <EEPROM.h>
#include <LiquidCrystal_I2C.h>

// loadcell
const int HX711_sck = 13;
const int HX711_dout = 12;
const float calibrationValue = -43360;
float beban_gas_spring;

// relay dan push button
const int pin_relay = 8;
const int pin_button = 7;
bool nilai_relay = true;

// inisialisasi waktu
unsigned long milis;
unsigned long milis_relay_sebelumnya;
unsigned long milis_tampilan_sebelumnya;
const unsigned long jeda_relay = 500; // mengatur frekuensi

float nilai_ke = 0;                        // inisialisasi nilai untuk counter mundur
char user_input;                           // inisialisasi nilai untuk user input
bool test_endurance = true;                // sudah ada perintah dari komputer atau belum
const int banyak_cycles = 3000;            // banyak cycles
unsigned long serialPrintInterval = 100ul; // 100 ms

// inisialisasi
LiquidCrystal_I2C lcd(0x27, 16, 2);
HX711 LoadCell;

void layarTestProduksi(float j)
{
  j = j / 1000;
  lcd.setCursor(0, 0);
  lcd.print("LOAD ");
  lcd.print(j, 2);
  lcd.print("           ");
  lcd.setCursor(14, 0);
  lcd.print("Kg");
}

void layarTestEndurance(int j)
{
  lcd.setCursor(0, 0);
  lcd.print("TEST KE-");
  lcd.print(j);
  lcd.print("               ");
}

void setup()
{
  Serial.begin(115200); // mulai komunikasi serial di max speed

  pinMode(pin_relay, OUTPUT);
  pinMode(pin_button, INPUT);

  // inisialisasi loadcell dan lcd
  lcd.init();
  lcd.backlight();
  LoadCell.begin(HX711_dout, HX711_sck);
  LoadCell.tare();

  Serial.println();
  Serial.println("Memulai...");
  lcd.setCursor(0, 0);
  lcd.print("MEMULAI... ");
  delay(1000);
  Serial.println("siap!");
  Serial.println("------------------");
  Serial.println("M = untuk mulai");
  Serial.println("B = untuk berhenti");
}

void loop()
{
  static boolean newDataReady = 0;
  milis = millis(); // reset nilai milis
  beban_gas_spring = LoadCell.get_units();

  if (Serial.available() > 0)
  {
    user_input = Serial.read();
    user_input = toupper(user_input);
    if (user_input == 'M') // mulai
      test_endurance = true;
    else if (user_input == 'B') // berhenti
      test_endurance = false;
  }

  if (test_endurance) // jika test endurance dilakukan
  {
    // program untuk switching relay
    if ((milis - milis_relay_sebelumnya) >= jeda_relay)
    {
      milis_relay_sebelumnya = milis;
      if (nilai_ke < banyak_cycles)
        nilai_relay = !nilai_relay;
      else
        nilai_relay = true; // true == relay mati == program berhenti
      nilai_ke = nilai_ke + 0.5;
      digitalWrite(pin_relay, nilai_relay);
      layarTestEndurance(floor(nilai_ke));
    }

    // mengrimkan data ke komputer
    if (nilai_ke < banyak_cycles && test_endurance)
    {
      Serial.print(floor(nilai_ke));
      Serial.print(",");
      Serial.print(beban_gas_spring);
      Serial.print(",");
      Serial.println(nilai_relay);
    }
  }
  else // jika berada di mode manual
  {
    digitalWrite(pin_relay, digitalRead(pin_button));
    if ((milis - milis_tampilan_sebelumnya) >= serialPrintInterval)
    {
      milis_relay_sebelumnya = milis;
      layarTestProduksi(beban_gas_spring);
      newDataReady = 0;
    }
  }
}
