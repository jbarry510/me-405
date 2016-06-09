//************************************************************************************************************
/** @file imu_drv.cpp
 *    This file contains the driver for the 9 DOF IMU breakout board (BNO055). Most of the code used was
 *    copied and modifed from the Adafruit BNO055 driver files
 *    (https://github.com/adafruit/Adafruit_BNO055/blob/master/Adafruit_BNO055.cpp).
 *
 *  Revisions:
 *    @li 05-14-2016 ME405 Group 3 original file
 *
 */
//************************************************************************************************************

#include <stdlib.h>                       // Include standard library header files
#include <avr/io.h>

#include "rs232int.h"                     // Include header for serial port class
#include "imu_drv.h"                      // Include header for the motor class

//------------------------------------------------------------------------------------------------------------
/** \brief This constructor sets up the 9 DOF IMU object.
 *  \details The constructor creates a p_serial object for printing to the serial port and creates a i2c_comm
 *           object which allows for communication between the sensor and ME 405 board. The constructor also
 *           checks that the i2c_comm object can communicate with the IMU and verifies the address of the IMU.
 *           Lastly the constructor sets the default settings of the imu which are nDOF mode, metric units,
 *           and normal power mode.
 *  @param p_serial_port A pointer to the serial port which writes debugging info.
 */

imu_drv::imu_drv(emstream* p_serial_port)
{
    // Declares ptr_to_serial variable which is used for printing to serial port
    p_serial = p_serial_port;
    
    // Creates an I2C communication object
    i2c_master* i2c_temp = new i2c_master(p_serial);
    i2c_comm = i2c_temp;
  
    // Make sure we have the right device (chip ID is 0xA0)
    uint8_t id = i2c_comm->read(IMU_ADDRESS, BNO055_CHIP_ID_ADDR);
    if(id != BNO055_ID)
    {
      *p_serial << PMS ("IMU object intial communication error") << endl;
    }
    else
    {
      *p_serial << PMS ("IMU object intial communication sucess") << endl;
    }
    
    setOpMode(OPERATION_MODE_CONFIG);
    setPwrMode(POWER_MODE_NORMAL);
    setUnits();
    setOpMode(OPERATION_MODE_NDOF);

}


/// Setter Methods
//------------------------------------------------------------------------------------------------------------
/** \brief Sets operation mode of IMU
 *  \details Changes the operation mode to one of the operation mode values defineed in the header file and 
 *           prints a confirmation message.
 *  @param mode Variable of imu_opmode type as defined in the header file.
 */
void imu_drv::setOpMode(imu_opmode_t mode)
{
  i2c_comm->write(IMU_ADDRESS, BNO055_OPR_MODE_ADDR, mode);
  *p_serial << PMS ("IMU operation mode set") << endl;
}

//------------------------------------------------------------------------------------------------------------
/** \brief Sets power mode of IMU
 *  \details Changes the power mode to one of the operation mode values defineed in the header file and prints
 *           a confirmation message.
 *  @param mode Variable of imu_powermode type as defined in the header file.
 */
void imu_drv::setPwrMode(imu_powermode_t mode)
{
  i2c_comm->write(IMU_ADDRESS, BNO055_PWR_MODE_ADDR, mode);
  *p_serial << PMS ("IMU power mode set") << endl;
}

//------------------------------------------------------------------------------------------------------------
/** \brief Sets units selection mode of IMU
 *  \details Sets the units for the acceleration, linear acceleration, and gravity vector to m/s^2. Sets the 
	     units for the angular rate to deg/sec. Sets the units for the Euler Angles to degrees and the 
	     units for the temperature to degC.
 *  @param none
 */
void imu_drv::setUnits()
{
  uint8_t unitsel = (0 << 7) | // Orientation = Android
                    (0 << 4) | // Temperature = Celsius
                    (0 << 2) | // Euler = Degrees
                    (0 << 1) | // Gyro = Degrees
                    (0 << 0);  // Accelerometer = m/s^2
  i2c_comm->write(IMU_ADDRESS, BNO055_UNIT_SEL_ADDR , unitsel);
  *p_serial << PMS ("IMU units set") << endl;
}

/// Getter methods
//------------------------------------------------------------------------------------------------------------
/** \brief Reads system status register of the IMU
 *  \details Prints out the system status of the IMU (numbers correspond to values listed in comments). If an
 *           error status is determined then the error message is also printed (numbers correspond to values
 *           listed in commments).
 *  @param none
 */

void imu_drv::getSysStatus()
{
  // Reads the system status register and saves it.
  /* System Status
     ---------------------------------
     0 = Idle
     1 = System Error
     2 = Initializing Peripherals
     3 = System Iniitalization
     4 = Executing Self-Test
     5 = Sensor fusion algorithm running
     6 = System running without fusion algorithms 
  */
  uint8_t sys_status = i2c_comm->read(IMU_ADDRESS, BNO055_SYS_STAT_ADDR);
  
  // Reads the system self test register and saves it.
  /* Self Test Results
     --------------------------------
     1 = test passed, 0 = test failed
     0 = Accelerometer self test
     1 = Magnetometer self test
     2 = Gyroscope self test
     4 = MCU self test
     15 = all good! 
  */
  uint8_t test_status = i2c_comm->read(IMU_ADDRESS, BNO055_SELFTEST_RESULT_ADDR);
 
  // Prints out the system status register contents
//   *p_serial << PMS ("IMU system status: ") << sys_status << endl;
  
  // If there is an error state then print out the error code
  if(sys_status == 1)
  {
//     *p_serial << PMS ("Error Code: ") << i2c_comm->read(IMU_ADDRESS, BNO055_SYS_ERR_ADDR) << endl;
  }
  
  // Prints out the system self test register contents
//   *p_serial << PMS ("IMU system self test status: ") << test_status << endl << endl;
  
}

//------------------------------------------------------------------------------------------------------------
/** \brief Reads Euler angle registers and returns selected value.
 *  \details This method returns the Euler angle value desired based on the data select input. An input of 1
 *           returns the heading value, an input of 2 returns the roll value, and an input of 3 returns the
 *           pitch value. The values read are 16x the value in degrees.
 *  @param data_sel Variable used to pick which Euler angle value (heading, roll, or pitch) is desired. 
 */

int16_t imu_drv::getEulerAng(uint8_t data_sel)
{
  if(data_sel == 1)
  {
    int16_t heading = ((((i2c_comm->read(IMU_ADDRESS, BNO055_EULER_H_MSB_ADDR)) << 8) | 
		         (i2c_comm->read(IMU_ADDRESS, BNO055_EULER_H_LSB_ADDR))));
    return heading;
  }
  else if(data_sel == 2)
  {
    int16_t roll    = ((((i2c_comm->read(IMU_ADDRESS, BNO055_EULER_R_MSB_ADDR)) << 8) | 
		         (i2c_comm->read(IMU_ADDRESS, BNO055_EULER_R_LSB_ADDR))));
    return roll;
  }
  else if(data_sel == 3)
  {
    int16_t pitch   = ((((i2c_comm->read(IMU_ADDRESS, BNO055_EULER_P_MSB_ADDR)) << 8) | 
		         (i2c_comm->read(IMU_ADDRESS, BNO055_EULER_P_LSB_ADDR))));
    return pitch;
  }
  else
  {
    *p_serial << PMS("Error in getEulerAng") << endl;
    return 0;
  }
}