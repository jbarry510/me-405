 //***********************************************************************************************************
/** @file task_pid.cpp
 * 
 * 
 */
//***********************************************************************************************************
#include "textqueue.h"				// Header for text queue class
#include "taskshare.h"				// Header for thread-safe shared data
#include "shares.h"				// Shared inter-task communications
#include "pid.h"				// Header for pid functions

#include "task_pid.h"				// Header for this task

//-----------------------------------------------------------------------------------------------------------
/** 
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
     //int32_t setpoint_2_ticks = 0;		// Converted setpoint from user input of [in/sec] to [ticks/ms]
     //int32_t motor2_temp_ticks = 0;		// Temporary for converting ticks to PWM power range
     //int32_t max_motor_ticks = (192 * 20.4) / 6;		// Max speed ticks in one rev / time period for one rev
     
     // Make a variable which will hold times to use for precise task scheduling
     TickType_t previousTicks = xTaskGetTickCount ();
     
//       uint16_t max_motor_power = 255;					// Maximum motor power for PWM
//       uint16_t max_motor_lin_speed = (500 * 2 * 3.14159 * 1.75) / 60;	// [rev/min] * 2pi * 1.75 [in]) / (60 [min/sec])
	  
	  
     // Creating the PID objects for Motor 1 and Motor 2
     //pid* pid_1 = new pid(p_serial);
     pid* pid_2 = new pid(p_serial);
     
     // Motor 1 PID Constants
     //int16_t Kp_1 = 5 * 1024;					// K_p Proportional gain
     //int16_t Ki_1 = 0 * 1024;					// K_i Integral gain
     //int16_t Kw_1 = 0 * 1024;					// K_w Anti-windup gain
     //int16_t min_1 = -255;					// Minimum saturation limit
     //int16_t max_1 = 255;					// Maximum saturation limit
     //sh_motor_1_speed->put(0);					// Clear motor speed
     //sh_setpoint_1->put(0);					// Clear motor setpoint
     
     // pid::config{mode, Ki, Kp, Kd, Kw, min_satur, max_satur};
     //pid_1->set_config(pid::config_t{pid::PI, Ki_1, Kp_1, 0, Kw_1, min_1, max_1});
     
     // Motor 2 PID Constants
     int16_t Kp_2 = 5 * 1024;					// K_p Proportional gain
     int16_t Ki_2 = 1 * 1024;					// K_i Integral gain
     int16_t Kw_2 = 1 * 1024;					// K_w Anti-windup gain
     int8_t min_2 = -256;					// Minimum saturation limit
     int8_t max_2 = 255;					// Maximum saturation limit
     sh_motor_2_speed->put(0);					// Clear motor speed
     sh_setpoint_2->put(0);					// Clear motor setpoint
     
     // pid::config{mode, Ki, Kp, Kd, Kw, min_satur, max_satur};
     pid_2->set_config(pid::config_t{pid::PI, Ki_2, Kp_2, 0, Kw_2, min_2, max_2});
     
     for(;;)
     {
	  if (sh_PID_control->get() == 1)
	  {
	  // Set power for motor 1

	  // Set power for motor 2
	  // ([in/sec]) * ((192*20.4 [ticks/rev]) / (1000[ms/sec] * 2pi * 1.75[in]))
	  //setpoint_2_ticks = sh_setpoint_2->get() * ((192 * 20.4) / (1000 * 2 * 3.14159 * 1.75));
	  
	  //motor2_temp_ticks = ((pid_2->compute(sh_motor_2_speed->get(), setpoint_2_ticks)));
	  //motor2_temp_ticks = (pid_2->compute(sh_motor_2_speed->get(), sh_setpoint_2->get()));
	  
	  // [power] = [ticks] * max power / max ticks [ticks]
	  //sh_PID_2_power->put(((motor2_temp_ticks) * max_motor_power) / max_motor_ticks);
	  
	  sh_PID_2_power->put(pid_2->compute(sh_motor_2_speed->get(), sh_setpoint_2->get()));
	  
	       // Timer for serial print
	       if(runs % 167 == 0)
	       {
		    *p_serial << PMS ("Encoder count 2 = ") << dec << sh_encoder_count_2->get() << endl;
		    *p_serial << PMS ("sh_motor_2_speed: ") << sh_motor_2_speed->get() << endl;
		    *p_serial << PMS ("sh_setpoint_2   : ") << sh_setpoint_2->get( )<< PMS ("[ticks]") << endl;
		    *p_serial << PMS ("Feedback        : ") << pid_2->get_input() << endl;
		    *p_serial << PMS ("Ouput           : ") << pid_2->get_output() << endl;
		    *p_serial << endl;
		    
	       }
	  }
     
     runs++;					// Increment the run counter.
     delay_from_for_ms (previousTicks, 5);	// Time for pid to delay
     }
}