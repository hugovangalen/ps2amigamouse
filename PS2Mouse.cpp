
#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WConstants.h"
#endif

#include "HardwareSerial.h"
#include "PS2Mouse.h"

PS2Mouse::PS2Mouse(int clock_pin, int data_pin, int mode) {
  _clock_pin = clock_pin;
  _data_pin = data_pin;
  _mode = mode;
  _initialized = false;
  _disabled = true;
  _enabled = false;
}

int PS2Mouse::clock_pin() {
  return _clock_pin;
}

int PS2Mouse::data_pin() {
  return _data_pin;
}

void PS2Mouse::initialize() {
  pull_high(_clock_pin);
  pull_high(_data_pin);
  delay(20);

  write(0xff); // Send Reset to the mouse

  delay(20);
  read_byte();  // Read ack byte 
  
  if (_mode == REMOTE) {
    set_remote_mode();
  } else {
    enable_data_reporting(); // Tell the mouse to start sending data again
  }
  
  delayMicroseconds(100);
  _initialized = 1;  
}

void PS2Mouse::set_mode(int data) {
  if (_mode == STREAM) {
    disable_data_reporting(); // Tell the mouse to stop sending data.
  }
  write(data);  // Send Set Mode
  read_byte();  // Read Ack byte
  if (_mode == STREAM) {
    enable_data_reporting(); // Tell the mouse to start sending data again
  }
  if (_initialized) {
    delayMicroseconds(100);
  }
}

void PS2Mouse::set_remote_mode() {
  set_mode(0xf0);
  _mode = REMOTE;
}

void PS2Mouse::set_stream_mode() {
  set_mode(0xea);
  _mode = STREAM;
}

void PS2Mouse::set_sample_rate(int rate) {
  if (_mode == STREAM) {
    disable_data_reporting(); // Tell the mouse to stop sending data.
  }
  write(0xf3); // Tell the mouse we are going to set the sample rate.
  read_byte(); // Read Ack Byte
  write(rate); // Send Set Sample Rate
  read_byte(); // Read ack byte
  if (_mode == STREAM) {
    enable_data_reporting(); // Tell the mouse to start sending data again
  }
  delayMicroseconds(100);
}

void PS2Mouse::set_scaling_2_1() {
  set_mode(0xe7); // Set the scaling to 2:1
}

void PS2Mouse::set_scaling_1_1() {
  set_mode(0xe6); // set the scaling to 1:1
}

// This only effects data reporting in Stream mode.
void PS2Mouse::enable_data_reporting() {
  if (!_enabled) {
    write(0xf4); // Send enable data reporting
    read_byte(); // Read Ack Byte
    _enabled = true;
  }
}

// Disabling data reporting in Stream Mode will make it behave like Remote Mode
void PS2Mouse::disable_data_reporting() {
  if (!_disabled) {
    write(0xf5); // Send disable data reporting
    read_byte(); // Read Ack Byte    
    _disabled = true;
  }
}

void PS2Mouse::set_resolution(int resolution) {
  if (_mode == STREAM) {
    enable_data_reporting();
  }
  write(0xe8); // Send Set Resolution
  read_byte(); // Read ack Byte
  write(resolution); // Send resolution setting
  read_byte(); // Read ack Byte
  if (_mode == STREAM) {
    disable_data_reporting();
  }
  delayMicroseconds(100);
}


bool PS2Mouse::wait_for_clock_flip( long timeoutMicros ) {
    
    if (!wait_for_clock( HIGH, timeoutMicros )) {
        return false;
    }
    
    if (!wait_for_clock( LOW, timeoutMicros )) {
        return false;
    }
    return true;
}

bool PS2Mouse::wait_for_clock( int value, long timeoutMicros ) {
    long until = micros() + timeoutMicros;
    int readValue;

    pinMode( _clock_pin, INPUT );
    for(;;) {
        readValue = digitalRead( _clock_pin );
        
        if (readValue == value) {
            // we found it!
            Serial.print( "value found!\n" );
            return true;
        }
        
        //while (digitalRead(_clock_pin) != value) {
        if (micros() >= until) {
            break;
        }
        
        //delayMicroseconds( 10 );
    } 
    
    // failed!
    return false;
}

void PS2Mouse::has_died() {
    Serial.print( "PS2Mouse::has_died The clock line did not flip, assuming mouse has died.\n" );
}

void PS2Mouse::write(int data) {
  char i;
  char parity = 1;
  
  pull_high(_data_pin);
  pull_high(_clock_pin);
  delayMicroseconds(300);
  pull_low(_clock_pin);
  delayMicroseconds(300);
  pull_low(_data_pin);
  delayMicroseconds(10);
  pull_high(_clock_pin); // Start Bit
  
  /*if (!wait_for_clock( LOW, PS2_TIMEOUT_MICROS )) {
      has_died();
      return;
  }
  */
  pinMode( _clock_pin, INPUT );
  while (digitalRead(_clock_pin)) {;} // wait while HIGH (for mouse to take control of clock)
  /**/
  
  // clock is low, and we are clear to send data 
  for (i=0; i < 8; i++) {
    if (data & 0x01) {
      pull_high(_data_pin);
    } else {
      pull_low(_data_pin);
    }
    
    // wait for clock cycle 
    /*
    if (!wait_for_clock_flip( PS2_TIMEOUT_MICROS )) {
        has_died();
        Serial.print( "from write()\n" );
        return;
    }
    */
    pinMode( _clock_pin, INPUT );
    while (!digitalRead(_clock_pin)) {;} // wait while LOW
    while (digitalRead(_clock_pin)) {;} // wait while HIGH

    /*
    // while (!digitalRead(_clock_pin)) {;}
    if (!wait_for_clock( HIGH, PS2_TIMEOUT_MICROS )) {
        has_died();
        return;
    }
    
    // while (digitalRead(_clock_pin)) {;}
    if (!wait_for_clock( LOW, PS2_TIMEOUT_MICROS )) {
        has_died();
        return;
    }
    */
    
    parity = parity ^ (data & 0x01);
    data = data >> 1;
  }
  // parity 
  if (parity) {
    pull_high(_data_pin);
  } else {
    pull_low(_data_pin);
  }
  
  pinMode( _clock_pin, INPUT );
  while (!digitalRead(_clock_pin)) {;}
  while (digitalRead(_clock_pin)) {;}  
  /* if (!wait_for_clock_flip( PS2_TIMEOUT_MICROS )) {
      has_died();
      return;
  } */
  pull_high(_data_pin);
  delayMicroseconds(50);
  
  pinMode( _clock_pin, INPUT );
  while (digitalRead(_clock_pin)) {;}
  /* if (!wait_for_clock( LOW, PS2_TIMEOUT_MICROS )) {
      has_died();
      return;
  } */  
  while ((!digitalRead(_clock_pin)) || (!digitalRead(_data_pin))) {;} // wait for mouse to switch modes
  pull_low(_clock_pin); // put a hold on the incoming data.
}

byte* PS2Mouse::report(byte data[]) {
  write(0xeb); // Send Read Data
  read_byte(); // Read Ack Byte
  data[0] = read(); // Status bit
  data[1] = read_movement_x(data[0]); // X Movement Packet
  data[2] = read_movement_y(data[0]); // Y Movement Packet
  return data;
}

int PS2Mouse::read() {
  return read_byte();
}

int PS2Mouse::read_byte() {
  int data = 0;
  pull_high(_clock_pin);
  pull_high(_data_pin);
  delayMicroseconds(50);
  // Serial.print( "read_byte!\n" );
  pinMode( _clock_pin, INPUT );
  pinMode( _data_pin, INPUT );
  while (digitalRead(_clock_pin)) {;}
  delayMicroseconds(5);  // not sure why.
  while (!digitalRead(_clock_pin)) {;} // eat start bit
  for (int i = 0; i < 8; i++) {
    bitWrite(data, i, read_bit());
  }
  read_bit(); // Partiy Bit 
  read_bit(); // Stop bit should be 1
  pull_low(_clock_pin);
  return data;
}

int PS2Mouse::read_bit() {
  while (digitalRead(_clock_pin)) {;}
  int bit = digitalRead(_data_pin);
  while (!digitalRead(_clock_pin)) {;}
  return bit;
}

int PS2Mouse::read_movement_x(int status) {
  int x = read();
  if (bitRead(status, 4)) {
    for(int i = 8; i < 16; ++i) {
      x |= (1<<i);
    }
  }
  return x;
}

int PS2Mouse::read_movement_y(int status) {
  int y = read();
  if (bitRead(status, 5)) {
    for(int i = 8; i < 16; ++i) {
      y |= (1<<i);
    }
  }
  return y;
}

void PS2Mouse::pull_low(int pin) {
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
  //pinMode(pin, INPUT);
}

void PS2Mouse::pull_high(int pin) {
  pinMode(pin, OUTPUT);//pinMode(pin, INPUT);
  digitalWrite(pin, HIGH);
  // pinMode(pin, INPUT);
}
