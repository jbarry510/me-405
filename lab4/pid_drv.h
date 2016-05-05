//===========================================================================================================
/** @file pid_drv.h
 *
 *  WRITE STUFF HERE
 */
//===========================================================================================================

/// This define prevents this .H file from being included multiple times in a .CPP file
#ifndef _PID_DRV_H_
#define _PID_DRV_H_

#include "emstream.h"                       // Header for serial ports and devices
#include "FreeRTOS.h"                       // Header for the FreeRTOS RTOS
#include "task.h"                           // Header for FreeRTOS task functions
#include "queue.h"                          // Header for FreeRTOS queues
#include "semphr.h"                         // Header for FreeRTOS semaphores

#include "taskshare.h"			    // Header for thread-safe shared data
#include "textqueue.h"                      // Header for text queue class
#include "shares.h"                         // Shared inter-task communications

//-----------------------------------------------------------------------------------------------------------
/** @brief   WRITE
 *  @details WRITE
 */

class pid_drv
{
	protected:
	/// The motor class uses this pointer to print debug messages via the serial port
	emstream* ptr_to_serial;
	

	public:
	/// The constructor sets up the encoder driver for use. The "= NULL" part is a default parameter,
	/// signifying that if that parameter isn't given on the line where this constructor is called, the
	/// compiler will just fill in "NULL". In this case this has the effect of turning off diagnostic
	/// printouts.
	/// The "uint8_t = 0" fills in a zero for the interrupt channel select if user does not define.

	pid_drv (emstream* = NULL, uint8_t = 0);
	
	private:
	  

	  
}; /// end of class pid_drv

#endif /// _PID_DRV_H_