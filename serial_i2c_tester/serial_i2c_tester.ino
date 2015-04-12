/**
 * Test I2C device by using a serial interface.
 *
 * Copyright (c) 2015 PhiBo (DinoTools)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 */
#include <Wire.h>

#define SERIAL_PORT Serial

uint8_t slave_addr = 0x01;
uint8_t data[128];
uint8_t data_length = 0;

void clearSerial()
{
  while(SERIAL_PORT.available() > 0) {
    SERIAL_PORT.read();
  }
}

void readAddress()
{
  uint8_t c;
  int tmp = 0;
  SERIAL_PORT.println("Enter slave address as decimal value(1-127) and press the Enter-Key");
  SERIAL_PORT.print("Slave address: ");
  while (true) {
    if(SERIAL_PORT.available() == 0) {
      continue;
    }
    c = SERIAL_PORT.read();
    if(isDigit(c)) {
      tmp = tmp * 10 + (c - '0');
      SERIAL_PORT.write(c);
    } else if (c == 27) {
      return;
    } else if (isControl(c)) {
      break;
    }
  }
  SERIAL_PORT.println();
  clearSerial();
  if (tmp > 0 && tmp < 128) {
    slave_addr = tmp;
  } else {
    SERIAL_PORT.println("Only values in the range 1-127 are allowed");
    SERIAL_PORT.println("Press any key to continue");
    waitSerial();
  }
}

bool readData()
{
  // Data
  uint8_t tmp_data[128];
  // Length
  uint8_t tmp_data_length = 0;
  // data octet
  uint8_t pos_data = 0;
  // nibble to process
  uint8_t lower = 0;
  // character read from serial
  uint8_t c;

  SERIAL_PORT.println("Enter data as hex string");
  SERIAL_PORT.print("Data: ");
  
  while(true) {
    if (SERIAL_PORT.available() > 0) {
      c = SERIAL_PORT.read();
      
      if (c == 27) {
        return false;
      } else if (isControl(c)) {
        break;
      } else if (!isHexadecimalDigit(c)) {
        continue;
      }
      
      SERIAL_PORT.write(c);

      if(isLowerCase(c)) {
        // lower case characters
        c -= 87;
      } else if(isUpperCase(c)) {
        // upper case characters
        c -= 55;
      } else if(isDigit(c)){
        // numbers
        c -= 48;
      }
      if(lower == 0) {
        // process first nibble
        c = c << 4;
        pos_data = c;
        lower = 1;
      } else {
        // process second nibble
        pos_data |= c;
        tmp_data[tmp_data_length++] = pos_data;
        lower = 0;
      }
    }
  }
  SERIAL_PORT.println();
  if (lower == 1) {
    SERIAL_PORT.println("Data incomplete.");
    SERIAL_PORT.println("Press any key to continue");
    waitSerial();
    return false;
  }
  memcpy(data, tmp_data, tmp_data_length);
  data_length = tmp_data_length;
  return true;
}

void sendI2C(uint8_t *data, uint8_t len) {
  uint8_t i;
  SERIAL_PORT.print("Sending data '");
  for(i = 0; i < len; i++) {
    if (data[i] < 16) {
      SERIAL_PORT.print("0");
    }
    SERIAL_PORT.print(data[i], HEX);
  }
  SERIAL_PORT.print("' to ");
  SERIAL_PORT.println(slave_addr);
  
  Wire.beginTransmission(slave_addr);
  Wire.write(data, len);
  Wire.endTransmission();
}

void waitSerial()
{
  clearSerial();
  while (SERIAL_PORT.available() == 0);
}

void setup(){
    Wire.begin();
    SERIAL_PORT.begin(9600);
}

void loop(){
  uint8_t c;
  uint8_t i;
  SERIAL_PORT.println();
  SERIAL_PORT.println("Serial to I2C");
  SERIAL_PORT.println("=============");
  SERIAL_PORT.print("Slave Address: ");
  SERIAL_PORT.println(slave_addr);
  SERIAL_PORT.print("Data: ");
  for(i=0; i < data_length; i++) {
    if(i % 8 == 0 && i != 0) {
      SERIAL_PORT.println();
      SERIAL_PORT.print("      ");
    }
    if(data[i] < 16) {
      SERIAL_PORT.print('0');
    }
    SERIAL_PORT.print(data[i], HEX);
  }
  SERIAL_PORT.println();
  SERIAL_PORT.println("-------------");
  SERIAL_PORT.println("a = Set slave address");
  SERIAL_PORT.println("d = Set data to send");
  SERIAL_PORT.println("s = Send data");
  SERIAL_PORT.println("w = Read and send data");
  SERIAL_PORT.println();
  SERIAL_PORT.print("Action: ");
  clearSerial();
  waitSerial();
  c = SERIAL_PORT.read();
  SERIAL_PORT.println();
  switch(c) {
    case 'a':
      readAddress();
      break;
    case 'd':
      readData();
      break;
    case 's':
      sendI2C(data, data_length);
      break;
    case 'w':
      if(readData()) {
        sendI2C(data, data_length);  
      }
      break;
  }
}
