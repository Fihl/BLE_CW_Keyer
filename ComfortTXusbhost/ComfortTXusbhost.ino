/* ComfortTXusbhost
 *  
 * Author: Christen Fihl OZ1AAB
 * 
*/

#include "printf.h" //Installed library

#define doDebug true
String sendBuf = ""; 
volatile char sendBufKbd; //Set in USBkbd
bool doPrintInfo = true;
byte curSpeed=18;
char Farnsworth='0'; //'0'..'9'

void setup()
{
  Serial.begin(115200);
  while (!Serial);
  Serial.println("Start ComfordTXusbHost");
  BLE_setup();
  USBkbdSetup();
}

// DO NOT use LED_BUILTIN = 13 on RF-Nano 
void Blink() {
  //pinMode(LED_BUILTIN, OUTPUT);
  //digitalWrite(LED_BUILTIN, 1-digitalRead(LED_BUILTIN));
}

unsigned long next;

void loop() {
  USBkbdIdle();
  BLE_loop();

  if (next<millis()) {
    next = millis()+5000;
    sendBuf += "X";
  }

  if (Serial.available()) {
    while (Serial.available()) {
      char ch=Serial.read();
      if (ch>=' ')
        sendBuf += ch;
    }
  }
  
  if (sendBufKbd>=' ') 
    sendBuf += sendBufKbd;
  sendBufKbd = 0;

  if (doPrintInfo) {
    doPrintInfo = false;
    Serial.print("Speed:"); Serial.print(curSpeed);
    Serial.print(", Farnsworth:"); Serial.print(Farnsworth);
    Serial.print(", sendBuf:"); Serial.println(sendBuf);
  }
}
