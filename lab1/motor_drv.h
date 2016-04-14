//======================================================================================
/** @file motor_drv.h
 *    This file contains a simple motor driver that controls the two H-bridge chips on the
 *    ME 405 board.
 *
 *  Revisions:
 *    @li 04-13-2016 ME405 Group 3 original file
 *
 */
//======================================================================================

// This define prevents this .H file from being included multiple times in a .CPP file
#ifndef _AVR_MOTOR_H_
#define _AVR_MOTOR_H_

#include "emstream.h"                       // Header for serial ports and devices
#include "FreeRTOS.h"                       // Header for the FreeRTOS RTOS
#include "task.h"                           // Header for FreeRTOS task functions
#include "queue.h"                          // Header for FreeRTOS queues
#include "semphr.h"                         // Header for FreeRTOS semaphores


//-------------------------------------------------------------------------------------
/** @brief   This class will enable the H-bridge motor driver chips on the ME 405 board.
 *       
 *  @details The class has two protected variables, one pointer that is used  to print debug 
 *    messages to the serial port and an select variable that is used to choose the appropriate 
 *    registers for controlling the motor. The driver has three methods available to the user. 
 *    The set_power method allows the user to set the power supplied to the motor on a scale of 
 *    -255 to 255 where the positive values turn the motor @b clockwise and negative values 
 *    turn the motor @b counterclockwise. The brake_full method effectively stops power being 
 *    supplied to the motor. The brake method sets PWM controlled braking for motor.
 */

class motor_drv
{
	protected:
	// The motor class uses this pointer to print debug messages via the serial port
	emstream* ptr_to_serial;
	
	// The motor class uses this variable to keep track of which registers to modify for the
	// two H-bridge chips on the ME405 board
	uint8_t select;

	public:
	/* The constructor sets up the motor driver for use. The "= NULL" part is a
	* default parameter, meaning that if that parameter isn't given on the line
	* where this constructor is called, the compiler will just fill in "NULL".
	* In this case that has the effect of turning off diagnostic printouts.
	* The "uint8_t = 0" fills in a zero for the motor select variable which will give an
	* error message when the constructor is done executing.
	*/
	motor_drv (emstream* = NULL, uint8_t = 0);

	// The set_power method allows the user to set the power supplied to the motor on a 
	// scale of -255 to 255 where the positive values turn the motor clockwise and negative 
	// values turn the motor counterclockwise.
	void set_power(int16_t power) ;
	
	// The brake_full method effectively stops power being supplied to the motor. It does this
	// by enabling the brake to Vcc operating mode of the H-bridge chip.
        void brake_full();
	
	// The brake method sets PWM controlled braking for motor. It does this by enabling the brake
	// to GND operating mode of the H-bridge chip.
	void brake(uint8_t strength);

}; // end of class motor_drv

#endif // _AVR_MOTOR_H_
