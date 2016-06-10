//***********************************************************************************************************
/** @file task_control.cpp
 *  This file contains the code for a task that configures and operates the motor PID and the major route
 *  features.
 */
//***********************************************************************************************************
#include "textqueue.h"				// Header for text queue class
#include "taskshare.h"				// Header for thread-safe shared data 
#include "shares.h"				// Shared inter-task communications

#include "task_control.h"			// Header for this task

//-----------------------------------------------------------------------------------------------------------
/** 
 *  This constructor creates a task which controls the output of two pid loops and operates major route 
 *  features. The main job of this constructor is to call the constructor of parent class (\c frt_task );
 *  the parent's constructor the work.
 *  @param a_name A character string which will be the name of this task
 *  @param a_priority The priority at which this task will initially run (default: 0)
 *  @param a_stack_size The size of this task's stack in bytes (default: configMINIMAL_STACK_SIZE)
 *  @param p_ser_dev Pointer to a serial device (port, radio, SD card, etc.) which can be used by this task
 *		     to communicate (default: NULL)
 */

task_control::task_control (const char* a_name, unsigned portBASE_TYPE a_priority, size_t a_stack_size, 
			    emstream* p_ser_dev): TaskBase (a_name, a_priority, a_stack_size, p_ser_dev)
{
	// Nothing is done in the body of this constructor. 
}

//-----------------------------------------------------------------------------------------------------------
/** This method is called once by the RTOS scheduler. Each time around the for (;;) loop, it instatiates a
 *  new encoder object, giving the interrupt pin, 7, enabling external interrupt derived encoder data about
 *  the motor rotation.
 */

void task_control::run (void)
{	  
     // Make a variable which will hold times to use for precise task scheduling
     TickType_t previousTicks = xTaskGetTickCount ();
	  
     // Creating the PID objects for Motor 1 and Motor 2
     pid* pid_1 = new pid(p_serial);
     pid* pid_2 = new pid(p_serial);
     pid* pid_3 = new pid(p_serial);
     
     // Motor 1 PID Constants
     int16_t Kp_1 = 1 * 1024;					// K_p Proportional gain
     int16_t Ki_1 = 1 * 256;					// K_i Integral gain
     int16_t Kw_1 = 1 * 256;					// K_w Anti-windup gain
     int16_t min_1 = -1600;					// Minimum saturation limit
     int16_t max_1 = 1600;					// Maximum saturation limit
     sh_motor_1_speed->put(0);					// Clear motor speed
     sh_setpoint_1->put(0);					// Clear motor setpoint
     
     // pid::config{mode, Ki, Kp, Kd, Kw, min_satur, max_satur};
     pid_1->set_config(pid::config_t{pid::PI, Kp_1, Ki_1, 0, Kw_1, min_1, max_1});
     
     // Motor 2 PID Constants
     int16_t Kp_2 = 1 * 1024;					// K_p Proportional gain
     int16_t Ki_2 = 1 * 256;					// K_i Integral gain
     int16_t Kw_2 = 1 * 256;					// K_w Anti-windup gain
     int16_t min_2 = -1600;					// Minimum saturation limit
     int16_t max_2 = 1600;					// Maximum saturation limit
     sh_motor_2_speed->put(0);					// Clear motor speed
     sh_setpoint_2->put(0);					// Clear motor setpoint
     
     //pid::config{mode, Kp, Ki, Kd, Kw, min_satur, max_satur};
     pid_2->set_config(pid::config_t{pid::PI, Kp_2, Ki_2, 0, Kw_2, min_2, max_2});
     
     // Servo PID Constants
     int16_t Kp_3 = 1 * 1024;					// K_p Proportional gain
     int16_t Kd_3 = 1 * 128;					// K_i Integral gain
     int16_t min_3 = -10;					// Minimum saturation limit
     int16_t max_3 = 10;					// Maximum saturation limit
     
     //pid::config{mode, Kp, Ki, Kd, Kw, min_satur, max_satur};
     pid_3->set_config(pid::config_t{pid::PD, Kp_3, 0, Kd_3, 0, min_2, max_2});
     
     int16_t setpoint_1 = 0;					// Velocity set point Motor 1
     int16_t setpoint_2 = 0;					// Velocity set point Motor 2
     int32_t distance = 0;					// Linear route distance input
     uint16_t encoder_count = 0;				// Distance change from encoders
     uint16_t inch_to_ticks = 356;				// Distance unit conversion
     int8_t  counter = 0;					// Debug counter
     int16_t new_servo_error = 0;				// Heading error
     int16_t new_servo_angle = 0;				// New calculated servo angle
     
     for(;;)
     {
	      // Set power for motor 1
	      setpoint_1 = sh_setpoint_1->get();
	      
	      // Saturates maximum and minimum new power setting to +- 80 for Motor 1
	      if(setpoint_1 >= -80 && setpoint_1 <= 80)
	      {
		  sh_PID_1_power->put(pid_1->compute(sh_motor_1_speed->get(), setpoint_1));
		  sh_power_set_flag->put(1);
	      }
	      else if(setpoint_1 < -80) 
	      {
		  setpoint_1 = -80;
		  sh_PID_1_power->put(pid_1->compute(sh_motor_1_speed->get(), setpoint_1));
		  sh_power_set_flag->put(1);
	      }
	      else if(setpoint_1 > 80)
	      {
		  setpoint_1 = 80;
		  sh_PID_1_power->put(pid_1->compute(sh_motor_1_speed->get(), setpoint_1));
		  sh_power_set_flag->put(1);
	      }
	      else
		  *p_serial << PMS ("PID 1 error") << endl;

	      // Set power for motor 2
	      setpoint_2 = -sh_setpoint_2->get();
	      
	      // Saturates maximum and minimum new power setting to +- 40 for Motor 2
	      if(setpoint_2 >= -80 && setpoint_2 <= 80)
	      {
		  sh_PID_2_power->put(pid_2->compute(sh_motor_2_speed->get(), setpoint_2));
	          sh_power_set_flag->put(1);
	      }
	      else if(setpoint_2 < -80) 
	      {
		  setpoint_2 = -80;
		  sh_PID_2_power->put(pid_2->compute(sh_motor_2_speed->get(), setpoint_2));
		  sh_power_set_flag->put(1);
	      }
	      else if(setpoint_2 > 80)
	      {
		  setpoint_2 = 80;
		  sh_PID_2_power->put(pid_2->compute(sh_motor_2_speed->get(), setpoint_2));
		  sh_power_set_flag->put(1);
	      }
	      else
		  *p_serial << PMS ("PID 2 error") << endl; // Debugs error message
		  
	  if (sh_PID_control->get() == 1)					// Linear Path Adherance
	  {
	       // Sets velocity setpoints for constant travel
	       sh_setpoint_1->put(sh_path_velocity->get());			// Motor 1
	       sh_setpoint_2->put(sh_setpoint_1->get());			// Motor 2
	       
	       // Change in motor 1 ticks indicates distance travelled
	       encoder_count = sh_motor_1_speed->get();		
	       
	       // Initialization block
	       if (sh_linear_start->get() == 1)				
	       {
		    sh_servo_setpoint->put(3000);				// Sets neutral servo position
		    sh_setpoint_1->put(0);					// Clears motor setpoints
		    sh_setpoint_2 ->put(0);					
		    distance = inch_to_ticks * sh_linear_distance ->get();	// Calculates total travel
		    sh_linear_start ->put(0);
	       }
	       
	       // Main operation block
	       if(distance >= 0)
	       {    
		    new_servo_error = (sh_heading_setpoint->get() - sh_euler_heading->get())/10; // Determines heading error
		    new_servo_angle = new_servo_error;			// Calculates new servo angle
		    sh_servo_setpoint->put(routes::servo_power(new_servo_angle));		// Sets new servo position
		    distance -= encoder_count;							// Subtracts the encoder distance travelled from the total
	       }
	       else // Closing block
	       {
		   sh_PID_control->put(0);					// Ends route operation
		   sh_setpoint_1->put(0);					// Clears motor setpoints
		   sh_setpoint_2->put(0);	
		   distance = 0;						// Clears distance
		   encoder_count = 0;						// Clears encoder_count
		   sh_servo_setpoint -> put(3000);				// Puts servo in neutral position
	       }
	       
	  }
	  else if (sh_PID_control->get() == 2)				// Circular Path Adherance
	  {
// 	       sh_setpoint_1->put(sh_path_velocity->get());
// 	       sh_setpoint_2->put(sh_setpoint_1->get());
// 	       encoder_count = sh_motor_1_speed->get();
// 	       
// 	       if (sh_circular_start->get() == 1)
// 	       {
// 		    velocity_setpoint = routes::motor_velocity(sh_path_velocity->get());
// 		    heading_change_setpoint = velocity_setpoint/sh_path_radius->get();
// 		    sh_servo_setpoint->put(routes::servo_angle(9/sh_path_radius->get()));
// 		    velocity_ratio = 
// 		    sh_setpoint_1->put(0);
// 		    sh_setpoint_2 ->put(0);
// 		    distance = inch_to_ticks * sh_linear_distance ->get();
// 		    sh_linear_start ->put(0);
// 		    *p_serial << PMS ("Path velocity: ") << sh_path_velocity << endl;
//  		    *p_serial << PMS ("Starting heading: ") <<  sh_euler_heading->get() << endl;
// 		    *p_serial << PMS ("sh_linear_distance: ") <<  sh_linear_distance->get() << endl;
// 		    *p_serial << PMS ("Distance: ") << distance << endl;
// 		    *p_serial << PMS ("Finished Initialization ") << endl << endl;
// 	       }
// 	       
// 	       if(distance >= 0)
// 	       {    
// 		    if(counter<=10)
// 		    {
// 			 *p_serial << PMS ("Old Servo Position : ") << sh_servo_setpoint->get() << endl;
// 			 *p_serial << PMS ("Intended Heading   : ") << sh_heading_setpoint->get() << endl;
// 			 *p_serial << PMS ("sh_euler_heading   : ") << sh_euler_heading->get() << endl << endl;
// 			 *p_serial << PMS ("Heading Error      : ") << new_servo_error<< endl;
// 			 *p_serial << PMS ("New Servo Angle    : ") << new_servo_angle << endl << endl;
// 			 counter++;
// 		    }
// 		    new_servo_error = sh_heading_setpoint->get() - sh_euler_heading->get();
// 		    new_servo_angle = new_servo_angle + new_servo_error;
// 		    sh_servo_setpoint->put(routes::servo_power(new_servo_angle));
// 		    distance -= encoder_count;
// 	       }
// 	       else
// 	       {
// 		   sh_PID_control->put(0);
// // 		   sh_braking_full_flag->put(1);
// 		   sh_setpoint_1->put(0);
// 		   sh_setpoint_2->put(0);
// 		   distance = 0;
// 		   encoder_count = 0;
// 		   sh_servo_setpoint -> put(3000);
// 		   sh_servo_set_flag ->put(1);
// 		   *p_serial << PMS ("Finished Route! ") << endl << endl;
// 	       }
	  }
		 
     runs++;					// Increment the timer run counter.
     delay_from_for_ms (previousTicks, 10);	// Task runs every 10 ms
     }
}