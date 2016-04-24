


class SampleQueue {
  public:
    SampleQueue();
    SampleQueue(int numSamp);

    bool add(int sample);

    int getRMS();

    bool transient_over();
  
  private:
    void zero_out();
    long sample_squared_sum;
    int *storage;
    int capacity;
    int empty_slots;
    int head;
  
};

