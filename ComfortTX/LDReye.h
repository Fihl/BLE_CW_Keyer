
class LDReye { 
  public:
    LDReye();
    byte InPin;
    bool doDebug;
    void init(bool _doDebug, byte port);
    bool loop();
    int mean;
    int inp;
    bool output;
  private:
    int meanBuf[256]; 
    byte meanPtr; //0..255
    long int meanL;
};

LDReye::LDReye(){}

void LDReye::init(bool _doDebug, byte port)
{
  doDebug = _doDebug;
  pinMode (A0, INPUT_PULLUP);
  if (_doDebug) Serial.println("A0 Mean output");
}


bool LDReye::loop()
{
  inp = analogRead(InPin);
  meanL += (inp - meanBuf[meanPtr]);
  mean = meanL / 256;
  meanBuf[meanPtr++] = inp;
  output = inp < (mean-20);

  if (doDebug) {
    Serial.print(inp);       Serial.print(" ");
    Serial.print(mean);      Serial.print(" ");
    Serial.println(output * 30 + 200);     Serial.print(" ");
    Serial.println();
  }
  return output;
}
