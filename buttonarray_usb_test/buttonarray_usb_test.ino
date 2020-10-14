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
byte switchPin[NUM_OF_PINS] = { 0, 1, 2 }; // pins to which the array is connected
byte buttoncount = 5; // number of buttons (starting from button #0)
byte switchValue[42];

const int DEBOUNCE_TIME = 5; //button debounce time

Bounce buttonPins[NUM_OF_PINS] =
{
  Bounce (0, DEBOUNCE_TIME),
  Bounce (1, DEBOUNCE_TIME),
  Bounce (2, DEBOUNCE_TIME)
};

void setup() {
  Serial.begin(9600); // USB is always 12Mbit/sec no matter the baud rate here
  
  for (byte i = 0; i < NUM_OF_PINS; i++) { // Configure the pins for input mode with pullup resistors.
    pinMode(switchPin[i], INPUT_PULLUP);
  }
}

void loop() {
  
  byte n = 0; // This is used to enumerate every switch starting from switch 0
  
  for (byte i = 0; i <= 2; i++) { // Loop through every digital pin
    pinMode(switchPin[i], OUTPUT); // Sets the digital pin as output
    
    digitalWrite(switchPin[i], LOW); // 0 volts (https://www.arduino.cc/en/Reference/DigitalWrite)
    
    for (byte j = 0; j <= 2; j++) { // Loop through each pin again,
      if (j != i) { // If it's not the same as the output pin (e.g. only input pins pls,)
      
        // At this point, we are examining "button" n
        buttonPins[switchPin[j]].update(); // Get an update (https://www.pjrc.com/teensy/td_libs_Bounce.html)
        if (buttonPins[switchPin[j]].fallingEdge()) { // Check each button for "falling" edge. Falling = high (not pressed - voltage from pullup resistor) to low (pressed - button connects pin to ground)
          // Button is pushed?
          Serial.println("Blep... on!!");
        }

        if (buttonPins[switchPin[j]].risingEdge()) { // Check each button for "rising" edge. Rising = low (pressed - button connects pin to ground) to high (not pressed - voltage from pullup resistor)
          // Button is pushed?
          Serial.println("Yelp... off!!");
        }
        
        switchValue[n++] = digitalRead(switchPin[j]); // Set switchValue of the button to the result, then move to the next switch automatically
      }
    }
    pinMode(switchPin[i], INPUT_PULLUP); // Stop the digitalWrite
    
  }
  // At this point, we should have looped through each button and written the value to the switchValue array
    
  //for (byte i = 0; i <= buttoncount; i++) { // loop through each button
  //  char buf[22];
  //  sprintf(buf, "Button %d value is %d", i, switchValue[i]);
  //  Serial.println(buf);
  //}
  //delay(1000); // Wait 1 second! Do not print too fast
}
