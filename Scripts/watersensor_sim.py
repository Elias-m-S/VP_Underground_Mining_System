import serial
import serial.tools.list_ports
import time
import struct
import sys

# ========== KONFIGURATION ==========
PORT = '/dev/ttyACM0'   # Linux/VM: /dev/ttyACM0, /dev/ttyUSB0 | Windows: COM3, COM4
BAUDRATE = 115200
TEST_MODE = False       # True = Nur Ausgabe ohne Hardware, False = Echtes Senden

# ========== PORT-SETUP ==========
ser = None

if not TEST_MODE:
    try:
        ser = serial.Serial(port=PORT, baudrate=BAUDRATE, timeout=1)
        print(f"✓ Verbunden mit {PORT} @ {BAUDRATE} Baud")
    except serial.SerialException as e:
        print(f"✗ Fehler beim Öffnen von {PORT}: {e}")
        print("\nVerfügbare Ports:")
        ports = serial.tools.list_ports.comports()
        if ports:
            for p in ports:
                print(f"  - {p.device}: {p.description}")
        else:
            print("  Keine COM-Ports gefunden!")
        print("\nLösung:")
        print("  1. STM32 anschließen und nochmal versuchen")
        print("  2. PORT-Variable im Script anpassen")
        print("  3. TEST_MODE = True setzen für Simulation ohne Hardware")
        sys.exit(1)
else:
    print("⚠ TEST_MODE aktiv - Daten werden nur angezeigt, nicht gesendet")

# Paket-Counter (startet bei 0 und wird inkrementiert)
packet_counter = 0

# Test-Phase Tracking
test_phase = 0
phase_start_time = time.time()

def get_measurement_for_phase(elapsed_seconds, packet_counter):
    """
    Gibt Messwerte für verschiedene Test-Phasen zurück.
    Complete workflow test:
    
    Phase 0 (0-15s):   Normal: 80-90 cm (variierend, im Display-Range)
    Phase 1 (15-30s):  Warning: 260 cm (> 250cm Warning)
                       15s Dauer, sollte nach 10s LED1 triggern
    Phase 2 (30-40s):  Emergency: 310 cm (> 300cm Emergency)
                       10s Dauer, sollte nach 5s EMERGENCY state triggern
    Phase 3 (40-55s):  Back to Normal: 80-90 cm (variierend, für Reset-Test)
                       User kann B1 drücken um aus EMERGENCY zurück zu OPERATIONAL
    Phase 4 (55+s):    Error: Bad Checksum
                       → FAILURE state mit allen LEDs
    """
    if elapsed_seconds < 15:
        # Phase 0: Normal operation - 80-90 cm (varying, visible on display)
        base_value = 85
        variation = (packet_counter % 10) - 5  # -5 to +5
        cm_value = base_value + variation
        uv_value = cm_value * 2105
        return (0, uv_value, False, cm_value)
    elif elapsed_seconds < 30:
        # Phase 1: Water warning - 260 cm (> 250cm threshold)
        # Hold time is 10 seconds - LED1 should turn on after 10s
        return (1, 260 * 2105, False, 260)
    elif elapsed_seconds < 40:
        # Phase 2: Water emergency - 310 cm (> 300cm threshold)
        # Hold time is 5 seconds - EMERGENCY state after 5s
        return (2, 310 * 2105, False, 310)
    elif elapsed_seconds < 55:
        # Phase 3: Back to normal - 80-90 cm (varying, for alarm reset test)
        # User can press B1 to exit EMERGENCY and return to OPERATIONAL
        base_value = 85
        variation = (packet_counter % 10) - 5  # -5 to +5
        cm_value = base_value + variation
        uv_value = cm_value * 2105
        return (3, uv_value, False, cm_value)
    else:
        # Phase 4: Error with bad checksum
        return (4, 85 * 2105, True, 85)

def calculate_checksum(data):
    """
    Berechnet die Checksumme nach Radio Connect Spezifikation.
    - Summiert alle Bytes (ohne Checksumme)
    - Nimmt das niederwertigste Byte (LSB) der Summe
    - Berechnet das Zweierkomplement (Bits invertieren und 1 addieren)
    """
    # Summe aller Bytes berechnen
    byte_sum = sum(data)
    
    # Niederwertiges Byte (LSB) nehmen
    lsb = byte_sum & 0xFF
    
    # Zweierkomplement berechnen: XOR mit 0xFF und dann 1 addieren
    checksum = ((lsb ^ 0xFF) + 1) & 0xFF
    
    return checksum

def send_packet(measurement_value, trigger_bad_checksum=False, display_cm=0):
    """Sendet ein Datenpaket mit der definierten Struktur."""
    global packet_counter

    # Paket aufbauen:
    # 1. Paket-Counter (1 Byte, unsigned)
    # 2. Sensor Value (4 Byte, unsigned, little-endian) - Spannung in µV
    # 3. Reserved Bytes 0xC0DE (2 Byte)
    # 4. Checksum (1 Byte, signed - Zweierkomplement)

    # Paket-Counter (1 Byte, unsigned)
    counter_byte = struct.pack('B', packet_counter)

    # Sensor Value (4 Byte, unsigned, little-endian) - Spannung in µV
    measurement_bytes = struct.pack('<I', measurement_value)

    # Reserved Bytes 0xC0DE (2 Byte, Little-Endian - konsistent mit Sensor Value)
    fixed_bytes = struct.pack('<H', 0xC0DE)

    # Daten für Checksumme vorbereiten (alle Bytes außer Checksumme)
    data_for_checksum = counter_byte + measurement_bytes + fixed_bytes

    # Checksum berechnen (Zweierkomplement der Summe)
    checksum = calculate_checksum(data_for_checksum)
    
    # If bad checksum is requested, corrupt it
    if trigger_bad_checksum:
        checksum = (checksum + 1) & 0xFF  # Flip the checksum

    # Checksum anhängen (signed byte)
    packet = data_for_checksum + struct.pack('b', checksum if checksum < 128 else checksum - 256)

    # Paket senden (nur wenn nicht im Test-Modus)
    if ser is not None:
        ser.write(packet)
    
    # Paket-Inhalt im Terminal ausgeben
    hex_packet = ' '.join(f'{b:02X}' for b in packet)
    status = " [BAD CHECKSUM]" if trigger_bad_checksum else ""
    print(f"[{packet_counter:3d}] {display_cm:2d}cm µV={measurement_value:10d} | Bytes: {hex_packet} | Checksum: 0x{checksum:02X}{status}")

    # Paket-Counter erhöhen (modulo 256, da 1 Byte)
    packet_counter = (packet_counter + 1) % 256


# Hauptschleife: Alle 50 ms ein Paket senden
try:
    print("\n=== WASSERSENSOR SIMULATOR - FULL WORKFLOW TEST ===")
    print()
    print("Phase 0 (0-15s):   Normal (80-90cm varying)")
    print("  → Display shows: 80, 81, 82... 90 (varying)")
    print()
    print("Phase 1 (15-30s):  Warning (260cm > 250cm threshold, 10s hold time)")
    print("  → Display shows: '-' (260 > 99)")
    print("  → After 10s at >250cm: LED1 (Alarm) turns on")
    print()
    print("Phase 2 (30-40s):  Emergency (310cm > 300cm threshold, 5s hold time)")
    print("  → Display shows: '-' (310 > 99)")
    print("  → After 5s at >300cm: Switch to EMERGENCY state")
    print("  → LED D1 (Alarm) flashing at 500ms interval")
    print()
    print("Phase 3 (40-55s):  Back to Normal (80-90cm varying - for reset test)")
    print("  → Display shows: '-' (still in EMERGENCY state)")
    print("  → Press B1 to reset alarm → returns to OPERATIONAL")
    print("  → Display shows: 80, 81... 90 again (varying)")
    print()
    print("Phase 4 (55+s):    Sensor error (bad checksum)")
    print("  → SENSOR_DEFECT event → FAILURE state")
    print("  → All LEDs on (D0, D1, D2, D4)")
    print("  → Only way out: RESET button")
    print()
    
    phase_start_time = time.time()
    
    while True:
        elapsed_seconds = time.time() - phase_start_time
        phase, measurement, bad_checksum, display_cm = get_measurement_for_phase(elapsed_seconds, packet_counter)
        
        # Log phase changes
        if elapsed_seconds < 15:
            phase_name = "NORMAL (80-90cm varying)"
        elif elapsed_seconds < 30:
            phase_name = "WARNING (260cm > 250cm)"
        elif elapsed_seconds < 40:
            phase_name = "EMERGENCY (310cm > 300cm)"
        elif elapsed_seconds < 55:
            phase_name = "BACK TO NORMAL (80-90cm varying - press B1)"
        else:
            phase_name = "ERROR (bad checksum)"
        
        # Print phase header every ~5 seconds or at phase transitions
        if packet_counter % 100 == 0:
            print(f"\n--- Phase: {phase_name} (t={elapsed_seconds:.1f}s) ---")

        send_packet(measurement, trigger_bad_checksum=bad_checksum, display_cm=display_cm)
        time.sleep(0.05)  # 50 ms Verzögerung

except KeyboardInterrupt:
    print("\n✓ Skript beendet.")
    if ser is not None:
        ser.close()
