
#define ADC_RES     12
#define ADC_MAX     4095

#define SAMPLE_RATE   10000 //Hz
#define WINDOW_PERIOD (0.003) //seconds
#define NUM_SAMP  (WINDOW*SAMPLE_RATE)

//Remember that the midpoint is at ADC_MAX/2, roughly
#define NOTEON_THRESH  4000
#define NOTEOFF_THRESH 3400 
#define CHANNEL        1

