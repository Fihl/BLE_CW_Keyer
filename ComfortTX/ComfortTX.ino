//ComfortTX.ino
/*
 * nrf24 Documentation at https://nRF24.github.io/RF24
 * https://github.com/keywish/keywish-nano-plus/tree/master/RF-Nano
 * 
 * Author: Christen Fihl OZ1AAB
 * 
 * Last step of CW chain into 0.5Watt QO-100
 * or CW keying output
 * Input from nrf24 or simple CW keying 
 * 
 *   The circuit:  
 *   - LDR connected from A0  to ground 
 *   
 * Info: https://www.deviceplus.com/arduino/nrf24l01-rf-module-tutorial/ 
 * https://forum.mysensors.org/topic/10327/rf-nano-nano-nrf24-for-just-3-50-on-aliexpress/3
 * PARIS:   http://www.kent-engineers.com/codespeed.htm
 * 
 */

#define doDebug 1
#define LDRsimulator 0

#include <SPI.h>
#include "printf.h" //Installed library
#include "RF24.h"
#include <Cth.h> //CopyThreads, real nice tool

#define BEACON false

//nRF24L01 transceiver
//pin # for the CE pin, and pin # for the CSN pin
RF24 radio(9,10);     //UNO, or nano With external antenna //Nano with nrf, Board: Nano, Normal bootloader
//RF24 radio(10,9);     //Nano, without external antenna DEN ER DØD !!
//RF24 radio(7,8);      //Den røde!!! (Old bootloader)
//RF24 radio(2,3);      //DUE, med nrf24l01 på ISP port (i bunden)

uint8_t RFaddress[] = "Z1aab";

// do NOT use LED_BUILTIN = 13 on RF-Nano. Does not work along with RF parts
#define KEY_LED   2
#define KEY_NPN   3
#define LDRpin    A0
#define LDRpinGND A1
#define LEDpin1   A4
#define LEDpin2   A5

#define ledRF   0
#define ledCW   1
#define ledCW2 ledCW //Use same led

#include "LDReye.h"
LDReye LDR;

#define maxBuf 30
char RXbuffer[maxBuf+1];
char TXbufferIdle[] = "P0.ComfordTX";
char TXbufferLDR[]  = "L0.LDRx."; // "." => "S" when simulated

char serBuf[80];

byte LEDs = B00; //B01 = Yellow, B10=Blue

void Blink(byte no) {
  LEDs = LEDs ^ (1 << no);
}

void doLEDs() {
  static byte BIT = 0;
  byte NEW = (BIT = 1-BIT)? LEDs&B10:LEDs&B01;
  pinMode(LEDpin1, OUTPUT); 
  pinMode(LEDpin2, OUTPUT); 
  digitalWrite(LEDpin1, NEW & B01);
  digitalWrite(LEDpin2, NEW & B10);
}

void setup() {
  Serial.begin(115200);
  while (!Serial); //Leonardo is slow
  if (doDebug) Serial.println("\nCW tx via BLErx"); 
  printf_begin();
  
  pinMode(LDRpinGND, OUTPUT); digitalWrite(LDRpinGND, 0); 
  pinMode(LDRpin, INPUT_PULLUP);
  LDR.LDRinit(false, LDRsimulator, LDRpin); //LDRdebug, Simulate, A0

  digitalWrite(KEY_LED, 0); pinMode(KEY_LED, OUTPUT); 
  digitalWrite(KEY_NPN, 0); pinMode(KEY_NPN, OUTPUT);

  if (!radio.begin()) 
  { Serial.println("Radio not found!!");
    LEDs = B11;
    while (1) { //STOP all
      Blink(ledRF);
      for (int n=0; n<250;n++) { doLEDs(); delay(1); }
    }
  }
  radio.setPALevel(RF24_PA_LOW);  //0..3 = RF24_PA_MIN, LOW, HIGH, MAX
  radio.setPayloadSize(maxBuf);     // default value is the maximum 32 bytes
  radio.openWritingPipe(RFaddress);
  if (!BEACON) radio.openReadingPipe(1, RFaddress); // using pipe 1, RX address of the receiving end
  //radio.openReadingPipe(2, 'a'); // using pipe 2 (Last byte)
  
  radio.startListening(); // put radio in RX mode
  
  if (doDebug) {
    printf_begin();               // needed only once for printing details
    radio.printPrettyDetails();   // (larger) function that prints human readable data
    //radio.printDetails();       // (smaller) function that prints raw register values
  }
  Scheduler.startLoop(LoopKeyer);
  Scheduler.startLoop(LoopIdle);
  Scheduler.startLoop(LoopLDR);
}

bool doIdleBeep;
bool doSendOk;
int speed_ms;  // 100 = 12wpm
int txBits;
byte Farnsworth;
volatile int msDIHcounter;
char TXbufferOk[4];

void loop() {
  doKeying();
  doLEDs();
  Scheduler.yield(); //delay(0);
    
  if (radio.available()) {
    Blink(ledRF);
    memset(RXbuffer,0,sizeof(RXbuffer));
    radio.read(&RXbuffer, maxBuf);     //get from FIFO
    //Serial.print("all RX: "); Serial.println(RXbuffer);

    if (TXbufferOk[1] == RXbuffer[1]) {
      //Serial.print("dublicate: "); Serial.println(RXbuffer);
      return;
    }
    
    if (txBits>1) { 
      Serial.print("busyRX: "); Serial.print(RXbuffer); Serial.println(" stop");
      return; 
    }
    
    if (RXbuffer[0] != 'T') return;
    TXbufferOk[0] = RXbuffer[0];
    TXbufferOk[1] = RXbuffer[1];
    
    // RX= one of...
    // TxxRf121100000011 = TX('1100000011', speed=12, F=Farnsworth)
    // TxxP24s = TX('s', speed=24, F=Farnsworth)
    // fx T2AP018c T5AP418E,  TxxRf121100000011
    
    if (RXbuffer[3] == 'P') 
      txBits = decode(toupper(RXbuffer[7]));
    else
    if (RXbuffer[3] == 'R')  {
      txBits=1;
      for (int n=7; n<maxBuf; n++) {      //TxxRf121100000011 => "1100000011"
        if (RXbuffer[n]==0) break;
        txBits *= 2;
        if (RXbuffer[n]=='1') txBits +=1;
      }
    } else
      return; //Unknown
    Farnsworth = RXbuffer[4] - '0';     //TxxRf121100000011 => 'f'
    if (Farnsworth>10) Farnsworth = 0;
    String speed = "";
    speed += (char)RXbuffer[5];         //TxxRf121100000011 => "12"
    speed += (char)RXbuffer[6];
    int speed_int = speed.toInt();
    if (speed_int<6) speed_int = 6;
    speed_ms = 1200 / speed_int;
    if (doDebug) {
      sprintf(serBuf, "RX: %s [Speed=%d, ms=%d, Farnsworth=%d]", RXbuffer, speed_int, speed_ms,Farnsworth);
      Serial.println(serBuf);
    }
    if (!txBits) { //Space etc
      Scheduler.delay(6*speed_ms);
      doSendOk = 1;
    }
    //for (delay(1); radio.available(); radio.read(&RXbuffer, maxBuf) ); //Flush
  }
  
  if (doIdleBeep) {   //hver sekund måske
    Blink(ledRF);
    doIdleBeep=0;
    if (TXbufferIdle[1]++ == '9') TXbufferIdle[1]='0';
    radio.stopListening();
    radio.write(&TXbufferIdle, sizeof(TXbufferIdle) );
    radio.startListening();
    if (doDebug) Serial.print("TX idle ");
    if (doDebug) Serial.println(TXbufferIdle);
  }
  
  if (doSendOk) {
    Blink(ledRF);
    doSendOk=0;
    radio.stopListening();
    radio.write(&TXbufferOk, 2);
    radio.write(&TXbufferOk, 2);
    radio.write(&TXbufferOk, 2);
    radio.startListening();
  }  
}

void LoopLDR() {  
  bool lastLDRstate=99;
  byte retry=0;
  for (;;) {
    Scheduler.delay(5);

    continue;   /////////////
    
    bool LDRstate = LDR.LDRpoll();
    if (TXbufferLDR[1]++ == '9') TXbufferLDR[1]='0';
    TXbufferLDR[6]=LDRstate+'0';
    if (LDRsimulator) TXbufferLDR[7]='S'; //Simulated
    radio.stopListening();
    if (!radio.write(&TXbufferLDR, sizeof(TXbufferLDR)))    //Anybody rx this?
      if (!radio.write(&TXbufferLDR, sizeof(TXbufferLDR)))  //Just somebody...
        Serial.println("Retried twise");
    radio.startListening();
    continue;


    if (lastLDRstate != LDRstate) {
      lastLDRstate = LDRstate;
      retry = (LDRsimulator)? 4:2;
      LEDs = LDRstate? LEDs | (1 << ledCW2): LEDs & ~(1 << ledCW2);
      if (TXbufferLDR[1]++ == '9') TXbufferLDR[1]='0';
      Blink(ledRF);
    }
    
    while (retry >= 1) {
      retry--;
      //char TXbufferLDR[]  = "L0.LDRx.";
      TXbufferLDR[6]=LDRstate+'0';
      if (LDRsimulator) TXbufferLDR[7]='S'; //Simulated
      radio.stopListening();
      if (!radio.write(&TXbufferLDR, sizeof(TXbufferLDR)))  //Anybody rx this?
        radio.write(&TXbufferLDR, sizeof(TXbufferLDR));
      radio.startListening();
      delay(retry);
      if (!retry) Serial.println(TXbufferLDR); //Only one line
    }
  
  }
}

void LoopKeyer() {
  while (txBits<=1) 
    Scheduler.delay(10);
  String cw = "";
  do {
    if (txBits & 1) cw="-"+cw; else cw="."+cw; 
    txBits = txBits/2;
  } while (txBits>1);
  if (doDebug) Serial.println(cw);
  for (byte n=0; n<cw.length(); n++) {
    int len = cw[n]=='-'?3*speed_ms:speed_ms;
    msDIHcounter = len / 10;
    Scheduler.delay(len+speed_ms); //interdih = 1
  }
  doSendOk = 1;
  Scheduler.delay(speed_ms* (2+Farnsworth)); //interchar = 3
  //test digitalWrite(KEY_LED, 1); delay(5); digitalWrite(KEY_LED, 0);
}

unsigned long nextMillisCW;
void doKeying(){
  if (nextMillisCW > millis()) return;
  nextMillisCW = millis()+10;
  byte curBit = 0;
  if (msDIHcounter) {
    msDIHcounter--;
    curBit = 1;
  }
  //if (txBits>1) Serial.print(curBit?"A":"B");
  LEDs = curBit? LEDs | (1 << ledCW): LEDs & ~(1 << ledCW);
  digitalWrite(KEY_LED, curBit);
  digitalWrite(KEY_NPN, curBit);
}

void LoopIdle() {
  for (;;) {
    if (BEACON) Scheduler.delay(10000); else Scheduler.delay(30000);
    doIdleBeep = true;
  }
}
