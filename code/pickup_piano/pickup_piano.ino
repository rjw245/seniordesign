#include <DueTimer.h>
#include "MIDIUSB.h"
#include "samplequeue.h"
#include "constants.h"

#define DEBUG       1
#define PRINTRMS    0
#define TESTDELAY   1  //If set to 1, sends a pin high when note is on
#define TESTPIN     1  //Pin to set high

#define BAUDRATE    115200
#define ZEROTHRES   20

#define MAX_VOLUME  900
#define MAX_VEL     127
#define LOUD_VEL    110



#define NOTEON_VARIANCE   0.20 //.33 works pretty ok
#define NUM_TO_BE_ON 10

#define NOTEOFF_VARIANCE  0.08 // HIGHER NUMBER TURNS OFF AT LOWER VOLUME, 0 IS NEVER OFF

#define CALIBRATIONTIME 100 //millisecnds

#define DBG_PRINT(SOMETHING) \
  if(DEBUG) { Serial.print(SOMETHING); }

#define DBG_PRINTLN(SOMETHING) \
  if(DEBUG) { Serial.println(SOMETHING); }

// All note information to be passed to macros
// GEN(NAME, PIN, MIDINUM, PERIOD)
#define FOREACH_NOTE(GEN) \
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


typedef struct {
    String name;
    int pin;
    int midinum;
    float period;  // seconds
    int on_thresh;
    int off_thresh;
    int midpt;
    int maxvol;
    volatile int samplesTaken;
} Note;

#define GEN_STRUCT(NAME, PIN, MIDINUM, PERIOD) \
{   .name    = #NAME, \
    .pin     = PIN, \
    .midinum = MIDINUM, \
    .period  = PERIOD, }, 

// Enumerate our notes in order
#define GEN_ENUM(NAME, PIN, MIDINUM, PERIOD) NAME,
enum Note_enum {
    FOREACH_NOTE(GEN_ENUM)
    NUM_NOTES
};

// Array of note structs
static Note notes[NUM_NOTES] =
{
    FOREACH_NOTE(GEN_STRUCT)
};

// Keeps track of whether a note is currently on
static bool note_on[NUM_NOTES] = { 0 };
static bool velocityDetermined[NUM_NOTES] = { 0 };
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

    #if TESTDELAY
        pinMode(TESTPIN,OUTPUT);
    #endif

    DBG_PRINTLN("In Setup");

    //CALIBRATION
    for(int i = 0; i < NUM_NOTES; i++) {
        int t_i = millis();
        int numiter = 0;
        unsigned int sum = 0;
        while(millis() < t_i + CALIBRATIONTIME) {
            ++numiter;
            sum += analogRead(notes[i].pin);
        }
        notes[i].midpt = sum / numiter;
        if (DEBUG) {
            Serial.print("Midpoint for note: ");
            Serial.print(i);
            Serial.print(" is ");
            Serial.println(notes[i].midpt);
        }
        notes[i].on_thresh  = notes[i].midpt * NOTEON_VARIANCE;
        notes[i].off_thresh = notes[i].midpt * NOTEOFF_VARIANCE;
    }
    
    //Set up sampling timer
    Timer3.attachInterrupt(sample_isr).setFrequency(SAMPLE_RATE).start();
    for(int i=0; i<NUM_NOTES; i++) {
      //notes[i].maxvol = calibrateVolume(i);
    }
}

// ISR to be called at regular interval.
// Samples every note
// and adds the sample to that note's
// sample queue.
void sample_isr() {
    for(int i=0; i<NUM_NOTES; i++) {
        samplequeues[i].push(analogRead(notes[i].pin) - notes[i].midpt );
        new_sample[i] = true;
        notes[i].samplesTaken++;
    }
}

void noteOn(byte channel, byte pitch, byte velocity) {
    midiEventPacket_t noteOn = {0x09, 0x90 | channel, pitch, velocity};
    #if TESTDELAY
        digitalWrite(TESTPIN,HIGH);
    #endif
    MidiUSB.sendMIDI(noteOn);
    DBG_PRINTLN("Note on");
    
}

void noteOff(byte channel, byte pitch, byte velocity) {
    midiEventPacket_t noteOff = {0x08, 0x80 | channel, pitch, velocity};
    #if TESTDELAY
        digitalWrite(TESTPIN,LOW);
    #endif
    MidiUSB.sendMIDI(noteOff);
    DBG_PRINTLN("Note off");
    DBG_PRINTLN("-------------------");
}

void loop() {
    detect_notes();
    //fake_sample_data();
    //print_new_samples();
}



//metric for amplitude mapping
int computeMetric(int i,int numSamps) {
/*  int n=0;
  while(abs(samplequeues[i].nthFromEnd(n)) > ZEROTHRES) {
    n++;
  }
  int rv = ( abs(samplequeues[i].getNewest()) - abs(samplequeues[i].nthFromEnd(n)) ) / n; */
  
  // int rv = sumLast50(i);
  //int rv = samplequeues[i].getNewest() - samplequeues[i].nthFromEnd(30);
  int sum =0;
  for(int j = 0; j < numSamps; j++) {
    sum += abs(samplequeues[i].nthFromEnd(j));
  }
  int rv = sum / numSamps;
  DBG_PRINT("Volume metric for note "); DBG_PRINT(i); DBG_PRINT(" "); DBG_PRINTLN(rv);
  return rv;
}

bool didNoteComeOn(int i) {
  int numOver=0;
  for(int n=0; n<10; n++){
    if(abs(samplequeues[i].nthFromEnd(n))>notes[i].on_thresh) {
      numOver++;
    }
  }
  return (numOver>8);
}

int sumLast50(int i) {
  int sum=0;
  for(int n=0; n<7; n++){
    sum += abs(samplequeues[i].nthFromEnd(n));
  }
  return sum;
}

void detect_notes() {
    for(int i=0; i<NUM_NOTES; i++) {
        int rms = samplequeues[i].getRMS();
        int numSamplesForVelocity = SAMPLE_RATE*0.006;
        notes[i].samplesTaken++;
        
        //Note off --> action happened
        if (!note_on[i]  &&  didNoteComeOn(i)) {
            note_on[i]=true;
            velocityDetermined[i]=false;
        }

        else if(note_on[i] && !velocityDetermined[i]) {
           if(numSamplesForVelocity < notes[i].samplesTaken) {
              int vol = computeMetric(i, numSamplesForVelocity);
              vol = constrain(vol,0,MAX_VOLUME);
              int vel = vol/(MAX_VOLUME / MAX_VEL);
              vel = constrain(vel, 0, MAX_VEL);
              velocityDetermined[i] = true;
              DBG_PRINT("Velocity of "); DBG_PRINT(i); DBG_PRINT(": ");
              DBG_PRINTLN(vel);
              noteOn(CHANNEL, notes[i].midinum, vel);
              notes[i].samplesTaken = 0;
           }
        }

        //Note on --> off
        else if ( note_on[i]  &&  rms < notes[i].off_thresh) {
            note_on[i] = false;
            DBG_PRINTLN(rms);
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

        #if PRINTRMS
            if(i<2) {
                DBG_PRINT("RMS for note "); DBG_PRINT(i); DBG_PRINT(": ");
                DBG_PRINTLN(rms);
            }
        #endif
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


int calibrateVolume(int i) {
  delay(1000);
  DBG_PRINT("Play note really loud: "); DBG_PRINTLN(i); 
  int maxrms=0;

  //Wait for note to come on
  while(!note_on[i]) {
    int rms = samplequeues[i].getRMS();
    if(rms > maxrms) { maxrms = rms; }
    
    //Note off --> on
    if (rms > notes[i].on_thresh) {
        note_on[i]=true;
    }
  }

  while(note_on[i]) {
    int rms = samplequeues[i].getRMS();
    if(rms > maxrms) { maxrms = rms; }
    
    //Note off --> on
    if (rms < notes[i].off_thresh) {
        note_on[i]=false;
    }
  }
  DBG_PRINT("Note "); DBG_PRINT(i); DBG_PRINT(" max RMS: ");
  DBG_PRINTLN(maxrms);
  return maxrms;
}


void print_info(){
  DBG_PRINT("Sample rate: "); DBG_PRINT(SAMPLE_RATE);
  DBG_PRINT("Note on threshold: "); DBG_PRINT(NOTEON_THRESH);
  DBG_PRINT("Note off threshold: "); DBG_PRINT(NOTEOFF_THRESH);
  DBG_PRINT("RMS window: "); DBG_PRINT(WINDOW_PERIOD);
  DBG_PRINTLN();
}

