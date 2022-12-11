#include <avr/pgmspace.h>
// #include <avr/io.h>
#include <stdlib.h>
#include <string.h>

#define u08 unsigned char

#define u16 unsigned int
#define s16 signed int

#define u32 unsigned long int
#define s32 signed long int

#include <string.h>

void tmDiff2s(char* buf10, unsigned long tm1, unsigned long tm2)
{
  u16 i = tm1>tm2? tm1-tm2:tm2-tm1;
  u2s(&buf10[1],i);
  buf10[0]='<';
  iStrAdd(buf10, ">", 10);
}

void i2s(char* buf, s16 i)
{
  itoa(i,buf,10);
}
void u2s(char* buf, u16 i)
{
  utoa(i,buf,10);
}

void iStrCopy(char* dst, char* src, u16 max)  //max = exact [] size
{
  while ( --max > 0)
    if ( (*dst++ = *src++) == 0) return;
  *dst=0;
}

void iStrCopy_P(char* dst, const char* src, u16 max)  //max = exact [] size
{
  while ( --max > 0)
    if ( (*dst++ = pgm_read_byte(src++)) == 0) return;
  *dst=0;
}


void iStrAdd(char* dst, char* src, u16 max) //max = exact [] size
{
  while (*dst++) 
    if (--max <= 1) return;
  iStrCopy(--dst, src, max);
}


void iStrAddCh(char* dst, char Ch, u16 max) //max = exact [] size
{
  while (*dst++) 
    if (--max <= 1) return;
  *dst-- = 0;
  *dst = Ch;
}


char instr(char* buffer, char* find)
{
  u08 len = strlen(find);
  while (*buffer != 0)
  {
    if (strncmp(buffer, find, len) == 0) return 1;
    buffer++;
  }
  return 0;
}

u08 CmpStr(char* s1, char* s2)
{
  while (1) {
    if (tolower(*s1) != tolower(*s2)) return 0;
    if ((!*s1) && (!*s2)) return 1;
    s1++;
    s2++;
  }
}

// check free ram  
int freeRam()  {  
  extern int __heap_start, *__brkval;  
  int v;  
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);  
}


// http://hackaday.com/2010/11/09/debounce-code-one-post-to-rule-them-all/
// https://gist.github.com/Fihl/09f2c4e5104c7b409d235d60bb16f4dd
byte DebounceBit(byte Integrator, byte &state, byte val) {
  byte old = state & 0b01111111; 
  bool onoff  = state >> 7;
  if (!old) old++; //Initial
  if (Integrator > 100) Integrator = 100; //Initial
  if (val) old++; else old--;
  if (!old) { old = 1; onoff = 0; }
  if (old > Integrator) { old = Integrator; onoff = 1; }
  state = old | (onoff << 7);
  return onoff;
}


u32 HexVal(char* S)
{
  return strtol(S, (char**)NULL, 16);
}

void HexStr2Bin(char* Bin, char* Hex)
{
  char tmp[3];
  *Bin=0;
  while (1) {
    if (!(tmp[0]=*Hex++)) return;
    if (!(tmp[1]=*Hex++)) return;
    tmp[2]=0;
    *Bin++=HexVal(tmp);
    *Bin=0;
  } 
}
