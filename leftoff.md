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



Bug Fixed, didn't send the sizeACK back in inbound dataForTeamserver (on server side). GO DOUBLE CHECK THE STUPID BUFFER SIZES PLEASE.
I think 1024 * 1024 max is good, but that causes an overflow in the server code. Expirement, and check the example provided by fortra


TODO: 
 - [ ] Go check buffer sizes
	- Add to constnats where needed

 - [ ] Retest with buffer sizes. 

 - [ ] Doublec check if sizeAck is the correct way to do the callback thingy 

 - [ ] Final cleanup
