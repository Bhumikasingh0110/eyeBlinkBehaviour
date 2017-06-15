/***
 *       Filename:  main.ino
 *
 *    Description:  Protocol for EyeBlinkConditioning.
 *
 *        Version:  0.0.1
 *        Created:  2017-04-11

 *         Author:  Dilawar Singh <dilawars@ncbs.res.in>
 *   Organization:  NCBS Bangalore
 *
 *        License:  GNU GPL2
 */

// #define USE_MOUSE 
#ifdef USE_MOUSE
#include "arduino-ps2-mouse/PS2Mouse.h"
#endif


#ifdef USE_MOUSE
// Motion detection related.
#define         CLOCK_PIN               6
#define         DATA_PIN                7
#else
#define         MOTION1_PIN             6
#define         MOTION2_PIN             7
#endif 

#define         SPEED_OUT_PIN           5
#define         MAX_SPEED_COMPUTATIONS  5

#define         MAX_DIRECTION_COMPUTATION 10


// Motion detection based on motor
#define         MOTOR_OUT              A1


unsigned long stamp_            = 0;
char msg_[20];
bool reboot_                    = false;

const size_t l_ = 100;
int motion1_[l_];
int motion2_[l_];


int t_[ l_ ];

float speed_ = 0.0;
int direction_ = 0;

float speed_array_[ MAX_SPEED_COMPUTATIONS ];
int down_trans1_[ MAX_DIRECTION_COMPUTATION ];
int down_trans2_[ MAX_DIRECTION_COMPUTATION ];

// Latest value comes at the end. Useful for keeping time.
void append( int* array, int val, size_t l )
{
    // Upto array[1] from array[l-1]
    for( size_t i = 0; i < l - 1; i ++ )
        array[i] = array[i + 1];
    array[ l_ - 1 ] = val;
}

// Latest value comes in the begining.
void prepend( int* array, int val, size_t l )
{
    // Upto array[1] from array[l-1]
    for( size_t i = l-1; i > 0; i -- )
        array[i] = array[i - 1];

    array[ 0 ] = val;
}

void prependf( float* array, float val, size_t l )
{
    // Upto array[1] from array[l_-1]
    for( size_t i = l-1; i > 0; i -- )
        array[i] = array[i - 1];

    array[ 0 ] = val;
}

void print_array( int* arr, size_t n )
{
    for (size_t i = 0; i < n; i++) 
    {
        Serial.print( arr[i] );
        Serial.print( ' ' );
    }
    Serial.println( ' ' );
    Serial.flush( );
}

void print_farray( float* arr, size_t n )
{
    for (size_t i = 0; i < n; i++) 
    {
        float x = arr[i];
        Serial.print( x, 4 );
        Serial.print( ' ' );
    }
    Serial.println( ' ' );
    Serial.flush( );
}

void init_farray( float* arr, size_t n )
{
    for (size_t i = 0; i < n; i++) 
        arr[i] = 0.0;
}

void init_array( int* arr, size_t n )
{
    for (size_t i = 0; i < n; i++) 
        arr[i] = 0;
}


/*
 * MOUSE
 */
#ifdef USE_MOUSE
PS2Mouse mouse( CLOCK_PIN, DATA_PIN );
#endif


bool is_low_transition( int prev, int cur )
{
    bool res = ( 1 == prev ) && (0 == cur );
#if 0
    if( res )
    {
        Serial.print( prev );
        Serial.print( cur );
        Serial.print( 'x' );
    }
#endif
    return res;
}

/**
 * @brief Compute speed of threadmill. 
 * The maximum speed of mouse is taken to be 1 m/sec.
 *
 * On analog pin, 1m/s represents 255 i.e. we have a resolution of 1/255 m/s.
 */
float compute_speed( int* arr, size_t l, float dW = 14.0 )
{

    int numComputations = 0;

    float resolutionV_ = 1.0 / 255.0;
    bool startEstimate = false;
    int startT = 0;

    init_farray( speed_array_, MAX_SPEED_COMPUTATIONS );
    for (size_t i = 1; i < l; ++i) 
    {
        int prev = arr[i-1];
        int curr = arr[i];
        if( (! startEstimate) && is_low_transition( prev, curr ) )
        {
            startEstimate = true;
            startT = t_[i];
        }

        else if( startEstimate && is_low_transition( prev, curr ) )
        {
            startEstimate = false;
            float dt = t_[i] - startT;
            // Compute speed in this pulse.
            float speed = dW / dt;    // Speed in mm per ms 
            numComputations += 1;
            prependf( speed_array_, speed, MAX_SPEED_COMPUTATIONS );
        }
    }

    // Now compute the average speed.
    float sum = 0.0;
    //print_farray( speed_array_, MAX_SPEED_COMPUTATIONS );
    
    for (size_t i = 0; i < MAX_SPEED_COMPUTATIONS; i++ )
        sum += speed_array_[ i ];

    //print_farray( speed_array_, MAX_SPEED_COMPUTATIONS );
    float speed = sum / min( numComputations, MAX_SPEED_COMPUTATIONS );
    return speed;
}

int compute_direction( )
{
    init_array( down_trans1_, MAX_DIRECTION_COMPUTATION );
    init_array( down_trans2_, MAX_DIRECTION_COMPUTATION );

    bool lowTransInFirstSensor = false;
    int firstSignalLowTime = 0;
    for (size_t i = 1; i < l_; i++) 
    {
        int prev1 = motion1_[ i-1 ];
        int cur1 = motion1_[ i  ];
        if( (! lowTransInFirstSensor) && is_low_transition( prev1, cur1 ) )
        {
            prepend( down_trans1_, t_[i], MAX_DIRECTION_COMPUTATION );
            firstSignalLowTime = t_[i];
            lowTransInFirstSensor = true;
            continue;
        }

        if( lowTransInFirstSensor )
        {
            int prev2 = motion2_[ i - 1];
            int cur2 = motion2_[ i  ];
            if( is_low_transition( prev2, cur2 ) )
            {
                prepend( down_trans2_, t_[i], MAX_DIRECTION_COMPUTATION );
                lowTransInFirstSensor = false; // Reset
            }
        }
    }

    int diffN = 0;
    for (size_t i = 0; i < MAX_DIRECTION_COMPUTATION; i++) 
    {
        if( down_trans1_[i] != 0 && down_trans2_[i] != 0 )
        {
            if( down_trans1_[ i] >= down_trans2_[i] )
                diffN += down_trans1_[i] - down_trans2_[i];
        }
    }
        
    return diffN;
}



/**
 * @brief Write data line to Serial port.
 *   NOTE: Use python dictionary format. It can't be written at baud rate of
 *   38400 at least.
 * @param data
 * @param timestamp
 */
void compute( )
{

    // Just read the registers where pin data is saved.
    unsigned long timestamp = millis();
    int motion1;
    int motion2;

#ifdef USE_MOUSE
    // Read mouse data.
    MouseData data = mouse.readData( );
    motion1 = data.position.x;
    motion2 = data.position.y;
#else
    motion1 = digitalRead( MOTION1_PIN );
    motion2 = digitalRead( MOTION2_PIN );
#endif

    prepend( motion1_, motion1, l_ );
    prepend( motion2_, motion2, l_ );

    // Only time vector should be in APPEND. Rest are PREPEND.
    append( t_, timestamp, l_ );

    float speed1 = compute_speed( motion1_, l_, 12.5);
    float speed2 = compute_speed( motion2_, l_, 14.0);

    int direction = compute_direction( );

    analogWrite( SPEED_OUT_PIN, ceil(255.0 * speed1) );

    Serial.print( speed1, 4);
    Serial.print( ' ' );
    Serial.print( speed2, 4 );
    Serial.print( ' ' );
    Serial.println( direction );

}

void setup()
{
    Serial.begin( 38400 );
    stamp_ = millis( );

#ifdef USE_MOUSE
    // Configure mouse here
    mouse.initialize( );
    Serial.println( "Stuck in setup() ... mostly due to MOUSE" );
#else
    Serial.println( "Using LED/DIODE pair" );
    pinMode( MOTION1_PIN, INPUT );
    pinMode( MOTION2_PIN, INPUT );
#endif

    pinMode( SPEED_OUT_PIN, OUTPUT );

    init_array( motion1_, l_ );
    init_array( motion2_, l_ );
    init_array( t_, l_ );
    init_farray( speed_array_, MAX_SPEED_COMPUTATIONS );
}


void loop()
{
    compute( );
}
