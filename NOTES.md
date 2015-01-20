Pinout:

0: RFID serial out # note: you can't upload while this is connected
7: Servo
10: RFID enable

GND: RFID serial GND
+5V: RFID Vcc

2015-01-17:
Note: Don't try to use a software serial of any kind with this; the Uno
only has one 16-bit timer and Servo uses it.

Got the state machine working. Mostly. After a minute it freaks out and starts
spinning "timeout" and "rfid_wait" states. I think because it was overflowing
the timer?

Note: we can easily test the RFID serial input by just sending a string 
through the serial console.

Okay, it's outputting data but not the right data.
There, that's better. Mostly. There's still crap at the beginning of the
string: 1B 0D 00 and then the actual data. Must be on the client side--
the notification isn't being unwrapped correctly? Yes, hcidump shows that
it's correct coming into the interface. Packet contents:
02 22 20 12 00 0E 00 04 00 1B 0D 00 31 32 33 00 00 00 00 00 
  00 00 00

Ah. 1B is the packet type (notification), 0d 00 is the handle number.


