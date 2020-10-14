/* Jaminator Mk1 rev A
 *  Ableton 10 session Toy Midi Keytar
 *  
  Board: TeensyLC
  USB Type: Serial+Midi

  Drives a switch matrix
  
  Using a switch matrix built in to a toy jaminator

  To change the name of the USB-MIDI device, edit the STR_PRODUCT define
  in the /Applications/Arduino.app/Contents/Java/hardware/teensy/avr/cores/usb_midi/usb_private.h
  file. You may need to clear your computers cache of MIDI devices for the name change to be applied.

  See https://www.pjrc.com/teensy/td_midi.html for the Teensy MIDI library documentation.

*/

// Matrix Setup
// Assumes each button has a corresponding LED indicator

const byte NUM_OF_X_PINS = 7;
const byte NUM_OF_Y_PINS = 4;
const byte SWITCHPINS_X[NUM_OF_X_PINS] = { 4, 5, 6, 7, 8, 9, 10 };  // pins to which the x row of switches are connected.
const byte SWITCHPINS_Y[NUM_OF_Y_PINS] = { 0, 1, 2, 3 }; // pins to which the y col of switches are connected.

const byte NUM_OF_SWITCHES = NUM_OF_X_PINS * NUM_OF_Y_PINS ; // number of buttons w/ LEDs.
const byte SWITCHPIN_RESET = 22;

const byte NUM_OF_GROUPS = 4 ; // There are groups of virtual "buttons" with their own states tracked.
// Increase the memory dimension by this much to track the states of those vButtons.

const byte NUM_OF_STATES = 4 ; // number of states tracked for each button.
// switchValue tracks the state of all the physical switches and LEDs
// switchValue Switch states
// [x][y][0]=physical button state,
// [x][y][1]=led state,
// [x][y][2]=switch type,
// [x][y][3]=type memory.
//
// Switch type and Type memory
// [x][y][2]=0 - Pushbutton switch. 1 = unpressed. 0 = pressed. Equal to [x][y][0]
// [x][y][2]=1 - KeytarFret group. 12 keytar Frets. Equal to [x][y][0]
const byte NUM_OF_SWITCHVALUE_S1_GROUP = 12;
const byte SWITCHVALUE_S1_GROUP[NUM_OF_SWITCHVALUE_S1_GROUP][2] = { {3, 0}, {3, 1}, {3, 2}, {3, 3}, {4, 0}, {4, 1}, {4, 2}, {4, 3}, {5, 0}, {5, 1}, {5, 2}, {5, 3}, };
// [x][y][2]=2 - Octave Buttons. Global byte octave tracks octave
const byte NUM_OF_SWITCHVALUE_S2_GROUP = 5;
const byte SWITCHVALUE_S2_GROUP[NUM_OF_SWITCHVALUE_S2_GROUP][2] = { {0, 3}, {2, 3}, {2, 0}, {2, 1}, {2, 2} };
byte octave = 0;
// [x][y][2]=3 - Jam Bar group. Electrical Mess
const byte NUM_OF_SWITCHVALUE_S3_GROUP = 4;
const byte SWITCHVALUE_S3_GROUP[NUM_OF_SWITCHVALUE_S3_GROUP][2] = { {6, 0}, {6, 1}, {6, 2}, {6, 3} };
// [x][y][2]=4 - Activate Trigger mode Toggle button. bool triggerMode track activated (trigger mode) state
// triggerMode = FALSE - "sticky" mode, Activate group push once for midi on, again for midi off
// triggerMode = TRUE - "trigger" mode, Activate group push once for midi on, release for midi off
const byte NUM_OF_SWITCHVALUE_S4_GROUP = 0;
const byte SWITCHVALUE_S4_GROUP[NUM_OF_SWITCHVALUE_S4_GROUP][2] = { };
bool triggerMode = false;
// [x][y][2]=5 - Backspace keyboard key - for deleting clips and such
const byte NUM_OF_SWITCHVALUE_S5_GROUP = 0;
const byte SWITCHVALUE_S5_GROUP[NUM_OF_SWITCHVALUE_S5_GROUP][2] = { };
//
byte switchValue[NUM_OF_X_PINS][NUM_OF_Y_PINS][NUM_OF_STATES];

const int DEBOUNCE_TIME = 200; // button debounce time - microseconds between scans (1000 microsec - 1 millisec).
const byte MIDI_PULSE_TIME = 1; // amount of time to pulse midi on-off for automation triggers

int cycleCount = 0; // for tracking cycles & blinking effects
const int BLINKCYCLE = 250; // Toggle blink effect status once every this many execution cycles

// the MIDI channel number to send messages
const byte MIDI_CHAN = 3;

const byte MIDI_NOTES[3][NUM_OF_X_PINS][NUM_OF_Y_PINS] = { { { 105, 108, 111, 114 }, // Notes / Cntrls [0]
                                                             { 106, 109, 112, 115 }, 
                                                             { 107, 110, 113, 116 }, 
                                                             { 33, 34, 35, 36 },      // Keytar keys
                                                             { 37, 38, 39, 40 },     // + 5 octaves
                                                             { 41, 42, 43, 44 },     // = 33 - 104
                                                             { 117, 118, 119, 120 }
                                                           } ,
                                                           { { 127, 127, 127, 127 }, // Velocities [1]
                                                             { 127, 127, 127, 127 }, // 0 = ff
                                                             { 127, 127, 127, 127 }, // 1-127 = on
                                                             { 128, 128, 128, 128 }, // 128 = on + random 80-127
                                                             { 128, 128, 128, 128 }, //          + octaves
                                                             { 128, 128, 128, 128 }, //          ...
                                                             { 127, 127, 127, 127 }
                                                           } ,
                                                           { { 0, 0, 0, 0 }, // 0=midi note, 1=cntrl [2]
                                                             { 0, 0, 0, 0 }, 
                                                             { 0, 0, 0, 0 }, 
                                                             { 0, 0, 0, 0 }, 
                                                             { 0, 0, 0, 0 }, 
                                                             { 0, 0, 0, 0 }, 
                                                             { 0, 0, 0, 0 }
                                                           }
                                                         };

byte resetmode = 0; // To track how many times setup() is run, for the reset button

char serialBuf[60]; // Serial output buffer

void setup() {

  // Open up Serial for logging
  Serial.begin(9600); // DEBUG: USB is always 12Mbit/sec no matter the baud rate here

  // Initialize the reset button
  pinMode(SWITCHPIN_RESET, INPUT_PULLUP);

  // Initialize the switches & LEDs
    for (byte i = 0; i < NUM_OF_X_PINS; i++) {
    pinMode(SWITCHPINS_X[i], INPUT_PULLUP); // Configure the switch x pins for input mode with pullup resistors.
  }

  for (byte i = 0; i < NUM_OF_Y_PINS; i++) {
    pinMode(SWITCHPINS_Y[i], INPUT_PULLUP); // Configure the switch y pins for input mode with pullup resistors.
  }



  randomSeed(analogRead(23)); // Leave Analog pin 23 unconnected and random noise will seed the rand numbers

  // Initialize storage
  // Default out everything
  for (byte i = 0; i < NUM_OF_X_PINS; i++) {
    for (byte j = 0; j < NUM_OF_Y_PINS; j++) {
      switchValue[i][j][0] = 1; // set all switches to 'off' to start with (1 is off in this logic)
      switchValue[i][j][1] = HIGH; // set all LEDs to 'high' to start with (HIGH is off in this logic)
      switchValue[i][j][2] = 0; // Set to switchType 0 - pushbutton switch
      switchValue[i][j][3] = 1; // set all switches to 'off' to start with (1 is off in this logic)
    }
  }
  // Initializing First group - Keytar Frets
  for (byte i=0; i<NUM_OF_SWITCHVALUE_S1_GROUP; i++) {
    switchValue[SWITCHVALUE_S1_GROUP[i][0]][SWITCHVALUE_S1_GROUP[i][1]][2] = 1; // Set to switchType 1 - Keytar Fret group
  }
    // Initializing Second group - Octave buttons
  for (byte i=0; i<NUM_OF_SWITCHVALUE_S2_GROUP; i++) {
    switchValue[SWITCHVALUE_S2_GROUP[i][0]][SWITCHVALUE_S2_GROUP[i][1]][2] = 2; // Set to switchType 2 - Octave buttons
  }
  // Initializing Third group - Jam Bar group
  for (byte i=0; i<NUM_OF_SWITCHVALUE_S3_GROUP; i++) {
    switchValue[SWITCHVALUE_S3_GROUP[i][0]][SWITCHVALUE_S3_GROUP[i][1]][2] = 3; // Set to switchType 3 - Jam Bar
  }
  // Initializing Fourth group - TriggerMode toggle group
  for (byte i=0; i<NUM_OF_SWITCHVALUE_S4_GROUP; i++) {
    switchValue[SWITCHVALUE_S4_GROUP[i][0]][SWITCHVALUE_S4_GROUP[i][1]][2] = 4; // Set to switchType 4 - TriggerMode toggle group
  }
    // Initializing Fifth group - Backspace key
  for (byte i=0; i<NUM_OF_SWITCHVALUE_S5_GROUP; i++) {
    switchValue[SWITCHVALUE_S5_GROUP[i][0]][SWITCHVALUE_S5_GROUP[i][1]][2] = 5; // Set to switchType 5 - Backspace key
  }

  resetmode = 1; // Track that setup() has run

}


byte setButton (byte xpin, byte ypin, byte ledStateNew, byte switchValueNew, byte midiOnOff=0) {
  // midiOnOff
  // 0 = off
  // 1 = on
  // 2 = no note

  sprintf(serialBuf, " - MIDI: %d NOTE: %d VEL: %d CHAN: %d", midiOnOff, MIDI_NOTES[0][xpin][ypin], MIDI_NOTES[1][xpin][ypin], MIDI_CHAN);
  Serial.println(serialBuf);

  byte velocity = 0;
  byte note = 0;

  if (MIDI_NOTES[1][xpin][ypin] == 128) { // If velocity is 128
    velocity = 127 - random(47);          // mix in a little randomness
    note = MIDI_NOTES[0][xpin][ypin] + (octave * 12); // multiply in octave
  }
  else {
    velocity = MIDI_NOTES[1][xpin][ypin]; // straight velocity
    note = MIDI_NOTES[0][xpin][ypin]; // no octave adjustment
  }

  if (midiOnOff == 1) { // Send a midi On
    if (MIDI_NOTES[2][xpin][ypin] == 0 ) { // Send a Midi Note
      usbMIDI.sendNoteOn (note, velocity, MIDI_CHAN);
    }
    if (MIDI_NOTES[2][xpin][ypin] == 1 ) { // Send a Cntrl Message
      usbMIDI.sendControlChange (note, velocity, MIDI_CHAN);
    }
  }
  else if (midiOnOff == 0) { // Send a midi Off
    if (MIDI_NOTES[2][xpin][ypin] == 0 ) { // Send a Midi Note
      usbMIDI.sendNoteOff (note, 0, MIDI_CHAN);
    }
    if (MIDI_NOTES[2][xpin][ypin] == 1 ) { // Send a Cntrl Message
      usbMIDI.sendControlChange (note, 0, MIDI_CHAN);
    }
  }

  // Set the new LED and switch value states.
  switchValue[xpin][ypin][1] = ledStateNew;
  switchValue[xpin][ypin][3] = switchValueNew;

  return switchValueNew;
}


void loop() {
  
  for (byte xpin = 0; xpin < NUM_OF_X_PINS; xpin++) { // Loop through every digital x pin

    // Reset Button Scan
    if (digitalRead(SWITCHPIN_RESET) == LOW) {
      setup();
    }

    // Switch pins 'On'
    pinMode(SWITCHPINS_X[xpin], OUTPUT); // Sets the digital x pin as output
    digitalWrite(SWITCHPINS_X[xpin], LOW); // 0 volts (https://www.arduino.cc/en/Reference/DigitalWrite)

    // Reset Button Scan
    if (digitalRead(SWITCHPIN_RESET) == LOW) {
      setup();
    }

    // Switch Scan
    for (byte ypin = 0; ypin < NUM_OF_Y_PINS; ypin++) { // Loop through every digital y pin
      byte switchCurrentValue = digitalRead(SWITCHPINS_Y[ypin]); // Read Y Pin and set CurrentValue of the Switch. 1 = off, 0 = on
        
      if ((switchValue[xpin][ypin][0] != switchCurrentValue)) { // If there's an edge change in the switch
        switchValue[xpin][ypin][0] = switchCurrentValue; // Record the new value

        sprintf(serialBuf, "XPin: %d YPin: %d Val: %d Led: %d Typ: %d Mem: %d Oct: %d", xpin, ypin, switchValue[xpin][ypin][0], switchValue[xpin][ypin][1], switchValue[xpin][ypin][2], switchValue[xpin][ypin][3], octave);
        Serial.println(serialBuf);

        switch (switchValue[xpin][ypin][2]) {
          case 0: // Pushbutton Switch
            if (switchCurrentValue == 0) { // if the button is PRESSED
              setButton(xpin, ypin, LOW, 0, 1); // Turn on the switch, Pulse midi on
            }
            else { // if the button is RELEASED
              setButton(xpin, ypin, HIGH, 1, 0); // pulse midi off
            }
            break;
          case 1: // Keytar Fret Switch
            if (switchCurrentValue == 0) { // if the button is PRESSED
              setButton(xpin, ypin, LOW, 0, 1); // Turn on the switch, Pulse midi on
            }
            else { // if the button is RELEASED
              setButton(xpin, ypin, HIGH, 1, 0); // pulse midi off
            }
            break;
          case 2: // Octave Button
            if (switchCurrentValue == 0) { // if the button is PRESSED
              setButton(xpin, ypin, LOW, 0, 2); // Turn on the switch, Ignore Midi
              if ((xpin == 0) and (ypin == 3)) { // if it's "octave down" button
                if (octave > 0) { octave--; }
              }
              else if ((xpin == 2) and (ypin == 3)) { // if it's "octave up"
                if (octave < 3) { octave++; }
              }
              else if ((xpin == 2) and (ypin == 0)) { // return to 0 button
                octave = 0;
              }
              else if ((xpin == 2) and (ypin == 1)) { // octave 4
                octave = 4;
              }
              else if ((xpin == 2) and (ypin == 2)) { // octave 5
                octave = 5;
              }
            }
            else { // if the state is ON
              setButton(xpin, ypin, HIGH, 1, 2); // Turn off the switch, Ignore Midi
            }
            break;
          case 3: // Activate group
            if (switchCurrentValue == 0) { // if the button is PRESSED
              if (switchValue[xpin][ypin][3] == 1) { // If the state is OFF
                // Turn on this switch
                setButton(xpin, ypin, LOW, 0, 2); // Turn on the switch, Ignore midi
              }
            }
            else { // if the button is RELEASED
              if (triggerMode == true) {
                setButton(xpin, ypin, HIGH, 1, 2); // Turn off the switch, Ignore midi
              }
            }
            break;
          case 4: // TriggerMode toggle group
            if (switchCurrentValue == 0) { // if the button is PRESSED
              if (switchValue[xpin][ypin][3] == 1) { // If the state is OFF
                // Turn on this switch
                setButton(xpin, ypin, LOW, 0, 2); // Turn on the switch, Ignore Midi
                triggerMode = true;
              }
              else { // if the state is ON
                setButton(xpin, ypin, HIGH, 1, 2); // Turn off the switch, Ignore midi
                triggerMode = false;
              }
            }
            break;
            case 5: // Backspace key
            if (switchCurrentValue == 0) { // if the button is PRESSED
              if (switchValue[xpin][ypin][3] == 1) { // If the state is OFF
                // Turn on this switch
                setButton(xpin, ypin, LOW, 0, 2); // Turn on the switch, Ignore Midi
                //
              }
              else { // if the state is ON
                setButton(xpin, ypin, HIGH, 1, 2); // Turn off the switch, Ignore midi
                //
              }
            }
            break;
        }

      }
      delayMicroseconds(DEBOUNCE_TIME); // DEBOUNCE - Microseconds between each button scan to make button input more stable
      
    }

    // Switch pins 'Off'
    pinMode(SWITCHPINS_X[xpin], INPUT_PULLUP); // Stop the digitalWrite for the switchreading
  }
}
