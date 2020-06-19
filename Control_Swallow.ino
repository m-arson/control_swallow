#include <EEPROM.h>
#include <LiquidCrystal.h>
#include "DHT.h";
#define DHTPIN 7
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

LiquidCrystal lcd(13, 12, 11, 10, 9, 8);

uint8_t btn[6] = {6, 5, 4, 3, 2};
bool btn_cond[6];
bool last_cond;
bool to_break;
uint8_t flag_cond = 9;
bool mode = 0;

struct Relays {
  uint8_t cooler = A4;
  uint8_t mist_maker = A5;
} relay;

struct Tresholds {
  uint8_t temp = 26;
  uint8_t hum = 90;
} treshold;

struct Addrs {
  uint8_t temp = 21;
  uint8_t hum = 22;
} addr;

uint64_t last_millis = 0;
uint8_t time_delay = 50;

void do_action(uint8_t val) {
  switch (val) {
    case 0:
      treshold.temp++;
      break;
    case 1:
      treshold.temp--;
      break;
    case 2:
      treshold.hum++;
      break;
    case 3:
      treshold.hum--;
      break;
    case 4:
      EEPROM.update(addr.temp, treshold.temp);
      EEPROM.update(addr.hum, treshold.hum);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Saved!");
      to_break = 1;
      delay(1500);
      break;
  }
  flag_cond = 9;
}

void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2);
  dht.begin();
  if (EEPROM.read(addr.temp) == 0 || EEPROM.read(addr.temp) > 100 || EEPROM.read(addr.hum) == 0 || EEPROM.read(addr.hum) > 100) {
    EEPROM.update(addr.temp, treshold.temp);
    EEPROM.update(addr.hum, treshold.hum);
  }
  else {
    treshold.temp = EEPROM.read(addr.temp);
    treshold.hum = EEPROM.read(addr.hum);
  }
  for (uint8_t i = 2; i < 7; i++) {
    pinMode(i, INPUT_PULLUP);
  }
  pinMode(relay.cooler, OUTPUT);
  pinMode(relay.mist_maker, OUTPUT);
  digitalWrite(relay.cooler, HIGH);
  digitalWrite(relay.mist_maker, HIGH);
}
void loop() {
  mode = digitalRead(2);
  if (mode == 1) {
    lcd.setCursor(0, 0);
    lcd.print("Temp : ");
    float t = dht.readTemperature();
    float h = dht.readHumidity();
    if (t > treshold.temp) {
      digitalWrite(relay.cooler, LOW);
    }
    else {
      digitalWrite(relay.cooler, HIGH);
    }
    if (h < treshold.hum) {
      digitalWrite(relay.mist_maker, LOW);
    }
    else {
      digitalWrite(relay.mist_maker, HIGH);
    }

    lcd.print(t);
    lcd.print(' ');
    lcd.print((char) 223);
    lcd.print("C ");
    lcd.setCursor(0, 1);
    lcd.print("Humi : ");
    lcd.print(h);
    lcd.print("  ");
    lcd.print("% ");
  }
  else {
    lcd.clear();
    delay(250);
    while (true) {
      if (to_break == 1) {
        to_break = 0;
        return;
      }
      for (int i = 0; i < 6; i++) {
        btn_cond[i] = digitalRead(btn[i]);
        if (btn_cond[i] == 0 && last_cond == 1) {
          last_millis = millis();
          flag_cond = i;
        }
        if ((uint64_t) millis() - last_millis > time_delay) {
          if (flag_cond != 9) {
            do_action(flag_cond);
          }
        }
      }
      lcd.setCursor(0, 0);
      lcd.print("OK to save...");
      lcd.setCursor(0, 1);
      lcd.print("T: ");
      lcd.print(treshold.temp);
      lcd.print(", ");
      lcd.print("H: ");
      lcd.print(treshold.hum);
      lcd.print("  ");
      last_cond = digitalRead(2)&digitalRead(3)&digitalRead(4)&digitalRead(5)&digitalRead(6);
      delay(50);
    }
  }
}
