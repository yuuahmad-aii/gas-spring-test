#include <Arduino.h>
#include <HX711.h>
#include <EEPROM.h>
#include <LiquidCrystal_I2C.h>

// Loadcell
const int HX711_sck = 3;
const int HX711_dout = 2;
// const float calibrationValue = -43360;
const float calibrationValue = 14382;
float beban_gas_spring;

// Relay dan Push Button
const int pin_relay = 13;
const int pin_button = A0;
bool nilai_relay = false;

// Waktu
unsigned long milis;
unsigned long milis_relay_sebelumnya;
unsigned long milis_tampilan_sebelumnya;
unsigned long jeda_relay = 1500; // Frekuensi switching

float nilai_ke = 0;                        // Counter mundur
char user_input;                           // Buffer untuk input serial
bool test_endurance = false;               // Mode endurance
const int banyak_cycles = 3000;            // Banyak cycles
unsigned long serialPrintInterval = 100ul; // Interval tampilan serial

// LCD dan Load Cell
LiquidCrystal_I2C lcd(0x27, 16, 2);
HX711 LoadCell;

void layarTestProduksi(float j)
{
  j = j / 10000;
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
  Serial.begin(115200);
  pinMode(pin_relay, OUTPUT);
  pinMode(pin_button, INPUT);

  lcd.init();
  lcd.backlight();
  LoadCell.begin(HX711_dout, HX711_sck);
  LoadCell.tare();

  Serial.println("Memulai...");
  lcd.setCursor(0, 0);
  lcd.print("MEMULAI...");
  delay(1000);
  Serial.println("Siap menerima perintah.");
  Serial.println("------------------");
  Serial.println("M = untuk mulai");
  Serial.println("B = untuk berhenti");
  Serial.println("T = untuk tare");
  Serial.println("Sxxx = untuk ubah waktu hidup-mati, dalam ms");
}

void loop()
{
  milis = millis();
  beban_gas_spring = LoadCell.get_units();
  if (isnan(beban_gas_spring))
    beban_gas_spring = 0; // Validasi nilai load cell

  // Baca input serial
  if (Serial.available() > 0)
  {
    user_input = Serial.read();
    user_input = toupper(user_input);
    // Serial.readBytesUntil('\n', user_input, sizeof(user_input));
    // Serial.print(user_input);
    // if (strncmp(user_input, "M", 1) == 0)
    // {
    //   test_endurance = true;
    //   nilai_ke = 0; // Reset nilai_ke
    // }
    // else if (strncmp(user_input, "B", 1) == 0)
    // {
    //   test_endurance = false;
    // }
    // else if (strncmp(user_input, "T", 1) == 0)
    // {
    //   LoadCell.tare();
    // }
    // else if (strncmp(user_input, "S", 1) == 0)
    // {
    //   int value = atoi(&user_input[1]);
    //   if (value < 50)
    //   {
    //     Serial.println("Nilai terlalu kecil, atur ke 50 ms.");
    //     jeda_relay = 50;
    //   }
    //   else
    //   {
    //     jeda_relay = value;
    //     Serial.print("Nilai jeda diupdate: ");
    //     Serial.println(jeda_relay);
    //   }
    if (user_input == 'M') // mulai
      test_endurance = true;
    else if (user_input == 'B') // berhenti
      test_endurance = false;
    else if (user_input == 'T') // berhenti
      LoadCell.tare();
    else
    {
      Serial.print("perintah tidak valid");
    }
  }

  if (test_endurance)
  {
    if ((milis - milis_relay_sebelumnya) >= jeda_relay)
    {
      milis_relay_sebelumnya = milis;
      if (nilai_ke < banyak_cycles)
        nilai_relay = !nilai_relay;
      else
        nilai_relay = true;
      nilai_ke += 0.5;
      digitalWrite(pin_relay, !nilai_relay);
      layarTestEndurance(floor(nilai_ke));
    }

    if (nilai_ke < banyak_cycles && test_endurance)
    {
      Serial.print(floor(nilai_ke));
      Serial.print(",");
      Serial.print(beban_gas_spring / 10000);
      Serial.print(",");
      Serial.println(nilai_relay);
    }
  }
  else
  {
    digitalWrite(pin_relay, !digitalRead(pin_button));
    if ((milis - milis_tampilan_sebelumnya) >= serialPrintInterval)
    {
      milis_tampilan_sebelumnya = milis;
      layarTestProduksi(beban_gas_spring);
    }
  }
}
