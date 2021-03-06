//======================================================================================
/** @file servo_drv.h
 *    This driver contains the configuration and position setting method for a servo on
 *    timing channel 3. 
 *
 *  Revisions:
 *    @li May 19, 2016 -- BKK Created file.
 *
 */
//======================================================================================

/// This define prevents this .H file from being included multiple times in a .CPP file
#ifndef _AVR_SERVO_H_
#define _AVR_SERVO_H_

#include "emstream.h"                       // Header for serial ports and devices
#include "FreeRTOS.h"                       // Header for the FreeRTOS RTOS
#include "task.h"                           // Header for FreeRTOS task functions
#include "queue.h"                          // Header for FreeRTOS queues
#include "semphr.h"                         // Header for FreeRTOS semaphores


//-------------------------------------------------------------------------------------
/** @brief   This class will enable a servo to be used with the ME 405 board.
 *       
 *  @details The driver contains a function to set servo position.
 */

class servo_drv
{
	protected:
	/// The servo class uses this pointer to print debug messages via the serial port
	emstream* ptr_to_serial;

	public:
	/// The constructor sets up the servo driver for use. The "= NULL" part is a
	/// default parameter, meaning that if that parameter isn't given on the line
	/// where this constructor is called, the compiler will just fill in "NULL".
	/// In this case that has the effect of turning off diagnostic printouts.
	
	servo_drv (emstream* = NULL);
	
	/// Method that sets the PWM for changing servo position
	void set_Pos(uint16_t pos);		// Sets servo position

}; /// end of class servo_drv

#endif /// _AVR_SERVO_H_
