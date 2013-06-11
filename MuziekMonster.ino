#include <Bounce.h>
 
/*
      DJ Controller test 002
      Source: http://redbinary.com/pacmod-midi-dj-controller/
*/
 
// pin definitions
const int digital_pin[] = {  8, 9, 10,   11, 12, 13,    18, 19, 20,    21, 22, 23,    28, 29, 30, 31,      32, 33, 34, 35,     2, 3, 4, 5, 17,    36, 37};
const int notepincount = 27;
const int numberofpins = notepincount - 1;

const int analog_pin[] = { A4, A5, A6, A7, A3, A2, A1, A0 };
const int analogpins = 8 - 1;

// variables for the states of the controls
boolean digital_stored_state[6][numberofpins];
int analog_stored_state[6][analogpins];
 
// amount of change that constitutes sending a midi message
const int analog_threshold = 15;
const int analog_scale = 8;

// Debounce
const long debounceDelay = 20;
 
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

// 0 is default mode
// 1 is alternate mode
//const int modePin = 6;
int mode = 0;
int modeState = 1;
int modeStored = 0;

 
// MIDI settings
int midi_ch = 3;
int midi_vel = 100;

unsigned long t=0;

int beat;

const int digital_note[6][27] =  {{ 60, 62, 64,    65, 67, 69,    60, 62, 64,    65, 67, 69,    41, 40, 38, 36,     53, 52, 50, 48,     79, 76, 77, 74, 72,    70, 71}, 
                                  { 60, 62, 64,    65, 67, 69,    60, 62, 64,    65, 67, 69,    41, 40, 38, 36,     53, 52, 50, 48,     79, 76, 77, 74, 72,    70, 71},
                                  { 60, 62, 64,    65, 67, 69,    60, 62, 64,    65, 67, 69,    41, 40, 38, 36,     53, 52, 50, 48,     79, 76, 77, 74, 72,    70, 71},
                                  { 60, 62, 64,    65, 67, 69,    60, 62, 64,    65, 67, 69,    41, 40, 38, 36,     53, 52, 50, 48,     79, 76, 77, 74, 72,    70, 71},
                                  { 60, 62, 64,    65, 67, 69,    60, 62, 64,    65, 67, 69,    41, 40, 38, 36,     53, 52, 50, 48,     79, 76, 77, 74, 72,    70, 71},
                                  { 60, 62, 64,    65, 67, 69,    60, 62, 64,    65, 67, 69,    41, 40, 38, 36,     53, 52, 50, 48,     79, 76, 77, 74, 72,    70, 71}};
const int digital_chan[6][27] =  {{  2,  2,  2,     2,  2,  2,     1,  1,  1,     1,  1,  1,     1,  1,  1,  1,      1,  1,  1,  1,      1,  1,  1,  1,  1,     1,  1},
                                  {  2,  2,  2,     2,  2,  2,     1,  1,  1,     1,  1,  1,     1,  1,  1,  1,      1,  1,  1,  1,      1,  1,  1,  1,  1,     1,  1},
                                  {  2,  2,  2,     2,  2,  2,     1,  1,  1,     1,  1,  1,     1,  1,  1,  1,      1,  1,  1,  1,      1,  1,  1,  1,  1,     1,  1},
                                  {  2,  2,  2,     2,  2,  2,     1,  1,  1,     1,  1,  1,     1,  1,  1,  1,      1,  1,  1,  1,      1,  1,  1,  1,  1,     1,  1},
                                  {  2,  2,  2,     2,  2,  2,     1,  1,  1,     1,  1,  1,     1,  1,  1,  1,      1,  1,  1,  1,      1,  1,  1,  1,  1,     1,  1},
                                  {  2,  2,  2,     2,  2,  2,     1,  1,  1,     1,  1,  1,     1,  1,  1,  1,      1,  1,  1,  1,      1,  1,  1,  1,  1,     1,  1}};
const int analog_control[6][8] = {{ 70, 21, 22, 23, 24, 25, 76, 27 },
                                  { 70, 21, 22, 23, 24, 25, 76, 27 },
                                  { 70, 21, 22, 23, 24, 25, 76, 27 },
                                  { 70, 21, 22, 23, 24, 25, 76, 27 },
                                  { 70, 21, 22, 23, 24, 25, 76, 27 },
                                  { 70, 21, 22, 23, 24, 25, 76, 27 }};                                  
const int analog_chan[6][8] =    {{  1,  1,  1,  1,  1,  1,  1,  1 },
                                  {  1,  1,  1,  1,  1,  1,  1,  1 },
                                  {  1,  1,  1,  1,  1,  1,  1,  1 },
                                  {  1,  1,  1,  1,  1,  1,  1,  1 },
                                  {  1,  1,  1,  1,  1,  1,  1,  1 },
                                  {  1,  1,  1,  1,  1,  1,  1,  1 }};                                  
 const int beat_leds[2][4] =     {{ 27,  0,  1, 15 },
                                  { 25, 26, 24, 14 }};
int beat_led_values[2][4] =      {{  0,  0,  0,  0 },
                                  {  0,  0,  0,  0 }};
 
void setup() {
  Serial.begin(38400);
 
  // set the pin modes && zero saved states
  int b = 0;
  
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
  pinMode( 6, OUTPUT);
  pinMode( 7, OUTPUT);
  analogWrite(14, 0);
  analogWrite(15, 0);
  analogWrite(16, 32);
  analogWrite(24, 0);  
  analogWrite(25, 0);  
  analogWrite(26, 0);  
  analogWrite(27, 0);  
  analogWrite( 0, 0);  
  analogWrite( 1, 0);  
  digitalWrite(6, 0);
  digitalWrite(7, 0);
  
//  pinMode(modePin, INPUT_PULLUP);
  
  // digital pins
  for (b = numberofpins; b >= 0; b--) {
    pinMode(digital_pin[b], INPUT_PULLUP);
    digital_stored_state[0][b] = false;
    digital_stored_state[1][b] = false;
    digital_stored_state[2][b] = false;
    digital_stored_state[3][b] = false;    
    digital_stored_state[4][b] = false;    
    digital_stored_state[5][b] = false;    
    digital_stored_state[6][b] = false;    
    digital_stored_state[7][b] = false;    
    Serial.print("setup: pin ");
    Serial.println(b);
  }
  
  // analog pins
  for (b = analogpins; b>= 0; b--) {
    analog_stored_state[0][b] = 0;
    analog_stored_state[1][b] = 0;    
    analog_stored_state[2][b] = 0;    
    analog_stored_state[3][b] = 0;    
    analog_stored_state[4][b] = 0;    
    analog_stored_state[5][b] = 0;    
    analog_stored_state[6][b] = 0;    
    analog_stored_state[7][b] = 0;    
  }
  
  //Serial.println("----------------");
  //Serial.println("MIDI DJ Controller Test - setup");
  //Serial.println("----------------");
}
 
 
void loop() {
  fcnProcessButtons();
}
 
//Function to process the buttons
void fcnProcessButtons() {
  int b = 0;
  int type, note, velocity, channel, d1, d2;
  int analog_state = 0;
  
/*  analog_state = analogRead(A3);
  if (analog_state - modeStored >= analog_threshold || modeStored - analog_state >= analog_threshold) {
    Serial.print("Mode_analog: ");
    Serial.print(analog_state);
    mode = analog_state/174;
    Serial.print(" mode result: ");
    Serial.println(mode);
    modeStored = analog_state;
  }*/
  
  ////////////////////////////////////////
  //  S  e  n  d
  ////////////////////////////////////////
  
  // digital pins
  for (b = numberofpins; b >= 0; b--) {
    digital_debouncer[b].update();
    boolean state = digital_debouncer[b].read();
    if (state != digital_stored_state[mode][b]) {
      // The notes are the first in the digital_pin array.
      if (b < notepincount){ 
        if (state == false) {
          usbMIDI.sendNoteOn(digital_note[mode][b], midi_vel, digital_chan[mode][b]);
          Serial.println(String("MIDI note on: ") + digital_note[mode][b] + String(" channel: ") + digital_chan[mode][b]);
          usbMIDI.send_now();
        } else {
          usbMIDI.sendNoteOff(digital_note[mode][b], midi_vel, digital_chan[mode][b]);
          Serial.println(String("MIDI note off: ") + digital_note[mode][b] + String(" channel: ") + digital_chan[mode][b]);
          usbMIDI.send_now();
        }
      }
      digital_stored_state[mode][b] = state;
    }
  }
  
  // analog pins
  for (b = analogpins; b >= 0; b--) {
    analog_state = analogRead(analog_pin[b]);
    if (analog_state - analog_stored_state[mode][b] >= analog_threshold || analog_stored_state[mode][b] - analog_state >= analog_threshold) {
      int scaled_value = analog_state / analog_scale;
      usbMIDI.sendControlChange(analog_control[mode][b], scaled_value, analog_chan[mode][b]);
      usbMIDI.send_now();
      Serial.print("analog value ");
      Serial.print(b);
      Serial.print(": ");
      Serial.print(analog_state);
      Serial.print(" scaled: ");
      Serial.print(scaled_value);
      Serial.print(" stored: ");
      Serial.println(analog_stored_state[mode][b]);
      analog_stored_state[mode][b] = analog_state;
    }
  }

  int beatindex, row;

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
        if (channel == 1) {
          switch (note) {
            case 41:              beatindex = 0;              row = 1;              break;
            case 52:              beatindex = 1;              row = 1;              break;
            case 50:              beatindex = 2;              row = 1;              break;
            case 48:              beatindex = 3;              row = 1;              break;
            case 53:              beatindex = 0;              row = 0;              break;
            case 40:              beatindex = 1;              row = 0;              break;
            case 38:              beatindex = 2;              row = 0;              break;
            case 36:              beatindex = 3;              row = 0;              break;
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
            case 41:              beatindex = 0;              row = 1;              break;
            case 52:              beatindex = 1;              row = 1;              break;
            case 50:              beatindex = 2;              row = 1;              break;
            case 48:              beatindex = 3;              row = 1;              break;
            case 53:              beatindex = 0;              row = 0;              break;
            case 40:              beatindex = 1;              row = 0;              break;
            case 38:              beatindex = 2;              row = 0;              break;
            case 36:              beatindex = 3;              row = 0;              break;
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
    t = millis();
  }
  if (millis() - t > 10000) {
    t += 10000;
    Serial.println("(inactivity)");
  }  
}

