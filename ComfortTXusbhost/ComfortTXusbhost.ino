/* ComfortTXusbhost
 *  
 * Author: Christen Fihl OZ1AAB
 * 
*/

#include "printf.h" //Installed library
#include <Cth.h> //CopyThreads, real nice tool

#define doDebug false

String sendBuf = ""; 
volatile char sendBufKbd; //Set in USBkbd
bool doPrintInfo = true;
byte curSpeed=18;
int speed_ms = 1200 / 12;  // = 1200 / curSpeed;
byte Farnsworth; //0..9
int txBits;
int msDIHcounter;
String cw;
String unBuf;

//#define KEY_LED   2
#define KEY_NPN   3

void setup()
{
  Serial.begin(115200);
  while (!Serial);
  if (doDebug) Serial.println("Start ComfordTXusbHost");
  //digitalWrite(KEY_LED, 0); pinMode(KEY_LED, OUTPUT); 
  digitalWrite(KEY_NPN, 0); pinMode(KEY_NPN, OUTPUT);  
  BLE_setup();
  USBkbdSetup();
  Scheduler.startLoop(LoopKbd);
  Scheduler.startLoop(LoopKeying1mSec);
}

void doKeyCW() {
  for (byte n=0; n<cw.length(); n++) {
    int len = cw[n]=='-'?3*speed_ms:speed_ms;
    msDIHcounter = len;  // Handled in "Interrupt" controller part
    Scheduler.delay(len+speed_ms); //interdih = +1 dih
  }
  Scheduler.delay( (Farnsworth+2) *speed_ms); //interchar = 3
}

void LoopKbd() 
{
  Scheduler.delay(1);

  if (sendBuf != "") {
    char ch= sendBuf[0];
    sendBuf.remove(0,1);
    txBits = decode(toupper(ch)); //' '=> 0
    if (!txBits) {
      Scheduler.delay(7*speed_ms);
      return;
    }
    cw="";
    do {
      if (txBits & 1) cw="-"+cw; else cw="."+cw; 
      txBits = txBits/2;
    } while (txBits>1);
    //if (doDebug) Serial.println(cw);
    TXraw(cw);
    doKeyCW();
    return;
  }

  if (unBuf!="") {
    cw=unBuf;
    unBuf="";
    //if (doDebug) Serial.println(cw);
    TXraw(cw);
    doKeyCW();
    return;
  }
}

void LoopKeying1mSec()
{
  static unsigned long next1mSec;
  while (next1mSec > millis()) Scheduler.yield();
  next1mSec = millis()+1;

  byte curBit = 0;
  if (msDIHcounter) {
    msDIHcounter--;
    curBit = 1;
  }
  //digitalWrite(KEY_LED, curBit);
  digitalWrite(KEY_NPN, curBit);
  static byte decay;
  if (curBit) decay=120;
  if (!doDebug & decay>1) {
    static byte div10;
    div10++;
    if (div10 == 5) {
      div10=0;
      decay--;
      Serial.println(curSpeed+curBit);
    }
  }
}

// DO NOT use LED_BUILTIN = 13 on RF-Nano 
void Blink() {
  //pinMode(LED_BUILTIN, OUTPUT);
  //digitalWrite(LED_BUILTIN, 1-digitalRead(LED_BUILTIN));
}

void loop() {
  Scheduler.yield(); //delay(0);
  USBkbdIdle();

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

  if (doPrintInfo & doDebug) {
    doPrintInfo = false;
    Serial.print("Speed:"); Serial.print(curSpeed);
    Serial.print(", delayMs:"); Serial.print(speed_ms);
    Serial.print(", Farnsworth:"); Serial.print(Farnsworth);
    Serial.print(", sendBuf:"); Serial.println(sendBuf);
  }
}
