#define SHIFT_DATA 2
#define SHIFT_CLOCK 3
#define SHIFT_LATCH 4
#define EEPROM_D0 5
#define EEPROM_D7 12
#define WRITE_EN 13

void setAddress(int address, bool outputEnable) {
  shiftOut(SHIFT_DATA, SHIFT_CLOCK, MSBFIRST, (address >> 8) | (outputEnable ? 0x00 : 0x80 ));
  shiftOut(SHIFT_DATA, SHIFT_CLOCK, MSBFIRST, address);

  digitalWrite(SHIFT_LATCH, LOW);
  delayMicroseconds(1);
  digitalWrite(SHIFT_LATCH, HIGH);
  delayMicroseconds(1);
  digitalWrite(SHIFT_LATCH, LOW);
}

byte readEEPROM(int address) {
  for (int pin = EEPROM_D0; pin <= EEPROM_D7; pin += 1) {
    pinMode(pin, INPUT);
  }
  setAddress(address, true);

  byte data = 0;
  for (int pin = EEPROM_D7; pin >= EEPROM_D0; pin -= 1) {
    data = (data << 1) + digitalRead(pin);
  }
  return data;
}
/*
   Poll D7 (MSB) until it matches the MSB of data being written
   as this indicates a completed write
*/
void pollForWriteCompletion(byte data) {
  pinMode(EEPROM_D7, INPUT);
  byte currentMSB = 0;
  byte dataMSB = data & 0x80;

  Serial.print("polling start");

  do {
    currentMSB = digitalRead(EEPROM_D7) << 7;
    delay(2);
  } while ( currentMSB != dataMSB );

  Serial.print("polling end");
}

/*
   Write a byte to the EEPROM at the specified address.
*/
void writeEEPROM(int address, byte data) {
  setAddress(address, /*outputEnable*/ false);
  for (int pin = EEPROM_D0; pin <= EEPROM_D7; pin += 1) {
    pinMode(pin, OUTPUT);
  }

  byte dataToWrite = data; // don't modify parameter
  for (int pin = EEPROM_D0; pin <= EEPROM_D7; pin += 1) {
    digitalWrite(pin, dataToWrite & 1);
    dataToWrite = dataToWrite >> 1;
  }
  digitalWrite(WRITE_EN, LOW);
  delayMicroseconds(1);
  digitalWrite(WRITE_EN, HIGH);

  delay(10);
  // pollForWriteCompletion(data);
}

/*
   Read the contents of the EEPROM and print them to the serial monitor.
*/
void printContents() {
  for (int base = 0; base <= 2047; base += 16) {
    byte data[16];
    for (int offset = 0; offset <= 15; offset += 1) {
      data[offset] = readEEPROM(base + offset);
    }

    char buf[80];
    sprintf(buf, "%03x:  %02x %02x %02x %02x %02x %02x %02x %02x   %02x %02x %02x %02x %02x %02x %02x %02x",
            base, data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7],
            data[8], data[9], data[10], data[11], data[12], data[13], data[14], data[15]);

    Serial.println(buf);
  }
}

void setup() {
  pinMode(SHIFT_DATA, OUTPUT);
  pinMode(SHIFT_CLOCK, OUTPUT);
  pinMode(SHIFT_LATCH, OUTPUT);

  digitalWrite(WRITE_EN, HIGH);
  pinMode(WRITE_EN, OUTPUT);

  Serial.begin(57600);

  byte digits[] = {
    B00111111, // 0
    B00000110, // 1
    B01011011, // 2
    B01001111, // 3
    B01100110, // 4
    B01101101, // 5
    B01111101, // 6
    B00000111, // 7
    B01111111, // 8
    B01101111, // 9
  };

  int minusChar = B01000000;
  int noChar    = B00000000;

  Serial.println("Programming seven seg (1s comp)");
  for (int value = 0; value <= 255; value++) {
    writeEEPROM(value, digits[value % 10]);
    writeEEPROM(value + 256, digits[(value / 10) % 10]);
    writeEEPROM(value + 512, digits[(value / 100) % 10]);
    writeEEPROM(value + 768, noChar);
  }
  Serial.println("Programming seven seg (2s comp)");
  for (int value = -128; value <= 127; value++) {
    writeEEPROM((byte)value + 1024, digits[abs(value) % 10]);
    writeEEPROM((byte)value + 1280, digits[abs(value / 10) % 10]);
    writeEEPROM((byte)value + 1536, digits[abs(value / 100) % 10]);

    if (value < 0) {
      writeEEPROM((byte)value + 1792, minusChar);
    } else {
      writeEEPROM((byte)value + 1792, noChar);
    }
  }
  Serial.println(" done");

  printContents();
}

void loop() {

}
