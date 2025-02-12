#define BUTTON_AFFIRM 0
#define BUTTON_TEASE 1

int lastAffirm;
int lastTease;

#include <M5Atom.h>

CRGB pixel;

unsigned long monChronoMessages;

#include <MicroOscSlip.h>
MicroOscSlip<128> monOsc(&Serial);

#include <M5_PbHub.h>
M5_PbHub myPbHub;

#include "M5_ADS1115.h"
 
#define M5_UNIT_VMETER_I2C_ADDR             0x49
#define M5_UNIT_VMETER_EEPROM_I2C_ADDR      0x53
#define M5_UNIT_VMETER_PRESSURE_COEFFICIENT 0.015918958F
 
ADS1115 Vmeter;
 
float resolution         = 0.0;
float calibration_factor = 0.0;

void setup() {
  // put your setup code here, to run once:
  M5.begin(false, false, false);
  FastLED.addLeds<WS2812, DATA_PIN, GRB>(&pixel, 1);  // Ajouter le pixel du M5Atom à FastLED
  Serial.begin(115200);

  unsigned long chronoDepart = millis();
  while (millis() - chronoDepart < 5000) {
    pixel = CRGB(255, 255, 255);
    FastLED.show();
    delay(100);

    pixel = CRGB(0, 0, 0);
    FastLED.show();
    delay(100);
  }

  pixel = CRGB(0, 0, 0);
  FastLED.show();

  Wire.begin();
  myPbHub.begin();
  myPbHub.setPixelCount(BUTTON_AFFIRM, 1);
  myPbHub.setPixelCount(BUTTON_TEASE, 1);

  myPbHub.setPixelColor(BUTTON_AFFIRM, 0, 0, 255, 0);
  myPbHub.setPixelColor(BUTTON_TEASE, 0, 255, 0, 0);

  while (!Vmeter.begin(&Wire, M5_UNIT_VMETER_I2C_ADDR, 26, 32, 400000U)) {
    Serial.println("Unit Vmeter Init Fail");
    delay(1000);
  }
  Vmeter.setEEPROMAddr(M5_UNIT_VMETER_EEPROM_I2C_ADDR);
  Vmeter.setMode(ADS1115_MODE_SINGLESHOT);
  Vmeter.setRate(ADS1115_RATE_8);
  Vmeter.setGain(ADS1115_PGA_512);
  // | PGA      | Max Input Voltage(V) |
  // | PGA_6144 |        128           |
  // | PGA_4096 |        64            |
  // | PGA_2048 |        32            |
  // | PGA_512  |        16            |
  // | PGA_256  |        8             |

  resolution = Vmeter.getCoefficient() / M5_UNIT_VMETER_PRESSURE_COEFFICIENT;
  calibration_factor = Vmeter.getFactoryCalibration();

}


void loop() {
  // put your main code here, to run repeatedly:
  M5.update();
  //monOsc.onOscMessageReceived(maReceptionMessageOsc);
  // À CHAQUE 20 MS I.E. 50x PAR SECONDE
  if (millis() - monChronoMessages >= 20) {
    monChronoMessages = millis();

    int AffirmKey = myPbHub.digitalRead(BUTTON_AFFIRM);
    int TeaseKey = myPbHub.digitalRead(BUTTON_TEASE);

    if(lastAffirm != AffirmKey) {
      if (AffirmKey == 0) {
        monOsc.sendInt("/Affirm", 1);
      } else {
        monOsc.sendInt("/Affirm", 0);
      }
    }
    lastAffirm = AffirmKey;

    if(lastTease != TeaseKey) {
      if (TeaseKey == 0) {
        monOsc.sendInt("/Tease", 1);
      } else {
        monOsc.sendInt("/Tease", 0);
      }
    }
    lastTease = TeaseKey;

    int16_t adc_raw = Vmeter.getSingleConversion();
    float voltage   = adc_raw * resolution * calibration_factor;
    /*Serial.printf("Cal ADC:%.0f\n", adc_raw * calibration_factor);
    Serial.printf("Cal Voltage:%.2f mV\n", voltage);
    Serial.printf("Raw ADC:%d\n\n", adc_raw);*/

    monOsc.sendFloat("/Raw", adc_raw * -1);

  }
}
