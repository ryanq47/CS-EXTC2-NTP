

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


actually, why don't I just try the real proxy method, by proxying everything out. Makes it easier.
Every message is a size, forTeamServer, or FromTeamServer, no givemepayload. Can still keep sessid, OR go to a threading style model
for each chunk.

client -> server (server strips NTP bs, just gets data) -> TS -> server (server adds NTP bs) -> client