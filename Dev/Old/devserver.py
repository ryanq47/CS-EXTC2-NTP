import socket
import struct

# Constants for field types
FIELD_TYPES = {
    "giveMePayload": b'\x00\x01',
    "getDataFromTeamserver": b'\x02\x04',
    "sendDataToTeamserver": b'\x02\x05',
    "sizePacket": b'\x51\x2E',
    "sessionID": b'\x53\x55',
}

FIELD_TYPES_REV = {v: k for k, v in FIELD_TYPES.items()}

# Session tracking
session_counter = 1
session_table = {}  # maps client addr to session ID

def print_packet_hex(data: bytes, label="Packet"):
    hex_string = ' '.join(f'{byte:02x}' for byte in data)
    print(f"{label} [{len(data)} bytes]: {hex_string}")


def parse_extension_field(data: bytes):
    if len(data) < 4:
        return None

    field_type = data[0:2]
    field_length = struct.unpack('<H', data[2:4])[0]
    payload = data[4:]

    return field_type, field_length, payload

def create_response_packet(field_type: bytes, payload: bytes) -> bytes:
    length = struct.pack('<H', len(payload))
    return field_type + length + payload

def handle_packet(data: bytes, addr, sock):
    global session_counter

    if len(data) <= 48:
        print("Packet too short, no extension fields found.")
        return

    # Strip off the 48-byte NTP header
    extension = data[48:]

    # 🔽 Print only the extension part in hex
    print_packet_hex(extension, label=f"Extension from {addr}")

    parsed = parse_extension_field(extension)
    if not parsed:
        print("Malformed extension field from", addr)
        return

    field_type, field_length, payload = parsed
    field_name = FIELD_TYPES_REV.get(field_type, "unknown")

    print(f"Received {field_name} from {addr}, len={field_length}")

    if field_type == FIELD_TYPES["sizePacket"]:
        session_id = session_counter & 0xFFFF
        session_counter += 1
        session_table[addr] = session_id

        response_payload = struct.pack('<H', session_id)
        response = create_response_packet(FIELD_TYPES["sessionID"], response_payload)
        sock.sendto(response, addr)
        print(f"Assigned session ID {session_id} to {addr}")

    elif field_type == FIELD_TYPES["sendDataToTeamserver"]:
        if len(payload) < 2:
            print("Invalid sendDataToTeamserver payload")
            return

        session_id = struct.unpack('<H', payload[:2])[0]
        chunk = payload[2:]
        print(f"Received data chunk from session {session_id}: {chunk.hex()}")

    elif field_type == FIELD_TYPES["giveMePayload"]:
        if len(payload) < 2:
            print("Invalid giveMePayload payload")
            return

        session_id = struct.unpack('<H', payload[:2])[0]
        print(f"Client requested payload for session {session_id}")

        dummy_data = b'\xde\xad\xbe\xef'
        response_payload = struct.pack('<H', session_id) + dummy_data
        response = create_response_packet(FIELD_TYPES["getDataFromTeamserver"], response_payload)
        sock.sendto(response, addr)
        print(f"Sent payload to session {session_id}")

    else:
        print(f"Unknown or unhandled field type {field_type.hex()}")

# Run the UDP server
def run_ntp_extension_server():
    UDP_IP = "0.0.0.0"
    UDP_PORT = 123  # Standard NTP port

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind((UDP_IP, UDP_PORT))
    print(f"NTP ExtensionField Server listening on UDP port {UDP_PORT}...")

    try:
        while True:
            data, addr = sock.recvfrom(1024)
            handle_packet(data, addr, sock)
    except KeyboardInterrupt:
        print("Server stopped.")
    finally:
        sock.close()

if __name__ == "__main__":
    run_ntp_extension_server()
