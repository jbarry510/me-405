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
      encoder_drv* p_enc_7 = new encoder_drv(p_serial, 7);
      
      for(;;)
      {
	*p_serial << PMS("Encoder count 2 = ") << dec << sh_encoder_count_2->get() << endl;
	*p_serial << PMS("OLD state 2 = ") << bin << sh_encoder_old_state_2->get() << endl;
	*p_serial << PMS("NEW state 2 = ") << bin << sh_encoder_new_state_2->get() << endl;
	*p_serial << PMS("Error count 2 = ") << dec << sh_encoder_error_count_2->get() << endl;
	*p_serial << endl;
	
	delay_ms(500);
	
      }
}