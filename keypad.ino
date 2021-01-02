#include <Keypad.h>

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

String get_num() {
  String num = "";
  while(true) {
    char key = kpd.getKey();
    if (key == '*') {
      break;
    }else if(key==NO_KEY){
    } else {
      //Serial.println(key);
      num += key;
    }
  }
  return num;
}
