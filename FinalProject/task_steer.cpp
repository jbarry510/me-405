//***********************************************************************************************************
/** @file task_steer.cpp
 *  TODO This file contains the code for a task that instatiates the servo object.
 *
 *  Revisions:
 *    @li May 19, 2016 -- BKK Created file
 *
 */
//***********************************************************************************************************
#include "textqueue.h"                      // Header for text queue class
#include "taskshare.h"			    // Header for thread-safe shared data
#include "shares.h"                         // Shared inter-task communications

#include "task_steer.h"                     // Header for this task

//-----------------------------------------------------------------------------------------------------------
/** This constructor creates a task which controls the ouput of the steering servo. The main job of this
 *  constructor is to call the constructor of parent class (\c frt_task ); the parent's constructor the work.
 *  @param a_name A character string which will be the name of this task
 *  @param a_priority The priority at which this task will initially run (default: 0)
 *  @param a_stack_size The size of this task's stack in bytes (default: configMINIMAL_STACK_SIZE)
 *  @param p_ser_dev Pointer to a serial device (port, radio, SD card, etc.) which can be used by this task
 *		     to communicate (default: NULL)
 */

task_steer::task_steer (const char* a_name, unsigned portBASE_TYPE a_priority, size_t a_stack_size, 
			emstream* p_ser_dev): TaskBase (a_name, a_priority, a_stack_size, p_ser_dev)
{
	// Nothing is done in the body of this constructor. 
}

//-----------------------------------------------------------------------------------------------------------
/** TODO This method is called once by the RTOS scheduler. Each time around the for (;;) loop, it instatiates a
 *  new servo object...
 */

void task_steer::run (void)
{
     // Make a variable which will hold times to use for precise task scheduling
     TickType_t previousTicks = xTaskGetTickCount ();
     
     // Declaration of servo object
     servo_drv* steer_servo = new servo_drv(p_serial);
     adc* adc_1 = new adc(p_serial);
     int16_t steering_trim = (adc_1->read_oversampled(0,10) / 2) + -127;
     sh_servo_setpoint->put(3000 + steering_trim);		// Straight position for servo at start up
     steer_servo->set_Pos(sh_servo_setpoint->get());
     
      
     //max servo PWM = 29, 15
     for(;;) 
     {
	  steering_trim = (adc_1->read_oversampled(0,10) / 2) + -127;
	  if (sh_servo_set_flag->get() == 1)
	  {
	       steer_servo->set_Pos(sh_servo_setpoint->get()+ steering_trim);
	  }
	  
	  sh_servo_set_flag->put(0);
	  runs++;					// Increment the timer run counter.
	  delay_from_for_ms (previousTicks, 10);	// Task runs every 10 ms
     }
}