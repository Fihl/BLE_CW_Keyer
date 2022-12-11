//ComfortTX.ino
/*
 * nrf24 Documentation at https://nRF24.github.io/RF24
 * 
 * Author: Christen Fihl OZ1AAB
 * 
 * Last step of CW chain into 0.5Watt QO-100
 * or CW keying output
 * Input from nrf24 or simple CW keying 
 * 
 * Info: https://www.deviceplus.com/arduino/nrf24l01-rf-module-tutorial/ 
 * https://forum.mysensors.org/topic/10327/rf-nano-nano-nrf24-for-just-3-50-on-aliexpress/3
 * PARIS:   http://www.kent-engineers.com/codespeed.htm
 * 
 */

#include <SPI.h>
#include "printf.h" //Installeret library
#include "RF24.h"
#include <Cth.h> //CopyThreads, super god

// #define PIN_tone 2  
// #define HzTone 700

#define KEY_LED 2
#define KEY_NPN 3

#define doDebug 1

//nRF24L01 transceiver
//pin # for the CE pin, and pin # for the CSN pin
RF24 radio(9,10);     //UNO, or nano With external antenna //Nano with nrf, Board: Nano, Normal bootloader
//RF24 radio(10,9);     //Nano, without external antenna DEN ER DØD !!
//RF24 radio(7,8);      //Den røde!!! (Old bootloader)
//RF24 radio(2,3);      //DUE, med nrf24l01 på ISP port (i bunden)

uint8_t TXaddress[6] = "1aabT";
uint8_t RXaddress[6] = "1aabR";

#define SIZE 15
char RXbuffer[SIZE+1] = "A";
char TXbuffer[SIZE+1] = "Pa";

void setup() {
  Serial.begin(115200);
  while (!Serial); //Leonardo is slow
  Serial.println("CW tx via BLE"); 
  
  digitalWrite(LED_BUILTIN, 0); pinMode(LED_BUILTIN, OUTPUT); 
  digitalWrite(KEY_LED, 0); pinMode(KEY_LED, OUTPUT); 
  digitalWrite(KEY_NPN, 0); pinMode(KEY_NPN, OUTPUT);
  
  //digitalWrite(KEY_NPN, 1); digitalWrite(KEY_LED, 1); delay(25000); //testing
  //while (1) { digitalWrite(KEY_LED, 1-digitalRead(KEY_LED)); delay(250); }
  //while (1) { digitalWrite(KEY_NPN, 1-digitalRead(KEY_NPN)); delay(250); }
  
  if (!radio.begin()) {
    Serial.println("Radio not found!!");
    while (1) { //STOP
      digitalWrite(LED_BUILTIN, 1-digitalRead(LED_BUILTIN) );
      delay(250);
    }
  }
  radio.setPALevel(RF24_PA_LOW);  //0..3 = RF24_PA_MIN, LOW, HIGH, MAX
  radio.setPayloadSize(SIZE);     // default value is the maximum 32 bytes
  radio.openWritingPipe(TXaddress);
  radio.openReadingPipe(1, RXaddress); // using pipe 1, RX address of the receiving end

  radio.startListening(); // put radio in RX mode
  
  printf_begin();             // needed only once for printing details
  radio.printPrettyDetails(); // (larger) function that prints human readable data
  //radio.printDetails();       // (smaller) function that prints raw register values

  Scheduler.startLoop(LoopKeyer);
  Scheduler.startLoop(LoopIdle);
}

bool doBeep;
bool doSendOk;
int speed_ms;  // 100 = 12wpm
int txBits;
byte Farnsworth;
volatile int msDIHcounter;

void loop() {
  doKeying();
  Scheduler.delay(1);
  if (radio.available()) {
    memset(RXbuffer,0,sizeof(RXbuffer));
    radio.read(&RXbuffer, SIZE);     //get FIFO
    //radio.stopListening();
    //radio.write(&RXbuffer, 10); //Echo
    //radio.startListening();

    // RX= one of...
    // Rf121100000011 = TX('1100000011', speed=12, F=Farnsworth)
    // Tf24s = TX('s', speed=24, F=Farnsworth)
    if (RXbuffer[0] == 'T') 
      txBits = decode(toupper(RXbuffer[4]));
    else {
      txBits=1;
      for (int n=4; n<SIZE; n++) {
        if (RXbuffer[n]==0) break;
        txBits = txBits*2;
        if (RXbuffer[n]=='1') txBits +=1;
      }
    }
    Farnsworth = RXbuffer[1] - '0';
    if (Farnsworth>10) Farnsworth = 0;
    String speed = "";
    speed += (char)RXbuffer[2];
    speed += (char)RXbuffer[3];
    speed_ms = speed.toInt();    
    if (speed_ms<6) speed_ms = 6;
    speed_ms = 1200 / speed_ms;
    if (doDebug) {
      Serial.print("RX: ");
      Serial.print(RXbuffer);
      Serial.print(" ["); Serial.print(speed); Serial.print(',');
      Serial.print(speed_ms); Serial.print(',');
      Serial.print(Farnsworth);
      Serial.println(']');
    }
    if (!txBits) { //Space etc
      Scheduler.delay(6*speed_ms);
      doSendOk = 1;
    }
    for (delay(1); radio.available(); radio.read(&RXbuffer, SIZE) ); //Flush
  }
  
  if (doBeep) {   //hver sekund måske
    doBeep=0;
    if (doDebug) Serial.println("TX idle ");
    TXbuffer[0]='P';
    //TXbuffer[1]++; if (TXbuffer[1]>'z') TXbuffer[1]='a';
    radio.stopListening();
    radio.write(&TXbuffer, 1);
    radio.write(&TXbuffer, 1);
    radio.startListening();
  }
  
  if (doSendOk) {
    doSendOk=0;
    //Serial.print("TX OK ");
    radio.stopListening();
    TXbuffer[0]='O';
    radio.write(&TXbuffer, 1);
    radio.write(&TXbuffer, 1);
    radio.write(&TXbuffer, 1);
    radio.startListening();
  }
}

byte curBit;
void LoopIdle() {
  for (;;) {
    Scheduler.delay(30000);
    doBeep = true;
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

unsigned long nextMillis;
void doKeying(){
  if (nextMillis > millis()) return;
  nextMillis = millis()+10;
  curBit = 0;
  if (msDIHcounter) {
    msDIHcounter--;
    curBit = 1;
  }
  //if (txBits>1) Serial.print(curBit?"A":"B");
  digitalWrite(KEY_LED, curBit);
  digitalWrite(KEY_NPN, curBit);
}
