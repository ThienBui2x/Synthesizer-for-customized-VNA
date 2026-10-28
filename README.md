# STM32F407 Firmware for a Customized VNA System

Firmware for a self-built, low-cost measurement system for **real-time resonance analysis without a commercial Vector Network Analyzer (VNA)**. An STM32F407 microcontroller drives an ADF4351 frequency synthesizer, reads the response of an RF circuit through an AD8302 gain/phase detector, and exchanges commands and measurement data with a PC application.

<!-- Add a photo of the real hardware setup here: -->
<!-- ![Hardware setup](docs/hardware.jpg) -->

## System overview
**Signal chain**

```
 PC (Qt6 app) ◄──► STM32F407 ──► ADF4351 ──► RF microstrip circuit ──► AD8302 ──► STM32F407
                   (control)     (RF source)   (device under test)     (gain/phase   (acquisition)
                                                                        detector)
```

## How it works

1. The PC application sends the sweep settings (start/stop frequency, step size, ...) to the MCU.
2. The MCU programs the **ADF4351** through <!-- SPI --> to generate the RF signal at each frequency step.
3. The signal passes through the **RF microstrip circuit** under test.
4. The **AD8302** compares the signals and outputs analog voltages for gain (magnitude) and phase.
5. The MCU digitizes these voltages with its <!-- ADC --> and sends the results to the PC for real-time display and resonance analysis.

## Hardware

| Component | Role |
|-----------|------|
| STM32F407 (Discovery board) | Main controller |
| ADF4351 evaluation board | Wideband frequency synthesizer, RF source |
| AD8302 evaluation board | RF gain and phase detector |
| RF microstrip circuit | Device under test (proposed resonant structure) |

## Features

- Frequency sweep control of the ADF4351 (start, stop, step)
- Gain and phase acquisition from the AD8302
- Real-time data transfer to the PC application

## Project context

This firmware was developed as part of a research project on **metamaterial-based frequency sensing** at the University of Stuttgart / Vietnamese-German University. The goal was a measurement concept that determines resonances without a commercial VNA.

## Publication
L. W. Y. Liu, T. V. Bui, N. T. H. Truong, C. K. Chan, N. H. Nguyen:
"Contact-Based Glucose Measurements Without Any Vector Network Analyzer",
Electronics, 15(17), 3922, 2026. https://doi.org/10.3390/electronics15173922
