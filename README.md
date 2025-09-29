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

### Payload Retrieval & Execution Extension Fields

These Extension Fields are used to tunnel the payload to the client, through the Controller, from the Teamserver.

1. getSize

This extension field is used to communicate the size of an inbound message. This is crucial for chunk based communication. 

```
Bytes 0-1: 0x00,0x00
Bytes 2-3: Size of Data
Bytes 4-7: Unique ID
Bytes 8-.. Size of total data to be sent 
```


SomeDesc

```
Bytes 0-1: 0x00,0x00
Bytes 2-3: Size of Data
Bytes 4-7: Unique ID (EMPTY)
Bytes 8: Architechure (0x86, 0x64, or 0x00) 0x00 = continue sending payload, 0x86/0x64 are their own respective arch.
```

#### The Flow:

1. Client sends a packet with a `giveMePayload` extension.
   1. The data in this packet will either be `0x86`, or `0x64`, depending on the architecture of the host. This must be manually set in the client, this is not dynamically determined.
2. In the NTP response, Server returns a packet with a `sizePacket` extension, denoting the size of the payload.
3. The client initiates chunking by iterating over the inbound payload size, retrieving chunks until the entire payload has been received.
   1. The data in this packet is `0x00`, which denotes "keep sending me chunks of the payload"
4. Once the entire payload has been retrieved, it is in injected into a new thread, and run.
   1. Note: This uses a basic CreateThread injection method. It’s going to be detected — please modify or replace with your preferred technique. (Code is located in `injector.cpp`)


...image here...



####

Inboudn Packet  - getSize

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
