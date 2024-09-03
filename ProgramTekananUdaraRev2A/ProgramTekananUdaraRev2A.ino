#include "LiquidCrystal_I2C.h"
#include "HX710B.h"

#define SCLK 3 // clock data pin
#define DOUT 2 // OUT data pin
#define pinPBOn 7
#define pinPBVault 6
#define pinLEDRun 5
#define pinLEDVault 4
#define pinValve 8

float UnderPressure = 0.0;

HX710B pressure_sensor;
LiquidCrystal_I2C lcd(0x27, 16, 2);


unsigned long lastmillis = 0;
float Pressure = 0.0;
bool isUnderPressure = false, isonpressed = false, isvaultpressed = false, CekUnderPressure = true, firstCek = true;

void setup() {
  Serial.begin(9600); // start the serial port
  lcd.begin();
  lcd.backlight();
  
  pinMode(pinPBOn, INPUT);
  pinMode(pinPBVault, INPUT);
  pinMode(pinLEDRun, OUTPUT);
  pinMode(pinLEDVault, OUTPUT);
  pinMode(pinValve, OUTPUT);

  digitalWrite(pinLEDRun, HIGH);
  digitalWrite(pinLEDVault, HIGH);
  
  digitalWrite(pinValve, HIGH);
  
  isonpressed = false;
  isvaultpressed = false;
  isUnderPressure = false;

  pressure_sensor.begin(DOUT, SCLK);
  lcd.setCursor(0, 0); lcd.print("   PRESS RUN   ");
  lcd.setCursor(0, 1); lcd.print("   TO  START   ");
}

uint8_t count = 0;
void loop() {
  CekTombol();
  if(isonpressed) {
    if(millis() - lastmillis >= 1000) {
      lastmillis = millis();
      if(count == 0) {
        lcd.clear();
        lcd.setCursor(0, 0); lcd.print("   CALIBRATE   ");
        lcd.setCursor(0, 1); lcd.print("BAR: "); lcd.print(UnderPressure, 2);
        Serial.print("Set UnderPressure: ");Serial.println(UnderPressure);
      }

      if(count < 6) { count++; }
      
      if (pressure_sensor.is_ready()) {
        Pressure = pressure_sensor.pascal();
        Serial.print("Pascal: ");Serial.println(Pressure);
        lcd.setCursor(0, 1); lcd.print("BAR: "); lcd.print(Pressure, 2);
      }

     if(firstCek && count == 5) {
        lcd.clear();
        lcd.setCursor(0, 0); lcd.print("  AIR PRESSURE ");
        lcd.setCursor(0, 1); lcd.print("BAR: "); lcd.print(UnderPressure, 2);
        firstCek = false;
        UnderPressure = Pressure;
        Serial.print("UnderPressure: ");Serial.println(UnderPressure);
      }
    }
  }
      if(!firstCek) {
        if(Pressure < (UnderPressure)) {
          if(!isUnderPressure && CekUnderPressure) {
            CekUnderPressure = false;
            isUnderPressure = true;
            Serial.println("Vault, Valve Active");
            digitalWrite(pinLEDVault, LOW);
            digitalWrite(pinValve, LOW);
          }
          if(isvaultpressed) {
            Serial.println("Vault Off1");
            digitalWrite(pinLEDRun, HIGH);
            digitalWrite(pinLEDVault, HIGH);
            digitalWrite(pinValve, HIGH);
            
//            isonpressed = false;
            isvaultpressed = false;
            isUnderPressure = false;
            CekUnderPressure = false;
          }
        }
      }
}

uint8_t ONState, LastONState = 0;
uint8_t VaultState, LastVaultState = 0;
void CekTombol() {
  ONState = digitalRead(pinPBOn);
  VaultState = digitalRead(pinPBVault);

  if(ONState != LastONState) {
    if(ONState == LOW) {
      isonpressed = true;
      CekUnderPressure = true;
      count = 0;
      Serial.println("Run");
      digitalWrite(pinLEDRun, LOW);
      digitalWrite(pinValve, LOW);//valve on
    }
  }
  LastONState = ONState;

  if(VaultState != LastVaultState && isUnderPressure) {
    if(VaultState == LOW) {
      Serial.println("Vault");
      isvaultpressed = true;
    }
  }
  LastVaultState = VaultState;
}
