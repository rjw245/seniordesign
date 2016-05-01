
#define ADC_RES     12
#define ADC_MAX     4095

#define SAMPLE_RATE   10000 //Hz
#define WINDOW_PERIOD (1) //seconds
#define NUM_SAMP  (WINDOW_PERIOD*SAMPLE_RATE)

// Remember that the midpoint is at ADC_MAX/2, roughly
// TODO: Tune these given that we're using RMS now
#define NOTEON_THRESH   2400
#define NOTEOFF_THRESH  2350

//MIDI constants
#define MAXVELOCITY     127
#define CHANNEL         1
