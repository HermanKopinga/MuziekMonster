#include <Bounce.h>
#include "Tlc5940.h"
 
/*
*/
 
// pin definitions
const int digital_pin[] = { 24, 23, 18,    19, 40, 41,    2, 0, 3,   13, 4, 5,   32, 33, 28, 29,      30, 31, 34, 35,     9, 10, 11, 12,     27,   36,   37};
const int analog_pin[] = { A0, A1, 0, 1, 2, 3, 4, 5, 6, 7 }; // 2 on Teensy, 8 on 4051
 
// variables for the states of the controls
boolean digital_stored_state[27];
int analog_stored_state[10];
 
// amount of change that constitutes sending a midi message
const int analog_threshold = 10;
const int analog_scale = 8;

// 4051 stuff
const byte s0 = 6;
const byte s1 = 7;
const byte s2 = 8;
const byte analog4051 = A7;
bool r0 = 0;
bool r1 = 0;
bool r2 = 0;
 
// Debounce
long debounceDelay = 20;
 
Bounce digital_debouncer[] = {
      Bounce(digital_pin[0], debounceDelay),
      Bounce(digital_pin[1], debounceDelay),
      Bounce(digital_pin[2], debounceDelay),
      Bounce(digital_pin[3], debounceDelay),
      Bounce(digital_pin[4], debounceDelay),
      Bounce(digital_pin[5], debounceDelay),
      Bounce(digital_pin[6], debounceDelay),
      Bounce(digital_pin[7], debounceDelay),
      Bounce(digital_pin[8], debounceDelay),
      Bounce(digital_pin[9], debounceDelay),
      Bounce(digital_pin[10], debounceDelay),
      Bounce(digital_pin[11], debounceDelay),
      Bounce(digital_pin[12], debounceDelay),
      Bounce(digital_pin[13], debounceDelay),
      Bounce(digital_pin[14], debounceDelay),
      Bounce(digital_pin[15], debounceDelay),
      Bounce(digital_pin[16], debounceDelay),
      Bounce(digital_pin[17], debounceDelay),
      Bounce(digital_pin[18], debounceDelay),
      Bounce(digital_pin[19], debounceDelay),
      Bounce(digital_pin[20], debounceDelay),
      Bounce(digital_pin[21], debounceDelay),
      Bounce(digital_pin[22], debounceDelay),
      Bounce(digital_pin[23], debounceDelay),
      Bounce(digital_pin[24], debounceDelay),
      Bounce(digital_pin[25], debounceDelay),
      Bounce(digital_pin[26], debounceDelay)
};
 
// Mode switch provisions.
// 0 is Powerpop
// 1 is Hiphop
// 3 is Hardcore
byte mode = 0;
int modeStored = 0;

// MIDI settings
const int midi_vel = 100;
const int midi_chan = 1;

int beat;

                                     // Bas                     // Accoorden                // Drumcomputer                        // Samples         //Synth  // Start  //stop
const int digital_note[3][27] =  {{  36, 38, 40, 41, 43, 45,    24, 26, 28, 29, 31, 33,     0,  1,  2,  3,      4,  5,  6,  7,     15, 14, 13, 12,    61,      9,        8},
                                  {  60, 62, 64, 65, 67, 69,    48, 50, 52, 53, 55, 57,     0,  1,  2,  3,      4,  5,  6,  7,     19, 18, 17, 16,    63,      10,       8},
                                  {  84, 86, 88, 89, 91, 93,    72, 74, 76, 77, 79, 81,     0,  1,  2,  3,      4,  5,  6,  7,     23, 22, 21, 20,    66,      11,       8}};                                  


/*
  -1 is een grapje, de eerste is pitchbend.
   0 is ook een grapje, mode heeft geen CC.
  21 Snare plekje
  22 Bas effect
  28 Tempo
  70 Bass triplets
  71 Accoorden effect
  73 Sample effect
  74 Master effect 1
  75 Master effect 2
*/

const int analog_control[10] = { 22,  71,   0,  70,  28,  21,  -1,  73,  74,  75 };
 
const byte beat_leds[2][4] ={{  7,  9, 11, 13 },
                             {  8, 10, 12, 14 }};
byte beat_led_values[2][4] ={{  0,  0,  0,  0 },
                             {  0,  0,  0,  0 }};

void setup() {
  Serial.begin(115200);

  Tlc.init();

  // set the pin modes && zero saved states
  int b = 0;
  
  // digital pins
  for (b = 0; b <= 26; b++) {
    pinMode(digital_pin[b], INPUT_PULLUP);
    digital_stored_state[b] = true;
    Serial.print("Setup: digital pin index ");
    Serial.print(b);
    Serial.print(" on teensy pin ");
    Serial.print(digital_pin[b]);
    Serial.print(" as MIDI note ");
    Serial.println(digital_note[mode][b]);
  }
  
  // analog pins
  for (b = 0; b<= 1; b++) {
    Serial.print("Setup: analog pin index ");
    Serial.print(b);
    Serial.print(" on teensy pin ");
    Serial.print(analog_pin[b]);
    Serial.print(" as MIDI CC ");
    Serial.println(analog_control[b]);
    analog_stored_state[b] = 0;     
  }
  
  for (b = 2; b<= 9; b++) {
    Serial.print("Setup: analog pin index ");
    Serial.print(b);
    Serial.print(" on 4051 pin ");
    Serial.print(analog_pin[b]);
    Serial.print(" as MIDI CC ");
    Serial.println(analog_control[b]);
    analog_stored_state[b] = 0; 
  }
  
  // For 4051 
  pinMode(s0, OUTPUT);
  pinMode(s1, OUTPUT);
  pinMode(s2, OUTPUT);  
  pinMode(analog4051, INPUT);
  
  // output pins
  //ToDo: 5940 stuff goes here.
}
 
void loop() {
  int b = 0;
  int type, note, velocity, channel, d1, d2, beatindex, row, analog_state;
  
  // digital pins
  for (b = 0; b <= 26; b++) {
    digital_debouncer[b].update();
    boolean state = digital_debouncer[b].read();
    if (state != digital_stored_state[b]) {
      if (state == false) {
        // For identifying the buttons during setup.
        Serial.print("Pin: ");
        Serial.print(digital_pin[b]);
        usbMIDI.sendNoteOn(digital_note[mode][b], midi_vel, midi_chan);
        /*Serial.print("MIDI note on: ");*/
        Serial.print(" On: ");
        Serial.println(digital_note[mode][b]);
      } else {
        usbMIDI.sendNoteOff(digital_note[mode][b], midi_vel, midi_chan);
        /*Serial.print("MIDI note off: ");
        Serial.println(digital_note[b]);*/
      }
      digital_stored_state[b] = !digital_stored_state[b];
    }
  }

  // analog pins
  for (b = 0; b <= 1; b++) {
    analog_state = analogRead(analog_pin[b]);
    if (analog_state - analog_stored_state[b] >= analog_threshold || analog_stored_state[b] - analog_state >= analog_threshold) {
      int scaled_value = analog_state / analog_scale;
      Serial.print("analog value ");
      Serial.print(analog_control[b]); 
      Serial.print(": ");
      Serial.print(analog_state);
      Serial.print(" scaled: ");
      Serial.println(scaled_value);
      analog_stored_state[b] = analog_state;
    }
  }

  // 4051 analog pins, 8 are connected.
  for (b = 0; b <= 7; b++) {

    // select the bit  
    r0 = bitRead(b,0);
    r1 = bitRead(b,1);
    r2 = bitRead(b,2);

    digitalWrite(s0, r0);
    digitalWrite(s1, r1);
    digitalWrite(s2, r2);

    analog_state = analogRead(analog4051);    

    if (b == 0) { 
      // Read Mode Switch.
      if (analog_state - modeStored >= analog_threshold || modeStored - analog_state >= analog_threshold) {
        Serial.print("Mode_analog: ");
        Serial.print(analog_state);
        mode = analog_state/174;
        if (mode > 2) {
          mode = 2;
        }
        Serial.print(" mode result: ");
        Serial.print(mode);
        modeStored = analog_state;
        Serial.print(". Sending: ");
        Serial.print(digital_note[mode][26]);
        Serial.print(" "); 
        Serial.print(digital_note[mode][26]);
        Serial.print(" ");
        Serial.println(digital_note[mode][25]);
        usbMIDI.sendNoteOn(digital_note[mode][26], midi_vel, midi_chan);
        usbMIDI.sendNoteOff(digital_note[mode][26], midi_vel, midi_chan);
        usbMIDI.send_now();
        delay(100);
        usbMIDI.sendNoteOn(digital_note[mode][26], midi_vel, midi_chan);
        usbMIDI.sendNoteOff(digital_note[mode][26], midi_vel, midi_chan);
        usbMIDI.send_now();        
        delay(100);        
        usbMIDI.sendNoteOn(digital_note[mode][25], midi_vel, midi_chan);      
        usbMIDI.sendNoteOff(digital_note[mode][25], midi_vel, midi_chan);        
      }
    }
    else {
      if (analog_state - analog_stored_state[b+2] >= analog_threshold || analog_stored_state[b+2] - analog_state >= analog_threshold) {
        int scaled_value = analog_state / analog_scale;
  
        if (b == 4) {
          usbMIDI.sendPitchBend(analog_state * 16, 1);
        Serial.print("Pitch ");
        }
        else {
          usbMIDI.sendControlChange(analog_control[b], scaled_value, midi_chan);
        }    
  
        Serial.print("analog 4051 ");
        Serial.print(analog_control[b+2]); 
        Serial.print(": ");
        Serial.print(analog_state);
        Serial.print(" scaled: ");
        Serial.println(scaled_value);
        analog_stored_state[b+2] = analog_state;
      }  
    }  
  }

  ////////////////////////////////////////
  //  R  e  a  d
  ////////////////////////////////////////
  
  if (usbMIDI.read()) {                    // Is there a MIDI message incoming ?
    byte type = usbMIDI.getType();
    switch (type) {
      case 1: // NoteOn
        note = usbMIDI.getData1();
        velocity = usbMIDI.getData2();
        channel = usbMIDI.getChannel();
        if (velocity > 0) {
          Serial.println(String("In Note On:  ch=") + channel + ", note=" + note + ", velocity=" + velocity);
        } else {
          Serial.println(String("In Note On: ch=") + channel + ", note=" + note);
        }
        // Special case to show what beat we are in.
        // Sent as midi note with changing velocity.
        if (channel == 16 && note == 60) {
          beat = (velocity / 32) + 1;
          int lastbeat = beat - 1;
          if (beat == 1) {
            lastbeat = 4;
          }
          Tlc.set(beat_leds[0][lastbeat - 1], beat_led_values[0][lastbeat - 1]);
          Tlc.set(beat_leds[0][beat - 1], beat_led_values[0][beat - 1] + 512);

          Tlc.set(beat_leds[1][lastbeat - 1], beat_led_values[1][lastbeat - 1]);
          Tlc.set(beat_leds[1][beat - 1], beat_led_values[1][beat - 1] + 512);
          Tlc.update();
          Serial.println(String("Beat: ") + beat + String(" Lastbeat: ") + lastbeat);
        }
        // A note we are 'following' is switched on.
        // Decide what row and index this is.
        if (channel == 1) {
          switch (note) {
            case 0:              beatindex = 0;              row = 0;              break;
            case 1:              beatindex = 1;              row = 0;              break;
            case 2:              beatindex = 2;              row = 0;              break;
            case 3:              beatindex = 3;              row = 0;              break;
            case 4:              beatindex = 0;              row = 1;              break;
            case 5:              beatindex = 1;              row = 1;              break;
            case 6:              beatindex = 2;              row = 1;              break;
            case 7:              beatindex = 3;              row = 1;              break;
            default:
              Serial.println("Whelp");
              break;
          } 
          beat_led_values[row][beatindex] = 16 ;
          if (beat -1 == beatindex) {
            Tlc.set(beat_leds[row][beatindex], 320);
          }
          else {
            Tlc.set(beat_leds[row][beatindex], 160);
          }
          Tlc.update();
        }
        break;
      case 0: // NoteOff
        note = usbMIDI.getData1();
        velocity = usbMIDI.getData2();
        channel = usbMIDI.getChannel();
        Serial.println(String("In Note Off: ch=") + channel + ", note=" + note + ", velocity=" + velocity);
        if (channel == 1) {
          switch (note) {
            case 0:              beatindex = 0;              row = 0;              break;
            case 1:              beatindex = 1;              row = 0;              break;
            case 2:              beatindex = 2;              row = 0;              break;
            case 3:              beatindex = 3;              row = 0;              break;
            case 4:              beatindex = 0;              row = 1;              break;
            case 5:              beatindex = 1;              row = 1;              break;
            case 6:              beatindex = 2;              row = 1;              break;
            case 7:              beatindex = 3;              row = 1;              break;       
            default:
              Serial.println("Whelp");
              break;
          } 
          beat_led_values[row][beatindex] = 0 ;
          if (beat -1 == beatindex) {
            Tlc.set(beat_leds[row][beatindex], 320);
          }
          else {
            Tlc.set(beat_leds[row][beatindex], 0);
          }
          Tlc.update();
        }
        break;
      default:
        d1 = usbMIDI.getData1();
        d2 = usbMIDI.getData2();
        channel = usbMIDI.getChannel();
        Serial.println(String("In Message, type=") + type + ", data = " + d1 + " " + d2 + ", channel= " + channel);
    }
  }
}
