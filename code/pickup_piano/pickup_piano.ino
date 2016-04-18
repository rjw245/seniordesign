#include <DueTimer.h>

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
#define GEN_PIN_ARRAY(NAME, PIN, MIDINUM) [NAME] = PIN,
#define GEN_MIDINUM_ARRAY(NAME, PIN, MIDINUM) [NAME] = MIDINUM,

//Enumerate our notes in order
enum Note {
    FOREACH_NOTE(GEN_ENUM)
    NUM_NOTES
}

// Array of note pins to make traversing
// all the pins easier
const int note_pins[NUM_NOTES] =
{
    FOREACH_NOTE(GEN_PIN_ARRAY)
}

//Array of MIDI note numbers
//to correspond with each note
const int note_midinum[NUM_NOTES] =
{
    FOREACH_NOTE(GEN_MIDINUM_ARRAY)
}

// Global variables that track
// the maximum reading seen on each
// channel for the current sample
// window
static const int note_max[NUM_NOTES] = { 0 }; //Init all to zero

void setup() {
    for(int i=0; i<NUM_NOTES; i++) {
        pinMode(note_pins[i],INPUT);
    }
}

// ISR to be called at regular interval.
// Samples every analog channel
// and adjusts the maximum seen on that channel
// in the current sample window
void sample_isr() {
    int reading;
    for(int i=0; i<NUM_NOTES; i++) {
        reading = analogRead(note_pins[i]);
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

void loop() {
    Serial.println("Hello");
}

void loop2() {
    digitalWrite(13,HIGH);
    delay(500);
    digitalWrite(13,LOW);
    delay(500);
}

