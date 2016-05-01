
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

        

    private:
        void zero_out();
        volatile long long sample_squared_sum;
        int *storage;
        int capacity;
        volatile int empty_slots;
        volatile int head;
        volatile int tail;

};

