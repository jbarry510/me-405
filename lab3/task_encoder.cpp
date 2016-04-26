//**************************************************************************************
/** @file task_encoder.cpp
 *  TODO This file contains the code for a task that...
 *
 *  Revisions:
 *    @li 04-13-2016 ME405 Group 3 original file
 *
 */
//**************************************************************************************
#include "task_encoder.h"                   // Header for this task

#include "textqueue.h"                      // Header for text queue class
#include "taskshare.h"			    // Header for thread-safe shared data
#include "shares.h"                         // Shared inter-task communications

//-------------------------------------------------------------------------------------
/** TODO This constructor creates a task which controls the...
 *  @param param TODO Param desciption

 */
// TODO:...
			
task_encoder::task_encoder (const char* a_name, unsigned portBASE_TYPE a_priority, size_t a_stack_size, 
			emstream* p_ser_dev): TaskBase (a_name, a_priority, a_stack_size, p_ser_dev)
{
	// Nothing is done in the body of this constructor. All the work is done in the
	// call to the frt_task constructor on the line just above this one
}

//-------------------------------------------------------------------------------------
/** TODO This method is called once by the RTOS scheduler. Each time around the for (;;)
 *  loop, it sets the power of the two motors and lets them run for two seconds then
 *  brakes them, waits for two seconds, and then runs them again in the opposite direction.
 */

void task_encoder::run (void)
{
      
      encoder_drv* p_enc_1 = new encoder_drv(p_serial, 5);
      
      //EICRB &= ~(1<<ISC61);	// Set bit 4 high on EICRB
      //EICRB |= 1<<(ISC60);	// set bit 5 low on EICRB
      //EIMSK |= (1<<interrupt_ch);
      
      for(;;)
      {
	*p_serial << sh_encoder_count_1->get() << endl;
	delay_ms(1000);
	
      }
      //sh_encoder_count
      
      
      //sh_error_count
  
}