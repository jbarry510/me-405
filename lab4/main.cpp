//***********************************************************************************************************
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
 *    @li April 28, 2016 -- BKK Added shared variables (encoder: count, state, error count), commented
 * 			    out task_brightness: was interfering with channel 4 interrupts.
 *
 *  License:
 *	This file is copyright 2015 by JR Ridgely and released under the Lesser GNU Public License, 
 * 	version 2. It intended for educational use only, but its use is not limited thereto.
 *	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
 *	IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 *	FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 *	CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *	DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *	DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 *	IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 *	THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */
//***********************************************************************************************************


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

#include "task_user.h"                      // Header for user interface task
#include "motor_drv.h"                      // Include header for the motor class
#include "task_motor.h"                     // Include header for motor task
#include "encoder_drv.h"		    // Include header for encoder driver
#include "task_encoder.h"		    // Include header for encoder task
#include "task_pid.h"
#include "pid.h"

// Declare the queues which are used by tasks to communicate with each other here. Each queue must also be
// declared 'extern' in a header file which will be read by every task that needs to use that queue. The
// format for all queues except the serial text printing queue is 'frt_queue<type> name (size)', where 'type'
// is the type of data in the queue and 'size' is the number of items (not necessarily bytes) which the queue
// can hold.

/** This is a print queue, descended from \c emstream so that things can be printed into the queue using the
 *  "<<" operator and they'll come out the other end as a stream of characters. It's used by tasks that send
 *  things to the user interface task to be printed. 
 */
TextQueue* p_print_ser_queue;

// Shared variables
TaskShare<int8_t>* sh_motor_select;			// Motor selection share

TaskShare<int16_t>* sh_power_entry;			// Power value share
TaskShare<int8_t>* sh_power_set_flag;			// Flag share indicating power value has changed

TaskShare<int16_t>* sh_braking_entry;			// Braking value share
TaskShare<int8_t>* sh_braking_set_flag;			// Flag share indicating braking value has changed

TaskShare<int8_t>* sh_braking_full_flag;		// Flag share indicating full braking requested

TaskShare<volatile uint16_t>* sh_encoder_count_1;	// Motor 1 encoder count
TaskShare<volatile uint16_t>* sh_encoder_count_2;	// Motor 1 encoder count

TaskShare<volatile uint8_t>* sh_encoder_old_state_1;	// Motor 1 encoder previous state
TaskShare<volatile uint8_t>* sh_encoder_new_state_1;	// Motor 1 encoder next state

TaskShare<volatile uint8_t>* sh_encoder_old_state_2;	// Motor 2 encoder previous state
TaskShare<volatile uint8_t>* sh_encoder_new_state_2;	// Motor 2 encoder next state

TaskShare<uint16_t>* sh_encoder_error_count_1;		// Motor 1 tick jump error count
TaskShare<uint16_t>* sh_encoder_error_count_2;		// Motor 2 tick jump error count

TaskShare<volatile int16_t>* sh_motor_1_speed;		// Motor 1 speed
TaskShare<volatile int16_t>* sh_motor_2_speed;		// Motor 2 speed

TaskShare<int32_t>* sh_setpoint_1;			// Motor 1 PID setpoints
TaskShare<int32_t>* sh_setpoint_2;			// Motor 2 PID setpoints

TaskShare<int16_t>* sh_PID_1_power;			// Motor 1 Power values from PID control
TaskShare<int16_t>* sh_PID_2_power;			// Motor 2 Power values from PID control

TaskShare<uint8_t>* sh_PID_control;			// Flag to indicate PID control enabled

//===========================================================================================================
/** The main function sets up the RTOS.  Some test tasks are created. Then the scheduler is started up; the
 *  scheduler runs until power is turned off or there's a reset.
 *  @return This is a real-time microcontroller program which doesn't return.
 */

int main (void)
{
     // Disables the watchdog timer; may have been left on or reverts on.
     MCUSR = 0;
     wdt_disable ();

     // Configure a serial port.
     rs232* p_ser_port = new rs232 (9600, 1);
     
     // Print a starting line to display program information
     *p_ser_port << clrscr << PMS ("-------- ME405 Lab 4 Starting Program --------") << endl;

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
     sh_encoder_count_1 = new TaskShare<volatile uint16_t> ("sh_encoder_count_1");
     sh_encoder_count_2 = new TaskShare<volatile uint16_t> ("sh_encoder_count_2");
     
     // Create motor 1 shares for previous and new states to determine direction and tick skips
     sh_encoder_old_state_1 = new TaskShare<volatile uint8_t> ("sh_encoder_old_state_1") ;
     sh_encoder_new_state_1 = new TaskShare<volatile uint8_t> ("sh_encoder_new_state_1");

     // Create motor 2 shares for previous and new states to determine direction and tick skips
     sh_encoder_old_state_2 = new TaskShare<volatile uint8_t> ("sh_encoder_old_state_2");
     sh_encoder_new_state_2 = new TaskShare<volatile uint8_t> ("sh_encoder_new_state_2");
     
     // Create encoder tick jump error counts for motor 1 and motor 2
     sh_encoder_error_count_1 = new TaskShare<uint16_t> ("sh_encoder_error_count_1");
     sh_encoder_error_count_2 = new TaskShare<uint16_t> ("sh_encoder_error_count_2");
     
     // Create motor 1 and 2 speed variables
     sh_motor_1_speed = new TaskShare<volatile int16_t> ("sh_motor_1_speed"); // Motor 1
     sh_motor_2_speed = new TaskShare<volatile int16_t> ("sh_motor_2_speed"); // Motor 2
     
     // Create motor 1 and 2 PID setpoint variables
     sh_setpoint_1 = new TaskShare<int32_t> ("sh_setpoint_1");		// Motor 1
     sh_setpoint_2 = new TaskShare<int32_t> ("sh_setpoint_2");		// Motor 2

     // Create motor 1 and 2 PID power variables
     sh_PID_1_power = new TaskShare<int16_t> ("sh_PID_1_power");	// Motor 1
     sh_PID_2_power = new TaskShare<int16_t> ("sh_PID_2_power");	// Motor 2
     
     // Flag to indicate PID control enabled
     sh_PID_control = new TaskShare<uint8_t> ("sh_PID_control");
     
     // The user interface is at low priority; it could have been run in the idle task
     // but it is desired to exercise the RTOS more thoroughly in this test program
     new task_user ("UserInterface", task_priority (1), 260, p_ser_port);
     
     // Creating a task that operates the motor and runs a defined program
     new task_motor ("    Motor    ", task_priority (2), 280, p_ser_port);
     
     // Creating a task that operates the encoder and runs a defined program
     new task_encoder ("   Encoder   ", task_priority (4), 280, p_ser_port);
     
     new task_pid ("   Pid    ", task_priority(3), 280, p_ser_port);

     // The RTOS scheduler, ran indefinetly:
     vTaskStartScheduler ();
}