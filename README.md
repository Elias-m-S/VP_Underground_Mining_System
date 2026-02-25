# Underground Mining Monitoring System (UMMS)

Das **Underground Mining Monitoring System (UMMS)** ist eine sicherheitskritische Bare-Metal-Software zur Überwachung von Gasvorkommen und Wasserständen in Bergbauumgebungen. Das System wurde für höchste Zuverlässigkeit und Integrität entwickelt und nutzt eine zweistufige Boot-Architektur.

## 🚀 Hardware-Plattform
- **Mikrocontroller:** STM32G474RE (ARM Cortex-M4) auf Nucleo-Board
- **Peripherie:** PiEye EduShield (LEDs, Taster, 7-Segment-Anzeige, Potentiometer)
- **Schnittstellen:** UART (RS232) für Sensordaten und Authentifizierung

---

## 🏗 System-Architektur

Die Software ist in zwei unabhängige Binaries unterteilt, die in strikt getrennten Flash-Speicherbereichen liegen. Beide Komponenten folgen einer **Layered Architecture** (HAL, Service, Application).

### 1. Authenticator (Bootloader-Ebene)
* **Flash-Bereich:** `0x08000000` (64 kiB)
* **Logik:** Implementiert als Switch-Case State Machine.
* **Sicherheits-Prozess:**
    - Wartet via UART auf das Startzeichen `'A'`.
    - Empfängt einen 8-Byte Entschlüsselungs-Key.
    - Kopiert die verschlüsselte `.auth`-Section vom Flash ins RAM und führt eine **XOR-Entschlüsselung** durch.
    - **Validierung:** Die Funktion `verify()` prüft die Signatur `'UMMS'` im Applikations-Flash, bevor der Sprung zur Startadresse erfolgt.

### 2. Application (Anwendungs-Ebene)
* **Flash-Bereich:** `0x08010000` (128 kiB)
* **Logik:** **Tabellenbasierter Zustandsautomat** (Table-based State Machine) für deterministisches Verhalten.
* **Sensorik:**
    - **Gassensoren:** Redundante analoge Messung (POT1/POT2) mit Signalfilterung.
    - **Wassersensor:** Empfang von Datenpaketen via RS232 mit CRC-Prüfung und Timeout-Überwachung.
    - **HMI:** Status- und Alarmmeldung über LEDs und 7-Segment-Anzeigen.

--