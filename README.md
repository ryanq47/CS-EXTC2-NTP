# CS-EXTC2-NTP

An NTP tunnel for Cobalt Strike beacons using External-C2


## Tunnel
...
Operates on extension fields

First 48 bytes, normal NTP. Next X bytes, Extension Field

#### Extension Fields

0. Overall Foramt

```
Bytes 0-1: Extension Field ID
Bytes 2-3: Size of extension field data
Bytes 4-7:  Session ID for NTP packet
```

1. getSize

SomeDesc

```
Bytes 0-1: 0x00,0x00
Bytes 2-3: Size of Data
Bytes 4-7: Unique ID
Bytes 8-.. Size of total data to be sent (explain this is for chunking)
```

**Payload Retrieveal **

### Outbound Packet - 'giveMePayload'

SomeDesc

```
Bytes 0-1: 0x00,0x00
Bytes 2-3: Size of Data
Bytes 4-7: Unique ID (EMPTY)
Bytes 8: Architechure (0x86, 0x64, or 0x00) 0x00 = continue sending payload, 0x86/0x64 are their own respective arch.
```
    


### Inboudn Packet  - getSize

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