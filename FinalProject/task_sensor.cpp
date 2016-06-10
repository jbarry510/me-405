//***********************************************************************************************************
/** @file task_sensor.cpp
 *  This file contains the header for a task class that creates an IMU sensor object and adc objects for the
 *  IR distance sensors. The readings are saved to a shared variables to be used by other tasks.
 */
//***********************************************************************************************************
#include "textqueue.h"                      // Header for text queue class
#include "taskshare.h"			    // Header for thread-safe shared data
#include "shares.h"                         // Shared inter-task communications

#include "adc.h"			    // Header for the adc class 
#include "task_sensor.h"                    // Header for this task

//-----------------------------------------------------------------------------------------------------------
/** 
 *  This constructor creates a task which creates an IMU sensor and adc objects that give IR sensor readings.
 *  The main job of this constructor is to call the constructor of parent class 
 *  (\c frt_task ); the parent's constructor the work.
 *  @param a_name A character string which will be the name of this task
 *  @param a_priority The priority at which this task will initially run (default: 0)
 *  @param a_stack_size The size of this task's stack in bytes (default: configMINIMAL_STACK_SIZE)
 *  @param p_ser_dev Pointer to a serial device (port, radio, SD card, etc.) which can be used by this task
 *		     to communicate (default: NULL)
 */

task_sensor::task_sensor (const char* a_name, unsigned portBASE_TYPE a_priority, size_t a_stack_size, 
			emstream* p_ser_dev): TaskBase (a_name, a_priority, a_stack_size, p_ser_dev)
{
	// Nothing is done in the body of this constructor. 
}

//-----------------------------------------------------------------------------------------------------------
/** This method is called once by the RTOS scheduler. Each time around the for (;;) loop, it instatiates a
 *  new IMU object and two adc objects for IR distance readings.
 */

void task_sensor::run (void)
{
     /// Make a variable which will hold times to use for precise task scheduling
     TickType_t previousTicks = xTaskGetTickCount ();
     
     /// Creates adc objects for IR distance sensors
     adc* side_IR_adc = new adc(p_serial);
     adc* front_IR_adc = new adc(p_serial);
     
     /// Creates a new IMU object
     imu_drv* imu_sensor = new imu_drv(p_serial);
     
     /// Initializes the sensor reading variables
     int16_t heading = 0; 
     int16_t side_IR_reading = 0;
     int16_t front_IR_reading = 0;
     
     /// Main task loop 
     for(;;)
     {
       
	  /// First paraemter is channel of ADC to read from
	  /// Second parameter is number of samples to take
	  side_IR_reading = side_IR_adc->read_oversampled(1,10);
	  front_IR_reading = front_IR_adc->read_oversampled(2,10);
	  
	  /// Calls the system status method in the imu_drv which prints a message regarding the status
	  if(sh_imu_status->get() == 1)
	  {
	       imu_sensor->getSysStatus();
	       sh_imu_status->put(0);
	  }
	  
	  /// Gets the Euler angle variables by calling the getEulerAng method in the imu_drv.
	  heading = imu_sensor->getEulerAng(1);
	  
	  /// Saves Euler heading reading to a shared variable
	  sh_euler_heading -> put(heading);
	  
	  runs++;					// Increment the timer run counter.
	  delay_from_for_ms (previousTicks, 10);	// Task runs every 10 ms
     }
}