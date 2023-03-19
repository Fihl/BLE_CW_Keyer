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
#include "printf.h" //Installed library
#include "RF24.h"

//nRF24L01 transceiver
//pin # for the CE pin, and pin # for the CSN pin
RF24 radio(9,10);     //UNO, or nano With external antenna. //Nano with nrf, Board: Nano, Normal bootloader
//RF24 radio(10,9);     //Nano, without external antenna
//RF24 radio(7,8);      //Den røde!!! (Old bootloader)
//RF24 radio(2,3);        //DUE, med nrf24l01 på ISP port (i bunden)

uint8_t RFaddress[] = "Z1aab";
#define maxBuf 30

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
  radio.setPayloadSize(maxBuf);     // default value is the maximum 32 bytes
  radio.openWritingPipe(RFaddress);
  radio.openReadingPipe(1, RFaddress); // using pipe 1, RX address of the receiving end
  radio.startListening();

  if (doDebug) {
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
    TXchar(sendBuf[0]);
    sendBuf.remove(0,1);
  }
}

void pollRX()
{
  while (radio.available()) {
    char RXbuffer[maxBuf+1];
    memset(RXbuffer,0,sizeof(RXbuffer));
    radio.read(&RXbuffer, maxBuf);
    Serial.print("pollRX: "); Serial.println(RXbuffer);
    if (RXbuffer[0]=='T') {
      delay(2); //remove dublicates
      txOk = 1;
      //Serial.println("OK");
    }
  }
}

//sprintf https://www.programmingelectronics.com/sprintf-arduino/

char ser = '0';

// Txx   Pf ss c   //Txx=Transmit,ser,crc, f=Farnsworth bits('0'..'9'), ss=wpm, c = character (simple ones)
void TXchar(char ch) {
  txOk=-3000;
  byte chk = 65; ///!!!!!!!!
  //if (curSpeed<10) curSpeed=10; //format error...
  char buff[30];
  // Txx   Pf ss c   //Txx=Transmit,ser,crc, f=Farnsworth bits('0'..'9'), ss=wpm, c = character (simple ones)
  sprintf(buff, "T%c%cP%c%.2d%c", ser,chk,Farnsworth,curSpeed,ch);
  if (ser++ == '9') ser='0'; //'0'..'9'
  radio.stopListening();
  radio.write(&buff, strlen(buff) );
  radio.write(&buff, strlen(buff) );
  radio.write(&buff, strlen(buff) );
  radio.startListening();
  Serial.print(strlen(buff)); Serial.print("-TXchr: <"); Serial.print(buff); Serial.println(">");
}

// R f ss bb  //R=RawBits,  f=Farnsworth bits('0'..'9'), ss=wpm, bb=0x0082 raw bits
void TXraw(String raw2) 
{
  txOk=-3000;
  if (curSpeed<10) curSpeed=10; //format error...
  byte chk = 65; ///!!!!!!!!
  char buff[30];
  // Txx   Pf ss c   //Txx=Transmit,ser,crc, f=Farnsworth bits('0'..'9'), ss=wpm, c = character (simple ones)
  char raw[20]; raw2.toCharArray(raw, 20);
  sprintf(buff, "T%c%cR%c%.2d%s", ser,chk,Farnsworth,curSpeed,&raw);
  if (ser++ == '9') ser='0'; //'0'..'9'
  radio.stopListening();
  radio.write(&buff, strlen(buff) );
  radio.write(&buff, strlen(buff) );
  radio.write(&buff, strlen(buff) );
  radio.startListening();
  Serial.print(">>>>>"); Serial.println(raw); 
  Serial.print(strlen(buff)); Serial.print("-TXraw: <"); Serial.print(buff); Serial.println(">");
}
