//
// Initialize.cpp
// C++ code
// ----------------------------------
// Developed with embedXcode
// http://embedXcode.weebly.com
//
// Project 		eye-Blink_Conditioning
//
// Created by 	Kambadur Ananthamurthy, 13/09/15 11:07 am
// 				Kambadur Ananthamurthy
//
// Copyright	© Kambadur Ananthamurthy, 2015
// Licence   	<#license#>
//
// See 			Initialize.h and ReadMe.txt for references
//

// Code
#include "Globals.h"
#include "LCDRelated.h"
#include "Initialize.h"
#include "Solenoid.h"
#include <avr/wdt.h>


extern String mouseName;
extern int sessionType_ind;
extern int session;
extern boolean start;
extern int traceTime;
extern int totalTrials;

/**
 * @brief Wait for delay milliseconds for read. If no data appears in this time,
 * return.
 *
 * NOTE: This means the if writing to a serial port, always put delay less than a
 * second. Probably 0.1 second is a good value (see write_msg in python script).
 *
 * @param delay, unsigned int. For that much time we should wait.
 */
bool wait_for_read( unsigned long delay )
{
    unsigned long tstart = millis();
    while( ! Serial.available() )
    {
        if( (millis() - tstart) > delay )
            return false;
    }
    // Got something within delay.
    return true;
}

/**
 * @brief Initialize watchdog timer.
 */
void watchdog_setup( void )
{
    // Clear all interuppts.
    cli();

    reset_watchdog();

    // Set the timer to 2 sec.
    wdt_enable( WDTO_2S );
    // Set interuppts again.
    sei();
}

/**
 * @brief Interrupt serviving routine.
 *
 * @param _vect
 */
ISR(WDT_vect)
{
    // Handle interuppts here.
    // Nothing to handle.
    Serial.println("+++ WATCHDOG reset");
}

void initialize()
{

    // set the reset_ global to false at every initialization.
    reset_ = false;

    int tone_key = read_lcd_button();
    int puff_key = read_lcd_button();

    /*
     * Keep sending question onto serial port every second.
     */
    while(true)
    {
        reset_watchdog();
        delay(500);
        Serial.println("#Q1: Please enter the mouse ID number: ");
        if( wait_for_read( 500 ))
        {
            // Probably answer to our question.
            String answer = Serial.readString();
            if( answer.toInt() == 0 ) // Mouse name can't be zero
                Serial.println(" >>>> Expected a positive interger.");
            else
            {
                mouseName = "MouseK" + answer;
                Serial.println("#Got mouse name: #" + mouseName);
                break;
            }
        }
    }

    while(true)
    {
        reset_watchdog();
        delay(500);
        Serial.println("#Q2: Please enter the session type index: ");
        if( wait_for_read( 500 ))
        {
            // Probably answer to our question.
            String answer = Serial.readString();
            sessionType_ind = answer.toInt();
            Serial.println("#Got session Type: " + String(sessionType_ind));
            break;
        }
    }

    while(true)
    {
        reset_watchdog();
        delay(500);
        Serial.println("#Q3: Please enter the session number: ");
        if( wait_for_read( 500 ))
        {
            // Probably answer to our question.
            String answer = Serial.readString();
            if( answer.toInt() == 0 ) // Session number can't be zero
                Serial.println(" >>>> Expected a positive interger.");
            else
            {
                session = answer.toInt();
                Serial.println("#Got session :" + answer);
                break;
            }
        }
        delay(500);
    }

    Serial.println("#Please press the SELECT button to begin!");
    startT = millis();
    
    // I should also be able to trigger that loop by writing START to serial
    // port.
    while (read_lcd_button() != btnSELECT)
    {
        reset_watchdog();
        if (puff_key == btnUP)
        {   playPuff(puff_do, HIGH);
            delay(puffTime);
            playPuff(puff_do, LOW);
        }
        if (tone_key == btnDOWN)
        {   tone( tonePin, CS_PLUS_ToneFreq);
            delay(CSTime);
            noTone(tonePin);
        }

        if (startT > sampleInterval)
        {
            blink = analogRead(blink_ai);
            Serial.println(blink);
            startT = millis();
        }

        if( Serial.available() )
        {
            Serial.setTimeout( 100 );
            String incoming = Serial.readString( );

            int count = 0;
            while( incoming == "~" )
            {
                count += 1;
                incoming = Serial.readString( );
                if(count == 3) 
                { 
                    break; 
                }
            }
            if( count == 3 ) 
            { 
                break; 
            }
        }
    }

    // From here, the Arduino will start running the behaviour
    startT = millis();
    start = 1;
    lcd.setCursor( 0, 1 );
    lcd.print("S");
    lcd.setCursor(1, 1);
    lcd.print(session);
    lcd.setCursor( 4, 1 );
    lcd.print("T");

    lcd.setCursor( 0, 0 );
    lcd.print(mouseName);

    lcd.setCursor(8, 0);
    lcd.print(sessionType[sessionType_ind]);
    lcd.setCursor(6, 1);
    lcd.print("          ");
    Serial.println(SESSION_BEGIN_MARKER);
    Serial.println(TRIAL_DATA_MARKER);
    //Serial.print(trialNum);
    Serial.println("1 1"); // Just to not confuse data saving; NOTE: we always start the session with a CS+ trial
    Serial.println(DATA_BEGIN_MARKER);

    // Get totalTrials and traceTime based on the Session Type
    if (sessionType_ind == 2)
    {
        traceTime = 0; //in ms
        totalTrials = 100;
    }
    else
    {
        traceTime = 250; //in ms
        if (sessionType_ind == 0)
        {
            totalTrials = 50;
        }
        else
        {
            totalTrials = 100;
        }
    }
}