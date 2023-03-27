
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
  InPin = port;
  pinMode (InPin, INPUT_PULLUP);
  if (_LDRdebug) Serial.println("A0 Mean Tresh output");
}

const char PROGMEM LDRtest0[] = // VV CQ DE OZ1AAB (just for test)
".1.1.1.111...1.1.1.111.......111.1.111.1...111.111.1.111.......111.1.1...1.......111.111.111...111.111.1.1...1.111.111.111.111...1.111...1.111...111.1.1.1.";
String LDRtest;

int speedDelay = 100; // alt between 50&100
bool LDReye::LDRpoll()
{
  if (timeKeeper <= millis()) {
    if (LDRsimulate) {
      timeKeeper = millis()+speedDelay;
      if (LDRtest == "") { 
        //LDRtest = LDRtest0;
        for (byte k = 0; k < strlen_P(LDRtest0); k++) {
          char myChar = pgm_read_byte_near(LDRtest0 + k);
          LDRtest += myChar;
        }
        
        //LDRtest = ".1.1.1.111..........111.111.1.1........1."; //TWE
        //no alternate speed speedDelay = 150-speedDelay;
        
        timeKeeper = millis()+10000; //Long delay.  First byte already send!
      }
      output = LDRtest[0]=='1'? true:false;
      LDRtest.remove(0,1);
      Serial.print(output); Serial.print(", "); Serial.println(LDRtest);
    } else {
      timeKeeper = millis()+5;
      inp = analogRead(InPin);
      meanL += (inp - meanBuf[meanPtr]);
      mean = meanL / 256;
      meanBuf[meanPtr++] = inp;
      tresh = mean / 16;
      output = inp < (mean - tresh);
      if (LDRdebug) {
        Serial.print(inp);                Serial.print(" ");
        Serial.print(mean);               Serial.print(" ");
        Serial.print(mean-tresh);         Serial.print(" ");
        Serial.print(output * 30 + 200);  Serial.print(" ");
        Serial.println();
      }
    } 
  }
  return output;
}
