#include <stdint.h>

class SampleQueue {
    public:
        SampleQueue();
        SampleQueue(int numSamp);

        bool push(int sample);
        int pop();

        int getOldest();
        int getNewest();

        int getRMS();

        bool is_transient_over();

        void init_size(int numSamps);

        

    private:
        void zero_out();
        volatile uint64_t sample_squared_sum;
        int *storage;
        int capacity;
        volatile int empty_slots;
        volatile int head;
        volatile int tail;

};

