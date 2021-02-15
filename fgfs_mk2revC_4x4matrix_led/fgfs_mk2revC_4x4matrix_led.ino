/* Geoff's button matrix Keyboard controller Teensy code
  Board: TeensyLC
  USB Type: Keyboard

  Drives a switch matrix and a corresponding LED matrix
  
  Using a switch matrix design I found at https://forum.arduino.cc/index.php?topic=253215.0
  Schematic https://forum.arduino.cc/index.php?action=dlattach;topic=253215.0;attach=87528

*/

// Matrix Setup
// Assumes each button has a corresponding LED indicator

const byte NUM_OF_X_PINS = 4;
const byte NUM_OF_Y_PINS = 4;
const byte SWITCHPINS_X[NUM_OF_X_PINS] = { 4, 5, 6, 7 };  // pins to which the x row of switches are connected.
const byte SWITCHPINS_Y[NUM_OF_Y_PINS] = { 0, 1, 2, 3 }; // pins to which the y col of switches are connected.
const byte LEDPINS_X[NUM_OF_X_PINS] = { 14, 15, 16, 17 }; // anodes
const byte LEDPINS_Y[NUM_OF_Y_PINS] = { 18, 19, 20, 21 }; // cathodes
const byte NUM_OF_SWITCHES = NUM_OF_X_PINS * NUM_OF_Y_PINS ; // number of buttons w/ LEDs.
const byte SWITCHPIN_RESET = 22;

// const byte NUM_OF_GROUPS = 4 ; // There are groups of virtual "buttons" with their own states tracked.
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

// How To Address each Button:
// {0,3} {1,3} {2,3} {3,3}
// {0,2} {1,2} {2,2} {3,2}
// {0,1} {1,1} {2,1} {3,1}
// {0,0} {1,0} {2,0} {3,0}

// [x][y][2]=0 - Pushbutton switch. 1 = unpressed. 0 = pressed. Equal to [x][y][0]

// [x][y][2]=1 - One-At-A-Time-Only group. 4 button group of exclusive buttons. [x][y][3] tracks activated state
// Only one in the group is "on" at a time
const byte NUM_OF_SWITCHVALUE_S1_GROUP = 0;
const byte SWITCHVALUE_S1_GROUP[NUM_OF_SWITCHVALUE_S1_GROUP][2] = {  };

// [x][y][2]=2 - Individual Pushbutton toggle. [x][y][3] tracks activated state
const byte NUM_OF_SWITCHVALUE_S2_GROUP = 1;
const byte SWITCHVALUE_S2_GROUP[NUM_OF_SWITCHVALUE_S2_GROUP][2] = { {1, 0} };

// [x][y][2]=3 - Activate group. [x][y][3] tracks activated state
// Push for on, release for off
const byte NUM_OF_SWITCHVALUE_S3_GROUP = 1;
const byte SWITCHVALUE_S3_GROUP[NUM_OF_SWITCHVALUE_S3_GROUP][2] = { {0, 0} };

// [x][y][2]=4 - Toggle button
// Push once for on, push again for off
const byte NUM_OF_SWITCHVALUE_S4_GROUP = 0;
const byte SWITCHVALUE_S4_GROUP[NUM_OF_SWITCHVALUE_S4_GROUP][2] = {  };


//
byte switchValue[NUM_OF_X_PINS][NUM_OF_Y_PINS][NUM_OF_STATES];

const int DEBOUNCE_TIME = 100; // button debounce time - microseconds between scans (1000 microsec - 1 millisec).
const byte PULSE_TIME = 1; // amount of time to pulse output on-off for automation triggers

int cycleCount = 0; // for tracking cycles & blinking effects
const int BLINKCYCLE = 250; // Toggle blink effect status once every this many execution cycles

const int CHAN = 0; // channel setting for things that need channels.  Like, Midi.

// How this is arranged
// {0,0} {0,1} {0,2} {0,3}
// {1,0} {1,1} {1,2} {1,3}  (Top edge of unit)
// {2,0} {2,1} {2,2} {2,3}
// {3,0} {3,1} {3,2} {3,3}

const int BUTTON_TRIG_VALS[2][NUM_OF_X_PINS][NUM_OF_Y_PINS] =  { { { MODIFIERKEY_GUI | MODIFIERKEY_SHIFT, MODIFIERKEY_GUI | MODIFIERKEY_SHIFT, MODIFIERKEY_CTRL | MODIFIERKEY_ALT | MODIFIERKEY_GUI, MODIFIERKEY_CTRL | MODIFIERKEY_ALT }, // Values [0]
                                                                   { MODIFIERKEY_GUI | MODIFIERKEY_SHIFT, 0, MODIFIERKEY_CTRL | MODIFIERKEY_ALT, MODIFIERKEY_CTRL | MODIFIERKEY_ALT }, 
                                                                   { 0, 0, MODIFIERKEY_CTRL | MODIFIERKEY_ALT | MODIFIERKEY_GUI, MODIFIERKEY_CTRL | MODIFIERKEY_ALT }, 
                                                                   { MODIFIERKEY_SHIFT | MODIFIERKEY_GUI, 0, 0, MODIFIERKEY_SHIFT | MODIFIERKEY_ALT | MODIFIERKEY_GUI }
                                                                 } ,
                                                                 { { KEY_A, KEY_V, KEY_LEFT, KEY_LEFT }, // Velocities [1]
                                                                   { KEY_A, 0, KEY_BACKSPACE, KEY_ENTER }, 
                                                                   { KEY_ESC, 0, KEY_RIGHT, KEY_RIGHT }, 
                                                                   { KEY_4, 0, 0, KEY_L }
                                                                 } 
                                                               };

byte resetmode = 0; // To track how many times setup() is run, for the reset button

char serialBuf[60]; // Serial output buffer

byte setButton (byte xpin, byte ypin, byte ledStateNew, byte switchValueNew, byte outputOnOff=0) {
  // outputOnOff
  // 0 = off
  // 1 = on
  // 2 = no output

  sprintf(serialBuf, " - OUTPUT: %d NOTE: %d VEL: %d CHAN: %d", outputOnOff, BUTTON_TRIG_VALS[0][xpin][ypin], BUTTON_TRIG_VALS[1][xpin][ypin], CHAN);
  Serial.println(serialBuf);

  if (outputOnOff == 1) {
    // OUTPUT HAPPENS HERE (ON ACTION)
    // usbMIDI.sendNoteOn (BUTTON_TRIG_VALS[0][xpin][ypin], BUTTON_TRIG_VALS[1][xpin][ypin], CHAN);
    Keyboard.set_modifier(BUTTON_TRIG_VALS[0][xpin][ypin]);
    Keyboard.send_now(); // Press and hold modifier keys
    delay(PULSE_TIME); // Hold for PULSE_TIME, in case OS needs to recognise modifiers before the keypress
    Keyboard.set_key1(BUTTON_TRIG_VALS[1][xpin][ypin]);
    Keyboard.send_now(); // Press the shortcut key
  }
  else {
    // OUTPUT HAPPENS HERE (OFF ACTION)
    // usbMIDI.sendNoteOff (BUTTON_TRIG_VALS[0][xpin][ypin], 0, CHAN);
    Keyboard.set_modifier(0);
    Keyboard.set_key1(0);
    Keyboard.send_now(); // Release everything
  }
  
  switchValue[xpin][ypin][1] = ledStateNew;
  switchValue[xpin][ypin][3] = switchValueNew;

  return switchValueNew;
}

void setup() {

  // Open up Serial for logging
  Serial.begin(9600); // DEBUG: USB is always 12Mbit/sec no matter the baud rate here

  // Initialize the reset button
  pinMode(SWITCHPIN_RESET, INPUT_PULLUP);

  // Initialize the switches & LEDs
    for (byte i = 0; i < NUM_OF_X_PINS; i++) {
    pinMode(SWITCHPINS_X[i], INPUT_PULLUP); // Configure the switch x pins for input mode with pullup resistors.
    pinMode(LEDPINS_X[i], OUTPUT); // Configure the led x pins for output mode
  }

  for (byte i = 0; i < NUM_OF_Y_PINS; i++) {
    pinMode(SWITCHPINS_Y[i], INPUT_PULLUP); // Configure the switch y pins for input mode with pullup resistors.
    pinMode(LEDPINS_Y[i], OUTPUT); // Configure the led y pins for output mode
    digitalWrite(LEDPINS_Y[i], HIGH); // Take the Y pins (the cathodes) HIGH
  }

  // Wipe Display Effect

  if (resetmode == 0) {
    // The first time we have run setup(), do the FGFS Logo effect
    byte startUpLedPattrn[4][NUM_OF_X_PINS][NUM_OF_Y_PINS] = {
                                                    {
                                                      {  LOW, HIGH, HIGH, HIGH }, //F
                                                      {  LOW, LOW, LOW, HIGH }, 
                                                      {  LOW, HIGH, HIGH, HIGH }, 
                                                      {  LOW, LOW, LOW, LOW }
                                                    }, 
                                                    {
                                                      {  LOW, LOW, LOW,  LOW }, //G
                                                      {  LOW, HIGH, LOW, HIGH }, 
                                                      {  LOW, HIGH, HIGH, HIGH }, 
                                                      {  LOW, LOW, LOW, LOW }
                                                    }, 
                                                    {
                                                      {  LOW, HIGH, HIGH, HIGH }, //F
                                                      {  LOW, LOW, LOW, HIGH }, 
                                                      {  LOW, HIGH, HIGH, HIGH }, 
                                                      {  LOW, LOW, LOW, LOW }
                                                    }, 
                                                    {
                                                      {  LOW, LOW, LOW, LOW }, //S
                                                      {  HIGH, HIGH, LOW, HIGH }, 
                                                      {  LOW, LOW, HIGH, HIGH }, 
                                                      {  LOW, LOW, LOW, LOW }, 
                                                    }
                                                  };

    for (byte r = 0; r < 4; r++) { // loop through r letters
      for (byte i=0; i<100; i++) { // loop around i times to show each letter for a while
        for (byte xpin = 0; xpin < NUM_OF_X_PINS; xpin++) { // Loop through every digital x pin
          digitalWrite(LEDPINS_X[xpin], HIGH); // set the LED anodes HIGH
          for (byte ypin = 0; ypin < NUM_OF_Y_PINS; ypin++) { // Loop through every digital y pin
            digitalWrite(LEDPINS_Y[ypin], startUpLedPattrn[r][xpin][ypin]); // LOW cathode, anode row is HIGH, led ON
          }
          delay(1); // Gross Dumb Slow Way to do this but it's just a pointless fancy effect anyway.
          for (byte ypin=0; ypin < NUM_OF_Y_PINS; ypin++) { // Loop through all the ypins again
            digitalWrite(LEDPINS_Y[ypin], HIGH); // Turn all the cathodes off again
          }
          digitalWrite(LEDPINS_X[xpin], LOW); // bring down the anodes
        }
      }
    }
  }
  else {
    // Every subsequent time we run setup(), do this LED wash effect
    for (byte i = 1; i < 10; i++) {
      for (byte xpin = 0; xpin < NUM_OF_X_PINS; xpin++) { // Loop through every digital x pin
        digitalWrite(LEDPINS_X[xpin], HIGH); // set the LED anodes HIGH
        for (byte ypin = 0; ypin < NUM_OF_Y_PINS; ypin++) { // Loop through every digital y pin
          if (random(2) == 0) { // whatever cool random effect
            digitalWrite(LEDPINS_Y[ypin], LOW); // LOW cathode, anode row is HIGH, led ON
            delay(50/i);
            digitalWrite(LEDPINS_Y[ypin], HIGH);
          }
        }
        digitalWrite(LEDPINS_X[xpin], LOW); // bring down the anodes
      }
    }
  }

  // Continue with setup stuff

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
  // Initializing First group - Loop
  for (byte i=0; i<NUM_OF_SWITCHVALUE_S1_GROUP; i++) {
    switchValue[SWITCHVALUE_S1_GROUP[i][0]][SWITCHVALUE_S1_GROUP[i][1]][2] = 1; // Set to switchType 1 - One-At-A-Time-Only group
  }
    // Initializing Second group - Individual Pushbutton toggle
  for (byte i=0; i<NUM_OF_SWITCHVALUE_S2_GROUP; i++) {
    switchValue[SWITCHVALUE_S2_GROUP[i][0]][SWITCHVALUE_S2_GROUP[i][1]][2] = 2; // Set to switchType 2 - Individual Pushbutton toggle
    setButton(SWITCHVALUE_S2_GROUP[i][0], SWITCHVALUE_S2_GROUP[i][1], LOW, 0, 2); // Lets default these to ON quietly
  }
  // Initializing Third group - Activate group
  for (byte i=0; i<NUM_OF_SWITCHVALUE_S3_GROUP; i++) {
    switchValue[SWITCHVALUE_S3_GROUP[i][0]][SWITCHVALUE_S3_GROUP[i][1]][2] = 3; // Set to switchType 3 - Activate group
  }
  // Initializing Fourth group - Toggle group
  for (byte i=0; i<NUM_OF_SWITCHVALUE_S4_GROUP; i++) {
    switchValue[SWITCHVALUE_S4_GROUP[i][0]][SWITCHVALUE_S4_GROUP[i][1]][2] = 4; // Set to switchType 4 - Toggle group
  }

  resetmode = 1; // Track that setup() has run

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

    // LED pins 'On'
    digitalWrite(LEDPINS_X[xpin], HIGH); // set the LED anodes HIGH for this row

    // LED 'On' Scan
    // Turn all necessary LEDs on (to LOW) before looping through the switch scan
    // Switch Scan DEBOUNCE_TIME delay becomes the LED on-time
    for (byte ypin = 0; ypin < NUM_OF_Y_PINS; ypin++) { // Loop through every digital y pin
      digitalWrite(LEDPINS_Y[ypin], switchValue[ypin][xpin][1]); // LED matrix is wired mirror of the switch. FIXME fixing in software
    }

    // Reset Button Scan
    if (digitalRead(SWITCHPIN_RESET) == LOW) {
      setup();
    }

    // Switch Scan
    for (byte ypin = 0; ypin < NUM_OF_Y_PINS; ypin++) { // Loop through every digital y pin
      byte switchCurrentValue = digitalRead(SWITCHPINS_Y[ypin]); // Read Y Pin and set CurrentValue of the Switch. 1 = off, 0 = on
        
      if ((switchValue[xpin][ypin][0] != switchCurrentValue)) { // If there's an edge change in the switch
        switchValue[xpin][ypin][0] = switchCurrentValue; // Record the new value

        sprintf(serialBuf, "XPin: %d YPin: %d Val: %d Led: %d Typ: %d Mem: %d", xpin, ypin, switchValue[xpin][ypin][0], switchValue[xpin][ypin][1], switchValue[xpin][ypin][2], switchValue[xpin][ypin][3]);
        Serial.println(serialBuf);

        switch (switchValue[xpin][ypin][2]) {
          case 0: // Pushbutton Switch
            if (switchCurrentValue == 0) { // if the button is PRESSED
              setButton(xpin, ypin, LOW, 0, 1); // Turn on the switch and fire setButton
            }
            else { // if the button is RELEASED
              setButton(xpin, ypin, HIGH, 1, 0); // Turn off the switch and fire setButton
            }
            break;
          case 1: // Only-One-At-A-Time group
            if (switchCurrentValue == 0) { // if the button is PRESSED
              if (switchValue[xpin][ypin][3] == 1) { // If the state is OFF
                // Shut off the other group switches
                for (byte i=0; i<NUM_OF_SWITCHVALUE_S1_GROUP; i++) { // loop through each of the switches in the switchgroup
                  if (switchValue[SWITCHVALUE_S1_GROUP[i][0]][SWITCHVALUE_S1_GROUP[i][1]][3] == 0) { // if the switch is on
                  setButton(SWITCHVALUE_S1_GROUP[i][0], SWITCHVALUE_S1_GROUP[i][1], HIGH, 1, 2); // Turn off the switch and fire setButton
                  }
                }
                // Turn on this switch
                setButton(xpin, ypin, LOW, 0, 1); // Turn on the switch and fire setButton
                delay(PULSE_TIME);
                setButton(xpin, ypin, LOW, 0, 1); // Turn off the switch and fire setButton
              }
              else { // if the state is ON
                // Turn off this switch
                setButton(xpin, ypin, HIGH, 1, 1); // Turn on the switch and fire setButton
                delay(PULSE_TIME);
                setButton(xpin, ypin, HIGH, 1, 1); // Turn off the switch and fire setButton
              }
            }
            break;
          case 2: // Individual Pushbutton toggle
            if (switchCurrentValue == 0) { // if the button is PRESSED
              if (switchValue[xpin][ypin][3] == 1) { // If the state is OFF
                // Turn on this switch
                setButton(xpin, ypin, LOW, 0, 1); // Turn on the switch and fire setButton
                delay(PULSE_TIME);
                setButton(xpin, ypin, LOW, 0, 0); // Turn off the switch and fire setButton
              }
              else { // if the state is ON
                setButton(xpin, ypin, HIGH, 1, 1); // Turn on the switch and fire setButton
                delay(PULSE_TIME);
                setButton(xpin, ypin, HIGH, 1, 0); // Turn off the switch and fire setButton
              }
            }
            break;
          case 3: // Activate group
            if (switchCurrentValue == 0) { // if the button is PRESSED
              if (switchValue[xpin][ypin][3] == 1) { // If the state is OFF
                // Turn on this switch
                setButton(xpin, ypin, LOW, 0, 1); // Turn on the switch and fire setButton
                delay(PULSE_TIME);
                setButton(xpin, ypin, LOW, 0, 0); // Release keystrokes
              }
            }
            else { // if the button is RELEASED
              setButton(xpin, ypin, HIGH, 1, 1); // Turn off the switch and fire setButton
              delay(PULSE_TIME);
              setButton(xpin, ypin, HIGH, 1, 0); // Release keystrokes
            }
            break;
          case 4: // Toggle group
            if (switchCurrentValue == 0) { // if the button is PRESSED
              if (switchValue[xpin][ypin][3] == 1) { // If the state is OFF
                // Turn on this switch
                setButton(xpin, ypin, LOW, 0, 1); // Turn on the switch and fire setButton
              }
              else { // if the state is ON
                setButton(xpin, ypin, HIGH, 1, 1); // Turn off the switch and fire setButton
              }
            }
            break;
        }

      }
      delayMicroseconds(DEBOUNCE_TIME); // DEBOUNCE - Microseconds between each button scan to make button input more stable
      
    }

    if (cycleCount == BLINKCYCLE) { // if we have looped around enough for a blink
      for (int i = 0; i < NUM_OF_SWITCHVALUE_S1_GROUP; i++) { // loop through each of the one-at-a-time group buttons
        if (switchValue[SWITCHVALUE_S1_GROUP[i][0]][SWITCHVALUE_S1_GROUP[i][1]][3] == 0) { // if the switch is on
          // Invert the LED statuses
          if (switchValue[SWITCHVALUE_S1_GROUP[i][0]][SWITCHVALUE_S1_GROUP[i][1]][1] == HIGH) {
            switchValue[SWITCHVALUE_S1_GROUP[i][0]][SWITCHVALUE_S1_GROUP[i][1]][1] = LOW;
          }
          else {
            switchValue[SWITCHVALUE_S1_GROUP[i][0]][SWITCHVALUE_S1_GROUP[i][1]][1] = HIGH;
          }
        }
      }
      cycleCount = 0;
    }
    else {
      cycleCount++;
    }

    // LED 'Off' Scan
    // LED cathode pins 'Off'
    for (byte i = 0; i < NUM_OF_Y_PINS; i++) { //loop through Y pins
      if (switchValue[xpin][i][1] == LOW) { // If the pin is LOW ...(= on)
        digitalWrite(LEDPINS_Y[i], HIGH); // turn it off for now
      }
    }
    // LED anode pin 'Off'
    digitalWrite(LEDPINS_X[xpin], LOW); // Bring the LED Anodes Low

    // Switch pins 'Off'
    pinMode(SWITCHPINS_X[xpin], INPUT_PULLUP); // Stop the digitalWrite for the switchreading
  }
}
