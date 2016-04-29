#include <DueTimer.h>
#include "MIDIUSB.h"
#include "samplequeue.h"
#include "constants.h"

#define DEBUG      0

// All note information to be passed to macros
// GEN(NAME, PIN, MIDINUM, PERIOD)
#define FOREACH_NOTE_ORIGINAL(GEN) \
    GEN(A,      A0,     21,  0.0364) \
    GEN(BFLAT,  A1,     22,  0.0343) \
    GEN(B,      A2,     23,  0.0324) \
    GEN(C,      A3,     24,  0.0306) \
    GEN(CSHARP, A4,     25,  0.0289) \
    GEN(D,      A5,     26,  0.0272) \
    GEN(DSHARP, A6,     27,  0.0257) \
    GEN(E,      A7,     28,  0.0243) \
    GEN(F,      A8,     29,  0.0229) \
    GEN(FSHARP, A9,     30,  0.0216) \
    GEN(G,      A10,    31,  0.0204) \
    GEN(GSHARP, A11,    32,  0.0193)

#define FOREACH_NOTE(GEN) \
    GEN(A,      A0,     21,  0.0364)

typedef struct {
    String name;
    int pin;
    int midinum;
    float period;  // seconds
} Note;

#define GEN_STRUCT(NAME, PIN, MIDINUM, PERIOD) \
{   .name    = #NAME, \
    .pin     = PIN, \
    .midinum = MIDINUM, \
    .period  = PERIOD },

// Enumerate our notes in order
#define GEN_ENUM(NAME, PIN, MIDINUM, PERIOD) NAME,
enum Note_enum {
    FOREACH_NOTE(GEN_ENUM)
    NUM_NOTES
};

// Array of note structs
static const Note notes[NUM_NOTES] =
{
    FOREACH_NOTE(GEN_STRUCT)
};

// Keeps track of whether a note is currently on
static bool note_on[NUM_NOTES] = { 0 };

// Keeps track of the rolling RMS over the last
// 3ms of samples
static SampleQueue samplequeues[NUM_NOTES];

void setup() {
    setup_adc();
    #if DEBUG
        Serial.begin(9600);
    #endif
    for(int i=0; i<NUM_NOTES; i++) {
        pinMode(notes[i].pin,INPUT);
    }

    //Set up sampling timer
    Timer3.attachInterrupt(sample_isr).setFrequency(SAMPLE_RATE).start();
}

// ISR to be called at regular interval.
// Samples every note
// and adds the sample to that note's
// sample queue.
void sample_isr() {
    for(int i=0; i<NUM_NOTES; i++) {
        samplequeues[i].add(analogRead(notes[i].pin));
    }
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
    //fake_sample_data();
}

void detect_notes() {
    for(int i=0; i<NUM_NOTES; i++) {
        int rms = samplequeues[i].getRMS();
        //Note off --> on
        if (!note_on[i]  &&  rms > NOTEON_THRESH) {
            note_on[i]=true;
            int velocity = amp_to_vel(rms);
            noteOn(CHANNEL, notes[i].midinum, 127);
            MidiUSB.flush();
        }

        //Note on --> off
        else if (note_on[i]  &&  rms < NOTEOFF_THRESH) {
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
        //Serial.println(rms);
    #endif
    }
}

// Speeds up the ADC by setting certain bits
// and sets the resolutions to 12 bits.
void setup_adc() {
    analogReadResolution(12);
    REG_ADC_MR = (REG_ADC_MR & 0xFFF0FFFF) | 0x00020000; // Set startup time
    REG_ADC_MR = (REG_ADC_MR & 0xFFFFFF0F) | 0x00000080; // enable FREERUN mode
}

// Maps the wave amplitude to a velocity
int amp_to_vel(int amp) {
    // Intensity lin. proportional to Amplitude^2
    int intensity = 1 * sq(amp) + 0;

    // Volume lin. proportional to log(Intensity)
    int volume    = 1 * log(intensity) + 0;

    //TODO: How does velocity relate to volume?
    int velocity  = 1 * volume;
    velocity = constrain(velocity,0,MAXVELOCITY);
    return velocity;
}

void fake_sample_data() {
  for(int i=0; i<10000; i++) {
    samplequeues[0].add(i);
    #if DEBUG
      Serial.print("Oldest: ");
      Serial.print(samplequeues[0].getOldest());
      Serial.print(" Newest: ");
      Serial.print(samplequeues[0].getNewest());
      Serial.print(" RMS: ");
      Serial.println(samplequeues[0].getRMS());
    #endif
  }
  while(1);
}

