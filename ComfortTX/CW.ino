typedef struct {
  char ch;
  byte bits;
} morse_t;

const morse_t MorseCodes[] = {
  {'A', B101},
  {'B', B11000},
  {'C', B11010},
  {'D', B1100},
  {'E', B10},
  {'F', B10010},
  {'G', B1110},
  {'H', B10000},
  {'I', B100},
  {'J', B10111},
  {'K', B1101},
  {'L', B10100},
  {'M', B111},
  {'N', B110},
  {'O', B1111},
  {'P', B10110},
  {'Q', B11101},
  {'R', B1010},
  {'S', B1000},
  {'T', B11},
  {'U', B1001},
  {'V', B10001},
  {'W', B1011},
  {'X', B11001},
  {'Y', B11011},
  {'Z', B11100},

  {'1', B101111},
  {'2', B100111},
  {'3', B100011},
  {'4', B100001},
  {'5', B100000},
  {'6', B110000},
  {'7', B111000},
  {'8', B111100},
  {'9', B111110},
  {'0', B111111},

//  {'"', B10101},
//  {'\u00c6', B10101},
  {0xc6, B10101},

  //{'Ä', B10101},
  {'Ø', B11110},
  //{'Ö', B11110},
  {'Å', B101101},
  {'Ü', B10011},

  {'?', B1001100},
  // {'|', B1001100},

  //http://riddlehelper.blogspot.com/2011/04/morse-code.html
  {'.', B1010101},
  {',', B1110011},
  {':', B1111000},
  {'-', B110001},
  {'\'',B1011110},
  {'/', B110010},
  {0,   0}
};

void doSnd(int len)
{
  #ifdef PIN_tone
    tone(PIN_tone, HzTone, len);
  #endif
}

byte decode(char letter) {
  for (byte n=0; char ch = MorseCodes[n].ch; n++) {
    if (letter == ch) 
      return MorseCodes[n].bits;
  }
  return 0;
}
