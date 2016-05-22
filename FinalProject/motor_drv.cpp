//*************************************************************************************
/** @file motor_drv.cpp
 *    This file contains the motor driver the two H-bridge chips on the ME 405 board.
 *
 *  Revisions:
 *    @li 04-13-2016 ME405 Group 3 original file
 *
 */
//*************************************************************************************

#include <stdlib.h>                         // Include standard library header files
#include <avr/io.h>

#include "rs232int.h"                       // Include header for serial port class
#include "motor_drv.h"                      // Include header for the motor class

//-------------------------------------------------------------------------------------
/** \brief This constructor sets up the motor object.
 *  \details The timer for the PWM signal is set up in fast PWM 8-bit mode with a clock 
 *           prescaler of 8 which gives an operating frequency of approximately 8kHz. The 
 *           data direction registers and output enables are set for the appropriate
 *           motor based on the select variable. A debug message is printed stating if
 *           the motor constructor was sucessful or not.
 *  @param p_serial_port A pointer to the serial port which writes debugging info.
 *  @param motor_select Selects desired motor driver chip.
 */

motor_drv::motor_drv(emstream* p_serial_port, uint8_t motor_select)
{
	// Defines pointer for serial to inputted parameter
	ptr_to_serial = p_serial_port;
	select = motor_select;
	
	// Timing channel 1 setup (Fast PWM, Clock prescaler of 8) 
	TCCR1A |= (1 << WGM10) | (1 << COM1B1) | (1 << COM1A1);
	TCCR1A &= ~(1 << COM1B0) | ~(1 << COM1A0) | ~(1 << WGM11);
	TCCR1B |= (1 << WGM12) | (1 << CS11);
	TCCR1B &= ~(1 << CS12) | ~(1 << CS10) | ~(1 << WGM13);
	
	// If object is motor 1 then this block is used
	if(select == 1)
	{
	  // Sets direction of port C pins 0-2 to output (Direction control)
	  DDRC |= (1<<DDC0)|(1<<DDC1)|(1<<DDC2);
	  
	  // Sets output enable on motor driver 1
	  PINC |= (1 << PINC2);

	  // Sets direction of port B pin 6 to output (PWM control)
	  DDRB |= (1<<DDB6);
	
	}
	
	// If object is motor 2 then this block is used
	else if(select == 2)
	{
	  // Sets direction of port D pins 5-7 to output (Direction control)
	  DDRD |= (1<<DDD5)|(1<<DDD6)|(1<<DDD7);
	  
	  // Sets output enable on motor driver 2
	  PIND |= (1 << PIND7);
	
	  // Sets direction of port B pin 5 to output (PWM control)
	  DDRB |= (1<<DDB5);

	}
	
	// If object is not motor 1 or 2 then this block is used
	else
	{
	  // Prints an error message if select is not a 1 or 2
	  DBG(ptr_to_serial, "motor constructor FAIL (select must be a 1 or 2)" << endl);
	}
}


//-------------------------------------------------------------------------------------
/** @brief   This method takes an integer and sets the motor torque and direction.
 *  \details The value of the integer corresponds to the amount of torque applied by
 *  the motor. The sign of the integer corresponds to the direction the motor turns.
 *  Negative values turn the motor @b clockwise and positive values turn the motor 
 *  @b counterclockwise.
 *  @param   power Variable that sets power output to motor (must be between -255 and 255).
 *  @return  None
 */

void motor_drv::set_power(int16_t power) 
{
      // If object is motor 1 then this block is used
      if(select == 1)
      {
	  // Sets motor to turn clockwise and multiples power by -1 to get magnitude
	  if(power < 0)
	  {
	    PORTC &= ~(1<<PORTC0) | ~(1<<PORTC1);	// Clears INA and INB of motor 1
	    PORTC |= (1<<PORTC1);			// Sets INB of motor 1
	    PORTC &= ~(1<<PORTC0); 			// Clears INA of motor 1
	    power = power * -1;
	  }
	  
	  // Sets motor to turn counterclockwise
	  else
	  {
	    PORTC &= ~(1<<PORTC0) | ~(1<<PORTC1);	// Clears INA and INB of motor 1
	    PORTC &= ~(1<<PORTC1);			// Clears INB of motor 1
	    PORTC |= (1<<PORTC0); 			// Sets INA of motor 1
	  }
	  
	  // Sets output compare register for motor 1 PWM equal to the power
	  OCR1B = power;
      }
     
      // If object is motor 2 then this block is used
      if(select == 2)
      {
	  // Sets motor to turn clockwise and multiples power by -1 to get magnitude
	  if(power < 0)
	  {
	    PORTD &= ~(1<<PORTD5) | ~(1<<PORTD6);	// Clears INA and INB of motor 2
	    PORTD |= (1<<PORTD6);			// Sets INB of motor 2
	    PORTD &= ~(1<<PORTD5); 			// Clears INA of motor 2
	    power = power*-1;
	  }
	  
	  // Sets motor to turn counterclockwise
	  else
	  {
	    PORTD &= ~(1<<PORTD5) | ~(1<<PORTD6);	// Clears INA and INB of motor 2
	    PORTD &= ~(1<<PORTD6);			// Clears INB of motor 2
	    PORTD |= (1<<PORTD5); 			// Sets INA of motor 2
	  }
	  
	  // Sets output compare register for motor 2 PWM equal to the power
	  OCR1A = power;
      }
}


//-------------------------------------------------------------------------------------
/** @brief   This method causes the motor to brake fully.
 *  \details The H-bridge chip for the corresponding motor is set to operating mode
 *           brake to Vcc which effectively removes power from the motor causing it
 *           to brake.
 *  @return  None
 */

void motor_drv::brake_full()
{
    // If object is motor 1 then this block is used
    if(select == 1)
    {
      // Sets INA and INB for motor 1 which puts the operating mode into brake to Vcc
      PORTC |= (1<<PORTC0) | (1<<PORTC1);
    }
    
    // If object is motor 2 then this block is used
    if(select == 2)
    {
      // Sets INA and INB for motor 2 which puts the operating mode into brake to Vcc
      PORTD |= (1<<PORTD5) | (1<<PORTD6);
    }
}
//-------------------------------------------------------------------------------------
/** @brief   This method allows for the motor braking to be controlled.
 *  \details The H-bridge chip for the corresponding motor is set to operating mode
 *           brake to GND which allows for the use of the PWM signal to vary the braking
 *           strength.
 *  @param strength Variable that sets amount of motor braking (must be between -255 and 255).
 *  @return  None
 */

void motor_drv::brake(uint8_t strength)
{
    // If object is motor 1 then this block is used
    if(select == 1)
    {
      // Sets PWM controlled braking (brake to GND) for motor 1 and sets strength of braking
      PORTC &= ~(1<<PORTC0) | ~(1<<PORTC1);
      OCR1B = strength;
    }
    
    // If object is motor 2 then this block is used
    if(select == 2)
    {
      // Sets PWM controlled braking (brake to GND) for motor 2 and sets strength of braking
      PORTD &= ~(1<<PORTD5) | ~(1<<PORTD6);
      OCR1A = strength;
    }
}