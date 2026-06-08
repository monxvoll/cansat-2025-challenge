import serial
import time
from influxdb_client import InfluxDBClient, Point
from influxdb_client.client.write_api import SYNCHRONOUS

# SERIAL PORT CONFIGURATION
SERIAL_PORT = '/dev/ttyUSB0'  
BAUD_RATE = 115200

# INFLUXDB CONFIGURATION
INFLUX_URL = "http://localhost:8086"
INFLUX_TOKEN = "secret_token"
INFLUX_ORG = "cansat_team"
INFLUX_BUCKET = "cansat_data"

def main():
    print(f" Connecting to the Ground Station at {SERIAL_PORT}...")
    
    try:
        # Open serial connection with the receiving board
        ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
        print("[+] Successfully connected to the receiver!")
    except Exception as e:
        print(f"[-] Error connecting to the serial port: {e}")
        print("    Make sure the board is connected and the SERIAL_PORT is correct.")
        return

    # Prepare the InfluxDB client
    client = InfluxDBClient(url=INFLUX_URL, token=INFLUX_TOKEN, org=INFLUX_ORG)
    write_api = client.write_api(write_options=SYNCHRONOUS)

    print(" Waiting for flight data...")

    while True:
        try:
            if ser.in_waiting > 0:
                # Read the line that the Arduino outputs, remove line breaks, and decode it
                line = ser.readline().decode('utf-8').strip()
                
                # Ignore empty lines or initialization messages from the Arduino
                if not line or "Initializing" in line or "Waiting" in line or "OK" in line:
                    continue

                print(f"[LoRa RX] -> {line}")

                # The CSV format we send is: PacketId, Ax, Ay, Az, Gx, Gy, Gz, Temp, Press, Alt
                data_parts = line.split(',')
                
                # Verify that the packet has arrived complete (10 exact data points)
                if len(data_parts) == 10:
                    packet_id = int(data_parts[0])
                    ax = float(data_parts[1])
                    ay = float(data_parts[2])
                    az = float(data_parts[3])
                    gx = float(data_parts[4])
                    gy = float(data_parts[5])
                    gz = float(data_parts[6])
                    temp = float(data_parts[7])
                    press = float(data_parts[8])
                    alt = float(data_parts[9])

                    # --- SEND TO INFLUXDB ---
                    point = Point("telemetry") \
                        .field("packet_id", packet_id) \
                        .field("accel_x", ax) \
                        .field("accel_y", ay) \
                        .field("accel_z", az) \
                        .field("gyro_x", gx) \
                        .field("gyro_y", gy) \
                        .field("gyro_z", gz) \
                        .field("temperature", temp) \
                        .field("pressure", press) \
                        .field("altitude", alt)
                    
                    write_api.write(bucket=INFLUX_BUCKET, org=INFLUX_ORG, record=point)
                    print(f"    [✓] Packet {packet_id} saved to DB")
        except KeyboardInterrupt:
            print("\n Closing connection...")
            ser.close()
            break
        except Exception as e:
            # If a packet arrives corrupted due to radio interference, we ignore it and continue
            print(f"     Error parsing packet: {e}")
            pass

if __name__ == '__main__':
    main()
