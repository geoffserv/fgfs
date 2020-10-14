/* Geoff's button matrix MIDI controller Teensy code
  I stole lots of logic from Teensy USB-MIDI Buttons example code by Liam Lacey

  Using a switch matrix design I found at https://forum.arduino.cc/index.php?topic=253215.0
  Schematic https://forum.arduino.cc/index.php?action=dlattach;topic=253215.0;attach=87528

  You must select MIDI from the "Tools > USB Type" menu for this code to compile.

  To change the name of the USB-MIDI device, edit the STR_PRODUCT define
  in the /Applications/Arduino.app/Contents/Java/hardware/teensy/avr/cores/usb_midi/usb_private.h
  file. You may need to clear your computers cache of MIDI devices for the name change to be applied.

  See https://www.pjrc.com/teensy/td_midi.html for the Teensy MIDI library documentation.

*/
#include <Bounce.h>

const int NUM_OF_PINS = 3;
const byte SWITCHPIN[NUM_OF_PINS] = { 0, 1, 2 }; // pins to which the array is connected
const byte NUM_OF_BUTTONS = 5; // number of buttons (starting from button #0)

byte switchValue[NUM_OF_BUTTONS];

const int DEBOUNCE_TIME = 5; //button debounce time

Bounce buttonPins[NUM_OF_PINS] =
{
  Bounce (0, DEBOUNCE_TIME),
  Bounce (1, DEBOUNCE_TIME),
  Bounce (2, DEBOUNCE_TIME)
};

void setup() {
  Serial.begin(9600); // DEBUG: USB is always 12Mbit/sec no matter the baud rate here

  for (byte i = 0; i < NUM_OF_BUTTONS; i++) {
    switchValue[i] = 1; // set all buttons to 'off' to start with
  }
  
  for (byte i = 0; i < NUM_OF_PINS; i++) { // Configure the pins for input mode with pullup resistors.
    pinMode(SWITCHPIN[i], INPUT_PULLUP);
  }
}

void loop() {
  
  byte n = 0; // This is used to enumerate every switch starting from switch 0
  
  for (byte i = 0; i <= 2; i++) { // Loop through every digital pin
    pinMode(SWITCHPIN[i], OUTPUT); // Sets the digital pin as output
    
    digitalWrite(SWITCHPIN[i], LOW); // 0 volts (https://www.arduino.cc/en/Reference/DigitalWrite)
    
    for (byte j = 0; j <= 2; j++) { // Loop through each pin again,
      if (j != i) { // If it's not the same as the output pin (e.g. only input pins pls,)
      
        // At this point, we are examining "button" n
        // I've noticed that 1 = button unpressed, 0 = button pressed

        buttonPins[SWITCHPIN[j]].update(); // Get an update (https://www.pjrc.com/teensy/td_libs_Bounce.html)
        
        byte switchCurrentValue = digitalRead(SWITCHPIN[j]); // Set CurrentValue of the Switch. 1 = off, 0 = on
        
        if ((switchValue[n] != switchCurrentValue)) { // If there's an edge change ) { // Oh button n's state has changed from our last recorded state!
          switchValue[n] = switchCurrentValue; // Record the new value
          char buf[22];
          sprintf(buf, "Button %d value is %d", n+1, switchValue[n]);
          Serial.println(buf);
        }
        n++; // move on to the next button
        delay(1); // 1 ms between each button, just chill out OK
      }
    }
    pinMode(SWITCHPIN[i], INPUT_PULLUP); // Stop the digitalWrite
  }
}
