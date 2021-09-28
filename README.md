# TeensyLC Arduino pushbutton MIDI controller code

I've prototyped simple pushbutton switch matrix circuits, LED matrix circuits, charlieplexed circuits, and built control surfaces with foamcore and arcade buttons
on terminal blocks.  Then wrote simple, experimental Arduino code to drive USB Midi, USB Keyboard controls and so on.

It works.  It is not ideal but just experimental.  Just for starters:
* There should never be sleeps in the main loop, instead I should be keeping track of time and dropping in to the appropriate methods at the appropriate moments of
time as execution sweeps past
* Obviously well known and better-solved ways at debouncing
* Probably a well implemented framework for button and LED control overall, but this code solves brute force at the pin level
