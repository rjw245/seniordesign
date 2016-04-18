#include <DueTimer.h>

#include <Scheduler.h>

#define A_SENSE         A0
#define BFLAT_SENSE     A1
#define B_SENSE         A2
#define C_SENSE         A3
#define CSHARP_SENSE    A4
#define D_SENSE         A5
#define DSHARP_SENSE    A6
#define E_SENSE         A7
#define F_SENSE         A8
#define FSHARP_SENSE    A9
#define G_SENSE         A10
#define GSHARP_SENSE    A11

//Enumerate our notes in order
enum Note {
  A,
  BFLAT,
  B,
  C,
  CSHARP,
  D,
  DSHARP,
  E,
  F,
  FSHARP,
  G,
  GSHARP,
  NUM_NOTES
}

// Array of note pins to make traversing
// all the pins easier
const int note_pins[NUM_NOTES];

// Global variables that track
// the maximum reading seen on each
// channel for the current sample
// window
int A_max       = 0;
int BFLAT_max   = 0;
int B_max       = 0;
int C_max       = 0;
int CSHARP_max  = 0;
int D_max       = 0;
int DSHARP_max  = 0;
int E_max       = 0;
int F_max       = 0;
int FSHARP_max  = 0;
int G_max       = 0;
int GSHARP_max  = 0;

void setup() {
  set_pins();
}

// ISR to be called at regular interval.
// Samples every analog channel
// and adjusts the maximum seen on that channel
// in the current sample window
void sample_isr() {
  int reading;
  reading = analogRead(A_SENSE);
  if(reading > A_max) A_max = reading;
  
  reading = analogRead(BFLAT_SENSE);
  if(reading > BFLAT_max) BFLAT_max = reading;
  
  reading = analogRead(B_SENSE);
  if(reading > B_max) B_max = reading;
  
  reading = analogRead(A_SENSE);
  if(reading > A_max) A_max = reading;
  
  reading = analogRead(A_SENSE);
  if(reading > A_max) A_max = reading;
  
  reading = analogRead(A_SENSE);
  if(reading > A_max) A_max = reading;
  
  reading = analogRead(A_SENSE);
  if(reading > A_max) A_max = reading;
  
  reading = analogRead(A_SENSE);
  if(reading > A_max) A_max = reading;
  
  reading = analogRead(A_SENSE);
  if(reading > A_max) A_max = reading;
  
  reading = analogRead(A_SENSE);
  if(reading > A_max) A_max = reading;
  
  reading = analogRead(A_SENSE);
  if(reading > A_max) A_max = reading;
  
  reading = analogRead(A_SENSE);
  if(reading > A_max) A_max = reading;
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

