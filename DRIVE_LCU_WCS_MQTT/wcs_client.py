import socket
import json
import struct
import time

LCU_IP   = "127.0.0.1"
LCU_PORT = 6001

SEND_PERIOD = 0.1   # 100 ms (10 Hz)

def send_command(sock, cmd):
    payload = json.dumps(cmd, separators=(',', ':')).encode("utf-8")
    frame = struct.pack(">I", len(payload)) + payload
    sock.sendall(frame)
    print("[WCS] Sent:", cmd["name"])

def main():
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((LCU_IP, LCU_PORT))
    print("[WCS] Connected to LCU")

    try:
        # 1️⃣ Enable drive (ONCE)
        send_command(sock, {
            "v": 1,
            "id": "CMD_001",
            "type": "Command",
            "name": "EnableDrive",
            "src": "wcs",
            "body": { "axis": "PAN" },
            "meta": {}
        })
        time.sleep(0.2)

        # 2️⃣ Start continuous JOG (velocity mode)
        cmd_jog = {
            "v": 1,
            "id": "CMD_JOG",
            "type": "Command",
            "name": "Jog",
            "src": "wcs",
            "body": {
                "axis": "PAN",
                "mode": "VELOCITY_DEG",
                "direction": "FWD",
                "velocity": 50.0,
                "accel": 40.0,
                "decel": 30.0,
                "enable": True
            },
            "meta": {}
        }

        print("[WCS] Sending continuous Jog commands...")

        while True:
            send_command(sock, cmd_jog)
            time.sleep(SEND_PERIOD)

    except KeyboardInterrupt:
        print("\n[WCS] Stopping Jog")

        # 3️⃣ Stop Jog safely
        send_command(sock, {
            "v": 1,
            "id": "CMD_STOP",
            "type": "Command",
            "name": "Jog",
            "src": "wcs",
            "body": {
                "axis": "PAN",
                "enable": False
            },
            "meta": {}
        })

    finally:
        sock.close()
        print("[WCS] Connection closed")

if __name__ == "__main__":
    main()












# import socket
# import json
# import struct
# import time

# LCU_IP   = "127.0.0.1"
# LCU_PORT = 6001

# def send_command(sock, cmd):
#     payload = json.dumps(cmd, separators=(',', ':')).encode("utf-8")
#     frame = struct.pack(">I", len(payload)) + payload
#     sock.sendall(frame)
#     print("[WCS] Sent:", cmd["name"])

# def main():
#     sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
#     sock.connect((LCU_IP, LCU_PORT))
#     print("[WCS] Connected to LCU")

#     cmd = {
#         "v": 1,
#         "id": "CMD_001",
#         "type": "Command",
#         "name": "MoveToPositionDeg",
#         "axis": "PAN",
#         "target_deg": 45.0,
#         "velocity_deg_s": 20.0,
#         "accel_deg_s2": 10.0,
#         "decel_deg_s2": 10.0
#     }

#     try:
#         while True:
#             send_command(sock, cmd)
#             time.sleep(1)   # keep alive + continuous
#     except KeyboardInterrupt:
#         print("\n[WCS] Stopped by user")
#     finally:
#         sock.close()

# if __name__ == "__main__":
#     main()












# import socket
# import json
# import struct
# import time

# LCU_IP   = "127.0.0.1"
# LCU_PORT = 6001

# def send_command(sock, json_obj):
#     # Convert JSON to bytes
#     json_str = json.dumps(json_obj)
#     payload  = json_str.encode("utf-8")

#     # Build TCP frame: [LEN][JSON]
#     frame = struct.pack(">I", len(payload)) + payload

#     sock.sendall(frame)
#     print("[WCS] Sent:", json_str)

# def main():
#     sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

#     try:
#         print(f"[WCS] Connecting to LCU {LCU_IP}:{LCU_PORT} ...")
#         sock.connect((LCU_IP, LCU_PORT))
#         print("[WCS] Connected successfully")

#         # ---- SAMPLE COMMAND ----
#         cmd = {
#             "v": 1,
#             "id": "CMD_001",
#             "type": "Command",
#             "name": "MoveToPositionDeg",
#             "axis": "PAN",
#             "target_deg": 45.0,
#             "velocity_deg_s": 20.0,
#             "accel_deg_s2": 10.0,
#             "decel_deg_s2": 10.0
#         }

#         send_command(sock, cmd)

#         time.sleep(0.2)

#         # ---- ANOTHER EXAMPLE ----
#         cmd2 = {
#             "v": 1,
#             "id": "CMD_002",
#             "type": "Command",
#             "name": "EnableDrive",
#             "axis": "PAN"
#         }

#         send_command(sock, cmd2)

#         time.sleep(0.2)

#     except Exception as e:
#         print("[WCS] Error:", e)

#     finally:
#         sock.close()
#         print("[WCS] Connection closed")

# if __name__ == "__main__":
#     main()
