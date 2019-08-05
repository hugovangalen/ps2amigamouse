#include "PS2Mouse.h"

/**
 * --[ Mouse/joystick port connection --]
 * 
 * FROM AMIGA MOUSE CONNECTOR                        TO ARDUINO
 * (female connector, seen from the 
 * BACK of the connector)
 * 
 *  +--------------------+  1 - V-pulse           -> Arduino PIN 2
 *  | 1o  2o  3o  4o 5o  |  2 - H-pulse           -> Arduino PIN 3
 *   \  6o  7o  8o  9o  /   3 - VQ-pulse          -> Arduino PIN 4
 *    +----------------+    4 - HQ-pulse          -> Arduino PIN 5
 *                          ** IGNORED ** 5 - BUTTON 3(Middle)  -> Arduino PIN 8
 *                          6 - BUTTON 1(L)       -> Arduino PIN 6
 *                          7 - +5V (max 50mA)    -> Arduino VCC
 *                          8 - GND               -> Arduino GND
 *                          9 - BUTTON 2(R)       -> Arduino PIN 7
 *                          
 * --[ PS2/Mouse connection ]--                         
 * 
 * FROM PS2 CONNECTOR                               TO ARDUINO
 * (female connector, seen from the 
 * FRONT of the connector)
 * 
 *        6o [~] 5o         1 - DATA              -> Arduino PIN 9
 *           [_]            2 - not connected
 *        4o     3o         3 - GND               -> Arduino GND
 *                          4 - VCC               -> Arduino VCC
 *         2o   1o          5 - CLOCK             -> Arduino PIN 8
 *                          6 - not connected
 */ 

/** Dump stuff to Serial if this is defined. */
#define DEBUG_SERIAL

/** Enables mouse data dumping: */
#define DEBUG_MOUSEDATA

#if defined(DEBUG_MOUSEDATA) and !defined(DEBUG_SERIAL)
#error DEBUG_MOUSEDATA cannot work without DEBUG_SERIAL
#endif

/** Dumps pulse related stuff. Requires DEBUG_SERIAL */
//#define DEBUG_PULSE
#if defined(DEBUG_PULSE) and !defined(DEBUG_SERIAL)
#error DEBUG_PULSE cannot work without DEBUG_SERIAL
#endif


/**
 * Amiga mouse output stuff.
 * Amiga joystick port should be connected to the following
 * pins.
 * 
 * The Arduino board takes power from these pins from
 * the Amiga joystick/mouse port:
 * 
 * Amiga Pin 7 +5V
 * Amiga Pin 8 GND
 */
static int pin_ya = 2; // Amiga Pin 1 V-pulse
static int pin_xa = 3; // Amiga Pin 2 H-pulse
static int pin_yb = 4; // Amiga Pin 3 VQ-pulse
static int pin_xb = 5; // Amiga Pin 4 HQ-pulse
// static int pin_mb = 8; // Amiga Pin 5 MIDDLE BUTTON **IGNORED**
static int pin_lb = 6; // Amiga Pin 6 LEFT BUTTON
static int pin_rb = 7; // Amiga Pin 9 RIGHT BUTTON


/**
 * PS2Mouse input stuff.
 * 
 * The mouse is connected on these pins:
 */
#define MOUSE_DATA 9
#define MOUSE_CLOCK 8

PS2Mouse mouse(MOUSE_CLOCK, MOUSE_DATA, STREAM);
byte mouseData[3];

/**
 * Does what it says...
 */
void setAmigaMousePinsLow() 
{
  digitalWrite(pin_xb, LOW);
  digitalWrite(pin_xa, LOW);
    
  digitalWrite(pin_yb, LOW);
  digitalWrite(pin_ya, LOW);
}


/**
 * This just blinks the LED as an activity indicator.
 */
void blink( int times, int length_on, int length_off ) 
{
  for (int i=0; i<times; i++) 
  {
    if (i > 0) delay( length_off );
    
    digitalWrite( 13, HIGH );
    delay( length_on );
    digitalWrite( 13, LOW );
  }
}

void setup() {

  // Set up the LED output...
  pinMode( 13, OUTPUT );
  digitalWrite( 13, HIGH );
  

  // Optional SERIAL output...
#ifdef DEBUG_SERIAL
  delay( 125 );
  Serial.begin(2000000);
  while(!Serial) {}
#endif



#ifdef DEBUG_SERIAL
  Serial.write( "Setting up Amiga mouse output...\n" );
#endif

  setupAmigaMouse();



#ifdef DEBUG_SERIAL
  Serial.write( "Setting up PS/2 mouse. If it is not connected this will hang.\n" );
#endif

  mouse.initialize();
  

  // Blinking means OK...
  blink( 3, 200, 200 ); 
  
#ifdef DEBUG_SERIAL
  Serial.write( "READY" );
#endif

} // setup()

int prevX = 0, prevY = 0;

int mouseX = 0;
int mouseY = 0;

int multiplier = 1;
int ticks = 1;
unsigned char lastStatus = 0;

int hadLeft = 0, hadRight = 0;

/** 
 * PULSE STUFF 
 * 
 * These timings have been empirically determined.
 */
#define PULSE_DELAY_MICROS  10
#define PULSE_LOOP_COUNT    50

// PULSE_LENGTH is in microseconds:
#define PULSE_LENGTH        0


// For the button states:
int hasLeft=0, hasRight=0;

// The speed runs from -127..127
signed char x, y;

// Mouse status:
unsigned char status;

/**
 * Sends data to the Amiga pins, pulsing so that
 * the mouse moves in the correct direction.
 */
void sendAmigaMousePulses() 
{
#ifdef DEBUG_SERIAL
  if (hadLeft != hasLeft || hadRight != hasRight) 
  {
    if (hadLeft != hasLeft)  
    {
      Serial.print( "LEFT BUTTON " );
      Serial.print( !hasLeft ? "UP\n":"DOWN\n");
    }
      
    if (hasRight != hadRight)  
    {
      Serial.print( "RIGHT BUTTON " );
      Serial.print( !hasRight ? "UP\n":"DOWN\n");
    }
  }
#endif 

  // Button state:
  digitalWrite( pin_lb, hasLeft ? LOW : HIGH );
  digitalWrite( pin_rb, hasRight ? LOW : HIGH );

  // For the Amiga to be able to detect two mouse downs,
  // we keep on sending that signal, too.
  if (x != 0 || y != 0) 
  {
    // Actual movement to process:
    unsigned char absx, absy;

    absx = x > 0 ? x : 0 - x;
    absy = y > 0 ? y : 0 - y;

    if (absx > PULSE_LOOP_COUNT) absx = PULSE_LOOP_COUNT;
    if (absy > PULSE_LOOP_COUNT) absy = PULSE_LOOP_COUNT;
    
    int debugx = 0, debugy = 0;
    int p;
    
    int checkx = PULSE_LOOP_COUNT - absx;
    int checky = PULSE_LOOP_COUNT - absy;
    
    // This runs from 0..X
    for(p=1; p <= PULSE_LOOP_COUNT; p++) 
    {  
        if (p > checkx) 
        {
            // LEFT or RIGHT
            if (x > 0) 
            {
              
#ifdef DEBUG_PULSE                
                debugx++;
#endif                  
                // LEFT
                digitalWrite(pin_xa, HIGH);
#if PULSE_LENGTH > 0
                delayMicroseconds( PULSE_LENGTH );
#endif
                            
                digitalWrite(pin_xb, HIGH);
#if PULSE_LENGTH > 0
                delayMicroseconds( PULSE_LENGTH );
#endif

                digitalWrite(pin_xa, LOW );
#if PULSE_LENGTH > 0
                delayMicroseconds( PULSE_LENGTH );
#endif

                digitalWrite(pin_xb, LOW );
#if PULSE_LENGTH > 0
                delayMicroseconds( PULSE_LENGTH );
#endif
                
            } 
            else if (x < 0) 
            {
#ifdef DEBUG_PULSE                
                debugx++;
#endif
                // RIGHT
                digitalWrite(pin_xb, HIGH);
#if PULSE_LENGTH > 0
                delayMicroseconds( PULSE_LENGTH );
#endif
                            
                digitalWrite(pin_xa, HIGH);
#if PULSE_LENGTH > 0
                delayMicroseconds( PULSE_LENGTH );
#endif

                digitalWrite(pin_xb, LOW );
#if PULSE_LENGTH > 0
                delayMicroseconds( PULSE_LENGTH );
#endif

                digitalWrite(pin_xa, LOW );
#if PULSE_LENGTH > 0
                delayMicroseconds( PULSE_LENGTH );
#endif   
            }
        }
            

        if (p > checky) 
        {
            // DOWN or UP
            if (y > 0) 
            {
#ifdef DEBUG_PULSE                
                debugy++;
#endif           
                // UP
                digitalWrite(pin_yb, HIGH);
#if PULSE_LENGTH > 0
                delayMicroseconds( PULSE_LENGTH );
#endif
                          
                digitalWrite(pin_ya, HIGH);
#if PULSE_LENGTH > 0
                delayMicroseconds( PULSE_LENGTH );
#endif

                digitalWrite(pin_yb, LOW );
#if PULSE_LENGTH > 0
                delayMicroseconds( PULSE_LENGTH );
#endif

                digitalWrite(pin_ya, LOW );
#if PULSE_LENGTH > 0
                delayMicroseconds( PULSE_LENGTH );
#endif
                
            } 
            else if (y < 0) 
            {
#ifdef DEBUG_PULSE                
                debugy++;
#endif           

                // DOWN
                digitalWrite(pin_ya, HIGH);
#if PULSE_LENGTH > 0
                delayMicroseconds( PULSE_LENGTH );
#endif
                            
                digitalWrite(pin_yb, HIGH);
#if PULSE_LENGTH > 0
                delayMicroseconds( PULSE_LENGTH );
#endif

                digitalWrite(pin_ya, LOW );
#if PULSE_LENGTH > 0
                delayMicroseconds( PULSE_LENGTH );
#endif

                digitalWrite(pin_yb, LOW );
#if PULSE_LENGTH > 0
                delayMicroseconds( PULSE_LENGTH );
#endif   
            }
        }

#ifdef DEBUG_PULSE
        if (debugx > 0) {
          for(int i=0;i<debugx;i++) Serial.print( x > 0 ? "X" : "x" );
          Serial.println();
        }

        if (debugy > 0) {
          for(int i=0;i<debugy;i++) Serial.print( y > 0 ? "Y" : "y" );
          Serial.println();
        }
#endif
        
    } // for..
    
  }
    
  // After we've actually sent pulses, we wanna
  // wait a moment, I guess....?
  delayMicroseconds( PULSE_DELAY_MICROS );

  hadLeft = hasLeft;
  hadRight = hasRight;
     
} // sendAmigaMousePulses()



unsigned long next_blink = 0L;
bool blink_is_on = false;


/** 
 * Constantly poll mouse status and coords, 
 * and if we have some, try to update the mouse 
 * pulses.
 */
void loop() {  

#ifdef DEBUG_SERIAL
  //Serial.write( "[DEBUG] Reading mouse...\n" );
#endif
  
  mouse.report(mouseData);
  
  status = (char)mouseData[0];

  hasLeft = ((status & 1) == 1);
  hasRight = ((status & 2) == 2);

  x = (signed char)mouseData[1];
  y = (signed char)mouseData[2];

#ifdef DEBUG_MOUSEDATA
  if (x != 0 || y != 0 || status != lastStatus) 
  {
    Serial.print( "[DEBUG] mouseData: " );
    Serial.print(status); // Status Byte
    Serial.print(":");
    Serial.print(x); // X Movement Data
    Serial.print(",");
    Serial.print(y); // Y Movement Data
    Serial.println();

    lastStatus = status;
  }
#endif

  if (x != 0 || y != 0)
  {
    // When active, blink a lot..
    if (millis() > next_blink)
    {
      blink_is_on = !blink_is_on;
      digitalWrite( 13, blink_is_on ? HIGH : LOW );
  
      next_blink = millis() + 125;
    }
  } 
  else 
  {
    digitalWrite( 13, LOW );
  }

  // Always send the state, as we want the Amiga
  // to pick up left+right button press upon start-up.
  sendAmigaMousePulses();

  // 
  delayMicroseconds( 100 );
}


int curX = 0, curY = 0;

/**
 * Sends the appropriate pulse for the direction the
 * mouse needs to go and then sets all pins LOW again.
 */
void sendAmigaMouse()
{
   if (mouseX != 0 || mouseY != 0) 
   {
     int dx = (mouseX * multiplier) / ticks;
     int dy = (mouseY * multiplier) / ticks;

     if (mouseX != 0) send_right( dx );
     if (mouseY != 0) send_down( dy );

     delay( ticks );

     // We have updated it, so lets get out?
     mouseX = 0;
     mouseY = 0;
  }

  setAmigaMousePinsLow();
}



void updateAmigaMouseDest(int relx, int rely)
{
  mouseX -= relx;
  mouseY -= rely;
}



/**
 * Set up the output pins.
 */
void setupAmigaMouse() 
{  
  pinMode(pin_xa, OUTPUT);
  pinMode(pin_xb, OUTPUT);
  pinMode(pin_ya, OUTPUT);
  pinMode(pin_yb, OUTPUT);
  
  pinMode(pin_lb, OUTPUT);
  pinMode(pin_rb, OUTPUT);
  // pinMode(pin_mb, OUTPUT);
  
  digitalWrite(pin_lb, HIGH);
  digitalWrite(pin_rb, HIGH);
  // digitalWrite(pin_mb, HIGH);

  setAmigaMousePinsLow();
}

static int xspeed = 10;
static int yspeed = 10;

void send_down(int delta) 
{
  if (delta > 0) 
  {
    send_up();
    return;
  }
  
  digitalWrite(pin_yb, HIGH);
  //delay(yspeed);
  digitalWrite(pin_ya, HIGH);
  //delay(yspeed);
  digitalWrite(pin_yb, LOW);
  //delay(yspeed);
  digitalWrite(pin_ya, LOW);
  //delay(yspeed);
}


void send_up() 
{
  digitalWrite(pin_ya, HIGH);
  //delay(yspeed);
  digitalWrite(pin_yb, HIGH);
  //delay(yspeed);
  digitalWrite(pin_ya, LOW);
  //delay(yspeed);
  digitalWrite(pin_yb, LOW);
  //delay(yspeed);
}


void send_left() 
{
  digitalWrite(pin_xa, HIGH);
  //delay(yspeed);
  digitalWrite(pin_xb, HIGH);
  //delay(yspeed);
  digitalWrite(pin_xa, LOW);
  //delay(yspeed);
  digitalWrite(pin_xb, LOW);
  //delay(yspeed);
}


void send_right( int delta ) 
{
  if (delta < 0) {
    send_left();
    return;
  }
  
  digitalWrite(pin_xb, HIGH);
  //delay(yspeed);
  digitalWrite(pin_xa, HIGH);
  //delay(yspeed);
  digitalWrite(pin_xb, LOW);
  //delay(yspeed);
  digitalWrite(pin_xa, LOW);
  // delay(yspeed);
}



/**
int forward=1;
int count=0;

#define MAX_TICKS 30
#define DELAY_MS 667

void loopEx() {

    // Move the cursor gradually to the lower right for MAX_TICKS and move back...
    if (forward) 
    {
        send_down(0);
        send_right(0);        
    } 
    else
    {
        send_left();
        send_up();
    }
    
    count++;
    if (count > MAX_TICKS)
    {
        forward=forward ? 0 : 1;
        count=0;
    }
    
    // and wait DELAY_MS in between..
    delay( DELAY_MS );
}

**/
