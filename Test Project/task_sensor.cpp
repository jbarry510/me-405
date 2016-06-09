//***********************************************************************************************************
/** @file task_sensor.cpp
 *  This file contains the header for a task class that creates and tests the IMU object. This task tells the
 *  controller to print out the Euler angles (heading, pitch, and roll) every two seconds.
 * 
 */
//***********************************************************************************************************
#include "textqueue.h"                      // Header for text queue class
#include "taskshare.h"			    // Header for thread-safe shared data
#include "shares.h"                         // Shared inter-task communications
#include "adc.h"

#include "task_sensor.h"                       // Header for this task

//-----------------------------------------------------------------------------------------------------------
/** 
 *  This constructor creates a task which creates a sensor object and has it print the system status and Euler
 *  angles for testing purposes. The main job of this constructor is to call the constructor of parent class 
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
 *  new IMU object, giving it a pointer to the serial port, and requesting to print the system status of the
 *  IMU and return the Euler angles for printing.
 */

void task_sensor::run (void)
{
  
     //adc* pot_adc = new adc(p_serial);
     
     adc* side_IR_adc = new adc(p_serial);
     
     adc* front_IR_adc = new adc(p_serial);
     
     /// Creates a new IMU object
     imu_drv* imu_sensor = new imu_drv(p_serial);
     
     /// Initializes the Euler angle variables
     int16_t heading = 0; 
     int16_t pitch = 0; 
     int16_t roll = 0;
     int16_t old_heading = 0;
     int16_t side_IR_reading = 0;
     int16_t front_IR_reading = 0;
     
     /// Main task loop 
     for(;;)
     {
       
	  // first paraemter is channel of ADC to read from
	  // second parameter is number of samples to take
	  side_IR_reading = side_IR_adc->read_oversampled(1,10);
     
	  front_IR_reading = front_IR_adc->read_oversampled(2,10);
	  
	  //*p_serial << PMS("Front IR: ") << front_IR_reading << endl;
 	  //*p_serial << PMS("Side  IR: ")    << side_IR_reading << endl << endl;
	  
	  
	  /// Calls the system status method in the imu_drv which prints a message regarding the status
	  imu_sensor->getSysStatus();
	  
	  /// Gets the Euler angle variables by calling the getEulerAng method in the imu_drv.
	  heading = imu_sensor->getEulerAng(1);
	  roll = imu_sensor->getEulerAng(2);
	  pitch = imu_sensor->getEulerAng(3);
	  
	  old_heading = sh_euler_heading -> get();
	  sh_euler_heading -> put(heading);
	  
	  sh_euler_heading_change ->put(sh_euler_heading ->get()-old_heading);
	  
	  /// Prints the Euler angle variables to the serial port
// 	  *p_serial << PMS("Euler Heading: ") << heading << endl;
// 	  *p_serial << PMS("Euler Roll: ")    << roll    << endl;
// 	  *p_serial << PMS("Euler Pitch: ")   << pitch   << endl << endl;
	  
	  // Time that the task waits before looping
	  delay_ms(20);
     }
}