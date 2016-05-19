//======================================================================================
/** @file servo_drv.h
 *    TODO This file contains a simple servo motor...
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
/** @brief   This class will enable the H-bridge motor driver chips on the ME 405 board.
 *       
 *  @details The class has two protected variables...
 */

class motor_drv
{
	protected:
	/// The motor class uses this pointer to print debug messages via the serial port
	emstream* ptr_to_serial;
	
	/// The motor class uses this variable to keep track of which registers to modify for the two H-bridge chips on the ME405 board
	uint8_t select;

	public:
	/// The constructor sets up the motor driver for use. The "= NULL" part is a
	/// default parameter, meaning that if that parameter isn't given on the line
	/// where this constructor is called, the compiler will just fill in "NULL".
	/// In this case that has the effect of turning off diagnostic printouts.
	/// The "uint8_t = 0" fills in a zero for the motor select variable which will give an
	/// error message when the constructor is done executing.
	
	motor_drv (emstream* = NULL, uint8_t = 0);



}; /// end of class servo_drv

#endif /// _AVR_SERVO_H_
