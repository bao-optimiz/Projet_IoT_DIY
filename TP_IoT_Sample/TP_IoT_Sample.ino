#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>
#include "Device_Keys.h"

// Schedule the transmission every 
unsigned int TX_INTERVAL = 10; //seconds

static osjob_t sendjob;
static float Vcc;

// ---------------------------------------------- //
//      Read the Voltage regulated from TIC       //
long readVcc() {
  long result;
  // Read 1.1V reference against AVcc
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Convert
  while (bit_is_set(ADCSRA, ADSC));
  result = ADCL;
  result |= ADCH << 8;
  result = 1126400L / result;
  return result; // Return Vcc in mV
}
// ---------------------------------------------- //

// ---------------------------------------------- //
//   Update new values from TIC for the next TX   //
void updateParameters() {
  Vcc = (int)(readVcc()); // returns Vcc in mVolt for LoKy decoder
  Serial.print("Vcc = "); Serial.print(Vcc/1000); Serial.println(" V");
}
// ---------------------------------------------- //

// ---------------------------------------------- //
//              LoRaWAN events (LMiC)
void onEvent (ev_t ev) {
  Serial.print(os_getTime()); Serial.print(": ");
  switch (ev) {
    case EV_SCAN_TIMEOUT: Serial.println(F("EV_SCAN_TIMEOUT")); break;
    case EV_BEACON_FOUND: Serial.println(F("EV_BEACON_FOUND")); break;
    case EV_BEACON_MISSED: Serial.println(F("EV_BEACON_MISSED")); break;
    case EV_BEACON_TRACKED: Serial.println(F("EV_BEACON_TRACKED")); break;
    case EV_JOINING: Serial.println(F("EV_JOINING")); break;      
    case EV_JOINED: Serial.println(F("EV_JOINED")); LMIC_setLinkCheckMode(0); break;
    case EV_JOIN_FAILED: Serial.println(F("EV_JOIN_FAILED")); LMiC_Startup(); break;
    case EV_REJOIN_FAILED: Serial.println(F("EV_REJOIN_FAILED")); LMiC_Startup(); break; 
    case EV_LOST_TSYNC: Serial.println(F("EV_LOST_TSYNC")); break;
    case EV_RESET: Serial.println(F("EV_RESET")); break;
    case EV_RXCOMPLETE: Serial.println(F("EV_RXCOMPLETE")); break;
    case EV_LINK_DEAD: Serial.println(F("EV_LINK_DEAD")); break;
    case EV_LINK_ALIVE: Serial.println(F("EV_LINK_ALIVE")); break;
    
    case EV_TXCOMPLETE:
      Serial.println(F("EV_TXCOMPLETE"));
      if (LMIC.txrxFlags & TXRX_ACK) Serial.println(F("Received ack"));
      if (LMIC.dataLen) { Serial.print(F("Received ")); Serial.print(LMIC.dataLen); Serial.println(F(" bytes.")); }
      delay(50);
      os_setTimedCallback(&sendjob, os_getTime() + ms2osticks(TX_INTERVAL*1000), do_send);
      break;

    default: Serial.print(F("Unknown event: ")); Serial.println((unsigned) ev);
    break;
  }
}
// ---------------------------------------------- //


// ---------------------------------------------- //
//      Send the encrypted LoKy packet to TTN     //
void do_send(osjob_t* j) {
  // Check if there is not a current TX/RX job running
  if (LMIC.opmode & OP_TXRXPEND) {Serial.println(F("OP_TXRXPEND, not sending"));}
  else {
    updateParameters();
    // Formatting for uca decoder on TTN
    int       v = Vcc;
    unsigned char data[3];  // Change size of the packet here
    data[0] = 0x0;
    data[1] = v >> 8;
    data[2] = v & 0xFF;
    
    LMIC_setTxData2(1, data, sizeof(data), 0);
    Serial.println(F("Packet queued !!!"));
  }
  // Next TX is scheduled after TX_COMPLETE event.
}
// ---------------------------------------------- //


// ---------------------------------------------- //
//      Called from setup to activate LoKy        //
void LMiC_Startup() {
  os_init();
  LMIC_reset(); LMIC_setLinkCheckMode(1); LMIC_setAdrMode(1); // Reset the MAC state
  LMIC_setClockError(MAX_CLOCK_ERROR * 1 / 100); // Increase window time for clock accuracy problem
  LMIC_startJoining(); // Join the network, sending will be started after the event "Joined"
}
// ---------------------------------------------- //

// ---------------------------------------------- //
//                   LoKy setup                   //
void setup() {
  Serial.begin(9600);
  Serial.println("");
  Serial.println("*-____________________-*");
  Serial.println("  .::LoRa (Re)start::.");
  delay(100);
  LMiC_Startup();       // LMiC init
  do_send(&sendjob);    // OTAA start
}
// ---------------------------------------------- //

//*     .::Fixed LOOP - Do NOT interfered::.     *//
void loop() {os_runloop_once();}
