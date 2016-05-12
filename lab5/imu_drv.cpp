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

imu_drv::imu_drv(emstream* p_serial_port, int32_t sensorID, uint8_t address)
{
    p_serial = p_serial_port;
    _sensorID = sensorID;
    _address = address;
}

//------------------------------------------------------------------------------------------------------------
/** @brief   Gets a vector reading from the specified source
 *  \details
 *  @param   vector_type
 *  @return  xyz
 */

imu::Vector<3> imu_drv::getVector(imu_vector_type_t vector_type)
{
  imu::Vector<3> xyz;
  uint8_t buffer[6];
  memset (buffer, 0, 6);

  int16_t x, y, z;
  x = y = z = 0;

  /* Read vector data (6 bytes) */
  readLen((imu_reg_t)vector_type, buffer, 6);

  x = ((int16_t)buffer[0]) | (((int16_t)buffer[1]) << 8);
  y = ((int16_t)buffer[2]) | (((int16_t)buffer[3]) << 8);
  z = ((int16_t)buffer[4]) | (((int16_t)buffer[5]) << 8);

  /* Convert the value to an appropriate range (section 3.6.4) */
  /* and assign the value to the Vector type */
  switch(vector_type)
  {
    case VECTOR_MAGNETOMETER:
      /* 1uT = 16 LSB */
      xyz[0] = ((double)x)/16.0;
      xyz[1] = ((double)y)/16.0;
      xyz[2] = ((double)z)/16.0;
      break;
    case VECTOR_GYROSCOPE:
      /* 1rps = 900 LSB */
      xyz[0] = ((double)x)/900.0;
      xyz[1] = ((double)y)/900.0;
      xyz[2] = ((double)z)/900.0;
      break;
    case VECTOR_EULER:
      /* 1 degree = 16 LSB */
      xyz[0] = ((double)x)/16.0;
      xyz[1] = ((double)y)/16.0;
      xyz[2] = ((double)z)/16.0;
      break;
    case VECTOR_ACCELEROMETER:
    case VECTOR_LINEARACCEL:
    case VECTOR_GRAVITY:
      /* 1m/s^2 = 100 LSB */
      xyz[0] = ((double)x)/100.0;
      xyz[1] = ((double)y)/100.0;
      xyz[2] = ((double)z)/100.0;
      break;
  }

  return xyz;
}

//------------------------------------------------------------------------------------------------------------
/** @brief   Reads the sensor and returns the data as a sensors_event_t
 *  \details
 *  @param   vector_type
 *  @return  xyz
 */
 
bool imu_drv::getEvent(sensors_event_t *event)
{
  /* Clear the event */
  memset(event, 0, sizeof(sensors_event_t));

  event->version   = sizeof(sensors_event_t);
  event->sensor_id = _sensorID;
  event->type      = SENSOR_TYPE_ORIENTATION;
  event->timestamp = millis();

  /* Get a Euler angle sample for orientation */
  imu::Vector<3> euler = getVector(Adafruit_BNO055::VECTOR_EULER);
  event->orientation.x = euler.x();
  event->orientation.y = euler.y();
  event->orientation.z = euler.z();

  return true;
}
