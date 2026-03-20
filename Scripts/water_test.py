import serial
import time
import struct
import sys
import os
import select
import termios
import tty

# ---------------------------------------------------
# UART Einstellungen
# ---------------------------------------------------

BAUD = 115200
SEND_INTERVAL_S = 0.1

# ---------------------------------------------------
# Testwerte (uV)
# ---------------------------------------------------

NORMAL = 815000
WARNING = 942000
EMERGENCY = 1068000

# ---------------------------------------------------
# Globale Variablen
# ---------------------------------------------------

value_uV = NORMAL
counter = 0
send_bad_crc_once = False

# ---------------------------------------------------
# Verfügbare Ports automatisch suchen
# ---------------------------------------------------

def find_serial_ports():
    ports = []

    for i in range(4):
        port = f"/dev/ttyACM{i}"
        if os.path.exists(port):
            ports.append(port)

    return ports

# ---------------------------------------------------
# Port auswählen
# ---------------------------------------------------

def choose_port():
    ports = find_serial_ports()

    if len(ports) == 0:
        print("Fehler: Kein passender Port gefunden.")
        print("Geprüft wurden: /dev/ttyACM0 bis /dev/ttyACM3")
        sys.exit(1)

    if len(ports) == 1:
        print(f"Automatisch gefundener Port: {ports[0]}")
        return ports[0]

    print("Mehrere Ports gefunden:")
    for index, port in enumerate(ports):
        print(f"{index}: {port}")

    while True:
        selection = input("Bitte Port-Nummer auswählen: ").strip()

        if selection.isdigit():
            selection = int(selection)

            if 0 <= selection < len(ports):
                print(f"Verwende Port: {ports[selection]}")
                return ports[selection]

        print("Ungültige Eingabe. Bitte erneut versuchen.")

# ---------------------------------------------------
# Checksum Funktion
# ---------------------------------------------------

def calc_checksum(frame7):
    s = sum(frame7) & 0xFF
    chk = ((s ^ 0xFF) + 1) & 0xFF
    return chk

# ---------------------------------------------------
# Frame erzeugen
# ---------------------------------------------------

def build_frame(counter, value_uV, bad_crc=False):
    frame = bytearray(8)

    frame[0] = counter
    frame[1:5] = struct.pack("<I", value_uV)
    frame[5] = 0xDE
    frame[6] = 0xC0

    correct_crc = calc_checksum(frame[:7])

    if bad_crc:
        frame[7] = (correct_crc + 1) & 0xFF
    else:
        frame[7] = correct_crc

    return frame

# ---------------------------------------------------
# Frame senden
# ---------------------------------------------------

def send_frame(ser, counter, value_uV, bad_crc=False):
    frame = build_frame(counter, value_uV, bad_crc)
    ser.write(frame)

# ---------------------------------------------------
# Einzelzeichen lesen (ohne Enter)
# ---------------------------------------------------

def read_key_nonblocking():
    dr, _, _ = select.select([sys.stdin], [], [], 0)
    if dr:
        return sys.stdin.read(1)
    return None

# ---------------------------------------------------
# Terminal RAW Mode
# ---------------------------------------------------

class RawTerminal:
    def __enter__(self):
        self.fd = sys.stdin.fileno()
        self.old_settings = termios.tcgetattr(self.fd)
        tty.setcbreak(self.fd)
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        termios.tcsetattr(self.fd, termios.TCSADRAIN, self.old_settings)

# ---------------------------------------------------
# Hauptprogramm
# ---------------------------------------------------

def main():
    global value_uV
    global counter
    global send_bad_crc_once

    port = choose_port()

    try:
        ser = serial.Serial(port, BAUD, timeout=0)
    except serial.SerialException as e:
        print(f"Fehler beim Öffnen des Ports {port}: {e}")
        sys.exit(1)

    print("")
    print("Water Sensor Test")
    print("----------------------------")
    print(f"Port: {port}")
    print("1 = Normal")
    print("2 = Warning")
    print("3 = Emergency")
    print("4 = CRC Error senden (ein Frame)")
    print("q = quit")
    print("")
    print("Hinweis: Keine dauerhafte Konsolenausgabe.")
    print("Eine Meldung erscheint nur bei Aktionen.")
    print("")

    try:
        with RawTerminal():
            while True:

                key = read_key_nonblocking()

                if key is not None:
                    key = key.strip()

                    if key == "1":
                        value_uV = NORMAL
                        print("Mode: NORMAL")

                    elif key == "2":
                        value_uV = WARNING
                        print("Mode: WARNING")

                    elif key == "3":
                        value_uV = EMERGENCY
                        print("Mode: EMERGENCY")

                    elif key == "4":
                        send_bad_crc_once = True
                        print("")
                        print("TC_APP_12 – CRC Error ausgelöst")
                        print("Ein UART Frame mit falscher Checksumme wird gesendet.")
                        print("Erwarteter Effekt: System erkennt CRC Fehler und löst Failure Event aus.")
                        print("")

                    elif key.lower() == "q":
                        print("Beende Programm.")
                        break

                bad_crc = False

                if send_bad_crc_once:
                    bad_crc = True
                    send_bad_crc_once = False

                send_frame(ser, counter, value_uV, bad_crc)
                counter = (counter + 1) & 0xFF

                time.sleep(SEND_INTERVAL_S)

    except KeyboardInterrupt:
        print("\nProgramm durch Benutzer abgebrochen.")

    finally:
        if ser.is_open:
            ser.close()
            print("Serielle Verbindung geschlossen.")

if __name__ == "__main__":
    main()
