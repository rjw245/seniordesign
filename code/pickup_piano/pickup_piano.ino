#include <DueTimer.h>
#include "MIDIUSB.h"
#include "samplequeue.h"
#include "constants.h"

#define DEBUG       0
#define BAUDRATE    115200

#define DBG_PRINT(SOMETHING) \
  if(DEBUG) { Serial.print(SOMETHING); }

#define DBG_PRINTLN(SOMETHING) \
  if(DEBUG) { Serial.println(SOMETHING); }

// All note information to be passed to macros
// GEN(NAME, PIN, MIDINUM, PERIOD, ON_THRESHOLD, OFF_THRESHOLD)
#define FOREACH_NOTE(GEN) \
GEN(A,      A2,     21,  0.0364,  1860,  1818) \
GEN(BFLAT,  A0,     22,  0.0343,  1800,  1700) \
GEN(B,      A1,     23,  0.0324,  1950,  1818) \
GEN(C,      A3,     24,  0.0306,  900,  200) \
GEN(CSHARP, A4,     25,  0.0289,  900,  200) \
GEN(D,      A5,     26,  0.0272,  900,  200) \
GEN(DSHARP, A6,     27,  0.0257,  900,  200) \
GEN(E,      A7,     28,  0.0243,  900,  200) \
GEN(F,      A8,     29,  0.0229,  900,  200) \
GEN(FSHARP, A9,     30,  0.0216,  900,  200) \
GEN(G,      A10,    31,  0.0204,  900,  200) \
GEN(GSHARP, A11,    32,  0.0193,  900,  200)


typedef struct {
    String name;
    int pin;
    int midinum;
    float period;  // seconds
    int on_thresh;
    int off_thresh;
} Note;

#define GEN_STRUCT(NAME, PIN, MIDINUM, PERIOD, ON_THRESHOLD, OFF_THRESHOLD) \
{   .name    = #NAME, \
    .pin     = PIN, \
    .midinum = MIDINUM, \
    .period  = PERIOD, \
    .on_thresh  = ON_THRESHOLD, \
    .off_thresh =  OFF_THRESHOLD }, 

// Enumerate our notes in order
#define GEN_ENUM(NAME, PIN, MIDINUM, PERIOD, ON_THRESHOLD, OFF_THRESHOLD) NAME,
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

// Flag, set to true when there is a new sample.
// Set to false when flag is received.
bool new_sample[NUM_NOTES]={ false };

void setup() {
    setup_adc();
    #if DEBUG
        Serial.begin(BAUDRATE);
    #endif
    for(int i=0; i<NUM_NOTES; i++) {
        pinMode(notes[i].pin,INPUT);
        samplequeues[i].init_size(notes[i].period*SAMPLE_RATE);
    }

    DBG_PRINTLN("In Setup");

    //Set up sampling timer
    Timer3.attachInterrupt(sample_isr).setFrequency(SAMPLE_RATE).start();
}

// ISR to be called at regular interval.
// Samples every note
// and adds the sample to that note's
// sample queue.
void sample_isr() {
    for(int i=0; i<NUM_NOTES; i++) {
        samplequeues[i].push(analogRead(notes[i].pin));
        new_sample[i] = true;
    }
}

void noteOn(byte channel, byte pitch, byte velocity) {
    midiEventPacket_t noteOn = {0x09, 0x90 | channel, pitch, velocity};
    MidiUSB.sendMIDI(noteOn);
    DBG_PRINTLN("Note on");
}

void noteOff(byte channel, byte pitch, byte velocity) {
    midiEventPacket_t noteOff = {0x08, 0x80 | channel, pitch, velocity};
    MidiUSB.sendMIDI(noteOff);
    DBG_PRINTLN("Note off");
}

void loop() {
    detect_notes();
    //fake_sample_data();
    //print_new_samples();
}

void detect_notes() {
    for(int i=0; i<NUM_NOTES; i++) {
        int rms = samplequeues[i].getRMS();
        //Note off --> on
        if (!note_on[i]  &&  rms > notes[i].on_thresh) {
            note_on[i]=true;
            int velocity = amp_to_vel(rms);
            noteOn(CHANNEL, notes[i].midinum, 127);
            MidiUSB.flush();
        }

        //Note on --> off
        else if (note_on[i]  &&  rms < notes[i].off_thresh) {
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
    }
}

// Speeds up the ADC by setting certain bits
// and sets the resolutions to 12 bits.
void setup_adc() {
    analogReadResolution(12);
    REG_ADC_MR = (REG_ADC_MR & 0xFFF0FFFF) | 0x00020000; // Set startup time
    //REG_ADC_MR = (REG_ADC_MR & 0xFFFFFF0F) | 0x00000080; // enable FREERUN mode
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
    samplequeues[0].push(i);
    DBG_PRINT("Oldest: ");
    DBG_PRINT(samplequeues[0].getOldest());
    DBG_PRINT(" Newest: ");
    DBG_PRINT(samplequeues[0].getNewest());
    DBG_PRINT(" RMS: ");
    DBG_PRINTLN(samplequeues[0].getRMS());
  }
  while(1);
}

void print_new_samples() {
  for(int i=0; i<NUM_NOTES; i++) {
    int popped=samplequeues[i].pop();
    while(popped!=-1){
      #if DEBUG
        Serial.write(popped/16);
      #endif
      popped = samplequeues[i].pop();
    }
  }
}

void print_info(){
  DBG_PRINT("Sample rate: "); DBG_PRINT(SAMPLE_RATE);
  DBG_PRINT("Note on threshold: "); DBG_PRINT(NOTEON_THRESH);
  DBG_PRINT("Note off threshold: "); DBG_PRINT(NOTEOFF_THRESH);
  DBG_PRINT("RMS window: "); DBG_PRINT(WINDOW_PERIOD);
  DBG_PRINTLN();
}

