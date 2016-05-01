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
    tail = capacity-1;
    zero_out();
}

SampleQueue::SampleQueue(int numSamp){
    sample_squared_sum = 0;
    capacity = numSamp;
    storage = new int[capacity];
    empty_slots = capacity;
    head = 0;
    tail = capacity-1;
    zero_out();
}

int SampleQueue::pop() {
  if(empty_slots<capacity) {
    int popped = storage[(tail)%capacity];
    tail = (tail+1)%capacity;
    empty_slots++;
    return popped;
  } else {
    return -1;
  }
}

bool SampleQueue::push(int newest){
    if(empty_slots==capacity) {
      tail = head;
    }
    storage[(head)%capacity] = newest;
    head = (head+1)%capacity;
    if(empty_slots > 0) { empty_slots--; }
    if(empty_slots==0) {
      tail = (tail+1)%capacity;
    }

    //Add the square of the new sample to the sum
    //and subtract out the square of the oldest sample
    sample_squared_sum += sq(newest);
    if(empty_slots<capacity) {
      int oldest = storage[(tail)%capacity]; //At tail
      sample_squared_sum -= sq(oldest);
    }

    return true;
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

