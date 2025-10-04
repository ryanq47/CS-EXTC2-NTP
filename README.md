# CS-EXTC2-NTP

An NTP tunnel for Cobalt Strike beacons using External-C2

## TOC:

- [CS-EXTC2-NTP](#cs-extc2-ntp)
- [Tunnel](#tunnel)
- [How it works](#how-it-works)
  - [Payload Retrieval (initial)](#payload-retrieval-initial)
    - [Step-by-step flow diagram](#payload-retrieval-flow)
  - [Beacon Loop](#beacon-loop)
    - [Beacon loop flow diagram](#beacon-loop-flow)
- [Extension Fields](#extension-fields)
  - [Base Packet Format](#base-packet-format)
  - [Rationale](#rationale)
  - [Payload Retrieval & Execution Extension Fields](#payload-retrieval-and-execution-extension-fields)
    - [getSize](#getsize)
    - [giveMePayload](#givemepayload)
    - [getIdPacket](#getidpacket)
    - [idPacket](#idpacket)
    - [dataFromTeamserver](#datafromteamserver)
  - [The Beacon Loop Extension Fields](#the-beacon-loop-extension-fields)


## Video demonstration:

(Sorry for the quality, github limits to 10 MB, a full quality version is in the repo at `misc/CS-EXTC2-NTP_Proof.mp4`)

https://github.com/user-attachments/assets/17e7db18-4e81-42e9-93b6-88df9935ab41



## Tunnel

The tunnel is fairly simple. Every packet is a normal NTP packet, and all the data hides in extension fields. 


## How it works

There are two main jobs the Client and Controller have:

1. Payload Retrieveal: The initial Payload Retirival from the TeamServer
2. The Beacon Loop: The continuous comms between Client, Controller, and TeamServer

### Payload Retireval:
---
1. Client sends a `getIdPacket` packet to get a client ID
2. Controller responds with a `idPacket` packet containing the client ID. Client saves this for all further outbound packets
3. Client sends a packet with a `giveMePayload` extension.
   1. The data in this packet will either be `0x86`, or `0x64`, depending on the architecture of the host. This must be manually set in the client, this is not dynamically determined.
4. Controller reaches out to TeamServer to get the payload. 
5. In the NTP response (to step 3), Controller returns a packet with a `sizePacket` extension, denoting the size of the payload.
6. The client initiates chunking by iterating over the inbound payload size, retrieving chunks until the entire payload has been received.
   1. The data in this packet is `0x00`, which denotes "keep sending me chunks of the payload"
7. The packet back from the Controller is a `dataFromTeamserver` packet, which contains a chunk of payload.

8. (not shown below) Once the entire payload has been retrieved, it is in injected into a new thread, and run.
   1. Note: This uses a basic CreateThread injection method. It’s going to be detected — please modify or replace with your preferred technique. (Code is located in `injector.cpp`)

![payload_ret](https://github.com/user-attachments/assets/bddaa1f3-b0c1-4a12-baa2-120e22dd5459)


### Beacon Loop:
---
1. Client reads beacon data from the named pipe. 
2. Client sends a packet with a `sizePacket` extension, which contains the size of the data retreived from the beacon.
3. Controller responds with `sizePacketAcknowledge`
4. Client then initiates chunking by iterating over the beacon data size, sending chunks with `dataForTeamserver` extensions until the entire beacon data has been sent to the Controller
5. Controller sends `sizePacketAcknowledge` each chunked packet to signify it made it.
6. Once the Controller has all the data, it forwards the data onto the TeamServer. 
7. The controller then gets the response of the teamserver.
8. Client then sends a packet with the `getDataFromTeamserverSize` extension. 
9. The Controller responds with the size of the data from the Teamserver, via a packet with a `sizePacket` extension.
10. Client then initiates chunking by sending a packet with the `getDataFromTeamserver` extension. 
11. The Controller responds with a packet with the `dataFromTeamserver` extension. This packet contiains a chunk of data of the TeamServers response.
12. Once the Client has all the data, it forwards the data onto the beacon, via the named pipe. It then loops to step 1.

![beacon_loop (1)](https://github.com/user-attachments/assets/943eade2-9bbd-458c-a9a0-96ad4ddc319c)





<!-- REMOVE_ME:
// Payload-related
[ ] giveMePayload = { 0x00, 0x01 };

// Teamserver requests/responses
[ ] getDataFromTeamserver = { 0x00, 0x02 };
[ ] dataFromTeamserver = { 0x02, 0x04 };
[ ] getDataFromTeamserverSize = { 0x03, 0x04 };
[ ] dataForTeamserver = { 0x02, 0x05 };

// Identification / ID
[X] getIdPacket = { 0x12, 0x34 };
[X] idPacket = { 0x1D, 0x1D };

// Size-related
[X] sizePacket = { 0x51, 0x2E };
[ ] sizePacketAcknowledge = { 0x50, 0x50 }; -->

## Extension Fields

Both the Payload Retrieveal, and the Beacon Loop have their own set of extension fields

### Base Packet Format:

Before exploring the individual extension fields, it's important to understand their underlying structure.

All Extension Field packets follow this format:



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

#### Rationale:

Why extension fields, and why this exact structure? Two reasons:

1. RFC 5905 defines the structure of NTP extension fields
   - [RFC 5905 Section 7.5](https://datatracker.ietf.org/doc/html/rfc5905#section-7.5)

2. The packets show up as NTP in wireshark (other structs that I tried can throw a malformed packet error)

So, if I want these packets to look legit, they need to be structured as such. 


### Payload Retrieval & Execution Extension Fields

These Extension Fields are used to tunnel the payload to the client, through the Controller, from the Teamserver.


1. #### `getSize`

This extension field is used to communicate the size of an inbound message. This is crucial for chunk based communication.

```
Bytes 0-1: 0x51, 0x2E
Bytes 2-3: Size of Data
Bytes 4-7: Unique ID
Bytes 8-11 Size of total data to be sent 
```
---

2. #### `giveMePayload`

```
Bytes 0-1:  0x00, 0x01
Bytes 2-3: Size of Data
Bytes 4-7: ClientID
Bytes 8: Architechure (0x86, 0x64, or 0x00) 0x00 = continue sending payload, 0x86/0x64 are their own respective arch.
``` 
---

3. #### `getIdPacket`

Used by the client to get an ID from the Controller.

```
Bytes 0-1:  0x12, 0x34
Bytes 2-3: Size of Data
Bytes 4-7: ClientID - by default,  0xFF,0xFF,0xFF,0xFF, aka blank client ID
``` 
---

4. #### `idPacket`

Used in response to a `getIdPacket` from the Controller to give the client an ID. This ID is stored in the Data section of the 
extension field, NOT in the ClientID field (which is blank).

```
Bytes 0-1: 0x1D, 0x1D
Bytes 2-3: Size of Data
Bytes 4-7: ClientID - Blank (0xFF,0xFF,0xFF,0xFF)
Bytes 8-11: ClientID for the client. 
``` 
---

5. #### `dataFromTeamserver`

Used in response from the Controller to tunnel data back in. Contains data that came from the teamserver.

```
Bytes 0-1: 0x02, 0x04
Bytes 2-3: Size of Data
Bytes 4-7: ClientID - Blank (0xFF,0xFF,0xFF,0xFF)
Bytes 4-?: Chunked data from teamserver
``` 
---

### The Beacon Loop Extension Fields

These Extension Fields facilitate tunneling of beacon communications, routing data between the client and the Controller, and then between the Controller and the Teamserver.
