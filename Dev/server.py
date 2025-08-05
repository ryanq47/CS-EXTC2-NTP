import socket
import struct
import time

# Define the NTP timestamp delta (the difference between the Unix epoch and NTP epoch)
NTP_TIMESTAMP_DELTA = 2208988800

# Define NTP packet structure
NTP_PACKET_FORMAT = "!12I"  # 12 32-bit integers (48 bytes)

# Extension field size (if used)
EXTENSION_FIELD_FORMAT = "!HH8s"  # type (2 bytes), length (2 bytes), data (8 bytes)
EXTENSION_FIELD_DATA = b"AAAABBBB"


def create_ntp_packet():
    """Create a basic NTP packet without extension fields."""
    # NTP Packet Format (48 bytes)
    # LI=0, VN=4, Mode=3 (Client Mode), Stratum=1, Poll=4, Precision=-6
    ntp_packet = bytearray(48)
    ntp_packet[0] = 0b11100011  # LI=0, VN=4, Mode=3
    ntp_packet[1] = 1  # Stratum=1
    ntp_packet[2] = 4  # Poll Interval=4
    ntp_packet[3] = 0xF3  # Precision=-6 (log2 of 1 second)

    # Reference Timestamp (not used in this example, set to 0)
    ntp_packet[16:24] = b"\x00" * 8

    # Origin Timestamp, Receive Timestamp, Transmit Timestamp
    ntp_packet[24:32] = b"\x00" * 8
    ntp_packet[32:40] = b"\x00" * 8

    # Set the Transmit Timestamp to the current time
    current_time = time.time() + NTP_TIMESTAMP_DELTA  # Convert to NTP time
    transmit_time = struct.pack("!I", int(current_time))
    ntp_packet[40:44] = transmit_time[:4]  # Use only the seconds (no fraction part)

    extension_field = struct.pack(
        # H: 2 bytes unisgned int, 8s: 8 byte string
        EXTENSION_FIELD_FORMAT,
        6969,  # [type] 1B 39 in hex
        len(EXTENSION_FIELD_DATA) + 2 + 2,  # [length] data + type + length: 00 0C = 12
        EXTENSION_FIELD_DATA,  # [data]
    )

    # Combine the NTP packet and extension field
    ntp_packet.extend(extension_field)  # Append extension field to the original packet

    return ntp_packet


def handle_ntp_request(data, addr, sock):
    """Handle an incoming NTP request and send a response."""
    print(f"Handling NTP request from {addr}")

    # Create the standard NTP packet
    ntp_packet = create_ntp_packet()

    # Send the response back to the client
    sock.sendto(ntp_packet, addr)
    print(f"Responded to {addr} with NTP packet.")


def run_ntp_server(host="0.0.0.0", port=6969):
    """Run the NTP server to handle requests from clients."""
    # Create a UDP socket
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind((host, port))

    print(f"Listening for NTP requests on {host}:{port}")

    while True:
        # Wait for an NTP request from a client
        data, addr = sock.recvfrom(1024)  # Buffer size is 1024 bytes
        print(f"Received NTP request from {addr}")
        handle_ntp_request(data, addr, sock)


if __name__ == "__main__":
    run_ntp_server()
