/**
   File: lcd128_32_io.cpp
   Function: lcd128_32 I2C control source file using bit-banging
   Platform: Arduino, ESP32, Raspberry Pi Pico
   Author: Deng Fei
   Date: 2022.2.26
*/

#include "lcd128_32_io.h"
#include "Arduino.h"

lcd::lcd(int _sda, int _scl)
{
  SDA = _sda;
  SCL = _scl;
  addr = LCD_I2C_ADDR;  // Initialize I2C address (0x3F)
  pinMode(SDA, OUTPUT);  // SDA bus as output
  pinMode(SCL, OUTPUT);  // SCL bus as output
}

void lcd::WriteByte_command(int cmd) {  // Write a command to LCD
  I2CStart();
  I2CWrite(addr << 1);
  IIC_ACK();
  I2CWrite(0x00); // Co=0, A0=0
  IIC_ACK();
  I2CWrite(cmd);
  IIC_ACK();
  I2CStop();
}

void lcd::WriteByte_dat(int dat) {
  I2CStart();
  I2CWrite(addr << 1);
  IIC_ACK();
  I2CWrite(0x40); // Co=0, A0=1
  IIC_ACK();
  I2CWrite(dat);
  IIC_ACK();
  I2CStop();
}

void lcd::WriteByte_dat(const int *dat) {
  I2CStart();
  I2CWrite(addr << 1);
  IIC_ACK();
  I2CWrite(0x40); // Co=0, A0=1
  IIC_ACK();
  I2CWrite(*dat);
  IIC_ACK();
  I2CStop();
}

void lcd::Clear() {
  for (int x = 0; x < 4; x++) {
    WriteByte_command(0xb0 + x);   // y, page address
    WriteByte_command(0x10);       // x, column address (high nibble)
    WriteByte_command(0x00);       // x, column address (low nibble)
    for (int i = 0; i < 128; i++) {
      WriteByte_dat(0x00);         // Clear row
    }
  }
}

void lcd::Init() { // Initialize the LCD
  I2CStart();
  delay(10);
  WriteByte_command(0xe2);   // Software reset
  delay(10);
  WriteByte_command(0xa3);   // Select bias
  WriteByte_command(0xa0);   // SEG output scan direction
  WriteByte_command(0xc8);   // COM output scan direction
  WriteByte_command(0x22);   // Regulation ratio
  WriteByte_command(0x81);   // Electronic volume mode set
  WriteByte_command(0x30);   // Electronic volume register
  WriteByte_command(0x2c);   // Power control
  WriteByte_command(0x2e);
  WriteByte_command(0x2f);
  Clear();                   // Initialize DDRAM
  WriteByte_command(0xff);   // Enter extension command set
  WriteByte_command(0x72);   // Display setting mode on
  WriteByte_command(0xfe);   // Exit extension command set
  WriteByte_command(0xd6);   // Set display duty
  WriteByte_command(0x90);   // Bias priority
  WriteByte_command(0x9d);   // Frame rate for duty
  WriteByte_command(0xaf);   // Display on
  WriteByte_command(0x40);   // Set display start line
}

void lcd::Cursor(int y, int x) {
  if (x > 17) {
    x = 17;
  }
  if (y > 3) {
    y = 3;
  }
  _cursor[0] = y;
  _cursor[1] = x;
}

void lcd::WriteFont(int num) {
  for (int i = 0; i < 7; i++) {
    WriteByte_dat(font_7x8[num][i]);  // Fixed: Added [i] to access array element
  }
}

void lcd::Display(char *str) {
  int len = strlen(str);
  
  WriteByte_command(0xb0 + _cursor[0]);           // y, page address
  WriteByte_command(0x10 + _cursor[1] * 7 / 16);  // x, column address (high)
  WriteByte_command(0x00 + _cursor[1] * 7 % 16);  // x, column address (low)

  for (int num = 0; num < len; num++) {
    switch (str[num]) {
      case '0': WriteFont(0); break;
      case '1': WriteFont(1); break;
      case '2': WriteFont(2); break;
      case '3': WriteFont(3); break;
      case '4': WriteFont(4); break;
      case '5': WriteFont(5); break;
      case '6': WriteFont(6); break;
      case '7': WriteFont(7); break;
      case '8': WriteFont(8); break;
      case '9': WriteFont(9); break;
      case 'a': WriteFont(10); break;
      case 'b': WriteFont(11); break;
      case 'c': WriteFont(12); break;
      case 'd': WriteFont(13); break;
      case 'e': WriteFont(14); break;
      case 'f': WriteFont(15); break;
      case 'g': WriteFont(16); break;
      case 'h': WriteFont(17); break;
      case 'i': WriteFont(18); break;
      case 'j': WriteFont(19); break;
      case 'k': WriteFont(20); break;
      case 'l': WriteFont(21); break;
      case 'm': WriteFont(22); break;
      case 'n': WriteFont(23); break;
      case 'o': WriteFont(24); break;
      case 'p': WriteFont(25); break;
      case 'q': WriteFont(26); break;
      case 'r': WriteFont(27); break;
      case 's': WriteFont(28); break;
      case 't': WriteFont(29); break;
      case 'u': WriteFont(30); break;
      case 'v': WriteFont(31); break;
      case 'w': WriteFont(32); break;
      case 'x': WriteFont(33); break;
      case 'y': WriteFont(34); break;
      case 'z': WriteFont(35); break;
      case 'A': WriteFont(36); break;
      case 'B': WriteFont(37); break;
      case 'C': WriteFont(38); break;
      case 'D': WriteFont(39); break;
      case 'E': WriteFont(40); break;
      case 'F': WriteFont(41); break;
      case 'G': WriteFont(42); break;
      case 'H': WriteFont(43); break;
      case 'I': WriteFont(44); break;
      case 'J': WriteFont(45); break;
      case 'K': WriteFont(46); break;
      case 'L': WriteFont(47); break;
      case 'M': WriteFont(48); break;
      case 'N': WriteFont(49); break;
      case 'O': WriteFont(50); break;
      case 'P': WriteFont(51); break;
      case 'Q': WriteFont(52); break;
      case 'R': WriteFont(53); break;
      case 'S': WriteFont(54); break;
      case 'T': WriteFont(55); break;
      case 'U': WriteFont(56); break;
      case 'V': WriteFont(57); break;
      case 'W': WriteFont(58); break;
      case 'X': WriteFont(59); break;
      case 'Y': WriteFont(60); break;
      case 'Z': WriteFont(61); break;
      case '!': WriteFont(62); break;
      case '"': WriteFont(63); break;
      case '#': WriteFont(64); break;
      case '$': WriteFont(65); break;
      case '%': WriteFont(66); break;
      case '&': WriteFont(67); break;
      case '\'': WriteFont(68); break;
      case '(': WriteFont(69); break;
      case ')': WriteFont(70); break;
      case '*': WriteFont(71); break;
      case '+': WriteFont(72); break;
      case ',': WriteFont(73); break;
      case '-': WriteFont(74); break;
      case '/': WriteFont(75); break;
      case ':': WriteFont(76); break;
      case ';': WriteFont(77); break;
      case '<': WriteFont(78); break;
      case '=': WriteFont(79); break;
      case '>': WriteFont(80); break;
      case '?': WriteFont(81); break;
      case '@': WriteFont(82); break;
      case '{': WriteFont(83); break;
      case '|': WriteFont(84); break;
      case '}': WriteFont(85); break;
      case '~': WriteFont(86); break;
      case ' ': WriteFont(87); break;
      case '.': WriteFont(88); break;
      case '^': WriteFont(89); break;
      case '_': WriteFont(90); break;
      case '`': WriteFont(91); break;
      case '[': WriteFont(92); break;
      case '\\': WriteFont(93); break;
      case ']': WriteFont(94); break;
      default: break;
    }
  }
}

void lcd::I2CStart() {
  digitalWrite(SCL, HIGH);
  delayMicroseconds(1);
  digitalWrite(SDA, HIGH);
  delayMicroseconds(1);
  digitalWrite(SDA, LOW);
  delayMicroseconds(1);
}

void lcd::I2CStop() {
  digitalWrite(SCL, LOW);
  delayMicroseconds(1);
  digitalWrite(SDA, LOW);
  delayMicroseconds(1);
  digitalWrite(SCL, HIGH);
  delayMicroseconds(1);
  digitalWrite(SDA, HIGH);
  delayMicroseconds(1);
}

void lcd::IIC_ACK() {
  digitalWrite(SCL, LOW);
  digitalWrite(SDA, HIGH);
  delayMicroseconds(1);
  digitalWrite(SCL, HIGH);
  delayMicroseconds(1);
  digitalWrite(SCL, LOW);
}

void lcd::I2CWrite(unsigned char dat) {
  unsigned char mask;
  for (mask = 0x80; mask != 0; mask >>= 1) {
    digitalWrite(SCL, LOW);
    delayMicroseconds(1);
    if (mask & dat) {
      digitalWrite(SDA, HIGH);
    } else {
      digitalWrite(SDA, LOW);
    }
    delayMicroseconds(1);
    digitalWrite(SCL, HIGH);
    delayMicroseconds(1);
  }
}