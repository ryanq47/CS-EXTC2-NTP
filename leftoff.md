New BUg Tracker:


There's an issue in `else if (ntpPacketExtensionField == NtpExtensionField::dataForTeamserver)`, where the size of the 
stored size of the data from the client, does not match the actual vector size that holds that data. 

It should. This causes a "Beacon X response length 0 is invalid. [Received 4 bytes]" with the teamserver. I have no idea why.

My best guess is that it's an issue with padding itself, and the length/data being sent to the teamserver is wrong somehow.

That has seemignly been fixed

The new bug seems to be between the Client and the Controller. The beacon always checks in cuz it's from the controller, but no data seems
to be coming *out* of the beacon? it for sure gets sent in