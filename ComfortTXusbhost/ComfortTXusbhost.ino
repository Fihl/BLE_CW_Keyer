/* ComfortTXusbhost
 *  
 * Author: Christen Fihl OZ1AAB
 * 
 * http://www.arduino.cc/en/Tutorial/KeyboardController
 * https://www.arduino.cc/reference/en/libraries/usbhost/
 * 
*/

#include <KeyboardController.h>

USBHost usb;
KeyboardController keyboard(usb);

String sendBuf = ""; 
volatile char sendBufKbd;
bool doPrintInfo = true;
byte curSpeed=18;
char Farnsworth='0'; //'0'..'9'

void setup()
{
  Serial.begin(115200);
  while (!Serial);
  Serial.println("Start");
  pinMode(LED_BUILTIN, OUTPUT);
  BLE_setup();
}

// Intercepts key press
void keyPressed() {
  digitalWrite(LED_BUILTIN, 1-digitalRead(LED_BUILTIN));
  int key = keyboard.getOemKey(); //The OEM code associated with the key
  Serial.print("Key:");
  Serial.print(key);
  int mod = keyboard.getModifiers(); //bits with the modifiers-keys
  Serial.print(", Mod:");
  Serial.print(mod);
  Serial.print(" => ");

  if (mod & LeftCtrl) Serial.print("L-Ctrl ");
  if (mod & LeftShift) Serial.print("L-Shift ");
  if (mod & Alt) Serial.print("Alt ");
  if (mod & LeftCmd) Serial.print("L-Cmd ");
  if (mod & RightCtrl) Serial.print("R-Ctrl ");
  if (mod & RightShift) Serial.print("R-Shift ");
  if (mod & AltGr) Serial.print("AltGr ");
  if (mod & RightCmd) Serial.print("R-Cmd ");
  char chRaw = toupper(keyboard.getKey());
  char ch = toupper(chRaw);
  if (mod & RightCtrl) {
    if (key==40) curSpeed=10;     //'0' => 10
    else curSpeed = key-31 + 11;  //'1'..'9' => 11..19
    Serial.print(curSpeed);
    if (curSpeed<10) curSpeed=10;
    Serial.print(", speed: "); Serial.println(curSpeed);
    exit;
  }
  if (mod & LeftCtrl) {
    doPrintInfo = true;
    switch (ch) {
    case 'Q': Farnsworth='0'; break;
    case 'W': Farnsworth='2'; break;
    case 'E': Farnsworth='4'; break;
    case 'R': Farnsworth='6'; break;
    case 'T': Farnsworth='8'; break;
    case 'Y': Farnsworth='9'; break;
    case '7': TXraw("1100000011"); break; // 7+3 exit;    //TXraw(B111<<8 + B00000011); // 7+3 
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
      case 65: sendBuf = " e e e e e "; break;
      //F9..F12
      case 66: TXraw("1100000011"); break;  //_73_
      case 67: TXraw("1000101"); break;     //_BK_
    }
  }
  if (mod != 0) {
    doPrintInfo = true;
  } else {
    if (ch >= ' ') {
      Serial.write(ch); //ASCII translation 
      Serial.println(" <<<<");
      sendBufKbd = ch;
    }
  }
}

void loop() {
  usb.Task();
  BLE_loop();

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
