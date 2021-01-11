#define SHIFT_DATA 2
#define SHIFT_CLOCK 3
#define SHIFT_LATCH 4
#define EEPROM_D0 5
#define EEPROM_D7 12
#define WRITE_EN 13

#define X   0b0000000000111111 // NOP, should have this when there isn't an OUT/IN signal in the op step
// Multiplexed IN control lines
#define J   0b0000000000000110 // Program counter in (jump)
#define MI  0b0000000000000101 // Memory address reg in
#define II  0b0000000000000100 // Instruction reg in
#define AI  0b0000000000000011 // A reg in
#define BI  0b0000000000000010 // B reg in
#define OI  0b0000000000000001 // Output reg in (7-seg display)
#define DI  0b0000000000000000 // Display line in
// Multiplexed OUT control lines
#define PO  0b0000000000011000 // Program out (actual op code, not a count)
#define AO  0b0000000000010000 // A reg out
#define EO  0b0000000000001000 // ALU out
#define BO  0b0000000000000000 // B reg out
// RAM (mix of RAM direction and RAM enable)
#define RI  0b0000001000000111 // RAM In
#define RO  0b0000001000100000 // RAM out
// Non multiplexed lines (active low 0, or active high 1)
#define HLT 0b0000000001000000 // 1 Halt clock
#define PC  0b0000000010000000 // 1 Program counter enable (count)
#define FI  0b0000000100000000 // 0 Flags in
#define DC  0b0000010000000000 // 1 Display Clock
#define CAR 0b0000100000000000 // 0 ALU carry (+1)
#define ROT 0b0001000000000000 // 1 ALU rotate
#define POP 0b0010000000000000 // 1 ALU popcount
// Bit mask to swap the active lows
#define ACTIVE_LOW_MASK 0b0000101100000000

// jump ops
#define JE  0b1100
#define JC  0b1101
// array indexes for equal and carry flag sets
#define FLAGS_E0C0 0
#define FLAGS_E0C1 1
#define FLAGS_E1C0 2
#define FLAGS_E1C1 3

uint16_t mainOpsTemplate[16][8] = {
  { PO|II, PC|X,        X,      X,         X,         X,     X, X },   // 000 0000 - NOP ; no operation
  { PO|II, PC|X,        MI|PO,  RO|AI|PC,  X,         X,     X, X },   // 000 0001 - LDA ; load A from mem
  { PO|II, PC|X,        MI|PO,  RO|BI|PC,  X,         X,     X, X },   // 000 0010 - LDB ; load B from mem
  { PO|II, PC|X,        MI|PO,  RI|AO|PC,  X,         X,     X, X },   // 000 0011 - STA ; store mem from A
  { PO|II, PC|X,        MI|PO,  RI|BO|PC,  X,         X,     X, X },   // 000 0100 - STB ; store mem from B
  { PO|II, PC|X,        AI|PO,  PC|X,      X,         X,     X, X },   // 000 0101 - PTA ; load A from next line
  { PO|II, PC|X,        BI|PO,  PC|X,      X,         X,     X, X },   // 000 0110 - PTB ; load B from next line
  { PO|II, PC|AO|OI,    X,      X,         X,         X,     X, X },   // 000 0111 - OUT ; output to 7 seg from A
  { PO|II, PC|X,        MI|PO,  PC|X,      RI|PO,     PC|X,  X, X },   // 000 1000 - PTM ; store mem from next line
  { PO|II, PC|X,        MI|PO,  RO|MI|PC,  RO|AI,     X,     X, X },   // 000 1001 - RFA ; load A with byte found at mem address referenced
  { PO|II, PC|X,        MI|PO,  RO|MI|PC,  RI|AO,     X,     X, X },   // 000 1010 - RSA ; store A with byte found at mem address referenced
  { PO|II, PC|AO|DI,    DC|X,   X,         X,         X,     X, X },   // 000 1011 - OUTD 
  { PO|II, PC|X,        PC|X,   X,         X,         X,     X, X },   // 000 1100 - JE
  { PO|II, PC|X,        PC|X,   X,         X,         X,     X, X },   // 000 1101 - JC
  { PO|II, PC|X,        PO|J,   X,         X,         X,     X, X },   // 000 1110 - J
  { PO|II, PC|X,        HLT|X,  X,         X,         X,     X, X }    // 000 1111 - HLT
};

uint16_t mainOps[4][16][8];

uint16_t aluOps[7][8] = {
  { PO|II,  PC|X,          MI|PO,         RO|BI|FI|PC,      EO|AI,       X, X, X },   // 001 XXXX - ALU
  { PO|II,  PC|X,          MI|PO,         RO|BI|FI|CAR|PC,  EO|AI|CAR,   X, X, X },   // 010 XXXX - ALU_C
  { PO|II,  PC|EO|AI|ROT,  X,             X,                X,           X, X, X },   // 011 XXXX - ALU_R
  { PO|II,  PC|EO|AI|POP,  X,             X,                X,           X, X, X },   // 100 XXXX - ALU_P
  { PO|II,  PC|X,          BI|PO|FI,      EO|AI|PC,         X,           X, X, X },   // 101 XXXX - ALU_Q
  { PO|II,  PC|X,          BI|PO|FI|CAR,  EO|AI|PC|CAR,     X,           X, X, X },   // 110 XXXX - ALU_CQ
  { PO|II,  PC|X,          MI|PO,         RO|BI|FI|PC,      X,           X, X, X },   // 111 XXXX - CMP
};


void initMainOps() {
  memcpy(mainOps[FLAGS_E0C0], mainOpsTemplate, sizeof(mainOpsTemplate));
  memcpy(mainOps[FLAGS_E0C1], mainOpsTemplate, sizeof(mainOpsTemplate));
  memcpy(mainOps[FLAGS_E1C0], mainOpsTemplate, sizeof(mainOpsTemplate));
  memcpy(mainOps[FLAGS_E1C1], mainOpsTemplate, sizeof(mainOpsTemplate));

  // Carry flag set (0 because active low)
  mainOps[FLAGS_E0C0][JC][2] = PO|J;
  
  mainOps[FLAGS_E1C0][JE][2] = PO|J;
  mainOps[FLAGS_E1C0][JC][2] = PO|J;

  mainOps[FLAGS_E1C1][JE][2] = PO|J;
}

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
void printContents() {
  for (int base = 0; base < 8192 /* 8192 */; base += 16) {
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
  initMainOps();
  
  pinMode(SHIFT_DATA, OUTPUT);
  pinMode(SHIFT_CLOCK, OUTPUT);
  pinMode(SHIFT_LATCH, OUTPUT);

  digitalWrite(WRITE_EN, HIGH);
  pinMode(WRITE_EN, OUTPUT);

  Serial.begin(57600);

  delay(3000);

  Serial.println("Programming microcode");

  for (int address = 0; address < 4096; address++) {
    int flags       = (address & 0b110000000000) >> 10;
    int instruction = (address & 0b001111111000) >> 3;
    int stepIndex   = (address & 0b000000000111);
    
    if (instruction < 16) {
      // Main ops
      uint16_t opWord = mainOps[flags][instruction][stepIndex] ^ ACTIVE_LOW_MASK;
      writeEEPROM(address, opWord);
      writeEEPROM(address + 4096, opWord >> 8);
    } else {
      // ALU ops
      instruction = (instruction >> 4) - 1;
      uint16_t opWord = aluOps[instruction][stepIndex] ^ ACTIVE_LOW_MASK;
      writeEEPROM(address, opWord);
      writeEEPROM(address + 4096, opWord >> 8);
    }
  }
    
 Serial.println("done");

  printContents();
}

void loop() {

}
