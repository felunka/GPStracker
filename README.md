# GPStracker
A minimal low-cost GPS tracker using Arduino Nano and SIM808 hardware.

## Idea
The problem with _normal_ GPS trackers is, that they are expensive to buy and often expensive to operate, because they need a data plan. My solution: only reply to SMS with the current location. By only replying and using SMS you only pay, if you need to know the location. The rest of the time, the system is free.

## Libraries
You will need the following libraries to use the code:
 * SoftwareSerial (default arduino lib)
 * [DFRobot_SIM808](https://github.com/DFRobot/DFRobot_SIM808)
 * [NeoGPS](https://github.com/SlashDevin/NeoGPS)
