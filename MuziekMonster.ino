#include <Bounce.h>
 
/*
*/
 
// pin definitions
const int digital_pin[] = { 8, 9, 10,   11, 12, 13,    18, 19, 20,    21, 22, 23,    28, 29, 30, 31,      32, 33, 34, 35,     2, 3, 4, 5,   41,   36,   37};
const int analog_pin[] = { A0, A4, A5, A7, A2, 0, 1, 2, 3, 4, 5, 6 }; // 5 on Teensy, 7 on 4051
 
// variables for the states of the controls
boolean digital_stored_state[27];
int analog_stored_state[12];
 
// amount of change that constitutes sending a midi message
const int analog_threshold = 10;
const int analog_scale = 8;

// 4051 stuff
const byte s0 = 17;
const byte s1 = 7;
const byte s2 = 6;
const byte analog4051 = A6;
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

                                     // Accoorden               // Bas                      // Drumcomputer                        // Samples         //Synth  // Start  //stop
const int digital_note[3][27] =  {{  36, 38, 40, 41, 43, 45,    24, 26, 28, 29, 31, 33,     0,  1,  2,  3,      4,  5,  6,  7,     15, 14, 13, 12,    61,      9,        8},
                                  {  60, 62, 64, 65, 67, 69,    48, 50, 52, 53, 55, 57,     0,  1,  2,  3,      4,  5,  6,  7,     19, 18, 17, 16,    63,      10,       8},
                                  {  84, 86, 88, 89, 91, 93,    72, 74, 76, 77, 79, 81,     0,  1,  2,  3,      4,  5,  6,  7,     23, 22, 21, 20,    65,      11,       8}};                                  


/*
  28 Tempo
  70 Bass triplets
  21 Snare plekje
  71 Accoorden effect
  22 Bas effect
  73 Sample effect
  74 Master effect 1
  75 Master effect 2
  -1 is een grapje, de eerste is pitchbend.
*/
                                     
const int analog_control[3][27] = {{ -1, 70, 21, 22, 71,  0,  74,  75,  28,  4,  73,  6 },
                                   { -1, 70, 21, 22, 71,  0,  74,  75,  28,  4,  73,  6 },
                                   { -1, 70, 21, 22, 71,  0,  74,  75,  28,  4,  73,  6 }};
 
const byte beat_leds[2][4] ={{ 27,  0,  1, 15 },
                             { 25, 26, 24, 14 }};
byte beat_led_values[2][4] ={{  0,  0,  0,  0 },
                             {  0,  0,  0,  0 }};

void setup() {
  Serial.begin(115200);
 
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
  for (b = 0; b<= 3; b++) {
    Serial.print("Setup: analog pin index ");
    Serial.print(b);
    Serial.print(" on teensy pin ");
    Serial.print(analog_pin[b]);
    Serial.print(" as MIDI CC ");
    Serial.println(analog_control[mode][b]);
    analog_stored_state[b] = 0;     
  }
  
  for (b = 4; b<= 10; b++) {
    Serial.print("Setup: analog pin index ");
    Serial.print(b);
    Serial.print(" on 4051 pin ");
    Serial.print(analog_pin[b]);
    Serial.print(" as MIDI CC ");
    Serial.println(analog_control[mode][b]);
    analog_stored_state[b] = 0; 
  }
  
  // For 4051 
  pinMode(17, OUTPUT);
  pinMode( 7, OUTPUT);
  pinMode( 6, OUTPUT);  
  pinMode(analog4051, INPUT);
  
  // output pins
  pinMode(14, OUTPUT);
  pinMode(15, OUTPUT);
  pinMode(16, OUTPUT); // Status
  pinMode(24, OUTPUT);  
  pinMode(25, OUTPUT);  
  pinMode(26, OUTPUT);    
  pinMode(27, OUTPUT);  
  pinMode( 0, OUTPUT);  
  pinMode( 1, OUTPUT); 
  
  analogWrite(14, 0);
  analogWrite(15, 0);
  analogWrite(16, 32);
  analogWrite(24, 0);  
  analogWrite(25, 0);  
  analogWrite(26, 0);  
  analogWrite(27, 0);  
  analogWrite( 0, 0);  
  analogWrite( 1, 0);  

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
        usbMIDI.sendNoteOn(digital_note[mode][b], midi_vel, midi_chan);
        /*Serial.print("MIDI note on: ");*/
        Serial.print("On: ");
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
  for (b = 0; b <= 4; b++) {
    analog_state = analogRead(analog_pin[b]);
    if (analog_state - analog_stored_state[b] >= analog_threshold || analog_stored_state[b] - analog_state >= analog_threshold) {
      int scaled_value = analog_state / analog_scale;
      if (analog_pin[b] == A0) {
        usbMIDI.sendPitchBend(analog_state * 16, 1);
        Serial.print("Pitch ");
      }
      else {
        usbMIDI.sendControlChange(analog_control[mode][b], scaled_value, midi_chan);
      }
      Serial.print("analog value ");
      Serial.print(analog_control[mode][b]); 
      Serial.print(": ");
      Serial.print(analog_state);
      Serial.print(" scaled: ");
      Serial.println(scaled_value);
      analog_stored_state[b] = analog_state;
    }
  }

  // 4051 analog pins, 7 are connected.
  for (b = 0; b <= 6; b++) {

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
        Serial.print(" mode result: ");
        Serial.println(mode);
        modeStored = analog_state;
      }
    }
    else {
      if (analog_state - analog_stored_state[b+5] >= analog_threshold || analog_stored_state[b+5] - analog_state >= analog_threshold) {
        int scaled_value = analog_state / analog_scale;
  
        usbMIDI.sendControlChange(analog_control[mode][b+5], scaled_value, midi_chan);
  
        Serial.print("analog 4051 ");
        Serial.print(analog_control[mode][b+5]); 
        Serial.print(": ");
        Serial.print(analog_state);
        Serial.print(" scaled: ");
        Serial.println(scaled_value);
        analog_stored_state[b+5] = analog_state;
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
          analogWrite(beat_leds[0][lastbeat - 1], beat_led_values[0][lastbeat - 1]);
          analogWrite(beat_leds[0][beat - 1], beat_led_values[0][beat - 1] + 128);

          analogWrite(beat_leds[1][lastbeat - 1], beat_led_values[1][lastbeat - 1]);
          analogWrite(beat_leds[1][beat - 1], beat_led_values[1][beat - 1] + 128);
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
          if (beat == beatindex + 1) {
            analogWrite(beat_leds[row][beatindex], 32);                      
          }
          else {
            analogWrite(beat_leds[row][beatindex], 16);          
          }          
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
          if (beat == beatindex + 1) {
            analogWrite(beat_leds[row][beatindex], 32);                      
          }
          else {
            analogWrite(beat_leds[row][beatindex], 0);          
          }
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
