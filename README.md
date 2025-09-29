# CS-EXTC2-NTP

An NTP tunnel for Cobalt Strike beacons using External-C2




## Tunnel

...
Operates on extension fields, quick overview

First 48 bytes, normal NTP. Next X bytes, Extension Field

## Extension Fields

Go by each item, such as payload (have all fields there, and a diagram of how it works), and tunnel.

... There are two sets/parts of extneino fields:

- Payload Retrieveal: The initial Payload Retirival from the TeamServer
- The Loop: The continuous comms between Client, Controller, and TeamServer

#### Base Packet Format:

All Extension Field packets follow this format:

Rationale:

- RFC Wahtever for NTP extension fields (link and name RFC here)
- Shows up as NTP in wireshark (other structs can throw a malformed packet error)

```
Bytes 0-1: Extension Field ID
Bytes 2-3: Size of extension field data
Bytes 4-7:  Session ID for NTP packet
Bytes 8-X: (optional) Additional Data (buffered with 0x00 to 4 bit boundary)
```

If we include the 48 byte NTP packet, this turns into:

```
Bytes 0-47: NTP Packet
Bytes 48-49: Extension Field ID
Bytes 50-51: Size of extension field data
Bytes 52-55:  Session ID for NTP packet
Bytes 56-X: (optional) Additional Data (buffered to 4 bit boundary)
```

There are various extension Fields, I'll break them down based on usecase/how they are used

### Payload Extension Fields

These Extension Fields are used to tunnel the payload to the client, through the Controller, from the Teamserver.

1. getSize

SomeDesc

```
Bytes 0-1: 0x00,0x00
Bytes 2-3: Size of Data
Bytes 4-7: Unique ID
Bytes 8-.. Size of total data to be sent (explain this is for chunking)
```


SomeDesc

```
Bytes 0-1: 0x00,0x00
Bytes 2-3: Size of Data
Bytes 4-7: Unique ID (EMPTY)
Bytes 8: Architechure (0x86, 0x64, or 0x00) 0x00 = continue sending payload, 0x86/0x64 are their own respective arch.
```

#### Inboudn Packet  - getSize

The server responds with a size of the

```
Bytes 0-1: 0x00,0x00
Bytes 2-3: Size of Data
Bytes 4-7: Unique ID
Bytes 8-.. Size of total data to be sent (explain this is for chunking)
```

**TeamServer Communications**

Standard comms with teamserver, while in comms loop,   use 2 common packet structures

### Outbound Packet – `dataForTeamserver`

This structure defines what the client sends **to the TeamServer**.

```
Bytes 0-1: 0x00, 0x00                // Reserved / header indicator  
Bytes 2-3: Size of Data              // Length of payload (excluding header)  
Bytes 4-7: Unique ID                 // Identifier for this client/session  
Bytes 8-..: Total Data Size          // Full size of data being transmitted 
                                     // (used for chunk reassembly if data is split)
```

---

### Inbound Packet – `dataFromTeamserver`

This structure defines what the TeamServer sends **back to the client** in response.

```
Bytes 0-1: 0x00, 0x00                // Reserved / header indicator  
Bytes 2-3: Size of Data              // Length of payload (excluding header)  
Bytes 4-7: Unique ID                 // Identifier for this client/session  
Bytes 8-..: Total Data Size          // Full size of data being transmitted 
                                     // (used for chunk reassembly if data is split)
```

** The Loop / At execution **

### The Loop Extension Fields
