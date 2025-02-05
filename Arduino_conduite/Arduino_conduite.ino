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

  }
}
