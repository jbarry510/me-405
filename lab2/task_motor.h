//======================================================================================
/** @file task_motor.h
 *  This file contains the header for a task class that sets the power of the two motors 
 *  and lets them run for two seconds then brakes them, waits for two seconds, and then 
 *  runs them again in the opposite direction.
 *
 *  Revisions:
 *    @li 04-13-2016 ME405 Group 3 original file
 *
 */
//======================================================================================

// This define prevents this .H file from being included multiple times in a .CPP file
#ifndef _Task_MOTOR_H_
#define _Task_MOTOR_H_

#include <stdlib.h>                         // Prototype declarations for I/O functions
#include <avr/io.h>                         // Header for special function registers

#include "emstream.h"                       // Header for serial ports and devices
#include "FreeRTOS.h"                       // Header for the FreeRTOS RTOS
#include "taskbase.h"                       // ME405/507 base task class
#include "task.h"                           // Header for FreeRTOS task functions
#include "taskqueue.h"                      // Header of wrapper for FreeRTOS queues
#include "queue.h"                          // Header for FreeRTOS queues
#include "semphr.h"                         // Header for FreeRTOS semaphores
#include "motor_drv.h"                      // Include header for the motor class
#include "taskshare.h"			    // Header for thread-safe shared data
#include "shares.h"                         // Shared inter-task communications


class task_motor : public TaskBase
{
private:
	// No private variables or methods for this class

protected:
	// No protected variables or methods for this class
  
public:
	// This constructor creates a generic motor task of which many copies can be made.
	task_motor (const char*, unsigned portBASE_TYPE, size_t, emstream*);
 
	// This method is called by the RTOS once to run the task loop for ever and ever.
	void run (void);
};

#endif // _Task_MOTOR_H__