# VP Underground Mining Monitoring System (UMMS)

> STM32G474 · PiEye EduShield · Dual Gas Sensor · Water Sensor · Authenticator + Application Binary

---

## Overview

This repository contains the project submission for the **Underground Mining Monitoring System** from the *Vertiefung Programmieren* (Advanced Programming) course at DHBW.

The UMMS is an embedded safety monitoring system for underground mining environments. It reads redundant gas sensors and a remote water level sensor, evaluates the data against configurable thresholds, and triggers alarms accordingly. The system runs on a Nucleo board with an STM32G474 controller and PiEye EduShield.

> **Note:** The main application source code is located inside the `VPTemplate/` subdirectory, which is based on the provided course template project. The root directory additionally contains Python utility scripts and project documents.

---

## System Overview

The software is split into two independent binaries residing in separate flash regions:

**Authenticator** (`0x08000000`, 64 kiB) — Receives a decryption key via UART, decrypts the `.auth` section using XOR, verifies the application signature and boots the Application binary.

**Application** (`0x08010000`, 128 kiB) — Contains the full UMMS logic: sensor acquisition, state machine, alarm handling and HMI control.

### Sensors
| Sensor | Interface | Range |
|---|---|---|
| Gas Sensor 1 & 2 (redundant) | ADC (POT1 / POT2) | 200 – 10,000 ppm |
| Water Sensor (remote) | UART via RadioConnect RS232 | 50 – 1,000 cm |

### Alarm Thresholds (Operational Mode)
| Sensor | Warning | Emergency |
|---|---|---|
| Gas | > 3,000 ppm for 5 s | > 5,000 ppm for 3 s |
| Water | > 250 cm for 10 s | > 300 cm for 5 s |

---

## Repository Structure

```
/
├── Scripts/
│   ├── encrypt_file.py          # Encryption script for the .auth section
│   ├── single-step-sp-dump.py   # Debug helper script
│   ├── water_test.py            # Water sensor test simulation
│   └── watersensor_sim.py       # Water sensor RS232 data frame simulator
├── TestCase_Doc3.pdf            # Test specification & test protocol
└── VPTemplate/                  # <- Main application source code
    ├── src/
    │   ├── App/                 # Application layer (UMMS logic)
    │   ├── Auth/                # Authenticator logic
    │   ├── HAL/                 # Hardware abstraction (LEDs, buttons, display, UART, ADC)
    │   ├── OS/                  # Scheduler & fault handler
    │   ├── Service/             # Gas sensor, water sensor, LED handler
    │   └── Util/                # EMA filter, stack monitor, state table, logging
    ├── lib/                     # STM32 HAL & CMSIS libraries
    ├── linker/                  # Linker scripts for App & Auth binaries
    └── Makefile                 # Build entry point (ARM GNU Toolchain >= 12.3)
```

---

## Build

```bash
cd VPTemplate
make all
```

Requires ARM GNU Toolchain >= 12.3. The build produces two ELF binaries — one for the Authenticator and one for the Application. The Authenticator build automatically extracts, encrypts and re-embeds the `.auth` section via `encrypt_file.py` and `arm-none-eabi-objcopy`.

To disable encryption for debugging, set the preprocessor flag:
```makefile
ENABLE_ENCRYPTION = 0
```

---

## Hardware

```
Nucleo STM32G474
├── PiEye EduShield
│   ├── LED D0  — Operation mode indicator
│   ├── LED D1  — Alarm / warning indicator
│   ├── LED D2  — System failure indicator
│   ├── LED D4  — Sensor failure indicator
│   ├── 7-Seg S1 & S2 — Water level display (cm)
│   ├── SW1     — Switch Pre-Operational <-> Operational
│   └── SW2     — Activate Test Mode
├── B1 (Nucleo) — Alarm reset
├── POT1        — Gas Sensor 1 (ADC)
├── POT2        — Gas Sensor 2 (ADC)
└── UART        — Water Sensor (RadioConnect RS232, 8-byte frame @ 250ms)
```

---

## Notes

- No floating point arithmetic is used anywhere (neither software-emulated nor hardware FPU).
- The application state machine uses a **table-based** approach; the authenticator uses **switch-case**.
- EMA filters are applied to both analog gas sensor inputs; buttons are debounced by 50 ms.
- Task cycles: 10 ms (sensor input), 50 ms (state machine), 250 ms (stack health monitoring).
