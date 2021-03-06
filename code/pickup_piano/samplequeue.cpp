#include "samplequeue.h"
#include "Arduino.h"
#include "constants.h"

#define DEFAULT_WINDOW  (SAMPLE_RATE*WINDOW_PERIOD) //samples

SampleQueue::SampleQueue() {
    sample_squared_sum = 0;
    head = 0;
    tail = head;
}

SampleQueue::SampleQueue(int numSamp){
    sample_squared_sum = 0;
    head = 0;
    tail = head;
    init_size(numSamp);
}

int SampleQueue::getSize() {
  return capacity;
}
void SampleQueue::init_size(int numSamps) {
    capacity = numSamps;
    storage = new int[capacity];
    empty_slots = capacity;
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
    storage[(head)%capacity] = newest;
    head = (head+1)%capacity;
    if(empty_slots > 0) { empty_slots--; }

    //Add the square of the new sample to the sum
    //and subtract out the square of the oldest sample
    sample_squared_sum += sq(newest);
    
    if(empty_slots==0) {
      tail = (tail+1)%capacity;
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

int SampleQueue::nthFromEnd(int n) {
  return storage[(head+capacity-1-n)%capacity];
}


int SampleQueue::getOldest() {
  return storage[(head+capacity)%capacity];
}

void SampleQueue::zero_out(){
    for(int i=0; i<capacity; i++) { storage[i]=0; }
    sample_squared_sum = 0;
}

