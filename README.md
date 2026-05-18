# Chip8

A CHIP-8 interpreter written in C++20. The CPU runs at 960 Hz with 60 Hz delay/sound timers, and the 64×32 framebuffer is uploaded each frame as a texture to a fullscreen quad rendered via Vulkan.

## Tools

- **C++20** - the interpreter and host application.
- **CMake >= 3.26** - build configuration. Pulls dependencies via `FetchContent`.
- **[MAGE-Engine](https://github.com/miguelpafume/MAGE-Engine)** (v0.1.2) - a small Vulkan rendering engine used to open the window, manage the swap chain, and draw the CHIP-8 display as a streamed texture on a textured quad. GLFW handles input.

## What it does

- Loads a `.ch8` ROM from the `roms/` directory and executes it.
- Renders the CHIP-8 monochrome display scaled to fit the window while preserving the 2:1 aspect ratio.
- Maps the CHIP-8 hex keypad onto the standard QWERTY layout:

  ```
   CHIP-8        Keyboard
   1 2 3 C       1 2 3 4
   4 5 6 D  ->   Q W E R
   7 8 9 E       A S D F
   A 0 B F       Z X C V
  ```

## Build & run

```sh
cmake -B build
cmake --build build
./build/chip8
```

The ROM is currently hardcoded in `src/Chip8App.cpp` (defaults to `roms/test_opcode.ch8`). Edit that line and rebuild to play a different one - `tetris.ch8` and `worm.ch8` are also included.

## TODO

- **ROM selection at runtime** - accept a ROM path as a CLI argument (`./chip8 roms/tetris.ch8`) so swapping ROMs doesn't require a rebuild.
- **Crude GUI** - add a minimal in-app overlay (e.g. Dear ImGui) for:
  - picking a ROM from `roms/` via a file browser / dropdown,
  - pause / reset / step controls,
  - adjusting CPU cycle rate live
- **Sound** - the sound timer ticks but no beep is emitted yet.
- **Configurable keymap** - let the user remap keys without recompiling.