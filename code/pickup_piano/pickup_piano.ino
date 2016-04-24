#include <DueTimer.h>
#include "MIDIUSB.h"

#define DEBUG       1

#define ADC_RES     12
#define ADC_MAX     4095

#define SAMPLE_RATE   10000 //Hz
#define WINDOW_PERIOD 37000 //usec

//Remember that the midpoint is at ADC_MAX/2, roughly
#define NOTEON_THRESH  1000
#define NOTEOFF_THRESH 850 
#define CHANNEL        1

//GEN(NAME, PIN, MIDINUM)
#define FOREACH_NOTE(GEN) \
    GEN(A,      A0,     21) \
    GEN(BFLAT,  A1,     22) \
    GEN(B,      A2,     23) \
    GEN(C,      A3,     24) \
    GEN(CSHARP, A4,     25) \
    GEN(D,      A5,     26) \
    GEN(DSHARP, A6,     27) \
    GEN(E,      A7,     28) \
    GEN(F,      A8,     29) \
    GEN(FSHARP, A9,     30) \
    GEN(G,      A10,    31) \
    GEN(GSHARP, A11,    32)

#define GEN_ENUM(NAME, PIN, MIDINUM) NAME,
#define GEN_STRUCT(NAME, PIN, MIDINUM) \
    { .name    = #NAME, \
      .pin     = PIN, \
      .midinum = MIDINUM },

typedef struct {
  String name;
  int pin;
  int midinum;
} Note;

//Enumerate our notes in order
enum Note_enum {
    FOREACH_NOTE(GEN_ENUM)
    NUM_NOTES
};

const Note notes[NUM_NOTES] =
{
    FOREACH_NOTE(GEN_STRUCT)
};

// Global variables that track
// the maximum reading seen on each
// channel for the current sample
// window
static volatile int note_max[NUM_NOTES] = { 0 }; //Init all to zero

//Keeps track of whether a note is currently on
static bool note_on[NUM_NOTES] = { 0 };

volatile bool window_complete = false;

void setup() {
    #if DEBUG
      Serial.begin(9600);
    #endif
    for(int i=0; i<NUM_NOTES; i++) {
        pinMode(notes[i].pin,INPUT);
    }

    //Set up timer callbacks
    Timer3.attachInterrupt(sample_isr).setFrequency(SAMPLE_RATE).start();
    Timer4.attachInterrupt(sample_window_ended_isr).setPeriod(WINDOW_PERIOD).start();
}

// ISR to be called at regular interval.
// Samples every analog channel
// and adjusts the maximum seen on that channel
// in the current sample window
void sample_isr() {
    int reading;
    for(int i=0; i<NUM_NOTES; i++) {
        reading = analogRead(notes[i].pin);
        if(reading > note_max[i]) { note_max[i] = reading; }
    }
}

//Called every time a sample window completes
//Resets the maximum seen for each note to zero
void new_sample_window() {
    for(int i=0; i<NUM_NOTES; i++) {
        note_max[i] = 0;
    }
}


//Triggered by some timer to signal the end of sample window
//Raises a flag to send a signal to the main loop
void sample_window_ended_isr()
{
    window_complete = true;
}

void noteOn(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOn = {0x09, 0x90 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOn);
  #if DEBUG
    Serial.println("Note on");
  #endif
}

void noteOff(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOff = {0x08, 0x80 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOff);
  #if DEBUG
    Serial.println("Note off");
  #endif
}

void loop() {
  detect_notes();
}

void detect_notes() {
    if (window_complete) {
        window_complete = false;

        for(int i=0; i<NUM_NOTES; i++) {
            //Note off --> on
            if (!note_on[i]  &&  note_max[i] > NOTEON_THRESH) {
                note_on[i]=true;
                int velocity = amp_to_vel(note_max[i]);
                noteOn(CHANNEL, notes[i].midinum, velocity);
                MidiUSB.flush();
            }

            //Note on --> off
            else if (note_on[i]  &&  note_max[i] < NOTEOFF_THRESH) {
                note_on[i] = false;
                noteOff(CHANNEL, notes[i].midinum, 0);
                MidiUSB.flush();
            }
            /*
            //Note on --> on again (hit again before dying off completely)
            //TODO: This has a problem: it will fire repeatedly once a note is on
            else if (note_on[i]  &&  note_max[i] > NOTEON_THRESH) {
                int velocity = amp_to_vel(note_max[i]);
                noteOn(CHANNEL, notes[i].midinum, velocity);
                MidiUSB.flush();
            }
            */
            #if DEBUG
              Serial.println(note_max[i]);
            #endif
        }
        new_sample_window();
    }
}

//Maps the wave amplitude to a velocity
int amp_to_vel(int amp) {
  return sqrt(amp);
}

