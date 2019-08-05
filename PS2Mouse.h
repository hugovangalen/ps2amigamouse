#ifndef PS2Mouse_h

#define PS2Mouse_h

#define REMOTE 1
#define STREAM 2

// If we don't get back a response waiting 
// for clock line, we assume the mouse is dead.
#define PS2_TIMEOUT_MICROS 1000000

class PS2Mouse
{
  private:
    int _clock_pin;
    int _data_pin;
    int _mode;
    int _initialized;
    int _enabled;
    int _disabled;
    int read_byte();
    int read_bit();
    int read_movement_x(int);
    int read_movement_y(int);
    void pull_high(int);
    void pull_low(int);
    void set_mode(int);
  public:
    PS2Mouse(int, int, int mode = REMOTE);
    void initialize();
    void has_died();
    
    int clock_pin();
    int data_pin();
    int read();
    byte* report(byte data[]);
    void write(int);
    void enable_data_reporting();
    void disable_data_reporting();
    void set_remote_mode();
    void set_stream_mode();
    void set_resolution(int);
    void set_scaling_2_1();
    void set_scaling_1_1();
    void set_sample_rate(int);
    
    //
    inline bool wait_for_clock( int value, long timeoutMicros );
    inline bool wait_for_clock_flip( long timeoutMicros );
    
};

#endif

