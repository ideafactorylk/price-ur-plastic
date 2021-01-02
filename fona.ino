#include "Adafruit_FONA.h"

#define FONA_RX 2
#define FONA_TX 3
#define FONA_RST 12


char replybuffer[255];
#include <SoftwareSerial.h>
SoftwareSerial fonaSS = SoftwareSerial(FONA_TX, FONA_RX);
SoftwareSerial *fonaSerial = &fonaSS;
Adafruit_FONA fona = Adafruit_FONA(FONA_RST);
uint8_t readline(char *buff, uint8_t maxbuff, uint16_t timeout = 0);
uint8_t type;

void init_fona() {
  Serial.println(F("FONA SMS"));
  fonaSerial->begin(4800);
  if (! fona.begin(*fonaSerial)) {
    Serial.println(F("Couldn't find FONA"));
    while (1);
  }
  type = fona.type();
  Serial.println(F("FONA is OK"));
}

void send_sms (String num,String msg) {
  char sendto[12], message[100];
  flushSerial();
 num.toCharArray(sendto, 12);
 msg.toCharArray(message, 100);
 
  if (!fona.sendSMS(sendto, message)) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("Sent!"));
  }
}

void flushSerial() {
  while (Serial.available())
    Serial.read();
}
