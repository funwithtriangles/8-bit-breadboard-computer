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
  // up_down.txt
  {
0b00001000,
0b00000000,
0b00000000,
0b01101001,
0b00000000,
0b00001101,
0b00001010,
0b00000111,
0b00001110,
0b00000011,
0b01010110,
0b00000000,
0b01110110,
0b00000000,
0b00001100,
0b00000011,
0b00000111,
0b00001110,
0b00001010,
  },
  // show_image.txt
  { 
    0b00001000,
    0b00000000,
    0b00000000,
    0b00001000,
    0b00000001,
    0b01100110,
    0b00001000,
    0b00000010,
    0b01100110,
    0b00001000,
    0b00000011,
    0b00000000,
    0b00001000,
    0b00000100,
    0b00000000,
    0b00001000,
    0b00000101,
    0b01000010,
    0b00001000,
    0b00000110,
    0b01111110,
    0b00001000,
    0b00000111,
    0b00000000,
    0b00001000,
    0b00001111,
    0b00000111,
    0b00001001,
    0b00001111,
    0b00001011,
    0b00000001,
    0b00001111,
    0b01010110,
    0b00000000,
    0b00001100,
    0b00101000,
    0b00000011,
    0b00001111,
    0b00001110,
    0b00011011,
    0b00001111,
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
  },
  // popcount_test.txt
  {
    0b00001000,
    0b00001111,
    0b00000111,
    0b00001000,
    0b00001110,
    0b00000000,
    0b00001001,
    0b00001111,
    0b00001011,
    0b01000000,
    0b00000111,
    0b00011001,
    0b00001110,
    0b00000011,
    0b00001110,
    0b00000001,
    0b00001111,
    0b01010110,
    0b00000000,
    0b00001100,
    0b00011001,
    0b00000011,
    0b00001111,
    0b00001110,
    0b00000110,
    0b00000001,
    0b00001110,
    0b00000111,
    0b00001111,
  },
  // neighbours.txt
  {
0b00001000,
0b00001111,
0b00001000,
0b00001000,
0b00001101,
0b10000010,
0b00001000,
0b00001100,
0b10000011,
0b00001000,
0b00001001,
0b00000111,
0b00001000,
0b00001000,
0b00000000,
0b00000001,
0b00001111,
0b01010110,
0b00000000,
0b01110110,
0b00001000,
0b00001100,
0b01011111,
0b00000011,
0b00001111,
0b00001000,
0b00001110,
0b00000111,
0b00001000,
0b00001011,
0b00000000,
0b00001001,
0b00001111,
0b10011011,
0b00001101,
0b01000000,
0b00011001,
0b00001011,
0b00000011,
0b00001011,
0b00000001,
0b00001111,
0b01101001,
0b00000000,
0b10011011,
0b00001001,
0b00000011,
0b00001010,
0b00001001,
0b00001010,
0b10011011,
0b00001100,
0b01000000,
0b00011001,
0b00001011,
0b00000011,
0b00001011,
0b00000001,
0b00001111,
0b01010110,
0b00000000,
0b10011011,
0b00001001,
0b00000011,
0b00001010,
0b00001001,
0b00001010,
0b10011011,
0b00001100,
0b01000000,
0b00011001,
0b00001011,
0b00000111,
0b00000001,
0b00001101,
0b00111100,
0b00000011,
0b00001101,
0b00000001,
0b00001100,
0b00111100,
0b00000011,
0b00001100,
0b00000001,
0b00001110,
0b01010110,
0b00000000,
0b01110110,
0b00001000,
0b00001100,
0b00001111,
0b00000011,
0b00001110,
0b00001110,
0b00011100,
0b00001111,
  },
// gol.txt
  {
0b00001000, // 00000000: PTM
0b00001000, // 00000001: currRow
0b00001000, // 00000010: 8
0b00001000, // 00000011: PTM
0b00001010, // 00000100: maskEdges
0b10000010, // 00000101: b10000010
0b00001000, // 00000110: PTM
0b00001011, // 00000111: maskFull
0b10000011, // 00001000: b10000011
0b00001000, // 00001001: PTM
0b00001100, // 00001010: maskOne
0b00000001, // 00001011: b00000001
0b00001000, // 00001100: PTM
0b00001111, // 00001101: modMask
0b00000111, // 00001110: b00000111
0b00001000, // 00001111: PTM
0b00010000, // 00010000: neg1
0b11111111, // 00010001: b11111111
0b00001000, // 00010010: PTM
0b00010001, // 00010011: nAliveDead
0b00000011, // 00010100: 3
0b00001000, // 00010101: PTM
0b00010010, // 00010110: nAlive
0b00000010, // 00010111: 2
0b00001000, // 00011000: PTM
0b00010011, // 00011001: lineBuild
0b00000000, // 00011010: b00000000
0b00000001, // 00011011: LDA
0b00001000, // 00011100: currRow
0b01010110, // 00011101: ALU_DEC
0b00000000, // 00011110: 0
0b01110110, // 00011111: ALU_CMP
0b00010000, // 00100000: neg1
0b00001100, // 00100001: JE
0b10000110, // 00100010: end
0b00000011, // 00100011: STA
0b00001000, // 00100100: currRow
0b00001000, // 00100101: PTM
0b00001001, // 00100110: currCol
0b00000111, // 00100111: 7
0b00001000, // 00101000: PTM
0b00001101, // 00101001: currCount
0b00000000, // 00101010: 0
0b00001001, // 00101011: RLA
0b00001000, // 00101100: currRow
0b10011011, // 00101101: ALU_AND
0b00001010, // 00101110: maskEdges
0b01000000, // 00101111: ALU_POP
0b00011001, // 00110000: ALU_ADD
0b00001101, // 00110001: currCount
0b00000011, // 00110010: STA
0b00001101, // 00110011: currCount
0b00000001, // 00110100: LDA
0b00001000, // 00110101: currRow
0b01101001, // 00110110: ALU_INC
0b00000000, // 00110111: 0
0b10011011, // 00111000: ALU_AND
0b00001111, // 00111001: modMask
0b00000011, // 00111010: STA
0b00001110, // 00111011: temp
0b00001001, // 00111100: RLA
0b00001110, // 00111101: temp
0b10011011, // 00111110: ALU_AND
0b00001011, // 00111111: maskFull
0b01000000, // 01000000: ALU_POP
0b00011001, // 01000001: ALU_ADD
0b00001101, // 01000010: currCount
0b00000011, // 01000011: STA
0b00001101, // 01000100: currCount
0b00000001, // 01000101: LDA
0b00001000, // 01000110: currRow
0b01010110, // 01000111: ALU_DEC
0b00000000, // 01001000: 0
0b10011011, // 01001001: ALU_AND
0b00001111, // 01001010: modMask
0b00000011, // 01001011: STA
0b00001110, // 01001100: temp
0b00001001, // 01001101: RLA
0b00001110, // 01001110: temp
0b10011011, // 01001111: ALU_AND
0b00001011, // 01010000: maskFull
0b01000000, // 01010001: ALU_POP
0b00011001, // 01010010: ALU_ADD
0b00001101, // 01010011: currCount
0b01110110, // 01010100: ALU_CMP
0b00010001, // 01010101: nAliveDead
0b00001100, // 01010110: JE
0b01110001, // 01010111: makeCellLive
0b00000001, // 01011000: LDA
0b00001010, // 01011001: maskEdges
0b00111100, // 01011010: ALU_ROT
0b00000011, // 01011011: STA
0b00001010, // 01011100: maskEdges
0b00000001, // 01011101: LDA
0b00001011, // 01011110: maskFull
0b00111100, // 01011111: ALU_ROT
0b00000011, // 01100000: STA
0b00001011, // 01100001: maskFull
0b00000001, // 01100010: LDA
0b00001100, // 01100011: maskOne
0b00111100, // 01100100: ALU_ROT
0b00000011, // 01100101: STA
0b00001100, // 01100110: maskOne
0b00000001, // 01100111: LDA
0b00001001, // 01101000: currCol
0b01010110, // 01101001: ALU_DEC
0b00000000, // 01101010: 0
0b00001100, // 01101011: JE
0b01111001, // 01101100: sendLine
0b00000011, // 01101101: STA
0b00001001, // 01101110: currCol
0b00001110, // 01101111: J
0b00101000, // 01110000: loopCol
0b00000001, // 01110001: LDA
0b00010011, // 01110010: lineBuild
0b00011001, // 01110011: ALU_ADD
0b00001100, // 01110100: maskOne
0b00000011, // 01110101: STA
0b00010011, // 01110110: lineBuild
0b00001110, // 01110111: J
0b01011000, // 01111000: rotMasks
0b00000101, // 01111001: PTA
0b00010100, // 01111010: tempLines
0b00011001, // 01111011: ALU_ADD
0b00001000, // 01111100: currRow
0b00000011, // 01111101: STA
0b00001110, // 01111110: temp
0b00000001, // 01111111: LDA
0b00010011, // 10000000: lineBuild
0b00001010, // 10000001: RSA
0b00001110, // 10000010: temp
0b00001011, // 10000011: OTD
0b00001110, // 10000100: J
0b00011000, // 10000101: loopRow
0b00001111, // 10000110: HLT
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
