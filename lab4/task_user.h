//***********************************************************************************************************
/** @file task_user.h
 *    This file contains header stuff for a user interface task for a ME507/FreeRTOS
 *    test suite. 
 *
 *  Revisions:
 *    @li 09-30-2012 JRR Original file was a one-file demonstration with two tasks
 *    @li 10-05-2012 JRR Split into multiple files, one for each task
 *    @li 10-25-2012 JRR Changed to a more fully C++ version with class task_user
 *    @li 11-04-2012 JRR Modified from the data acquisition example to the test suite
 *    @li 01-04-2014 JRR Changed base class names to TaskBase, TaskShare, etc.
 *
 *  License:
 *	This file is copyright 2015 by JR Ridgely and released under the Lesser GNU Public License, 
 *	version 2. It intended for educational use only, but its use is not limited thereto.
 *	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
 *	IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 *	FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 *	CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *	DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *	DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 *	IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 *	THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */
//***********************************************************************************************************

/// This define prevents this .h file from being included multiple times in a .cpp file
#ifndef _TASK_USER_H_
#define _TASK_USER_H_

#include <stdlib.h>                         /// Prototype declarations for I/O functions

#include "FreeRTOS.h"                       /// Primary header for FreeRTOS
#include "task.h"                           /// Header for FreeRTOS task functions
#include "queue.h"                          /// FreeRTOS inter-task communication queues

#include "rs232int.h"                       /// ME405/507 library for serial comm.
#include "adc.h"                            /// Header for A/D converter class driver
#include "time_stamp.h"                     /// Class to implement a microsecond timer
#include "taskbase.h"                       /// Header for ME405/507 base task class
#include "taskqueue.h"                      /// Header of wrapper for FreeRTOS queues
#include "textqueue.h"                      /// Header for a "<<" queue class
#include "taskshare.h"			    /// Header for thread-safe shared data

#include "shares.h"                         /// Shared inter-task communications

/// This macro defines a string.
#define PROGRAM_VERSION		PMS ("________ ME405 Lab 3 Modified Program ________")
#define UNKNOWN_CHAR		PMS ("<-- Unknown command. Type command from menu.")


//-----------------------------------------------------------------------------------------------------------
/// This task interacts with the user for force him/her to do what he/she is told. What
///  a rude task this is. Then again, computers tend to be that way; if they're polite
///  with you, they're probably spying on you. 
///

class task_user : public TaskBase
{
private:
	// No private variables or methods for this class

protected:
	/// This method displays the Main Menu message telling the user what to do
	void print_main_menu (void);	
	
	/// This method displays the Motor Menu message for motor control
	void print_motor_menu (void);

	/// This method displays information about the status of the system
	void show_status (void);
	
	/// This method displays the autonomous Menu message for PID control
	void print_PID_menu (void);

public:
	/// This constructor creates a user interface task object
	task_user (const char*, unsigned portBASE_TYPE, size_t, emstream*);

	/// This method is called by the RTOS once to run the task loop for ever and ever.
	void run (void);
};

#endif /// _TASK_USER_H_
