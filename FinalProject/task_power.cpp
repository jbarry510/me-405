//***********************************************************************************************************
/** @file task_power.cpp
 *  This file contains the code for a task that configures and operates both of the motors and encoders.
 *
 *  Revisions:
 *    @li 04-13-2016 ME405 Group 3 original file
 *    @li 06-10-2016 Combined task_motor and task_encoder into task_power
 *
 */
//***********************************************************************************************************
#include "textqueue.h"                      // Header for text queue class
#include "taskshare.h"			    // Header for thread-safe shared data
#include "shares.h"                         // Shared inter-task communications

#include "task_power.h"                     // Header for this task

//-------------------------------------------------------------------------------------
/** This constructor creates a task which controls the ouput of two motors and two encoders. The main job of this constructor
 *  is to call the constructor of parent class (\c frt_task ); the parent's constructor the work.
 *  @param a_name A character string which will be the name of this task
 *  @param a_priority The priority at which this task will initially run (default: 0)
 *  @param a_stack_size The size of this task's stack in bytes (default: configMINIMAL_STACK_SIZE)
 *  @param p_ser_dev Pointer to a serial device (port, radio, SD card, etc.) which can be used by this task
 *		     to communicate (default: NULL)
 */

task_power::task_power (const char* a_name, unsigned portBASE_TYPE a_priority, size_t a_stack_size, 
		        emstream* p_ser_dev):TaskBase (a_name, a_priority, a_stack_size, p_ser_dev)
{
	// Nothing is done in the body of this constructor. All the work is done in the
	// call to the frt_task constructor on the line just above this one
}

//-------------------------------------------------------------------------------------
/** This method is called once by the RTOS scheduler. Each time around the for (;;) loop, it measures and calculates
 *  encoder parameters and passes motor velocity changes to the motor.
 */

void task_power::run (void)
{
	// Make a variable which will hold times to use for precise task scheduling
        TickType_t previousTicks = xTaskGetTickCount ();
     
	// Create two motor driver object and a variable in which to store the output. 
        // The variables p_motor_1 and p_motor_2 only exist within this run() method, so the motors cannot
	// be used from any other function or method.
	motor_drv* p_motor_1 = new motor_drv (p_serial, 1);
	motor_drv* p_motor_2 = new motor_drv (p_serial, 2);
	sh_power_set_flag->put(0);		// Flag used to only set power when it has changed
	
	// Construction of encoder drivers
        encoder_drv* encoder_driver_1 = new encoder_drv(p_serial, 7);  // 6 and 7 aliased
        encoder_drv* encoder_driver_2 = new encoder_drv(p_serial, 3);  // 4 and 5 aliased
	
        // Initialization of encoder counts before/after used to determine speed 
        uint16_t encoder_count_new_motor_1 = 0;
        uint16_t encoder_count_old_motor_1 = 0;
        uint16_t encoder_count_new_motor_2 = 0;
        uint16_t encoder_count_old_motor_2 = 0;
	
	for(;;)
	{
	       // Sets the new/old variables so speed can be calculated
	       encoder_count_old_motor_1 = encoder_count_new_motor_1;
	       encoder_count_new_motor_1 = sh_encoder_count_1->get();
	       encoder_count_old_motor_2 = encoder_count_new_motor_2;
	       encoder_count_new_motor_2 = sh_encoder_count_2->get();
	       
	       // Sets motor speed variables using difference in encoder readings
	       sh_motor_1_speed->put(encoder_driver_1->calc_motor(encoder_count_old_motor_1, encoder_count_new_motor_1));
	       sh_motor_2_speed->put(encoder_driver_2->calc_motor(encoder_count_old_motor_2, encoder_count_new_motor_2));
	       
	       
	       // Check if power variable has changed, power flag = high, if not skip
	       if (sh_power_set_flag->get() == 1)
	       {
		    p_motor_1 -> set_power(sh_PID_1_power->get());	// Set power for motor 1
		    p_motor_2 -> set_power(sh_PID_2_power->get());	// Set power for motor 2
	       
		    sh_power_set_flag->put(0);		// Make power_set_flag low when succesful power set

	       }
	       // Clears both motor powers 
	       else if (sh_power_set_flag ->get() == 2)
	       {
		    p_motor_1 -> set_power(0);
		    p_motor_2 -> set_power(0);
		    sh_power_set_flag -> put(0);
	       }
       
	       // Check if braking full flag is high, if not skip
	       if (sh_braking_full_flag->get() == 1)
	       {
		    p_motor_1 -> brake_full();		// Stop motor 1
		    p_motor_2 -> brake_full();		// Stop motor 2
			 
		    sh_power_set_flag->put(2);	// Make power_set_flag low when successful power set
		    sh_braking_full_flag->put(0);	// Make braking_full_flag low when successful motor stop
	       }
	       
	       runs++;					// Increment the timer run counter.
	       delay_from_for_ms (previousTicks, 10);	// Task runs every 10 ms
	}
}