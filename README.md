# STM32F030F4P6 VFO

## Description

An Si5351 based VFO made with cheap STM32F030F4P6 board. The frequency is changed by sending the command `set_freq(freq_here)\r\n` to the serial port of the STM32F030F4P6 module (PA2/PA3 for TX/RX). For example, to set the VFO to 1 MHz:
```
set_freq(1000000)\r\n
```

To build the firmware:
```
make
```

To program the firmware, use STM32CubeProgrammer in UART bootloader mode (BOOT0 jumper to 3V3 pin):
```
STM32_Programmer_CLI -c port=COMx  -d build/stm32f030f4p6_vfo.hex -v
```
Replace COMx with the serial port number detected by your machine.
