#include "samplequeue.h"
#include "Arduino.h"
#include "constants.h"

#define DEFAULT_WINDOW  (SAMPLE_RATE*WINDOW_PERIOD) //samples

SampleQueue::SampleQueue() {
    sample_squared_sum = 0;
    capacity = DEFAULT_WINDOW;
    storage = new int[capacity];
    empty_slots = capacity;
    head = 0;
    zero_out();
}

SampleQueue::SampleQueue(int numSamp){
    sample_squared_sum = 0;
    capacity = numSamp;
    storage = new int[capacity];
    empty_slots = capacity;
    head = 0;
    zero_out();
}

bool SampleQueue::add(int newest){
    storage[(head)%capacity] = newest;

    //Add the square of the new sample to the sum
    //and subtract out the square of the oldest sample
    sample_squared_sum += sq(newest);
    int oldest = storage[(head+1)%capacity]; //At tail
    sample_squared_sum -= sq(oldest);

    if(empty_slots > 0) { empty_slots--; }

    head++;
}

bool SampleQueue::is_transient_over() {
    return (empty_slots==0);
}

int SampleQueue::getRMS(){
    return sqrt(((double)sample_squared_sum)/((double)capacity));
}

int SampleQueue::getNewest() {
  return storage[(head+capacity-1)%capacity];
}

int SampleQueue::getOldest() {
  return storage[(head+capacity)%capacity];
}

void SampleQueue::zero_out(){
    for(int i=0; i<capacity; i++) { storage[i]=0; }
}

