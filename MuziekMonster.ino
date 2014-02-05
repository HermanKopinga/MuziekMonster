#include <Bounce.h>
#include "Tlc5940.h"
  
// pin definitions
const int digital_pin[] = { 24, 23, 18,    19, 40, 41,    2, 0, 3,   13, 4, 5,   32, 33, 28, 29,      30, 31, 34, 35,     9, 10, 11, 12,     27,   43,   44};
const int analog_pin[] = { A0, A1, 0, 1, 2, 3, 4, 5, 6, 7 }; // 2 on Teensy, 8 on 4051
 
// variables for the states of the controls
boolean digital_stored_state[27];
int analog_stored_state[10];
 
// amount of change that constitutes sending a midi message
const int analog_threshold = 30 ; //(4,88 mV per treshold)
const int pitch_threshold = 1;
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
bool abletonRunning;

                                     // Bas                     // Accoorden                // Drumcomputer                        // Samples         //Synth  //Sampletje   //Stop    //Start
const int digital_note[3][28] =  {{  36, 38, 40, 41, 43, 45,    24, 26, 28, 29, 31, 33,     0,  1,  2,  3,      4,  5,  6,  7,     15, 14, 13, 12,    61,      96,           8,        9  },
                                  {  60, 62, 64, 65, 67, 69,    48, 50, 52, 53, 55, 57,     0,  1,  2,  3,      4,  5,  6,  7,     19, 18, 17, 16,    63,      98,           8,        10 },
                                  {  84, 86, 88, 89, 91, 93,    72, 74, 76, 77, 79, 81,     0,  1,  2,  3,      4,  5,  6,  7,     23, 22, 21, 20,    66,      100,          8,        11 }};                                  


/*
  -1 is een grapje, de eerste is pitchbend voor de Synth.
   0 is ook een grapje, mode heeft geen CC.
  21 Drum snare delay
  22 Bas effect
  28 Tempo
  70 Drum triplets
  71 Accoorden effect
  73 Sample effect
  74 Master effect 1
  75 Master effect 2
*/

const byte analog_control[10] = { 22,  71,   0,  70,  28,  21,  -1,  73,  74,  75 };

const byte sample_leds[4] = { 0, 9, 10, 15 };
 
const byte beat_leds[2][4] ={{  6,  5,  2,  1 },
                             {  4,  3,  8,  7 }};
int beat_led_values[2][4]  ={{  0,  0,  0,  0 },
                             {  0,  0,  0,  0 }};  
const int Ledoff = 0;
const int Leddim = 512;
const int Ledbright  = 2500;


void resetBeatLeds() {
  int b;
  for (b = 0; b <= 3; b++) {
    Tlc.set(beat_leds[0][b], beat_led_values[0][b]);
    Tlc.set(beat_leds[1][b], beat_led_values[1][b]);
  } 
}

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

}
 
int samplechange,lastsample, currentsample;

void loop() {
  int b = 0;
  int type, note, velocity, channel, d1, d2, beatindex, row, analog_state, sample;
  
  // digital pins
  for (b = 0; b <= 26; b++) {
    digital_debouncer[b].update();
    boolean state = digital_debouncer[b].read();
    if (state != digital_stored_state[b]) {
      if (state == false) {
        // Restart Ableton if nessesary
        if (!abletonRunning) {
          resetBeatLeds();
          usbMIDI.sendNoteOn(digital_note[mode][27], midi_vel, midi_chan);
          delay(100);
          usbMIDI.sendNoteOff(digital_note[mode][27], midi_vel, midi_chan);
          abletonRunning++;          
        }
        // For identifying the buttons during setup.
        Serial.print("Pin: ");
        Serial.println(digital_pin[b]);        

        // Start is done differently :)
/*        if (digital_pin[b] == 43) {
//          usbMIDI.sendProgramChange(0,1);
        }
        else {*/
        usbMIDI.sendNoteOn(digital_note[mode][b], midi_vel, midi_chan);
        /*Serial.print("MIDI note on: ");*/
        Serial.print(" On: ");
        Serial.println(digital_note[mode][b]);
        // Stop must be send twice to Ableton.
        if (digital_note[mode][b] == 8) {
          resetBeatLeds();          
          delay(100);
          usbMIDI.sendNoteOff(digital_note[mode][b], midi_vel, midi_chan);
          delay(100);
          usbMIDI.sendNoteOn(digital_note[mode][b], midi_vel, midi_chan);
          abletonRunning = 0;
        }
        // Light led for single shot sample button.
        if (digital_pin[b] == 12) {
          Tlc.set(sample_leds[3], Leddim);
          Tlc.update();          
        }
      } 
      else {
        usbMIDI.sendNoteOff(digital_note[mode][b], midi_vel, midi_chan);
          /*Serial.print("MIDI note off: ");
          Serial.println(digital_note[b]);*/
        // Light led for single shot sample button.
        if (digital_pin[b] == 12) {
          Tlc.set(sample_leds[3], 0);
          Tlc.update();          
        }          
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
      usbMIDI.sendControlChange(analog_control[b], scaled_value, midi_chan);
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
        mode = analog_state/400;
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
        Serial.println(digital_note[mode][27]);
        usbMIDI.sendNoteOn(digital_note[mode][26], midi_vel, midi_chan);
        delay(100);        
        usbMIDI.sendNoteOff(digital_note[mode][26], midi_vel, midi_chan);
        usbMIDI.send_now();
        delay(100);
        usbMIDI.sendNoteOn(digital_note[mode][26], midi_vel, midi_chan);
        delay(100);        
        usbMIDI.sendNoteOff(digital_note[mode][26], midi_vel, midi_chan);
        usbMIDI.send_now();        
        delay(100);        
        usbMIDI.sendNoteOn(digital_note[mode][27], midi_vel, midi_chan);      
        delay(100);        
        usbMIDI.sendNoteOff(digital_note[mode][27], midi_vel, midi_chan);        
      }
    }
    if (b == 4 && !digital_debouncer[24].read()) {
      // The pitchbend is read differently, noise is considered 'musical glitches' so all state changes pass trough :-)
      if (analog_state - analog_stored_state[b+2] >= pitch_threshold || analog_stored_state[b+2] - analog_state >= pitch_threshold) {
        analog_state = constrain(analog_state, 176,999);
        
        // Pitchbend has bigger reach, so multiply by 16 and the phisical fader is limited a bit, correct for that with map().
        usbMIDI.sendPitchBend(map(analog_state,175,1000,0,1024) * 16, 1);

        Serial.print("Pitch ");    
        Serial.print("analog 4051 ");
        Serial.print(analog_control[b+2]); 
        Serial.print(": ");
        Serial.print(analog_state);
        Serial.print(" scaled: ");
        Serial.println(analog_state * 16);
        analog_stored_state[b+2] = analog_state;
      }                
    }
    // The rest are regular midi controls.
    else {
      if (analog_state - analog_stored_state[b+2] >= analog_threshold || analog_stored_state[b+2] - analog_state >= analog_threshold) {
        int scaled_value = analog_state / analog_scale;
        
        // Master effect 1 has a different top limit.
        if (b == 6) {
          scaled_value = constrain(scaled_value, 11, 120);
          scaled_value = map(scaled_value, 11, 120, 0, 127);
        }
        usbMIDI.sendControlChange(analog_control[b+2], scaled_value, midi_chan);
    
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
        // Received as midi note with changing velocity (velocity / 32 is the current beat).
        if (channel == 16 && note == 60) {
          // Calculate the beat we are in.
          beat = (velocity / 32) + 1;
          // Set the last beat
          int lastbeat = beat - 1;

          if (beat == 1) {
            lastbeat = 4;
            // The 'window' and stop button are lit on the first beat.
            analogWrite(16,255);
            // TLC output 14 is the stop button LED.
            Tlc.set(14,Ledbright);            
          }
          else {
            // The 'window' and stop button are lit on the first beat.
            analogWrite(16,0);
            Tlc.set(13,Ledoff);
            Tlc.set(14,Ledoff);            
          }
          
          // Turn down the leds from the previous beat to their normal level.
          Tlc.set(beat_leds[0][lastbeat - 1], beat_led_values[0][lastbeat - 1]);
          Tlc.set(beat_leds[1][lastbeat - 1], beat_led_values[1][lastbeat - 1]);
          // Turn up the leds from the current beat.
          Tlc.set(beat_leds[0][beat - 1], beat_led_values[0][beat - 1] + Ledbright);
          Tlc.set(beat_leds[1][beat - 1], beat_led_values[1][beat - 1] + Ledbright);
          
          Tlc.update();
          
          
          
          // Debugging information, I was confused.
/*          Serial.print(String("Values: ") + beat_led_values[0][0]);
          Serial.print(String(" ") + beat_led_values[0][1]);
          Serial.print(String(" ") + beat_led_values[0][2]);
          Serial.print(String(" ") + beat_led_values[0][3]);          
          Serial.print(String(" ") + beat_led_values[1][0]);
          Serial.print(String(" ") + beat_led_values[1][1]);
          Serial.print(String(" ") + beat_led_values[1][2]);
          Serial.println(String(" ") + beat_led_values[1][3]);
          Serial.print(String("Beatled: ") + beat_leds[1][beat - 1]);
          Serial.println(String(" Beat: ") + beat + String(" Lastbeat: ") + lastbeat); */
        }

        else if (channel == 1 && samplechange && note >= 9 && note <= 11) {
          Serial.print("Samplechange ");
          Serial.print(lastsample);
          Serial.println(currentsample);
          Tlc.set(sample_leds[currentsample], 4000);
          if (samplechange > 9)
          {
            Tlc.set(sample_leds[lastsample], 0);
            // Negative value to catch restarting the last stopped sample.
            currentsample = lastsample = -1;
          }
          Tlc.update();
          samplechange = 0;
        }

        // Regular use for the drum computer.
        // First 8 notes are triggers to turn on lights for drum computer.
        else if (channel == 1) {
          switch (note) {
            case  0:              beatindex = 0;              row = 0;              break;
            case  1:              beatindex = 1;              row = 0;              break;
            case  2:              beatindex = 2;              row = 0;              break;
            case  3:              beatindex = 3;              row = 0;              break;
            case  4:              beatindex = 0;              row = 1;              break;
            case  5:              beatindex = 1;              row = 1;              break;
            case  6:              beatindex = 2;              row = 1;              break;
            case  7:              beatindex = 3;              row = 1;              break;
            case 15:              sample = 0;                                       break;
            case 14:              sample = 1;                                       break;
            case 13:              sample = 2;                                       break;
            case 12:              sample = 3;                                       break;
            case 19:              sample = 0;                                       break;
            case 18:              sample = 1;                                       break;
            case 17:              sample = 2;                                       break;
            case 16:              sample = 3;                                       break;
            case 23:              sample = 0;                                       break;
            case 22:              sample = 1;                                       break;
            case 21:              sample = 2;                                       break;
            case 20:              sample = 3;                                       break;            
            default:
//              Serial.println("No action for this note.");
              break;
          } 
          
          // For the drum computer notes.
          if (note < 8) {
            beat_led_values[row][beatindex] = Leddim ;
            // If the current beat is turned on give immediate feedback.
            if (beat -1 == beatindex) {
              Tlc.set(beat_leds[row][beatindex], Leddim + Ledbright);
            }
            else{
              Tlc.set(beat_leds[row][beatindex], Leddim);
            }
            Tlc.update();
          }
          
          // For the sampler notes.
          if (note > 11 && note < 24 && sample != currentsample) {
              Serial.println("Dim sample led, full on at next beat.");
              Tlc.set(sample_leds[sample], Leddim);
              Tlc.update();
              currentsample = sample;
              samplechange = samplechange + 1;
          }
        }
        break;
      case 0: // NoteOff
        note = usbMIDI.getData1();
        velocity = usbMIDI.getData2();
        channel = usbMIDI.getChannel();
//        Serial.println(String("In Note Off: ch=") + channel + ", note=" + note + ", velocity=" + velocity);
        if (channel == 1) {
          switch (note) {
            case  0:              beatindex = 0;              row = 0;              break;
            case  1:              beatindex = 1;              row = 0;              break;
            case  2:              beatindex = 2;              row = 0;              break;
            case  3:              beatindex = 3;              row = 0;              break;
            case  4:              beatindex = 0;              row = 1;              break;
            case  5:              beatindex = 1;              row = 1;              break;
            case  6:              beatindex = 2;              row = 1;              break;
            case  7:              beatindex = 3;              row = 1;              break;       
            case 15:              sample = 0;                                       break;
            case 14:              sample = 1;                                       break;
            case 13:              sample = 2;                                       break;
            case 12:              sample = 3;                                       break;
            case 19:              sample = 0;                                       break;
            case 18:              sample = 1;                                       break;
            case 17:              sample = 2;                                       break;
            case 16:              sample = 3;                                       break;
            case 23:              sample = 0;                                       break;
            case 22:              sample = 1;                                       break;
            case 21:              sample = 2;                                       break;
            case 20:              sample = 3;                                       break;            
            default:
//              Serial.println("No action for this note");
              break;
          } 
          
          // Only for the drum computer notes.
          if (note < 8) {
            beat_led_values[row][beatindex] = Ledoff;
            // If the current beat is turned off give immediate feedback.          
            if (beat -1 == beatindex) {
              Tlc.set(beat_leds[row][beatindex], Ledbright);
            }
            else {
              Tlc.set(beat_leds[row][beatindex], Ledoff);
            }
            Tlc.update();
          }
          // For the sampler notes
          if (note > 11 && note < 24) {
            Serial.println("Dim sample led, shuts off at next beat.");
            Tlc.set(sample_leds[sample], Leddim);
            Tlc.update();
            lastsample = sample;
            samplechange = samplechange + 10;
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
