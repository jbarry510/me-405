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
/** \brief This function performs saturated addition
 *  \details The function performs addition of two 16-bit signed integers, saturating at either the maximal
 *	     or minimal 16-bit values
 *  @param x Augend
 *  @param y Addend
 *  @return Sum
 */

int16_t routes::servo_angle(uint16_t power)
{
     int16_t coefficient = 3;
     int16_t constant = 89;
     return (constant - (coefficient*power)/100);
}

uint16_t routes::servo_power(int16_t angle)
{
     int16_t coefficient = 34;
     int16_t constant = 3034;
     
     if (angle >= 30)
	  angle = 30;
     if (angle <= -30)
	  angle = -30;
     
     return (constant - coefficient*angle);
}
uint16_t routes::motor_setpoint(uint16_t velocity)
{
     int16_t velocity_range = 88;
     int16_t setpoint_range = 80;
     
     return ((setpoint_range/velocity_range)*velocity);
}
uint16_t routes::motor_velocity(uint16_t setpoint)
{
     int16_t velocity_range = 88;
     int16_t setpoint_range = 80;
     
     return ((velocity_range/setpoint_range)*setpoint);
}
