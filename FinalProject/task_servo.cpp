//***********************************************************************************************************
/** @file task_servo.cpp
 *  This file contains the code for a task that instatiates the servo object.
 *
 *  Revisions:
 *    @li May 19, 2016 -- BKK Created file
 *
 */
//***********************************************************************************************************
#include "textqueue.h"                      // Header for text queue class
#include "taskshare.h"			    // Header for thread-safe shared data
#include "shares.h"                         // Shared inter-task communications

#include "task_servo.h"                    // Header for this task

//-----------------------------------------------------------------------------------------------------------
/** This constructor creates a task which controls the ouput of the steering servo. The main job of this
 *  constructor is to call the constructor of parent class (\c frt_task ); the parent's constructor the work.
 *  @param a_name A character string which will be the name of this task
 *  @param a_priority The priority at which this task will initially run (default: 0)
 *  @param a_stack_size The size of this task's stack in bytes (default: configMINIMAL_STACK_SIZE)
 *  @param p_ser_dev Pointer to a serial device (port, radio, SD card, etc.) which can be used by this task
 *		     to communicate (default: NULL)
 */

task_servo::task_servo (const char* a_name, unsigned portBASE_TYPE a_priority, size_t a_stack_size, 
			emstream* p_ser_dev): TaskBase (a_name, a_priority, a_stack_size, p_ser_dev)
{
	// Nothing is done in the body of this constructor. 
}

//-----------------------------------------------------------------------------------------------------------
/** TODO This method is called once by the RTOS scheduler. Each time around the for (;;) loop, it instatiates a
 *  new servo object...
 */

void task_servo::run (void)
{
      // Declaration of servo object
      servo_drv* steer_servo = new servo_drv(p_serial);
      sh_servo_setpoint->put(22);		// Straight position for servo at start up
      
      //max servo PWM = 29, 15
      for(;;) 
      {
	if (sh_servo_set_flag->get()==1)
	{
	  steer_servo -> set_Pos(sh_servo_setpoint->get());
	}
	else if (sh_servo_set_flag ->get() ==2)
	{
	  steer_servo -> set_Pos(22);
	}
	sh_servo_set_flag ->put(0);
	// Delay so that other tasks can run
	delay_ms(10);
      }
}