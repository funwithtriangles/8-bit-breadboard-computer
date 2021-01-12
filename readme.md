# 8 Bit Breadboard Computer

I've built a breadboard computer, inspired by [Ben Eater's project](https://eater.net/8bit). 

## What's in here
In this repo you'll find:

- Arduino code for the EEPROMs (Microcode, 7-Segment Display, Program Code)
- A simple assembler written in JS
- "Assembly" programs ([including Conway's Game of Life](programs/game_of_life.txt))

## How to assemble programs
To run the assembler, run the following shell command from the root of this project:

```shell
  node assembler ./assembly-programs/game_of_life.txt assembled
```

The compiled program will appear in `assembled`. You can then add the binary to the `program-code.ino` file.

## Credits
Some code has been taken from the following repos:

- [Ben Eater's EEPROM programmer](https://github.com/beneater/eeprom-programmer)
- [Gary Field's fork of the above](https://github.com/grfield/eeprom-programmer)
