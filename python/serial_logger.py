import serial
import os
import time
from datetime import datetime
import threading


def get_available_filename(base_name="data_logger", extension=".txt"):
    """Generate a unique filename if the base file exists."""
    counter = 0
    while True:
        if counter == 0:
            filename = f"{base_name}{extension}"
        else:
            filename = f"{base_name}_{counter}{extension}"
        if not os.path.exists(filename):
            return filename
        counter += 1


def read_from_serial(ser, log_filename):
    """Read data from serial port and log it to a file with timestamps."""
    try:
        with open(log_filename, "a") as log_file:
            while True:
                line = ser.readline().decode("utf-8").strip()
                if line:  # Only process non-empty lines
                    timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
                    log_entry = f"{timestamp} - {line}"
                    print(log_entry)  # Print to console
                    log_file.write(log_entry + "\n")  # Write to file
    except Exception as e:
        print(f"Error during logging: {e}")


def write_to_serial(ser):
    """Send commands to the serial port."""
    try:
        while True:
            command = input("Enter command (e.g., 'tare', 'start'): ")
            if command.lower() == "exit":
                print("Exiting write thread...")
                break
            ser.write((command + "\n").encode("utf-8"))
            print(f"Sent: {command}")
    except Exception as e:
        print(f"Error during writing: {e}")


def main(port="COM13", baudrate=9600, timeout=1):
    """Main function to handle serial communication."""
    # Open serial connection
    try:
        ser = serial.Serial(port, baudrate, timeout=timeout)
        print(f"Connected to {port} at {baudrate} baud.")
    except serial.SerialException as e:
        print(f"Error opening serial port: {e}")
        return

    # Get a unique filename for logging
    log_filename = get_available_filename()
    print(f"Logging data to file: {log_filename}")

    # Start reading thread
    read_thread = threading.Thread(target=read_from_serial, args=(ser, log_filename))
    read_thread.daemon = True
    read_thread.start()

    # Start writing in the main thread
    try:
        write_to_serial(ser)
    except KeyboardInterrupt:
        print("\nProgram stopped by user.")
    finally:
        ser.close()
        print("Serial port closed.")


if __name__ == "__main__":
    main(port="COM13", baudrate=9600)
