//*************************************************************************************
/** @file motor_drv.cpp
 *    This file contains a motor control driver.   
 *
 *  Revisions:
 *    @li 01-15-2008 JRR Original (somewhat useful) file
 *    @li 10-11-2012 JRR Less original, more useful file with FreeRTOS mutex added
 *    @li 10-12-2012 JRR There was a bug in the mutex code, and it has been fixed
 *
 *  License:
 *    This file is copyright 2015 by JR Ridgely and released under the Lesser GNU 
 *    Public License, version 2. It intended for educational use only, but its use
 *    is not limited thereto. */
/*    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 *    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 *    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 *    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
 *    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUEN-
 *    TIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS 
 *    OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
 *    CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
 *    OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 *    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */
//*************************************************************************************

#include <stdlib.h>                         // Include standard library header files
#include <avr/io.h>

#include "rs232int.h"                       // Include header for serial port class
#include "motor_drv.h"                      // Include header for the motor class


//-------------------------------------------------------------------------------------
/** \brief TODO This constructor... @b Make @b generic
 *  \details TODO
 *  @param p_serial_port A pointer to the serial port which writes debugging info.
 *  @param select Selects desired motor driver chip 
 */

motor_drv::motor_drv(emstream* p_serial_port, uint8_t select)
{
	// Defines pointer for serial to inputted parameter
	ptr_to_serial = p_serial_port;
	
	// Timing channel 1 setup (Fast PWM) 
	TCCR1A |= (1 << WGM10) | (1 << COM1B1) & ~(1 << COM1B0) | (1 << COM1A1) & ~(1 << COM1A0);
	TCCR1B |= (1 << WGM12) & ~(1 << CS12) | (1 << CS11)  & ~(1 << CS10);
	
	if(select == 1)
	{
	  // Sets direction of port C pins 0-2 to output (Direction control)
	  DDRC |= (1<<DDC0)|(1<<DDC1)|(1<<DDC2);
	  
	  // Sets output enable on motor driver 1
	  PINC |= (1 << PINC2);

	  // Sets direction of port B pin 6 to output (PWM control)
	  DDRB |= (1<<DDB6);
	
	  // Prints a message if motor 1 construction was sucessful
	  DBG(ptr_to_serial, "motor 1 constructor OK" << endl);
	  DBG(ptr_to_serial, "DDRC: " << bin <<DDRC << endl);
	  DBG(ptr_to_serial, "DDRB: " << DDRB << endl);
	}
	
	else if(select == 2)
	{
	  // Sets direction of port D pins 5-7 to output (Direction control)
	  DDRD |= (1<<DDD5)|(1<<DDD6)|(1<<DDD7);
	  
	  // Sets output enable on motor driver 2
	  PIND |= (1 << PIND7);
	
	  // Sets direction of port B pin 5 to output (PWM control)
	  DDRB |= (1<<DDB5);
	
	  // Prints a message if motor 2 construction was sucessful
	  DBG(ptr_to_serial, "motor 2 constructor OK" << endl);
	  DBG(ptr_to_serial, "DDRD: " << bin << DDRD << endl);
	  DBG(ptr_to_serial, "DDRB: " << DDRB << endl);
	}
	
	else
	{
	  // Prints an error message if select is not a 1 or 2
	  DBG(ptr_to_serial, "motor constructor FAIL (select must be a 1 or 2)" << endl);
	}
}


//-------------------------------------------------------------------------------------
/** @brief   This method takes an integer and sets the motor torque and direction. TODO @b Make @b generic 
 *  \details The value of the integer corresponds to the amount of torque applied by
 *  the motor. The sign of the integer corresponds to the direction the motor turns.
 *  Negative values turn the motor @b clockwise and positive values turn the motor @b counterclockwise .
 *  @param   power 
 *  @return  None
 */

uint16_t motor_drv::set_power(int16_t power, uint8_t select) 
{
      if(select == 1)
      {
	  if(power < 0)
	  {
	    PINC |= (1<<PINC0) & ~(1<<PINC1);
	    power = power*-1;
	  }
	  else
	  {
	    PINC |= (1<<PINC1) & ~(1<<PINC0);
	  }
      }
     
      if(select == 2)
      {
	  if(power < 0)
	  {
	    PIND |= (1<<PIND5) & ~(1<<PIND6);
	    power = power*-1;
	  }
	  else
	  {
	    PIND |= (1<<PIND6) & ~(1<<PIND5);
	  }
      }
      
      return power;
}


//-------------------------------------------------------------------------------------
/** @brief   TODO This method
 *  \details TODO
 *  @return  None
 */

void motor_drv::brake_full()
{
    PINC |= (1<<PINC0) | (1<<PINC1);
    PIND |= (1<<PIND5) | (1<<PIND6);
}
//-------------------------------------------------------------------------------------
/** @brief   TODO This method
 *  \details TODO
 *  @param strength 
 *  @return  None
 */

void motor_drv::brake(uint8_t strength)
{
    // Sets PWM controlled braking (brake to GND) for motor 1 and sets strength of braking
    PINC &= ~(1<<PINC0) | ~(1<<PINC1);
    OCR1B = strength;
    
    // Sets PWM controlled braking (brake to GND) for motor 2 and sets strength of braking
    PIND &= ~(1<<PIND5) | ~(1<<PIND6);
    OCR1A = strength;
}