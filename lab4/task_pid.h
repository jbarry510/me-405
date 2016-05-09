//===========================================================================================================
/** @file task_pid.h
 *  This file contains the header for a task class that instatiates the pid object.
 *
 */
//===========================================================================================================

/// This define prevents this .H file from being included multiple times in a .CPP file
#ifndef _TASK_PID_H_
#define _TASK_PID_H_

#include <stdlib.h>                         // Prototype declarations for I/O functions
#include <avr/io.h>                         // Header for special function registers

#include "emstream.h"                       // Header for serial ports and devices
#include "FreeRTOS.h"                       // Header for the FreeRTOS RTOS
#include "semphr.h"                         // Header for FreeRTOS semaphores

#include "taskbase.h"                       // ME405/507 base task class
#include "task.h"                           // Header for FreeRTOS task functions
#include "taskqueue.h"                      // Header of wrapper for FreeRTOS queues
#include "queue.h"                          // Header for FreeRTOS queues

#include "taskshare.h"			    // Header for thread-safe shared data
#include "textqueue.h"                      // Header for text queue class
#include "shares.h"                         // Shared inter-task communications

#include "pid.h"			    // Include header for the pid driver class


class task_pid : public TaskBase
{
     
private:
	/// No private variables or methods for this class

protected:
	/// No protected variables or methods for this class
  
public:
	/// This constructor creates a generic pid task of which many copies can be made.
	task_pid (const char*, unsigned portBASE_TYPE, size_t, emstream*);
 
	/// This method is called by the RTOS once to run the task loop indefinetly.
	void run (void);
};

#endif /// _TASK_PID_H__