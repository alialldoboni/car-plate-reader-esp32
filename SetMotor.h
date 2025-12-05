#ifndef SERMOTOR_H
#define SERMOTOR_H
#include "Arduino.h"
#include <Wire.h>

// Serial.begin(115200);
#define sda 14
#define scl 13
uint8_t slave_addr = 0x30;
uint8_t MB1_PWM = 0x01;
uint8_t MB1_IN = 0x02;
uint8_t MB2_IN = 0x03;
uint8_t MB2_PWM = 0x04;

void i2c_init() {
  Wire.begin(sda, scl);  // join I2C bus as master (no address provided).
}

// motor: Motor
// pwmvalue: PWM Value
void i2c_Write(uint8_t motor, uint8_t pwmvalue) {
  Wire.beginTransmission(slave_addr);  // transmit to device
  Wire.write(motor);                   // sends one byte
  Wire.write(pwmvalue);
  Wire.endTransmission();
}

void Car_forward(uint8_t MB1_speed, uint8_t MB2_speed) {
  // Move forward
  i2c_Write(MB1_PWM, MB1_speed);
  i2c_Write(MB1_IN, 0);
  i2c_Write(MB2_PWM, 0);
  i2c_Write(MB2_IN, MB2_speed);
}

void Car_backwards(uint8_t MB1_speed, uint8_t MB2_speed) {
  // Move backward
  i2c_Write(MB1_PWM, 0);
  i2c_Write(MB1_IN, MB1_speed);
  i2c_Write(MB2_PWM, MB2_speed);
  i2c_Write(MB2_IN, 0);
}

void Car_left(uint8_t MB1_speed, uint8_t MB2_speed) {
  // Turn left
  i2c_Write(MB1_PWM, MB1_speed);
  i2c_Write(MB1_IN, 0);
  i2c_Write(MB2_PWM, MB2_speed);
  i2c_Write(MB2_IN, 0);
}

void Car_right(uint8_t MB1_speed, uint8_t MB2_speed) {
  // Turn right
  i2c_Write(MB1_PWM, 0);
  i2c_Write(MB1_IN, MB1_speed);
  i2c_Write(MB2_PWM, 0);
  i2c_Write(MB2_IN, MB2_speed);
}

void Car_stop() {
  // Stop the car
  i2c_Write(MB1_PWM, 0);
  i2c_Write(MB1_IN, 0);
  i2c_Write(MB2_PWM, 0);
  i2c_Write(MB2_IN, 0);
}

#endif
