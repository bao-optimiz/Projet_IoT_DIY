#include "LowPower.h"
extern volatile unsigned long timer0_millis;
void addMillis(unsigned long extra_millis) {
  uint8_t oldSREG = SREG; cli();
  timer0_millis += extra_millis;
  SREG = oldSREG;         sei();
  }

// ---------------------------------------------- //
//                  Sleep LoKy
void do_sleep(unsigned int sleepyTime) {
  unsigned int eights = sleepyTime / 8;
  unsigned int fours = (sleepyTime % 8) / 4;
  unsigned int twos = ((sleepyTime % 8) % 4) / 2;
  unsigned int ones = ((sleepyTime % 8) % 4) % 2;

  Serial.print(F("LoKy sleeps in "));Serial.print(sleepyTime);Serial.println(F(" seconds. "));delay(100);
  for ( int x = 0; x < eights; x++) {LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);}
  for ( int x = 0; x < fours; x++)  {LowPower.powerDown(SLEEP_4S, ADC_OFF, BOD_OFF);}
  for ( int x = 0; x < twos; x++)   {LowPower.powerDown(SLEEP_2S, ADC_OFF, BOD_OFF);}
  for ( int x = 0; x < ones; x++)   {LowPower.powerDown(SLEEP_1S, ADC_OFF, BOD_OFF);}
  addMillis(sleepyTime * 1000);
  }
// ---------------------------------------------- //

// ---------------------------------------------- //
//           Set DataRate for LoRaWAN
void setDataRate() {
  switch (LMIC.datarate) {
    case DR_SF12:
      Serial.println(F("Datarate: SF12"));
      TX_INTERVAL = 4800;
      break;
    case DR_SF11:
      Serial.println(F("Datarate: SF11"));
      TX_INTERVAL = 2400;
      break;
    case DR_SF10:
      Serial.println(F("Datarate: SF10"));
      TX_INTERVAL = 1200;
      break;
    case DR_SF9:
      Serial.println(F("Datarate: SF9"));
      TX_INTERVAL = 900;
      break;
    case DR_SF8:
      Serial.println(F("Datarate: SF8"));
      TX_INTERVAL = 300;
      break;
    case DR_SF7:
      Serial.println(F("Datarate: SF7"));
      TX_INTERVAL = 120;
      break;
    case DR_SF7B:
      Serial.println(F("Datarate: SF7B"));
      TX_INTERVAL = 120;
      break;
    case DR_FSK:
      Serial.println(F("Datarate: FSK"));
      TX_INTERVAL = 180;
      break;
    default: Serial.print(F("Datarate Unknown Value: "));
      Serial.println(LMIC.datarate); TX_INTERVAL = 600;
      break;
  }
}
