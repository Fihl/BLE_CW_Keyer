#include <string.h>

class LDReye { 
  public:
    LDReye();
    void LDRinit(bool _LDRdebug, bool _LDRsimulate, byte port);
    bool LDRpoll();
  private:
    int meanBuf[256]; 
    byte meanPtr; //0..255
    long int meanL;
    int mean;
    long timeKeeper;
    int inp;
    bool output;
    byte InPin;
    bool LDRdebug;
    bool LDRsimulate;
    int tresh;
};

LDReye::LDReye(){}

void LDReye::LDRinit(bool _LDRdebug, bool _LDRsimulate, byte port)
{
  LDRdebug = _LDRdebug;
  LDRsimulate = _LDRsimulate;
  pinMode (A0, INPUT_PULLUP);
  if (_LDRdebug) Serial.println("A0 Mean Tresh output");
}

String LDRtest0 = 
"111.1.111.1...111.111.1.111.......111.1.1...1.......111.111.111...111.111.1.1...1.111.111.111.111...1.111...1.111...111.1.1.1..............";
String LDRtest;

bool LDReye::LDRpoll()
{
  if (timeKeeper <= millis()) {
    if (LDRsimulate) {
      timeKeeper = millis()+100;
      if (LDRtest == "") LDRtest = LDRtest0;
      output = LDRtest[0]=='1'? true:false;
      LDRtest.remove(0,1);
    } else {
      timeKeeper = millis()+5;
      inp = analogRead(InPin);
      meanL += (inp - meanBuf[meanPtr]);
      mean = meanL / 256;
      meanBuf[meanPtr++] = inp;
      tresh = mean / 16;
      output = inp < (mean - tresh);
    } 
    if (LDRdebug) {
      Serial.print(inp);                Serial.print(" ");
      Serial.print(mean);               Serial.print(" ");
      Serial.print(mean-tresh);         Serial.print(" ");
      Serial.print(output * 30 + 200);  Serial.print(" ");
      Serial.println();
    }
  }
  return output;
}
