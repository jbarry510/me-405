//*************************************************************************************
/** @file servo_drv.cpp
 *    TODO This file contains the servo driver.
 *
 *  Revisions:
 *    @li May 19, 2016 -- BKK Created file.
 *
 */
//*************************************************************************************

#include <stdlib.h>                         // Include standard library header files
#include <avr/io.h>

#include "rs232int.h"                       // Include header for serial port class
#include "servo_drv.h"                      // Include header for the motor class

//-------------------------------------------------------------------------------------
/** \brief This constructor sets up the servo object.
 *  \details Servo needs a works with a 20 ms period pulse (50 Hz) and the range of
 *           positions go from neutral (90 deg) at 1.5ms pulse width to a min (0 deg)
 *           at 1ms pulse width and max (180 deg) at 2ms pulse width. Timer channel 0A
 *           is used to generate the PWM signal (pin B7).
 *  @param p_serial_port A pointer to the serial port which writes debugging info.
 */

servo_drv::servo_drv(emstream* p_serial_port)
{
     // Timing channel 0 setup (Fast PWM, Clock prescaler of 1024) 
     TCCR0A |= (1 << WGM00) |  (1 << WGM01) | (1 << COM0A1);
     TCCR0A &= ~(1 << COM0A0);
     TCCR0B &= ~(1 << WGM02) | ~(1 << CS01);
     TCCR0B |= (1 << CS02) | (1 << CS00);
	
     // Sets direction of port B pin 6 to output (PWM control)
     DDRB |= (1<<DDB7);
     
}

//-------------------------------------------------------------------------------------
/** @brief   This method a position input and sets the servo to that position.
 *  \details The input is set to the output compare register used for the PWM signal to
 *           the servo (Timer 0A, Pin B7). The range of inputs allowed is 20 to 30
 *           which correspond to the max right turning angle and max left turning angle
 *           respectively. A input of 25 will set the servo to its neutral position.
 *  @param   pos Input that sets the position of the servo
 *  @return  None
 */

void servo_drv::set_Pos(uint8_t pos) 
{
     /// Checks if the servo position input is in the correct range, if outside the range
     /// the position input is saturated.
     if (pos < 15)
	  pos = 15;
     else if (pos > 29)
	  pos = 29;
     
     /// Sets the position input to the output compare register used to generate the 
     /// PWM signal.
     OCR0A = pos;
}