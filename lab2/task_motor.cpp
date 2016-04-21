//**************************************************************************************
/** @file task_motor.cpp
 *  This file contains the code for a task that sets the power of the two motors 
 *  and lets them run for two seconds then brakes them, waits for two seconds, and then 
 *  runs them again in the opposite direction.
 *
 *  Revisions:
 *    @li 04-13-2016 ME405 Group 3 original file
 *
 */
//**************************************************************************************
#include "task_motor.h"                     // Header for this task
#include "textqueue.h"                      // Header for text queue class
#include "taskshare.h"			    // Header for thread-safe shared data
#include "shares.h"                         // Shared inter-task communications

//-------------------------------------------------------------------------------------
/** This constructor creates a task which controls the ouput of two motors. The main job 
 *  of this constructor is to call the constructor of parent class (\c frt_task ); the 
 *  parent's constructor the work.
 *  @param a_name A character string which will be the name of this task
 *  @param a_priority The priority at which this task will initially run (default: 0)
 *  @param a_stack_size The size of this task's stack in bytes 
 *                      (default: configMINIMAL_STACK_SIZE)
 *  @param p_ser_dev Pointer to a serial device (port, radio, SD card, etc.) which can
 *                   be used by this task to communicate (default: NULL)
 */

task_motor::task_motor (const char* a_name, unsigned portBASE_TYPE a_priority, size_t a_stack_size, 
			emstream* p_ser_dev): TaskBase (a_name, a_priority, a_stack_size, p_ser_dev)
{
	// Nothing is done in the body of this constructor. All the work is done in the
	// call to the frt_task constructor on the line just above this one
}

//-------------------------------------------------------------------------------------
/** This method is called once by the RTOS scheduler. Each time around the for (;;)
 *  loop, it sets the power of the two motors and lets them run for two seconds then
 *  brakes them, waits for two seconds, and then runs them again in the opposite direction.
 */

void task_motor::run (void)
{
	// Create two motor driver object and a variable in which to store the output. 
        // The variables p_motor_1 and p_motor_2 only exist within this run() method,
	// so the motors cannot be used from any other function or method.
	motor_drv* p_motor_1 = new motor_drv (p_serial, 1);
	motor_drv* p_motor_2 = new motor_drv (p_serial, 2);
	sh_power_set_flag-> put(0);		// Flag used to only set power when it has changed
	
	for(;;)
	{
	  // Check if power variable has changed, power flag = high, if not skip
	  if (sh_power_set_flag->get() == 1)
	  {
	      if(sh_motor_select->get() == 1)
	      {
		    p_motor_1 -> set_power(sh_power_entry->get());	// Set power for motor 1
	      }
	      else if(sh_motor_select->get() == 2)
	      {
		    p_motor_2 -> set_power(sh_power_entry->get());	// Set power for motor 2
	      }
	      
	      sh_power_set_flag-> put(0);		// Make power_set_flag low when succesful power set

	  }
	  delay_ms(100);				// Delay for lower priority tasks
	      
	      
	      /*
	      // Sets both motor 1 and 2 to run clockwise for two seconds and prints a message
	      p_motor_1 -> set_power(100);
	      p_motor_2 -> set_power(220);
	      *p_serial << "Running" << endl;
	      delay_ms(2000);
	      
	      // Sets motor 1 to brake fully and motor 2 to brake with PWM control and prints a message
	      p_motor_1 -> brake_full();
	      p_motor_2 -> brake(50);
	      *p_serial << "Braking" << endl;
	      delay_ms(2000);
	      
	      // Sets both motor 1 and 2 to run counterclockwise for two seconds and prints a message
	      p_motor_1 -> set_power(-100);
	      p_motor_2 -> set_power(-220);
	      *p_serial << "Running backwards" << endl;
	      delay_ms(2000);
	      */
	}
}