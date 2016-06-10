//***********************************************************************************************************
/** \file routes.cpp
 *    This file contains a library of functions necessary to operate the route features.
 *
 *  Revisions:
 *    \li 06-08-2016 CTR Original file (.c and .h files)
 *
 *  License:
 *	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
 *	IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 *	FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 *	CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *	DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *	DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 *	IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 *	THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */
//***********************************************************************************************************

#include <stdint.h>
#include <stdlib.h>
#include "routes.h"

//-----------------------------------------------------------------------------------------------------------
/** \brief This function converts servo power setpoints to a wheel steering angle
 *  \details Servo_angle performs a linear conversion with real-world calibration values.
 *  @param coefficient Augend
 *  @param constant Addend
 *  @return Linear function result
 */
int16_t routes::servo_angle(uint16_t power)
{
     int16_t coefficient = 3;				// Linear function scalar
     int16_t constant = 89;				// Linear function offset
     return (constant - (coefficient*power)/100);
}

/** \brief This function converts wheel steering angle to a servo power setpoint
 *  \details Servo_power performs a linear conversion with real-world calibration values.
 *  @param coefficient Augend
 *  @param constant Addend
 *  @return Linear function result
 */
uint16_t routes::servo_power(int16_t angle)
{
     int16_t coefficient = 34;				// Linear function scalar
     int16_t constant = 3034;				// Linear function offset
     
     if (angle >= 30)					// Saturates the angle to +- 30
	  angle = 30;
     if (angle <= -30)
	  angle = -30;
     
     return (constant - coefficient*angle);
}

/** \brief This function converts a velocity (in/s) to a corresponding motor power value.
 *  \details Motor_setpoint scales a velocity by a ratio between velocity and setpoint maximums.
 *  @param setpoint_range Augend
 *  @param velocity_range Dividend
 *  @return Scalar
 */
uint16_t routes::motor_setpoint(uint16_t velocity)
{
     int16_t velocity_range = 88;			// Max velocity input [in/s]
     int16_t setpoint_range = 80;			// Max motor setpoint
     
     return ((setpoint_range/velocity_range)*velocity);
}

/** \brief This function converts a motor power setpoint to a corresponding velocity (in/s) value.
 *  \details Motor_setpoint scales a setpoint by a ratio between velocity and setpoint maximums.
 *  @param velocity_range Augend
 *  @param setpoint_range Dividend
 *  @return Scalar
 */
uint16_t routes::motor_velocity(uint16_t setpoint)
{
     int16_t velocity_range = 88;			// Max velocity input [in/s]
     int16_t setpoint_range = 80;			// Max motor setpoint
     
     return ((velocity_range/setpoint_range)*setpoint);
}
