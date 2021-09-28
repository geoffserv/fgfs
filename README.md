# TeensyLC Arduino pushbutton MIDI controller code

I've prototyped simple pushbutton switch matrix circuits, LED matrix circuits, charlieplexed circuits, and built control surfaces with foamcore and arcade buttons
on terminal blocks.  Then wrote simple, experimental Arduino code to drive USB Midi, USB Keyboard controls and so on.

It works.  It is not ideal but just experimental.  Just for starters:
* There should never be sleeps in the main loop, instead I should be keeping track of time and dropping in to the appropriate methods at the appropriate moments of
time as execution sweeps past
* Obviously well known and better-solved ways at debouncing
* Probably a well implemented framework for button and LED control overall, but this code solves brute force at the pin level

## Real uses

This is very quick, naive, straightforward code to run a switch matrix possibly with an associated LED matrix and do any kind of thing needed.

It's very suboptimal.  But, it's stable, and **it's been easy to understand and modify quickly**, so I've found real-world use for it in:

* Pushbutton Zoom video conferencing macro controller, sending keyboard shortcuts for Mic mute, Camera on/off, window positioning, etc.
* MIDI controller automating Ableton Live Session view in multi-step, stateful ways.
* Controlling a circuit-bent kids toy, "the jaminator", to push USB MIDI.  Interestingly "the jaminator" is built with two switch matricies identical in circuitry
to the prototypes I designed my code around.  Drop-in solution.  Cool.
