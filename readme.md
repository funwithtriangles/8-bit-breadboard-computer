# 8 Bit Breadboard Computer

I've built a breadboard computer, inspired by [Ben Eater's project](https://eater.net/8bit). You can see a video of it on [Reddit](https://www.reddit.com/r/beneater/comments/kvwvic/conways_game_of_life_running_on_an_8bit/).

## What's in here
In this repo you'll find:

- Arduino code for the EEPROMs (Microcode, 7-Segment Display, Program Code)
- A simple assembler written in JS
- "Assembly" programs ([including Conway's Game of Life](assembly-programs/game_of_life.txt))
- A rough outline (below) on the hardware / design

## How to assemble programs
To run the assembler, run the following shell command from the root of this project:

```shell
  node assembler ./assembly-programs/game_of_life.txt assembled
```

The compiled program will appear in `assembled`. You can then add the binary to the `program-code.ino` file.

## An overview of the computer design
As this is inspired by Ben Eater's [very well documented](https://eater.net/8bit) design, I'm only writing about where mine differs from his.

### Display
Probably the most exciting thing about this build is the 8x8 LED display. It's driven similarly to [Ben Eater's seven segment display](https://www.youtube.com/watch?v=dLh1n2dErzE), using a timer, counter and a 74LS138 decoder to display one line at a time. In my design, both the 8x8 and numerical display share the timer and counter, which are sped up to give the illusion of all 8 lines showing at once.

The display has it's own RAM to store the image. I'm using the smallest dual port RAM I could find, which happens to be a [whopping 1k](https://www.mouser.de/ProductDetail/Renesas-IDT/7130LA55PDGI/?qs=SmUuHNCnblqI4owWVG3MRQ%3D%3D)! I only need 16 of those 1000 bytes, heh. 

As it's dual port, it can constantly be outputing 8 bytes to display the current frame, while receiving bytes to store for the next frame in another part of memory. I think you might call this a "ping pong buffer" and it means the rest of the computer can take it's time sending lines of "image" to the display, without any timing concerns. You can think of the memory in two halves, 8 bytes each.

The lines being displayed are addressed using the fast counter mentioned above. The lines being written come from the bus and are addressed using a separate 4 bit counter. Every 8 bytes written, the top bit will flip. This top bit from the counter is inverted and fed to the top address bit of the "displaying" side of the chip, so what's being displayed is always a different 8 bytes to what's being written.

### ALU
The ALU makes use of two 74LS181 ALU ICs. These things are great fun!
Thanks to some great advice from [Tom Nisbet](https://github.com/TomNisbet/nqsap), I've got 5 lines from my instruction register directly hooked up to the various control inputs of the 181s. This saves a bunch of control lines from the control logic EEPROMs.

As well as the basic arithmetic and logic functions, I also have a "rotate left" function (achieved by wiring up the carry out to the carry in via a selector).

The real magic however, is the "popcount" function, which I included specifically to make life easier when programming Conway's Game of Life. It's an EEPROM with a lookup table of the popcount (e.g. the number of set 1s in a byte). This allows for easy counting of neighbours in the GOL algorithm.

### Program Counter
The program counter is fed directly to an EEPROM where the programs are held. The IO lines from the EEPROM then go back out onto the bus. The extra address lines of the EEPROM are hooked up to a DIP switch to allow switching between multiple programs.

### RAM
I'm using the same type of dual port RAM I used for the display mainly out of familiarity. One side of the chip has IO lines and address hooked up via dip switches, for manual writing. The other side has IO lines outputting to the bus, and address lines coming from the address register. 

However I'm still in need of 74ls157 multiplexers for the address lines on the second side of the chip, to display what's there while in programming mode.

In all honesty this isn't the most elegant solution and I'm not sure I'd recommend it.

### Instruction Register / Control logic
Only 7 bits of the instruction register are hooked up to the addresses of the microde EEPROMS. There are two types of op code that the EEPROMS read, 16 "main" op codes and 7 "ALU" op codes. The bottom four bits are used for the main op codes and the next 3 bits are used for ALU op codes. Extra functionality is gained because 5 bits from the instruction register are hooked up directly to the ALU. 

It's also worth mentioning that I've multiplexed the various IN and OUT lines to reduce the number EEPROMs needed.

## Credits
Some code has been taken from the following repos:

- [Ben Eater's EEPROM programmer](https://github.com/beneater/eeprom-programmer)
- [Gary Field's fork of the above](https://github.com/grfield/eeprom-programmer)
