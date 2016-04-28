//*************************************************************************************
/** \file main.cpp
 *    This file contains the main() code for a program which runs the ME405 board for
 *    ME405 lab 1. This program currently uses the H-bridge chips on the board to set 
 *    the power of the two motors and let them run for two seconds then brakes them, 
 *    waits for two seconds, and then runs them again in the opposite direction.
 *
 *  Revisions:
 *    \li 09-30-2012 JRR Original file was a one-file demonstration with two tasks
 *    \li 10-05-2012 JRR Split into multiple files, one for each task plus a main one
 *    \li 10-30-2012 JRR A hopefully somewhat stable version with global queue 
 *                       pointers and the new operator used for most memory allocation
 *    \li 11-04-2012 JRR FreeRTOS Swoop demo program changed to a sweet test suite
 *    \li 01-05-2012 JRR Program reconfigured as ME405 Lab 1 starting point
 *    \li 03-28-2014 JRR Pointers to shared variables and queues changed to references
 *    @li 01-04-2015 JRR Names of share & queue classes changed; allocated with new now
 *
 *  License:
 *		This file is copyright 2015 by JR Ridgely and released under the Lesser GNU 
 *		Public License, version 2. It intended for educational use only, but its use
 *		is not limited thereto. */
/*		THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 *		AND	ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * 		IMPLIED 	WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * 		ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
 * 		LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUEN-
 * 		TIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS 
 * 		OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
 * 		CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
 * 		OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 * 		OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */
//*************************************************************************************


#include <stdlib.h>                         // Prototype declarations for I/O functions
#include <avr/io.h>                         // Port I/O for SFR's
#include <avr/wdt.h>                        // Watchdog timer header
#include <string.h>                         // Functions for C string handling

#include "FreeRTOS.h"                       // Primary header for FreeRTOS
#include "task.h"                           // Header for FreeRTOS task functions
#include "queue.h"                          // FreeRTOS inter-task communication queues
#include "croutine.h"                       // Header for co-routines and such

#include "rs232int.h"                       // ME405/507 library for serial comm.
#include "time_stamp.h"                     // Class to implement a microsecond timer
#include "taskbase.h"                       // Header of wrapper for FreeRTOS tasks
#include "textqueue.h"                      // Wrapper for FreeRTOS character queues
#include "taskqueue.h"                      // Header of wrapper for FreeRTOS queues
#include "taskshare.h"                      // Header for thread-safe shared data
#include "shares.h"                         // Global ('extern') queue declarations
#include "task_brightness.h"                // Header for the data acquisition task
#include "task_user.h"                      // Header for user interface task
#include "motor_drv.h"                      // Include header for the motor class
#include "task_motor.h"                     // Include header for motor task
#include "encoder_drv.h"
#include "task_encoder.h"

// Declare the queues which are used by tasks to communicate with each other here. 
// Each queue must also be declared 'extern' in a header file which will be read 
// by every task that needs to use that queue. The format for all queues except 
// the serial text printing queue is 'frt_queue<type> name (size)', where 'type' 
// is the type of data in the queue and 'size' is the number of items (not neces-
// sarily bytes) which the queue can hold

/** This is a print queue, descended from \c emstream so that things can be printed 
 *  into the queue using the "<<" operator and they'll come out the other end as a 
 *  stream of characters. It's used by tasks that send things to the user interface 
 *  task to be printed. 
 */
TextQueue* p_print_ser_queue;

// Shared variables
TaskShare<int8_t>* sh_motor_select;			// Motor selection share

TaskShare<int16_t>* sh_power_entry;			// Power value share
TaskShare<int8_t>* sh_power_set_flag;			// Flag share indicating power value has changed

TaskShare<int16_t>* sh_braking_entry;			// Braking value share
TaskShare<int8_t>* sh_braking_set_flag;			// Flag share indicating braking value has changed

TaskShare<int8_t>* sh_braking_full_flag;		// Flag share indicating full braking requested

TaskShare<volatile uint8_t>* sh_encoder_count_1;	// Motor 1 encoder count
TaskShare<volatile uint8_t>* sh_encoder_count_2;	// Motor 1 encoder count

TaskShare<volatile uint8_t>* sh_encoder_old_state_1;	// Motor 1 encoder previous state
TaskShare<volatile uint8_t>* sh_encoder_new_state_1;	// Motor 1 encoder next state

TaskShare<volatile uint8_t>* sh_encoder_old_state_2;	// Motor 2 encoder previous state
TaskShare<volatile uint8_t>* sh_encoder_new_state_2;	// Motor 2 encoder next state

TaskShare<uint8_t>* sh_encoder_error_count_1;		// Motor 1 tick jump error count
TaskShare<uint8_t>* sh_encoder_error_count_2;		// Motor 2 tick jump error count

//=====================================================================================
/** The main function sets up the RTOS.  Some test tasks are created. Then the 
 *  scheduler is started up; the scheduler runs until power is turned off or there's a 
 *  reset.
 *  @return This is a real-time microcontroller program which doesn't return. Ever.
 */

int main (void)
{
	// Disable the watchdog timer unless it's needed later. This is important because
	// sometimes the watchdog timer may have been left on...and it tends to stay on
	MCUSR = 0;
	wdt_disable ();

	// Configure a serial port which can be used by a task to print debugging infor-
	// mation, or to allow user interaction. The serial port will be used by the user
	// interface task after setup is complete and the task scheduler has been started
	// by the function vTaskStartScheduler()
	rs232* p_ser_port = new rs232 (9600, 1);
	*p_ser_port << clrscr << PMS ("ME405 Lab 3 Starting Program") << endl;

	// Create the queues and other shared data items here
	p_print_ser_queue = new TextQueue (32, "Print", p_ser_port, 30);
	
	// Create a motor select share
	sh_motor_select = new TaskShare<int8_t> ("sh_motor_select");
	
	// Create a motor power variable share and flag to indicate a power value change
	sh_power_entry = new TaskShare<int16_t> ("sh_power_entry");
	sh_power_set_flag = new TaskShare<int8_t> ("sh_power_set_flag");
	
	// Create a motor braking variable share and flag to indicate a braking value change
	sh_braking_entry = new TaskShare<int16_t> ("sh_braking_entry");
	sh_braking_set_flag = new TaskShare<int8_t> ("sh_braking_set_flag");
	
	// Create a flag to indicate a full braking requested
	sh_braking_full_flag = new TaskShare<int8_t> ("sh_braking_full_flag");
	
	// Create encoder counters for motor 1 and motor 2
	sh_encoder_count_1 = new TaskShare<volatile uint8_t> ("sh_encoder_count_1");	// Motor 1 encoder count
	sh_encoder_count_2 = new TaskShare<volatile uint8_t> ("sh_encoder_count_2");	// Motor 1 encoder count
	
	// Create motor 1 shares for previous and new states to determine direction and tick skips
	sh_encoder_old_state_1 = new TaskShare<volatile uint8_t> ("sh_encoder_old_state_1") ;	// Motor 1 encoder previous state
	sh_encoder_new_state_1 = new TaskShare<volatile uint8_t> ("sh_encoder_new_state_1");	// Motor 1 encoder next state

	// Create motor 2 shares for previous and new states to determine direction and tick skips
	sh_encoder_old_state_2 = new TaskShare<volatile uint8_t> ("sh_encoder_old_state_2");	// Motor 2 encoder previous state
	sh_encoder_new_state_2 = new TaskShare<volatile uint8_t> ("sh_encoder_new_state_2");	// Motor 2 encoder next state
	
	// Create encoder tick jump error counts for motor 1 and motor 2
	sh_encoder_error_count_1 = new TaskShare<uint8_t> ("sh_encoder_error_count_1");		// Motor 1 tick jump error count
	sh_encoder_error_count_2 = new TaskShare<uint8_t> ("sh_encoder_error_count_2");		// Motor 2 tick jump error count

	// The user interface is at low priority; it could have been run in the idle task
	// but it is desired to exercise the RTOS more thoroughly in this test program
	new task_user ("UserInt", task_priority (1), 260, p_ser_port);

	// Create a task which reads the A/D and adjusts an LED's brightness accordingly
// 	new task_brightness ("Bright", task_priority (2), 280, p_ser_port);
	
	// Creating a task that operates the motor and runs a defined program
	new task_motor ("Motor", task_priority (3), 280, p_ser_port);
	
	new task_encoder ("Encoder", task_priority (2), 280, p_ser_port);

	// Here's where the RTOS scheduler is started up. It should never exit as long as
	// power is on and the microcontroller isn't rebooted
	vTaskStartScheduler ();
}