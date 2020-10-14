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

const byte NUM_OF_X_PINS = 3;
const byte NUM_OF_Y_PINS = 3;
const byte SWITCHPINS_X[NUM_OF_X_PINS] = { 0, 1, 2 };  // pins to which the x row is connected
const byte SWITCHPINS_Y[NUM_OF_Y_PINS] = { 8, 9, 10 }; // pins to which the y col is connected
const byte NUM_OF_SWITCHES = NUM_OF_X_PINS * NUM_OF_Y_PINS ; // number of buttons

byte switchValue[NUM_OF_SWITCHES];

const int DEBOUNCE_TIME = 5; //button debounce time

void setup() {
  Serial.begin(9600); // DEBUG: USB is always 12Mbit/sec no matter the baud rate here

  for (byte i = 0; i < NUM_OF_SWITCHES; i++) {
    switchValue[i] = 1; // set all switches to 'off' to start with (1 is off in this logic)
  }
  
  for (byte i = 0; i < NUM_OF_X_PINS; i++) { // Configure the x pins for input mode with pullup resistors.
    pinMode(SWITCHPINS_X[i], INPUT_PULLUP);
  }

  for (byte i = 0; i < NUM_OF_Y_PINS; i++) { // Configure the y pins for input mode with pullup resistors.
    pinMode(SWITCHPINS_Y[i], INPUT_PULLUP);
  }
}

void loop() {
  
  byte n = 0; // This is used to enumerate every actual button starting from switch 0
  
  for (byte xpin = 0; xpin < NUM_OF_X_PINS; xpin++) { // Loop through every digital x pin
    
    pinMode(SWITCHPINS_X[xpin], OUTPUT); // Sets the digital x pin as output
    
    digitalWrite(SWITCHPINS_X[xpin], LOW); // 0 volts (https://www.arduino.cc/en/Reference/DigitalWrite)
    
    for (byte ypin = 0; ypin < NUM_OF_Y_PINS; ypin++) { // Loop through every digital y pin
      
      // At this point, we are examining "button" n
      // I've noticed that 1 = button unpressed, 0 = button pressed
        
      byte switchCurrentValue = digitalRead(SWITCHPINS_Y[ypin]); // Read Y Pin and set CurrentValue of the Switch. 1 = off, 0 = on
        
      if ((switchValue[n] != switchCurrentValue)) { // If there's an edge change ) { // Oh button n's state has changed from our last recorded state!
        switchValue[n] = switchCurrentValue; // Record the new value
        char buf[22];
        sprintf(buf, "Button %d value is %d", n+1, switchValue[n]);
        Serial.println(buf);
      }
      
      n++; // move on to the next button
      
      delay(1); // 1 ms between each button, just chill out OK
      
    }
    pinMode(SWITCHPINS_X[xpin], INPUT_PULLUP); // Stop the digitalWrite
  }
}
