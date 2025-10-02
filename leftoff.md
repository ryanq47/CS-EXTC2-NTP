New BUg Tracker:


There's an issue in `else if (ntpPacketExtensionField == NtpExtensionField::dataForTeamserver)`, where the size of the 
stored size of the data from the client, does not match the actual vector size that holds that data. 

It should. This causes a "Beacon X response length 0 is invalid. [Received 4 bytes]" with the teamserver. I have no idea why.

My best guess is that it's an issue with padding itself, and the length/data being sent to the teamserver is wrong somehow.

That has seemignly been fixed

The new bug seems to be between the Client and the Controller. The beacon always checks in cuz it's from the controller, but no data seems
to be coming *out* of the beacon? it for sure gets sent in. 
 - TLDR: Disconnect between beacon & client -> server & teamserver

 		/* write to our named pipe Beacon */
		//write_frame(handle_beacon, dataForBeacon, read);
		write_frame(handle_beacon, dataForBeacon, dataFromTeamserver.size());

        fixed wit that


Next: Clean everything up

 - Cleaning up client & putting together nicely
    >> maybe explore a differnet injection method too


Bug: Large commands casuse a crash somehwere. Not quite sure where, but somethign to keep in mind/fix

Seems to be a socket failing somehweer, and teh TS says connection reset. SOmething must be up with teh socket/size, on the server side? Not sure. 

cranked the buffers, new bug now, it seems to freeze:
```
client
======================
Started sendBeaconDataToTeamserver
======================
[?] Total Data Size: 292276
[?] Max Chunk Size: 1016
[?] Number of Chunks needed: 288
[?] Sending size packet
Sending packet to 127.0.0.1:123 of size 64
----------------------
Sending NTP Packet [1/288]
----------------------
Sending packet to 127.0.0.1:123 of size 1076\
```

```
Server:
----------------------
PCKT: sizePacket
----------------------
[?] Looking up client ID: 8d2d6496
[DEBUG] setFromClientBufferSize called, value set to 475b4
[DEBUG] getFromClientBufferSize called, returning 475b4
[?] Stored message size in client class, size: 475b4
[?] Ext Length:         8
[?] padded Length:      8
[?] Sending NTP packet
[?] Sent successfully
Received 434 bytes from 127.0.0.1:d78a
[?] Parsing packet of size: 434
[?] Extension Type:     2 5
[?] Extension Length:   400
[?] Client Session ID:
----------------------
PCKT: dataForTeamserver
----------------------
[DEBUG] getFromClientBufferSize called, returning 475b4
[DEBUG] getFromClientBufferSize called, returning 475b4
```

This makes me wonder if the ubffers are still not big enough/some data is not fully being sent over to the TS

Bug Fixed, didn't send the sizeACK back. GO DOUBLE CHECK THE STUPID BUFFER SIZES PLEASE