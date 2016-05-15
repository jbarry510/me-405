//************************************************************************************************************
/** @file imu_drv.cpp
 *    This file contains the driver for the 9 DOF IMU breakout board (BNO055). Most of the code used was
 *    copied and modifed from the Adafruit BNO055 driver files
 *    (https://github.com/adafruit/Adafruit_BNO055/blob/master/Adafruit_BNO055.cpp).
 *
 *   TODO: Need calibration and i2c stuffs. UNTESTED
 *
 *  Revisions:
 *    @li 05-08-2016 ME405 Group 3 original file
 *
 */
//************************************************************************************************************

#include <stdlib.h>                       // Include standard library header files
#include <avr/io.h>

#include "rs232int.h"                     // Include header for serial port class
#include "imu_drv.h"                      // Include header for the motor class

//------------------------------------------------------------------------------------------------------------
/** \brief This constructor sets up the 9 DOF IMU object.
 *  \details
 *  @param p_serial_port A pointer to the serial port which writes debugging info.
 */

imu_drv::imu_drv(emstream* p_serial_port,i2c_master* i2c_comm_port)
{
    // Declares ptr_to_serial variable which is used for printing to serial port
    p_serial = p_serial_port;
    
    // Declares i2c_comm variable which is used for communicating with the sensor 
    i2c_comm = i2c_comm_port;
    
    // Make sure we have the right device
    uint8_t id = i2c_comm->read(IMU_ADDRESS, BNO055_CHIP_ID_ADDR);
    if(id != BNO055_ID)
    {
	 id = i2c_comm->read(IMU_ADDRESS, BNO055_CHIP_ID_ADDR);
	 if(id != BNO055_ID) 
	 {
	      *p_serial << PMS ("It broke") << endl;
	 }
	 else
	 {
	      *p_serial << PMS ("All good fam") << endl;
	 }
    }
}


//------------------------------------------------------------------------------------------------------------
/** \brief TODO
 *  \details
 *  @param mode
 */
void imu_drv::setMode(imu_opmode_t mode)
{
     return;
}
