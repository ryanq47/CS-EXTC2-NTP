import socket
import struct
import time

NTP_TIMESTAMP_DELTA = 2208988800


def print_packet_details(data, addr):
    """Unpacks and prints the details of a received NTP packet."""
    print("\n" + "=" * 50)
    print(f"[*] Received Packet from: {addr[0]}:{addr[1]}")
    print(f"[*] Total Packet Length: {len(data)} bytes")
    print("=" * 50)

    if len(data) < 48:
        print("[!] Error: Packet is too small.")
        return

    try:
        header = struct.unpack("!B B B b", data[0:4])
        li_vn_mode, stratum, poll, precision = header
        li = li_vn_mode >> 6
        vn = (li_vn_mode >> 3) & 0b111
        mode = li_vn_mode & 0b111

        print("--- NTP Header ---")
        print(f"  Version Number (VN): {vn}")
        print(f"  Mode: {mode} (3=client)")
        print(f"  Stratum: {stratum}")

    except struct.error:
        print("[!] Error unpacking NTP header.")
        return

    if len(data) > 48:
        print("\n--- Extension Fields ---")
        ext_data = data[48:]
        try:
            ext_type, ext_length = struct.unpack("!HH", ext_data[0:4])
            payload_size = ext_length - 4
            payload = ext_data[4 : 4 + payload_size]
            print(f"  [*] Found Extension Field:")
            print(f"    Type: 0x{ext_type:04X}")
            print(f"    Length: {ext_length} bytes")
            print(f"    Data: \"{payload.decode('utf-8', errors='ignore')}\"")
        except struct.error:
            print("  [!] Could not parse extension field.")
    print("=" * 50 + "\n")


# --- THIS IS THE CORRECTED FUNCTION ---
def handle_ntp_request(data, addr, sock):
    """
    Handles an incoming request by modifying it into a response
    and sending it back.
    """
    # First, print the details of the request we just received.
    print_packet_details(data, addr)

    # Create a mutable copy of the request packet.
    response_packet = bytearray(data)

    # --- Modify the packet to turn it into a server response ---

    # 1. Update LI, VN, and Mode. Set Mode to 4 (server).
    # We keep the client's VN.
    vn = (response_packet[0] >> 3) & 0b111
    response_packet[0] = 0b00000100 | (vn << 3)  # LI=0, Mode=4

    # 2. Set Stratum to 1 (primary reference)
    response_packet[1] = 1

    # 3. Copy client's transmit time to our origin time (standard NTP practice)
    client_tx_time = response_packet[40:48]
    response_packet[24:32] = client_tx_time

    # 4. Set our Receive and Transmit Timestamps to the current time
    server_time_ntp = time.time() + NTP_TIMESTAMP_DELTA
    server_time_struct = struct.pack("!II", int(server_time_ntp), 0)  # Secs, Frac
    response_packet[32:40] = server_time_struct  # Receive Timestamp
    response_packet[40:48] = server_time_struct  # Transmit Timestamp

    # The extension field sent by the client is already in the packet, so we don't
    # need to add it. We are "echoing" it back.

    sock.sendto(response_packet, addr)
    print(f"[*] Responded to {addr[0]}:{addr[1]} with a modified packet.")


def run_ntp_server(host="0.0.0.0", port=6969):
    """Run the NTP server to handle requests from clients."""
    # Use 127.0.0.1 if you only want to connect from your own machine.
    # Use 0.0.0.0 if you want to connect from other machines on the network.
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind((host, port))
    print(f"[*] Python NTP server listening on {host}:{port}")
    print("    Press Ctrl+C to exit.")

    try:
        while True:
            data, addr = sock.recvfrom(1024)
            handle_ntp_request(data, addr, sock)
    except KeyboardInterrupt:
        print("\n[*] Server shutting down.")
    finally:
        sock.close()


if __name__ == "__main__":
    run_ntp_server(host="127.0.0.1", port=6969)
