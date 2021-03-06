//***********************************************************************************************************
/** \file main.cpp
 *    This file contains the main() code for a program which runs the ME405 board for
 *    ME405 lab 5.
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
#include "task_power.h"                     // Include header for motor task
#include "task_control.h"		    // Include header for pid task
#include "task_sensor.h"		    // Include header for imu task
#include "task_steer.h"			    // Include header for servo task

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
TaskShare<int8_t>* sh_power_set_flag;			// Flag share indicating power value has changed

TaskShare<int8_t>* sh_braking_full_flag;		// Flag share indicating full braking requested

TaskShare<volatile uint16_t>* sh_encoder_count_1;	// Motor 1 encoder count
TaskShare<volatile uint16_t>* sh_encoder_count_2;	// Motor 1 encoder count

TaskShare<volatile uint8_t>* sh_encoder_old_state_1;	// Motor 1 encoder previous state
TaskShare<volatile uint8_t>* sh_encoder_new_state_1;	// Motor 1 encoder next state

TaskShare<volatile uint8_t>* sh_encoder_old_state_2;	// Motor 2 encoder previous state
TaskShare<volatile uint8_t>* sh_encoder_new_state_2;	// Motor 2 encoder next state

TaskShare<volatile uint32_t>* sh_motor_1_speed;		// Motor 1 speed
TaskShare<volatile uint32_t>* sh_motor_2_speed;		// Motor 2 speed

TaskShare<uint16_t>* sh_encoder_error_count_1;		// Motor 1 tick jump error count
TaskShare<uint16_t>* sh_encoder_error_count_2;		// Motor 2 tick jump error count

TaskShare<int32_t>* sh_setpoint_1;			// Motor 1
TaskShare<int32_t>* sh_setpoint_2;			// Motor 2

TaskShare<int16_t>* sh_PID_1_power;			// Motor 1 Power values from PID control
TaskShare<int16_t>* sh_PID_2_power;			// Motor 2 Power values from PID control

TaskShare<uint8_t>* sh_PID_control;			// Flag to indicate PID control enabled

TaskShare<uint16_t>* sh_servo_setpoint;			// Servo motor position setpoint

TaskShare <uint8_t>* sh_path_radius;			// Circular path radius

TaskShare <uint8_t>* sh_circular_start;			// Circular route initialization flag

TaskShare <uint8_t>* sh_path_velocity;			// Route path velocity

TaskShare <uint8_t>* sh_linear_start;			// Linear route initialization flag

TaskShare <uint16_t>* sh_linear_distance;		// Linear route distance

TaskShare <int32_t>* sh_heading_setpoint;		// Heading setpoint for linear path control

TaskShare <int32_t>* sh_euler_heading;			// Euler heading

TaskShare <uint8_t>* sh_imu_status;			// IMU status check flag


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
     rs232* p_ser_port = new rs232 (9600, 0);
     
     // Print a starting line to display program information
     *p_ser_port << clrscr << PMS ("-------- ME405 Lab 5 Starting Program --------") << endl;

     // Create the queues and other shared data items here
     p_print_ser_queue = new TextQueue (32, "Print", p_ser_port, 30);
     
     // Create a motor power variable share and flag to indicate a power value change
     sh_power_set_flag = new TaskShare<int8_t> ("sh_power_set_flag");
     
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
     
     // Create motor 1 and 2 speed variables
     sh_motor_1_speed = new TaskShare<volatile uint32_t> ("sh_motor_1_speed"); // Motor 1
     sh_motor_2_speed = new TaskShare<volatile uint32_t> ("sh_motor_2_speed"); // Motor 2
     
     // Create encoder tick jump error counts for motor 1 and motor 2
     sh_encoder_error_count_1 = new TaskShare<uint16_t> ("sh_encoder_error_count_1");
     sh_encoder_error_count_2 = new TaskShare<uint16_t> ("sh_encoder_error_count_2");
     
     // Motor PID setpoints
     sh_setpoint_1 = new TaskShare<int32_t> ("sh_setpoint_1");			// Motor 1
     sh_setpoint_2 = new TaskShare<int32_t> ("sh_setpoint_2");			// Motor 2

     // Motor Power values from PID control
     sh_PID_1_power = new TaskShare<int16_t> ("sh_PID_1_power");		// Motor 1
     sh_PID_2_power = new TaskShare<int16_t> ("sh_PID_2_power");		// Motor 2

     // Flag to indicate PID control enabled
     sh_PID_control = new TaskShare<uint8_t> ("sh_PID_control");

     // Servo motor position setpoint
     sh_servo_setpoint = new TaskShare<uint16_t> ("sh_servo_setpoint");		
     
     // Circular path radius value
     sh_path_radius = new TaskShare<uint8_t> ("sh_path_radius");
     
     // Circular route initialization flag
     sh_circular_start = new TaskShare<uint8_t> ("sh_circular_start");
     
     // Linear route initialization flag
     sh_linear_start = new TaskShare<uint8_t> ("sh_linear_start");
     
     // Linear route distance
     sh_linear_distance = new TaskShare<uint16_t> ("sh_linear_distance");
     
     // Current IMU heading (Euler coordinates)
     sh_euler_heading = new TaskShare<int32_t> ("sh_euler_heading");
     
     // Route feature initial IMU heading (Euler coordinates)
     sh_heading_setpoint = new TaskShare <int32_t> ("sh_heading_setpoint");
     
     // IMU status check flag
     sh_imu_status = new TaskShare<uint8_t> ("sh_imu_status");

     // Creating a task that operates the serial user interface and accepts feature inputs
     new task_user    ("UserInterface", task_priority(1), 280, p_ser_port);
     
     // Creating a task that operates the motors and encoders 
     new task_power   ("Power        ", task_priority(4), 280, p_ser_port);
     
     // Creating a task that operates motor PID and feature computation/execution
     new task_control ("Control      ", task_priority(3), 350, p_ser_port);
     
     // Creating a task that configures and operates the IMU and both IR sensors 
     new task_sensor  ("Sensor       ", task_priority(2), 280, p_ser_port);
     
     // Creating a task that configures and operate  the servo-powered motor
     new task_steer   ("Steering     ", task_priority(4), 280, p_ser_port);

     // The RTOS scheduler, ran indefinetly:
     vTaskStartScheduler ();
}