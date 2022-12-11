/* BLEtx, ComfortTXusbhost
 *  
 * Author: Christen Fihl OZ1AAB
 * 
 * nrf24 Documentation at https://nRF24.github.io/RF24
 * Info: https://www.deviceplus.com/arduino/nrf24l01-rf-module-tutorial/ 
 * https://forum.mysensors.org/topic/10327/rf-nano-nano-nrf24-for-just-3-50-on-aliexpress/3
 * 
 */

#include <SPI.h>
#include "printf.h" //Installeret library
#include "RF24.h"

//nRF24L01 transceiver
//pin # for the CE pin, and pin # for the CSN pin
//RF24 radio(9,10);     //UNO, or nano With external antenna. //Nano with nrf, Board: Nano, Normal bootloader
//RF24 radio(10,9);     //Nano, without external antenna
//RF24 radio(7,8);      //Den røde!!! (Old bootloader)
RF24 radio(2,3);        //DUE, med nrf24l01 på ISP port (i bunden)

uint8_t TXaddress[6] = "1aabT";
uint8_t RXaddress[6] = "1aabR";

#define SIZE 15
char RXbuffer[SIZE+1];

void BLE_setup() {
  if (!radio.begin()) {
    Serial.println("Radio not found!!");
    pinMode(LED_BUILTIN, OUTPUT);
    while (1) { //STOP
      digitalWrite(LED_BUILTIN, 1-digitalRead(LED_BUILTIN) );
      delay(100);
    }
  }
  radio.setPALevel(RF24_PA_LOW);  //0..3 = RF24_PA_MIN, LOW, HIGH, MAX
  radio.openWritingPipe(RXaddress);
  radio.setPayloadSize(SIZE);     // default value is the maximum 32 bytes
  radio.openWritingPipe(RXaddress);
  radio.openReadingPipe(1, TXaddress); // using pipe 1, RX address of the receiving end
  radio.startListening();

  if (0) {
    printf_begin();             // needed only once for printing details
    radio.printPrettyDetails(); // (larger) function that prints human readable data
    //radio.printDetails();       // (smaller) function that prints raw register values
  }
}

int txOk = 0; //-3000..0 = busy, else 1

unsigned long nextMillis;
void BLE_loop() {
  pollRX();

  if (nextMillis > millis()) return;
  nextMillis = millis()+1;
  
  if (txOk < 1) txOk++;
  
  if (txOk==1 & sendBuf != "") {
    txOk=-5000;
    TXchar(sendBuf[0]);
    sendBuf.remove(0,1);
  }
}

void pollRX()
{
  while (radio.available()) {
    memset(RXbuffer,0,sizeof(RXbuffer));
    radio.read(&RXbuffer, SIZE);
    if (RXbuffer[0] == 'O') {
      delay(2); //remove dublicates
      txOk = 1;
      //Serial.println("OK");
    }
  }
}

// T f ss c   //T=Transmit, f=Farnsworth bits('0'..'9'), ss=wpm, c = character (simple ones)
void TXchar(char ch) {
  txOk=-5000;
  if (curSpeed<10) curSpeed=10; //format error...
  String cmd = "T";
  cmd += Farnsworth + String(curSpeed) + String(ch);
  radio.stopListening();
  radio.write(&cmd[0], cmd.length()+1 );
  radio.write(&cmd[0], cmd.length()+1 );
  radio.write(&cmd[0], cmd.length()+1 );
  radio.startListening();
  Serial.print("TXchr: <"); Serial.print(cmd); Serial.println(">");
}

// R f ss bb  //R=RawBits,  f=Farnsworth bits('0'..'9'), ss=wpm, bb=0x0082 raw bits
void TXraw(String raw) {
  txOk=-3000;
  if (curSpeed<10) curSpeed=10; //format error...
  String cmd = "R";
  cmd += Farnsworth + String(curSpeed) + raw; //String(raw, BIN);
  radio.stopListening();
  radio.write(&cmd[0], cmd.length()+1 );
  radio.write(&cmd[0], cmd.length()+1 );
  radio.write(&cmd[0], cmd.length()+1 );
  radio.startListening();
  Serial.print("TXraw: <"); Serial.print(cmd); Serial.println(">");
}
