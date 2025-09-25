

Left OFf:
 - Clean up client to print our similar to server, with what packet received, etc etc. Then continue to build bridging, and
 chunking as needed.


//hadnling multiple clients:

Loop in recv, for each packet. Each packet will have the unique client id. When each client
packet comes in, based on the id & packet type, things will happen on that clietn packet tothe class
(ex, add to a response buffer)

Then destroy the class after its done


size -> server
id   -> Client //need to add functinality on client to take the client ID as well
    >> client class created
//chunking start here


//8PM 9/19: SessionID done for client, need to teach server to handle that field as well.

Chunking problems: 
 - server needs to be "async" or a lil different due to multiple clients

 payload/large issues:
    - Client/Server comms are done in chunks/beacon out, so it can be slow. Server -> teamserver is fast, as it's not trying to hide its traffic, it's just a proxy. 


<!-- actually, why don't I just try the real proxy method, by proxying everything out. Makes it easier.
Every message is a size, forTeamServer, or FromTeamServer, no givemepayload. Can still keep sessid, OR go to a threading style model
for each chunk.

client -> server (server strips NTP bs, just gets data) -> TS -> server (server adds NTP bs) -> client


Left off by getting payload from server. Fill in for x64 as well, then work on chunking/the 0x00 setup to make sure it works -->

working up until 0x00 giveMePayload. Everything seems to be gonig good, but there's a lookup problem wher the client class eitehr isn't being created, or not being accessed correctly by the server, to get the data. Problem is that sessionID is 0 prolly due to parser


The lookup is failing for the session: 

```
[?] Payload Arch = 0x86
[?] X86 Payload Requested
[?] Getting x86 from TeamServer
[?] Sending frame to TS
[?] Sending frame to TS
[?] Sending frame to TS
[?] Sending frame to TS
[?] Payload of size 239616 recieved.
Printing Sesions: 1800238816
[?] Could not find client class: 6b 4d 76 e0
[?] Sending normal NTP packet back
```
So something is goign wrong with that, and is the next spot to investigate (in x86)

^^ There's a byte order mixmatch. clientID/SessionId comes in as opposite endian than what it's stored as.

uint32_t vectorToUint32(const std::vector<uint8_t>& vec) {
    if (vec.size() < 4) {
        throw std::runtime_error("Vector too small to convert to uint32_t");
    }

    uint32_t value;
    std::memcpy(&value, vec.data(), sizeof(value));
    return ntohl(value);  // Converts the value to host byte order
}


<!-- fixed by adding ntohl here, which is odd. I didn't think it was coming in as network order.

Something weird is happenign with chunking now, not getting full payload from server 
^^ Fix: badif else tree

New bug: Some bug with paylaod size, where server has correct size, but client is not seeing correct size
//fixed, sesion ID was not included on packets back to client. Fixed.

//do some CLEANUP! -->

<!-- Cleanup done, need to fix injection and figure out why it isn't working.
It appears to specifially be something with the trasfered in payload, not the declared 0x90 shellcode, so 
something is getting weird. review that chain. 

proiblm: not running as 64 bit lol -->

tested with multi client, it works, is a bit slower due to multiple access at once though, especially when printing.
Need to add in sleep for eahc packet so it doesn;t spam too hard

Workign on smb read loop & those commsn, see pipeStuff in client.

//Bug where the response packet says its only 48 bytes, so a normap packet I think is getting sent back to the client somewhere 

Fixed

new bug, size is not being received correctly/stored by sizePacket. Client is fine, serer is fucked somehweere

<!-- BUUUUUG: [?] Stored size of message in client class, size: 1005847518, this is the ClientID NOTOTTT the size FUCK -->

<!-- New bug: fromClientBuffer & fromClientBuffersize are bugged out and not being stored in the clietn class corercty -->

Fixed. Continue building. Ack is succsuflly received.



New bug: MixMatch in size of data coming from clinet, to stored in client class, to teamserver, which only sends part of the data cuz it thinks its all there:

----------------------
PCKT: sizePacket
----------------------
[?] sizePacket recieved
[?] Looking up client ID: 85e5bfd7
[DEBUG] setFromClientBufferSize called, value set to 84
[DEBUG] getFromClientBufferSize called, returning 84
[?] Stored size of message in client class, size: 132
[?] Ext Length:         4
[?] padded Length:      4
[?] Sending NTP packet
[?] Sent successfully
Received 188 bytes from 127.0.0.1:52294
[?] Parsing packet of size: 188 ## see, it's 188 here. (client also says 188 - which is correct based on counting bits) 188-48 -8 = 132 shuold be size.  (see above, this is correct: [?] Stored size of message in client class, size: 132)
[?] Struct size:        48
[?] Extension Type:     2 5
[?] Extension Length:   88  ## this says 88, odd
[?] Client Session ID:
----------------------
PCKT: dataForTeamserver
----------------------
sendDataToTeamserver
[?] Looking up client ID: 85e5bfd7
Extension Data for dataForTeamserver:
[DEBUG] getFromClientBufferSize called, returning 84
[DEBUG] getFromClientBufferSize called, returning 84
[+] Data from client complete, sending to teamserver
[?] Data Size: 84
[TS] Sending frame to TS        # SO - this is sending too early for some reason, not at 132, but isntead at 84, meaning something is failing somehwere
[TS] Sending size of frame: 84 # BUT, only 84 is being sent...
[TS] Sending data of frame.
[+] Sent to teamserver
[?] Ext Length:         4
[?] padded Length:      4
[?] Sending NTP packet
[?] Sent successfully
Received 38 bytes from 127.0.0.1:cc47
[?] Parsing packet of size: 38
[?] Struct size:        30
[?] Extension Type:     3 4
[?] Extension Length:   4
[?] Client Session ID:

...84 may be interpreted as hex, cuz 84 in hex = 132 in deciimal

idfk that seems to be fine. im confused at waht is happening. Make sure data is being read from the pipe and actualyl has data? then
make sure it maeks to to the TS. TS has read length read -1 thign going on still

is pipe being read correctly: YES
Is server getting exact copy of data correctly: Yes Seems so, the printed buffer for TS is 124 chars (which would make sene with 8 bytes of header? unsure) This may be where a mixmatch is
Is server STORING exact copy of data correctly: Yes
is data being passed to TS correctly: ?
is data being recieved from TS correctly: recv_frame works, I guess the data is bad? Can hook client directly up to TS and see if that worsk too

