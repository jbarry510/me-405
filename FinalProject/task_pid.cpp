//***********************************************************************************************************
/** @file task_pid.cpp
 * 
 * 
 */
//***********************************************************************************************************
#include "textqueue.h"                      // Header for text queue class
#include "taskshare.h"			    // Header for thread-safe shared data
#include "shares.h"                         // Shared inter-task communications
//#include "pid_drv.h"

#include "task_pid.h"                   // Header for this task

//-----------------------------------------------------------------------------------------------------------
/** 
 *  NEEDS CHANGE!!! 
 *  This constructor creates a task which controls the ouput of two encoders. The main job of this
 *  constructor is to call the constructor of parent class (\c frt_task ); the parent's constructor the work.
 *  @param a_name A character string which will be the name of this task
 *  @param a_priority The priority at which this task will initially run (default: 0)
 *  @param a_stack_size The size of this task's stack in bytes (default: configMINIMAL_STACK_SIZE)
 *  @param p_ser_dev Pointer to a serial device (port, radio, SD card, etc.) which can be used by this task
 *		     to communicate (default: NULL)
 */

task_pid::task_pid (const char* a_name, unsigned portBASE_TYPE a_priority, size_t a_stack_size, 
			emstream* p_ser_dev): TaskBase (a_name, a_priority, a_stack_size, p_ser_dev)
{
	// Nothing is done in the body of this constructor. 
        // This new task just waits for encoder_drv to share encoder data: state, direction, etc...
}

//-----------------------------------------------------------------------------------------------------------
/** This method is called once by the RTOS scheduler. Each time around the for (;;) loop, it instatiates a
 *  new encoder object, giving the interrupt pin, 7, enabling external interrupt derived encoder data about
 *  the motor rotation.
 */

void task_pid::run (void)
{
      //pid_drv* pid_driver_1 = new pid_drv(p_serial, ... );
      //pid_drv* pid_driver_2 = new pid_drv(p_serial, ... );
      
      for(;;)
      {
	
	//*p_serial << PMS("Generic Print") << dec << sh_name_of_variable << endl;
	//p_serial << endl;
	
	delay_ms(1000); // add in time for pid to delay
      }
}