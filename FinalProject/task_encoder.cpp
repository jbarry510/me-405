//***********************************************************************************************************
/** @file task_encoder.cpp
 *  This file contains the code for a task that instatiates the encoder object and
 *  debugs encoder data.
 *
 *  Revisions:
 *    @li April 13, 2016 -- BKK ME405 Group 3 original file
 *    @li April 28, 2016 -- BKK Added an encoder object debugs and object instatiation
 *
 */
//***********************************************************************************************************
#include "textqueue.h"                      // Header for text queue class
#include "taskshare.h"			    // Header for thread-safe shared data
#include "shares.h"                         // Shared inter-task communications
#include "encoder_drv.h"

#include "task_encoder.h"                   // Header for this task

//-----------------------------------------------------------------------------------------------------------
/** This constructor creates a task which controls the ouput of two encoders. The main job of this
 *  constructor is to call the constructor of parent class (\c frt_task ); the parent's constructor the work.
 *  @param a_name A character string which will be the name of this task
 *  @param a_priority The priority at which this task will initially run (default: 0)
 *  @param a_stack_size The size of this task's stack in bytes (default: configMINIMAL_STACK_SIZE)
 *  @param p_ser_dev Pointer to a serial device (port, radio, SD card, etc.) which can be used by this task
 *		     to communicate (default: NULL)
 */

task_encoder::task_encoder (const char* a_name, unsigned portBASE_TYPE a_priority, size_t a_stack_size, 
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

// TODO Use task encoder to compute RPM values and process encoder data. Then this information will be passed
// to a task_pid?
void task_encoder::run (void)
{
      // Construction of Encoder Drivers
      //encoder_drv* p_enc_7 = new encoder_drv(p_serial, 7);
      encoder_drv* encoder_driver_1 = new encoder_drv(p_serial, 7);  // 6 and 7 aliased
      encoder_drv* encoder_driver_2 = new encoder_drv(p_serial, 5);  // 4 and 5 alised (4 doesn't work?)
      
      // maximum time for encoder to count 0 to 48 at max speed
      uint16_t speed_period_ms = 5; 
      
      // Reminder: Make shared variables!!
      // counts before/after used to determine speed
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
	
	sh_motor_1_speed->put(encoder_driver_1->calc_motor_speed(encoder_count_new_motor_1, encoder_count_old_motor_1,
			   speed_period_ms));
	
	sh_motor_2_speed->put(encoder_driver_2->calc_motor_speed(encoder_count_new_motor_2, encoder_count_old_motor_2,
			   speed_period_ms));
	
//  	*p_serial << PMS("Motor Speed_1 (rpm) = ") << dec << sh_motor_1_speed->get() << endl;
// 	*p_serial << PMS("Motor Speed_2 (rpm) = ") << dec << sh_motor_2_speed->get() << endl;
	
// 	*p_serial << PMS("Encoder count 2 = ") << dec << sh_encoder_count_2->get() << endl;
// 	*p_serial << PMS("OLD state 2 = ") << bin << sh_encoder_old_state_2->get() << endl;
// 	*p_serial << PMS("NEW state 2 = ") << bin << sh_encoder_new_state_2->get() << endl;
// 	*p_serial << PMS("Error count 2 = ") << dec << sh_encoder_error_count_2->get() << endl;
// 	*p_serial << endl;
	delay_ms(speed_period_ms);
      }
}