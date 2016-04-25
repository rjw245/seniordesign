
#define ADC_RES     12
#define ADC_MAX     4095

#define SAMPLE_RATE   10000 //Hz
#define WINDOW_PERIOD (0.003) //seconds
#define NUM_SAMP  (WINDOW*SAMPLE_RATE)

// Remember that the midpoint is at ADC_MAX/2, roughly
// TODO: These are probably too high given that we're using RMS now
#define NOTEON_THRESH   4000
#define NOTEOFF_THRESH  3400 
#define CHANNEL         1

//MIDI constants
#define MAXVELOCITY     127
