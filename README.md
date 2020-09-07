# SGBE
Nintendo Gameboy emulator written in C++ (11).  
The project was mainly done for educational reasons, to increase my knowledge over C++, software design and computer architecture.  
Also for fun and nostaligic factors, as the gameboy was my first gaming console from my childhood.  
This emulator does not aim for total emulation accuracy and features, I'm leaning more towards clean code and good software design.  
SGBE is dependent on SDL2 library that handles the window, graphics, sound and input.

# How to build
SGBE has been tested on Ubuntu:

SDL2 dependency
```sh
apt-get install libsdl2-dev
```
Fetch and build
```sh
git clone https://github.com/Dedis23/SGBE.git
cd SGBE
make all
```
# Usage
```
usage:
  SGBE --game <rom_file> 

optional:
  --logfile <file_name>     Enable printing logs into a file
  --silent                  Disable logs
```
Movement: <kbd>&larr;</kbd>, <kbd>&uarr;</kbd>, <kbd>&darr;</kbd>, <kbd>&rarr;</kbd>  
Controls: <kbd>Z</kbd>, <kbd>X</kbd>, <kbd>Enter</kbd>, <kbd>RShift</kbd>  
Change palettes: <kbd>F1</kbd>

# Features
* Cycle accurate Gameboy CPU instructions emulation
* GPU, Timer and Joypad emulation
* ROM only and MBC1 cartridge types
* Color palettes swap

# Todo
* MBC2, MBC3, MBC4, MBC5 cartridge types
* Audio support
* Emulator Save states
* Support for Gameboy Color games

# Screenshots
<h1 align="center">
  <img src="https://github.com/Dedis23/SGBE/blob/master/screenshots/nintendo.png"> 
</h1>

* Tetris with DMG GB color palette
<h1 align="center">
  <img src="https://github.com/Dedis23/SGBE/blob/master/screenshots/tetris_1.png"><img src="https://github.com/Dedis23/SGBE/blob/master/screenshots/tetris_2.png">  
</h1>

* Super Mario Land with a custom color palette
<h1 align="center"> 
  <img src="https://github.com/Dedis23/SGBE/blob/master/screenshots/super_mario_land_1.png"><img src="https://github.com/Dedis23/SGBE/blob/master/screenshots/super_mario_land_2.png">
</h1>

# Blargg's tests roms
<h1 align="center">
  <img src="https://github.com/Dedis23/SGBE/blob/master/screenshots/cpu_instrs.png"><img src="https://github.com/Dedis23/SGBE/blob/master/screenshots/instr_timing.png">  
</h1>

# Resources
* [The Ultimate GameBoy talk](https://www.youtube.com/watch?v=HyzD8pNlpwI) - video explains every component of the gameboy
* [GameBoy CPU Manual](http://marc.rawer.de/Gameboy/Docs/GBCPUman.pdf) - most of the cpu instructions taken from here
* [GameBoy Programming Manual](http://index-of.es/Varios-2/Game%20Boy%20Programming%20Manual.pdf) - correct cpu instructions timings taken from here
* [Codeslinger](http://www.codeslinger.co.uk/pages/projects/gameboy.html) - blog that explains alot about how to emulate the gameboy
* [Pandocs](http://bgb.bircd.org/pandocs.htm) - for info related to MMU, GPU, MBC cartridges
* [BGB emulator](http://bgb.bircd.org/) - helped alot with debugging
* [Blargg's Test ROMs](https://github.com/retrio/gb-test-roms) - test roms
