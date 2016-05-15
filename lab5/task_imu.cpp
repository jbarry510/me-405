//***********************************************************************************************************
/** @file task_imu.cpp
 * 
 * 
 */
//***********************************************************************************************************
#include "textqueue.h"                      // Header for text queue class
#include "taskshare.h"			    // Header for thread-safe shared data
#include "shares.h"                         // Shared inter-task communications

#include "task_imu.h"                       // Header for this task
#include "i2c_master.h"			    // Include header for the I2C communication class

//-----------------------------------------------------------------------------------------------------------
/** 
 *  TODO NEEDS CHANGE!!! 
 *  This constructor creates a task which controls the ouput of two encoders. The main job of this
 *  constructor is to call the constructor of parent class (\c frt_task ); the parent's constructor the work.
 *  @param a_name A character string which will be the name of this task
 *  @param a_priority The priority at which this task will initially run (default: 0)
 *  @param a_stack_size The size of this task's stack in bytes (default: configMINIMAL_STACK_SIZE)
 *  @param p_ser_dev Pointer to a serial device (port, radio, SD card, etc.) which can be used by this task
 *		     to communicate (default: NULL)
 */

task_imu::task_imu (const char* a_name, unsigned portBASE_TYPE a_priority, size_t a_stack_size, 
			emstream* p_ser_dev): TaskBase (a_name, a_priority, a_stack_size, p_ser_dev)
{
	// Nothing is done in the body of this constructor. 
        // This new task just waits for encoder_drv to share encoder data: state, direction, etc...
}

//-----------------------------------------------------------------------------------------------------------
/** This method is called once by the RTOS scheduler. Each time around the for (;;) loop, it instatiates a
 *  new encoder object, giving the interrupt pin, 7, TODO enabling external interrupt derived encoder data about
 *  the motor rotation.
 */

void task_imu::run (void)
{
     imu_drv* imu_sensor = new imu_drv(p_serial);
     i2c_master* i2c_comm = new i2c_master(p_serial);
     
     for(;;)
     {
	  if(i2c_comm->ping(IMU_ADDRESS))
	  {
	       *p_serial << PMS ("You got pinged, yo")<< endl;
	       *p_serial << PMS ("When we walk in da club we don't need ID: ") << hex << i2c_comm->read(IMU_ADDRESS,imu_drv::BNO055_CHIP_ID_ADDR)<< endl;
	       *p_serial << endl;
	  }
	  else
	  {
	       *p_serial << PMS (":(") << endl;
	  }
	  
	  delay_ms(5000); // Time that the task waits before looping
     }
}