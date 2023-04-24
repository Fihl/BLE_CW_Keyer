
/*
 *  When using gravitech board for Nano: 
 *  https://www.mouser.dk/ProductDetail/992-USBHOST-4NANO 
 *  https://www.gravitech.us/usadforarna.html
 *  Do change UsbCode.h   ... 
 *  typedef MAX3421e<P10, P9> MAX3421E; // Official Arduinos (UNO, Duemilanove, Mega, 2560, Leonardo, Due etc.), Intel Edison, Intel Galileo 2 or Teensy 2.0 and 3.x
 *  => typedef MAX3421e<P8, P2> MAX3421E; // gravitech board, from Mouser
**/

#include <hidboot.h>
#include <usbhub.h>
#include <SPI.h>

class KbdRptParser : public KeyboardReportParser
{
    void PrintKey(uint8_t m, uint8_t key);
    void OnKeyDown  (uint8_t mod, uint8_t key);
    void myKeyPressed(uint8_t m, int key, char ch);
};

USB Usb;
HIDBoot<USB_HID_PROTOCOL_KEYBOARD> HidKeyboard(&Usb);
KbdRptParser Prs;

void USBkbdSetup() 
{
  if (Usb.Init() == -1)
    Serial.println("OSC did not start.");
  HidKeyboard.SetReportParser(0, &Prs);
}
void USBkbdIdle() 
{
  Usb.Task();
}

void KbdRptParser::PrintKey(uint8_t m, uint8_t key)
{
  MODIFIERKEYS mod;
  *((uint8_t*)&mod) = m;
  Serial.print("Key ");
  Serial.print((mod.bmLeftCtrl   ) ? "C" : " ");
  Serial.print((mod.bmLeftShift  ) ? "S" : " ");
  Serial.print((mod.bmLeftAlt    ) ? "A" : " ");
  Serial.print((mod.bmLeftGUI    ) ? "G" : " ");
  Serial.print(" <");
  //Serial.print(key,HEX);
  PrintHex<uint8_t>(key, 0x80);
  Serial.print("> ");
  Serial.print((mod.bmRightCtrl  ) ? "C" : " ");
  Serial.print((mod.bmRightShift ) ? "S" : " ");
  Serial.print((mod.bmRightAlt   ) ? "A" : " ");
  Serial.print((mod.bmRightGUI   ) ? "G" : " ");
  Serial.print("  ");
};

void KbdRptParser::OnKeyDown(uint8_t m, uint8_t key)
{
  PrintKey(m, key);
  char ch = OemToAscii(m, key);
  myKeyPressed(m,key,ch);
  //if (ch>' ') Serial.println(ch); 
}


void KbdRptParser::myKeyPressed(uint8_t m, int key, char ch) 
{
  ch = toupper(ch);
  MODIFIERKEYS mod;
  *((uint8_t*)&mod) = m;
  Blink();
  Serial.print("Key:");
  Serial.print(key);
  Serial.print(", Mod:");
//  Serial.print(mod);
  Serial.print(" => ");

  if (mod.bmLeftCtrl) Serial.print("L-Ctrl ");
  if (mod.bmLeftShift) Serial.print("L-Shift ");
  if (mod.bmLeftAlt) Serial.print("LeftAlt ");

  if (mod.bmRightCtrl) Serial.print("R-Ctrl ");
  if (mod.bmRightShift) Serial.print("R-Shift ");
  if (mod.bmRightAlt) Serial.print("R-Alt ");
  if (mod.bmRightCtrl) {
    byte newSpeed=0;
    if (key==40) newSpeed=8;            //'0' => 10 .  ??
    else if (key==39) newSpeed=10;      //'0' => 10
    else newSpeed = (key-30)*2 + 12;    //'1'..'9' => 11..19
    if (newSpeed>=6 & newSpeed<=30)
      curSpeed = newSpeed;
    speed_ms = 1200 / curSpeed;
    Serial.print(", speed: "); Serial.println(curSpeed);
    return;
  }
  if (mod.bmLeftCtrl) {
    doPrintInfo = true;
    switch (ch) {
    case 'Q': Farnsworth=0; break;
    case 'W': Farnsworth=2; break;
    case 'E': Farnsworth=4; break;
    case 'R': Farnsworth=6; break;
    case 'T': Farnsworth=8; break;
    case 'Y': Farnsworth=9; break;
    case '7': unBuf="--......--"; break; // 7+3 73
    }
  }
  if (ch==0) {
    doPrintInfo = true;
    switch (key) {
      //ESC
      case 41: sendBuf = ""; break;
      //F5..F8
      case 62: sendBuf += " CQ TEST CQ TEST DE OZ1AAB OZ1AAB "; break;
      case 63: sendBuf += " CQ TEST TEST DE OZ1AAB OZ1AAB "; break;
      case 64: sendBuf = "eeeee"; break; ///??????
      case 65: sendBuf = " e e e e e "; break;
      //F9..F12
      case 66: unBuf="--......--"; break; // 7+3 _73_
      case 67: unBuf="-...-.-"; break; // _BK_
    }
  }
  if (m) {
    doPrintInfo = true;
  } else {
    if (ch >= ' ') {
      Serial.println(ch); //ASCII translation 
      //sendBufKbd = ch;
      sendBuf += ch;
      doPrintInfo = true;
      return;
    }
  }
  Serial.println();
}
