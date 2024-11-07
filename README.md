# Phase Detector for Frequencies from 0 to 200 MHz

## Description

This is a phase detector designed to measure phase differences for input frequencies ranging from 0 to approximately 200 MHz. The device includes configurable frequency dividers for each input channel.

An STM32 microcontroller is used to manage the operational parameters of the detector and its components. These settings can be modified via an Ethernet interface by sending specific commands to the device.

## Features

- **Frequency Range**: 0 - 200 MHz (may change, depending on chips availability)
- **Configurable Frequency Dividers** for each input channel
- **STM32 Microcontroller** to manage operational parameters
- **Ethernet Communication** for real-time configuration and monitoring
- **Outputs**:
  - Fast analog output from the phase detector
  - Slow analog output from the phase detector
  - Analog output after passing through a PI controller
- **Standards**: The device is compliant with **Sinara** and **Artic** standards, ensuring compatibility with other systems that adhere to these protocols.

## Communication

The device allows full configuration and monitoring via Ethernet. Users can send commands to the microcontroller to change parameters such as:
- Frequency divider values
- Phase detector settings
- Calibration and diagnostics

## Applications

The phase detector is suitable for a variety of applications, including:
- Laser to optical frequency comb frequency locking
- Laser to laser frequency locking.

## Documentation

Detailed documentation, including schematics, source code, and configuration instructions, will be provided in future releases of the repository.

