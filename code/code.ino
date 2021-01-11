#define SHIFT_DATA 2
#define SHIFT_CLOCK 3
#define SHIFT_LATCH 4
#define EEPROM_D0 5
#define EEPROM_D7 12
#define WRITE_EN 13

#define PROG_SIZE 256

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

  do {
    currentMSB = digitalRead(EEPROM_D7) << 7;
    delay(10);
  } while ( currentMSB != dataMSB );
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
  delayMicroseconds(10);
  digitalWrite(WRITE_EN, HIGH);

  pollForWriteCompletion(data);
}

/*
   Read the contents of the EEPROM and print them to the serial monitor.
*/
void printContents(int startAddress) {
  for (int address = startAddress; address < startAddress + PROG_SIZE; address++) {
    byte data = readEEPROM(address);
    Serial.println(data, BIN);
  }
}

byte progs[][PROG_SIZE] = {
  // add up and then down
  {
    0b01011001, // add 
    0b00000001, //   1
    0b00001101, // JC
    0b00000111, //   7
    0b00000111, // OUT
    0b00001110, // J
    0b00000000, //   0
    0b01010110, // SUB
    0b00000000, //   0
    0b00001100, // JE
    0b00000000, //   0
    0b00000111, // OUT
    0b00001110, // J
    0b00000111  //   7
  },
  // Show lines of image stored in address 0 - 7
  { 
    // Adding image to mem
    0b00001000, // PTM 
    0b00000000, //   0
    0b00000000, //   
    0b00001000, // PTM 
    0b00000001, //   1
    0b01100110, //   
    0b00001000, // PTM 
    0b00000010, //   2
    0b01100110, //  
    0b00001000, // PTM 
    0b00000011, //   3
    0b00000000, //  
    0b00001000, // PTM 
    0b00000100, //   4
    0b00000000, //  
    0b00001000, // PTM 
    0b00000101, //   5
    0b01000010, //  
    0b00001000, // PTM 
    0b00000110, //   6
    0b01111110, //  
    0b00001000, // PTM 
    0b00000111, //   7
    0b00000000, //  
    0b00001000, // PTM (put a 7 in address 15)
    0b00001111, //   15
    0b00000111, //   7
    0b00001001, // RFA (load A with byte from address referenced in 15)
    0b00001111, //   15
    0b00001011, // OUTD
    0b00000001, // LDA (load A with address stored at 15)
    0b00001111, //   15
    0b01010110, // DEC (decrement A)
    0b00000000, //   0 
    0b00001100, // JE (jump to end if 0)
            40, //   40
    0b00000011, // STA
    0b00001111, //   15
    0b00001110, // J (jump to start if not)
            27, //   27
    0b00001111, // HLT
  },
  // Show lines of image stored in address 0 - 7 and scroll horizontally
  {
    0b00001000, // PTM (put a 7 in address 15)
    0b00001111, //   15
    0b00000111, //   7
    0b00001001, // RFA (load A with byte from address referenced in 15)
    0b00001111, //   15
    0b00111100, // ROT (rotate the line by one)
    0b00001010, // RSA (store A into address referenced in 15)
    0b00001111, //   15
    0b00001011, // OUTD
    0b00000001, // LDA (load A with address stored at 15)
    0b00001111, //   15
    0b01010110, // DEC (decrement A)
    0b00000000, //   0 
    0b00001100, // JE (jump if 0)
    0b00000000, //   0
    0b00000011, // STA
    0b00001111, //   15
    0b00001110, // J (jump to line 3)
    0b00000011, //   3
  }
};

void setup() {  
  pinMode(SHIFT_DATA, OUTPUT);
  pinMode(SHIFT_CLOCK, OUTPUT);
  pinMode(SHIFT_LATCH, OUTPUT);

  digitalWrite(WRITE_EN, HIGH);
  pinMode(WRITE_EN, OUTPUT);

  Serial.begin(57600);

  delay(3000);

  int numProgs = sizeof(progs) / sizeof(progs[0]);

  for (int p = 0; p < numProgs; p++) {
    Serial.println("Adding prog");
    int startAddress = p * PROG_SIZE;
    for (int l = 0; l < PROG_SIZE; l++) {
      int address = startAddress + l;
      writeEEPROM(address, progs[p][l]);
    }
    printContents(startAddress);
  }
    
 Serial.println(" done");

  
}

void loop() {

}
