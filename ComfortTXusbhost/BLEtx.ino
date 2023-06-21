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
  radio.setPALevel(RF24_PA_HIGH);  //0..3 = RF24_PA_MIN, LOW, HIGH, MAX
  radio.setPayloadSize(maxBuf);     // default value is the maximum 32 bytes
  radio.openWritingPipe(RFaddress);
  radio.openReadingPipe(1, RFaddress); // using pipe 1, RX address of the receiving end
  radio.startListening();

  printf_begin();               // needed only once for printing details
  if (doDebug) {
    radio.printPrettyDetails(); // (larger) function that prints human readable data
    //radio.printDetails();       // (smaller) function that prints raw register values
  }
}

char ser = '0';

//sprintf https://www.programmingelectronics.com/sprintf-arduino/

void TXraw(String raw2) 
{
  byte chk = 65; //Unused..
  char buff[30];
  // Txx   CW ss cccc   //Txx=Transmit,ser,crc,CW,ss,bits('.' / '-'), ss=wpm, c = character (simple ones)
  char raw[20]; raw2.toCharArray(raw, 20);
  if (ser++ == '9') ser='0'; //'0'..'9'
  sprintf(buff, "T%c%cCW%.2d%s", ser,chk,curSpeed,&raw);
  radio.stopListening();
  for (byte n=0; n<3; n++) {
    delay(1);
    radio.write(&buff, strlen(buff) );
  }
  radio.startListening();
  if (doDebug) { Serial.print("TXraw: <"); Serial.print(buff); Serial.println(">"); }
}
