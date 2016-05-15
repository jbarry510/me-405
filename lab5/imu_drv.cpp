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
#include "i2c_master.h"			  // Include header for the I2C communication class

//------------------------------------------------------------------------------------------------------------
/** \brief This constructor sets up the 9 DOF IMU object.
 *  \details
 *  @param p_serial_port A pointer to the serial port which writes debugging info.
 */

imu_drv::imu_drv(emstream* p_serial_port)
{
    ptr_to_serial = p_serial_port;
}

