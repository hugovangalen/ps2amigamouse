# ps2amigamouse

This is a simple sketch that allows you to connect your PS/2 mouse to your Commodore Amiga 500. Other Amiga types may work, but are untested. I wrote this quite some time ago but never gotten around to share it. 

It uses a slighly modified PS2Mouse.cpp but I'm not sure where I got the original from and what has been modified.


The code is far from pretty..!

KNOWN ISSUES:
- Theoretically a middle button could be supported, but this only supported the LEFT and the RIGHT buttons.
- Mouse speed is hardcoded.

# Amiga joystick / mouse port connection

FROM AMIGA MOUSE CONNECTOR                        TO ARDUINO
(female connector, seen from the 
BACK of the connector)

```
 +--------------------+  1 - V-pulse           -> Arduino PIN 2
 | 1o  2o  3o  4o 5o  |  2 - H-pulse           -> Arduino PIN 3
  \  6o  7o  8o  9o  /   3 - VQ-pulse          -> Arduino PIN 4
   +----------------+    4 - HQ-pulse          -> Arduino PIN 5
                         ** IGNORED ** 5 - BUTTON 3(Middle)  -> Arduino PIN 8
                         6 - BUTTON 1(L)       -> Arduino PIN 6
                         7 - +5V (max 50mA)    -> Arduino VCC
                         8 - GND               -> Arduino GND
                         9 - BUTTON 2(R)       -> Arduino PIN 7
```

# PS2 mouse connection

FROM PS2 CONNECTOR                               TO ARDUINO
(female connector, seen from the 
FRONT of the connector)

```
       6o [~] 5o         1 - DATA              -> Arduino PIN 9
          [_]            2 - not connected
       4o     3o         3 - GND               -> Arduino GND
                         4 - VCC               -> Arduino VCC
        2o   1o          5 - CLOCK             -> Arduino PIN 8
                         6 - not connected
```

