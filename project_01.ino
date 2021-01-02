#include <Keypad.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>
#include<PriorityQueue.h>

#include "Adafruit_FONA.h"
#define FONA_RX 2
#define FONA_TX 3
#define FONA_RST 1
// this is a large buffer for replies
uint8_t type;
//char char_array[32];
int responceWaitTime = 60000;

/////////////////////////////////Numbpad conf
const byte rows = 4;
const byte cols = 4;
char keys[rows][cols] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[rows] = {11, 10, 9, 8};
byte colPins[cols] = {7, 6, 5, 4};
Keypad kpd = Keypad(makeKeymap(keys), rowPins, colPins, rows, cols);

//////////////////////////////I2C lcd config
LiquidCrystal_I2C lcd(0x27, 16, 2);

///////////////////////////////Proximity sensor
const byte proxy = 12;

///////////////////////////////Motor control pin
const byte motorA = A0;
const byte motorB = A1;

////////////////////////////////Indicate LED
const byte ledR = 2;
const byte ledG = 3;

//////////////////////////
const int listSize = 20;
int pCount[listSize];
String pNum[listSize];
int pointer = 0;


#include <SoftwareSerial.h>
SoftwareSerial fonaSS = SoftwareSerial(FONA_TX, FONA_RX);
SoftwareSerial *fonaSerial = &fonaSS;

Adafruit_FONA fona = Adafruit_FONA(FONA_RST);
void setup() {
  while (!Serial);


  Serial.begin(115200);
  Serial.println(F("FONA basic test"));
  Serial.println(F("Initializing....(May take 3 seconds)"));

  fonaSerial->begin(4800);
  if (! fona.begin(*fonaSerial)) {
    Serial.println(F("Couldn't find FONA"));
    while (1);
  }
  type = fona.type();
  Serial.println(F("FONA is OK"));
  Serial.print(F("Found "));
  switch (type) {
    case FONA800L:
      Serial.println(F("FONA 800L")); break;
    case FONA800H:
      Serial.println(F("FONA 800H")); break;
    case FONA808_V1:
      Serial.println(F("FONA 808 (v1)")); break;
    case FONA808_V2:
      Serial.println(F("FONA 808 (v2)")); break;
    case FONA3G_A:
      Serial.println(F("FONA 3G (American)")); break;
    case FONA3G_E:
      Serial.println(F("FONA 3G (European)")); break;
    default:
      Serial.println(F("???")); break;
  }

  // Print module IMEI number.
  char imei[15] = {0}; // MUST use a 16 character buffer for IMEI!
  uint8_t imeiLen = fona.getIMEI(imei);
  if (imeiLen > 0) {
    Serial.print("Module IMEI: "); Serial.println(imei);
  }





  lcd.init();
  lcd.backlight();
  delay(10000);
}





void loop() {

  Serial.print(F("FONA> "));
  while (! Serial.available() ) {
    if (fona.available()) {
      Serial.write(fona.read());
    }
  }
  String telNo = "";
  lcd.clear();
  lcdPrint("*** Welcome! ***", 0, 0);
  lcdPrint("Enter phone no", 1, 1);
  telNo = getFromPad(10000);
  while (telNo.length() != 10) {
    lcd.clear();
    lcdPrint("Enter valid no", 1, 0);
    lcdPrint("Ex. 07XXXXXXXX", 1, 1);
    telNo = getFromPad(10000);
  }

  byte bottleCount = 0;
  lcd.clear();
  lcdPrint("Insert bottles", 1, 0);

  bottleCount = getBottle();

  char key;
  lcd.clear();
  lcdPrint("You added ", 0, 0);
  lcdPrint(String(bottleCount), 10, 0);
  lcdPrint("bottles", 9, 1);
  do {
    key = kpd.getKey();
  } while (!(key != NO_KEY && key == '*'));

  insert(bottleCount, telNo);
  delay(5000);
  printList(pCount, pNum);

  int str_len = telNo.length() + 1;
  char char_array[32];
  telNo.toCharArray(char_array, 32);
  Serial.println("Sending reponse./..");
  Serial.println(char_array);

 if (!fona.sendSMS(char_array, "I got your text!")) {
        Serial.println(F("Failed"));
      } else {
        Serial.println(F("Sent!"));
      }
  delay(5000);


  lcdPrint("Thank you!", 3, 0);
  lcdPrint("** Come again **", 0, 1);
  lcd.clear();


}

/////////////////////////// Method area

void lcdPrint(String text, int loc, int line) { //////////////Display on lcd
  lcd.setCursor(loc, line);
  lcd.print(text);
}

String getFromPad(int waitTime) { ////////////////////Get String from numberpad
  String telNo = "";
  responceWaitTime = waitTime;
  startTime = millis();
  while (true) {
    if (millis() - startTime < responceWaitTime) { //
      char key = kpd.getKey();
      if (key != NO_KEY) {
        switch (key) {
          case '*': return telNo;
            break;
          case '#': telNo = "";
            lcd.clear();
            lcdPrint("Phone number:", 0, 0);
            break;
          default : telNo += key;
            lcd.clear();
            lcdPrint("Phone number:", 0, 0);
            lcdPrint(telNo, 3, 1);
            startTime = millis();
        }
      }
    } else {
      loop();
    }
  }
}

void indLed(char val) { ///////////////////////LED Indicators
  switch (val) {
    case 'R': digitalWrite(ledR, HIGH);
      digitalWrite(ledG, LOW); break;
    case 'G': digitalWrite(ledR, LOW);
      digitalWrite(ledG, HIGH); break;
    default : digitalWrite(ledR, LOW);
      digitalWrite(ledG, LOW);
  }
}

byte getBottle() { ///////////////////////////Get bottle one by one
  motorRotate('O');
  delay(400);
  motorRotate('N');
  indLed('G');
  byte count = 0;
  while (true) {
    if (kpd.getKey() == '*') {
      indLed('N');
      motorRotate('C');
      delay(400);
      motorRotate('N');
      return count;
    } else if (digitalRead(proxy) == LOW) { // get bottle
      count += 1;
      while (digitalRead(proxy) == LOW) {
        delay(50);
      }
      lcd.clear();
      lcdPrint("Insert bottles", 1, 0);
      lcdPrint("Count:", 0, 1);
      lcdPrint( String(count), 8, 1);
      delay(1500);
    }
  }
}

void motorRotate(char val) { ///////////control motor
  switch (val) {
    case 'O': analogWrite(motorA, 150);
      lcdPrint("Opened", 0, 1);
      analogWrite(motorB, 0); break;
    case 'C': analogWrite(motorA, 0);
      lcdPrint("Closed", 0, 1);
      analogWrite(motorB, 150); break;
    default : analogWrite(motorA, 0);
      analogWrite(motorB, 0);
  }
}



//////////////////// name list

void insertionSort(int arr[], int n, String arrs[])
{
  int i, key, j;
  String keys;
  for (i = 1; i < n; i++)
  {
    key = arr[i];
    keys = arrs[i];
    j = i - 1;
    while (j >= 0 && arr[j] < key)
    {
      arr[j + 1] = arr[j];
      arrs[j + 1] = arrs[j];
      j = j - 1;
    }
    arr[j + 1] = key;
    arrs[j + 1] = keys;
  }
}

void insert(int count, String num)
{
  int found = -1;
  for (int i = 0; i < pointer + 1; i++) {
    if (pNum[i] == num) {
      found = i;
      break;
    }
  }

  if (found == -1) {
    pNum[pointer] = num;
    pCount[pointer] = count;
    pointer = pointer + 1;
    insertionSort(pCount, listSize, pNum);

  } else {
    pCount[found] += count;
    insertionSort(pCount, listSize, pNum);
  }
}


String printList(int c[], String num[])
{
  Serial.println("\n\n\n\n\n\n\n\n\n\n\n");
  Serial.print("Number");
  Serial.println("\t\t Bottle count ");
  for (int i = 0; i < pointer; i++) {
    Serial.print(num[i]);
    Serial.print("\t\t ");
    Serial.print(c[i]);
    Serial.print("\n");
  }
  return " ";
}
