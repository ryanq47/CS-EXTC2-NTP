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

```
Bytes 0-1: 0x00,0x00
Bytes 2-3: Size of Data
Bytes 4-7: Unique ID
Bytes 8-.. Size of total data to be sent (explain this is for chunking)
```

2. dataForTeamserver

```
Bytes 0-1: 0x00,0x00
Bytes 2-3: Size of Data
Bytes 4-7: Unique ID
Bytes 8-.. Size of total data to be sent (explain this is for chunking)
```

2. dataFromTeamserver

```
Bytes 0-1: 0x00,0x00
Bytes 2-3: Size of Data
Bytes 4-7: Unique ID
Bytes 8-.. Size of total data to be sent (explain this is for chunking)
```