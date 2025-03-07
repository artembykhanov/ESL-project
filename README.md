# ESL Project - RGB LED Controller

## Introduction
An open project of the DSR Embedded Systems Laboratory

### Overview
The project is based on nRF52840 SoC. Board PCA10059 (USB Dongle).
This project implements an RGB LED controller for Nordic Semiconductor nRF52 microcontrollers. It allows controlling RGB LEDs , including color adjustment via HSB (Hue, Saturation, Brightness) parameters.

### PCA10059 USB Dongle
[DataSheet nRF52840 PCA10059 USB Dongle rev2.1.1](https://github.com/user-attachments/files/17661709/DataSheet.nRF52840.PCA10059.USB.Dongle.v2.1.1.pdf)

[Schematic and PCB nRF52840 PCA10059](https://github.com/user-attachments/files/17661461/pca10059_schematic_and_pcb.pdf)

### Features
- Button control with different actions:
  - Double click (changes mode)
  - Long press (updates HSB values)
- Multiple operation modes:
  - Sleep mode
  - Hue adjustment
  - Saturation adjustment
  - Brightness adjustment
- Non-volatile memory storage for saving settings
- Command-line interface (CLI) for advanced control
- PWM-based LED control for smooth color transitions
- USB logging capabilities

## Software Components
- `main.c` - Main application entry point and initialization
- `led_control.c/h` - RGB LED control functions and HSB/RGB color conversion
- `button_handler.c/h` - Button input processing with debouncing
- `pwm_control.c/h` - PWM signal generation for LED brightness control
- `nvmc_control.c/h` - Non-volatile memory control for persistent settings
- `cli_control.c/h` - Command-line interface for advanced control

## Compiling

To enable CLI need to use parameter `ESTC_USB_CLI_ENABLED=1`.

Example:

```sh
make dfu SDK_ROOT=~/devel/esl-nsdk/
```

> **Note:** The `SDK_ROOT` parameter should point to your Nordic SDK installation directory. Make sure to specify the correct path to your Nordic SDK on your system.

## Command Line Interface
The project includes a CLI for advanced control. Connect to the device via USB to access the command interface.

## Memory Management
The application uses the Non-Volatile Memory Controller (NVMC) to store settings between power cycles, ensuring your color preferences are maintained.