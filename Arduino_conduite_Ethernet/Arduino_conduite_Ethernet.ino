#include <M5Atom.h>

CRGB pixel;

unsigned long monChronoMessages;

#include <SPI.h>
#include <Ethernet.h>
// An EthernetUDP instance to let us send and receive packets over UDP
EthernetUDP myUdp;

IPAddress myDestinationIp(192, 168, 1, 157);
unsigned int myDestinationPort = 8004;

IPAddress myIp(192, 168, 1, 215);
unsigned int myPort = 7000;


#include <MicroOscUdp.h>

// THE NUMBER 1024 BETWEEN THE < > SYMBOLS  BELOW IS THE MAXIMUM NUMBER OF BYTES RESERVED FOR INCOMMING MESSAGES.
// OUTGOING MESSAGES ARE WRITTEN DIRECTLY TO THE OUTPUT AND DO NOT NEED ANY RESERVED BYTES.
// PROVIDE A POINTER TO UDP, AND THE IP AND PORT FOR OUTGOING MESSAGES.
// DO NOT FORGET THAT THE UDP CONNEXION MUST BE INITIALIZED IN SETUP() WITH THE RECEIVE PORT.
MicroOscUdp<1024> myMicroOsc(&myUdp, myDestinationIp, myDestinationPort);  // CREATE AN INSTANCE OF MicroOsc FOR UDP MESSAGES


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


  Serial.println();
  Serial.println("Starting Ethernet with STATIC IP");
  // CONFIGURE ETHERNET
  SPI.begin(22, 23, 33, 19);
  Ethernet.init(19);
  uint8_t myMac[6];
  // GET FACTORY DEFINED ESP32 MAC :
  esp_efuse_mac_get_default(myMac);
  // START ETHERNET WITH STATIC IP
  Ethernet.begin(myMac, myIp);

  myUdp.begin(myPort);

  Serial.println();
  Serial.println(__FILE__);
  Serial.print("myDestinationIp: ");
  Serial.println(myDestinationIp);
  Serial.print("myDestinationPort: ");
  Serial.println(myDestinationPort);
  Serial.print("myIp: ");
  Serial.println(Ethernet.localIP());
  Serial.print("myPort: ");
  Serial.println(myPort);

}


void loop() {
  // put your main code here, to run repeatedly:
  M5.update();
  //monOsc.onOscMessageReceived(maReceptionMessageOsc);
  // À CHAQUE 20 MS I.E. 50x PAR SECONDE
  if (millis() - monChronoMessages >= 20) {
    monChronoMessages = millis();


    int16_t adc_raw = Vmeter.getSingleConversion();
    float voltage   = adc_raw * resolution * calibration_factor;
    /*Serial.printf("Cal ADC:%.0f\n", adc_raw * calibration_factor);
    Serial.printf("Cal Voltage:%.2f mV\n", voltage);
    Serial.printf("Raw ADC:%d\n\n", adc_raw);*/
  
    // monOsc.sendFloat("/Raw", adc_raw * -1);
    Serial.printf("Raw ADC:%d\n\n", adc_raw);
    myMicroOsc.sendFloat("/Raw", adc_raw * -1);

  }
}
