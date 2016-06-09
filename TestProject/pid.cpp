//***********************************************************************************************************
/** \file pid.cpp
 *    This file contains a very simple PID.
 *
 *  Revisions:
 *    \li 01-19-2016 CTR Original file
 *    \li 04-27-2016 CTR Fixed bug in the derivative calculation
 * 			 Changed names of everything to be a bit more intuitive
 * 			 Moved enum and struct definitions inside class
 *    \li May 4, 2016 -- BKK Added original file
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

#include "pid.h"                            // Include header for the motor class


//-----------------------------------------------------------------------------------------------------------
/** \brief This constructor sets up a 16-bit PID controller
 *  \details The pid is initialized off with all gains set to 0
 *  @param p_serial_port A pointer to the serial port which writes debugging info.
 */
pid::pid (emstream* p_serial_port):
	p_serial(p_serial_port),
	config{OFF,0,0,0,0,INT16_MIN,INT16_MAX},
	input(0),
	output(0),
	setpoint(0),
	error(0),
	linput(0),
	esum(0)
{
}

//-----------------------------------------------------------------------------------------------------------
/** \brief This method sets the PID mode
 *  \details The method sets the PID mode and also clears esum, effectively resetting
 *  the PID if the same mode is set again
 *  @param my_mode An enumerated type @c pid_mode_t describing the new mode for the PID
 */
void pid::set_mode(mode_t my_mode)
{
	config.mode=my_mode;
	esum=0;
}

//-----------------------------------------------------------------------------------------------------------
/** \brief Updates the input to the PID, the value we are trying to control
 *  \details In addition to updating the input value, it also computes the deriviative
 *  @param my_input New input value
 */
void pid::set_input(int16_t my_input)
{
	linput=input;
	input=my_input;	
	// Compute the derivative of the input to use for derivative gain.
	dinput  = sssub(input,linput);
}

//-----------------------------------------------------------------------------------------------------------
/** \brief Manually updates the control output value
 *  \details This method is for updating the PID controller value while it is in manual
 *           mode
 *  @param my_output New controller output value
 */
void pid::set_output(int16_t my_output)
{
	output=my_output;
}

//-----------------------------------------------------------------------------------------------------------
/** \brief Updates the set-point for the PID
 *  @param my_setpoint New set-point
 */
void pid::set_setpoint(int16_t my_setpoint)
{
	setpoint=my_setpoint;
}

//-----------------------------------------------------------------------------------------------------------
/** \brief Updates the PID configuration and clears esum, resetting the PID controller
 *  @param my_config New controller configuration
 */
void pid::set_config(config_t my_config)
{
	config=my_config;
	esum=0;
}

//-----------------------------------------------------------------------------------------------------------
/** \brief Updates the proportional gain
 *  @param Kp New proportional gain (1024x the value should be entered to allow fractional
 *            gain values.)
 */
void pid::set_Kp(int16_t Kp)
{
	config.Kp=Kp;
}

//-----------------------------------------------------------------------------------------------------------
/** \brief Updates the integral gain
 *  @param Ki New integral gain (1024x the value should be entered to allow fractional
 *            gain values.)
 */
void pid::set_Ki(int16_t Ki)
{
	config.Ki=Ki;
}

//-----------------------------------------------------------------------------------------------------------
/** \brief Updates the derivative gain
 *  @param Kd New derivivative gain (1024x the value should be entered to allow fractional
 *            gain values.)
 */
void pid::set_Kd(int16_t Kd)
{
	config.Kd=Kd;
}

//-----------------------------------------------------------------------------------------------------------
/** \brief Updates the anti-windup gain
 *  @param Kw New anti-windup gain (1024x the value should be entered to allow fractional
 *            gain values.)
 */
void pid::set_Kw(int16_t Kw)
{
	config.Kw=Kw;
}

//-----------------------------------------------------------------------------------------------------------
/** \brief Updates the upper and lower saturator values
 *  \details Updates the minimal and maximal values allowed as output from the PID
 *           controller.
 *  @param min New minimum value
 *  @param max New maximum value
 */
void pid::set_saturator(int16_t min, int16_t max)
{
	config.min=min;
	config.max=max;
}

//-----------------------------------------------------------------------------------------------------------
/** \brief Gets the current mode of the PID
 *  @return The current PID mode
 */
pid::mode_t pid::get_mode()
{
	return config.mode;
}

//-----------------------------------------------------------------------------------------------------------
/** \brief Gets the current value of the parameter being controlled by the PID
 *  @return Input (value being controlled)
 */
int16_t pid::get_input()
{
	return input;
}

//-----------------------------------------------------------------------------------------------------------
/** \brief Gets the controller output value
 *  @return Current controller output
 */
int16_t pid::get_output()
{
	return output;
}

//-----------------------------------------------------------------------------------------------------------
/** \brief Gets the current PID setpoint value
 *  @return The current PID setpoint value
 */
int16_t pid::get_setpoint()
{
	return setpoint;
}

//-----------------------------------------------------------------------------------------------------------
/** \brief Gets the controller configuration
 *  @return Current controller configuration
 */
pid::config_t pid::get_config()
{
	return config;
}

//-----------------------------------------------------------------------------------------------------------
/** \brief Gets the integral gain
 *  @return Integral gain
 */
int16_t pid::get_Ki()
{
	return config.Ki;
}

//-----------------------------------------------------------------------------------------------------------
/** \brief Gets the proportional gain
 *  @return Propotional gain
 */
int16_t pid::get_Kp()
{
	return config.Kp;
}

//-----------------------------------------------------------------------------------------------------------
/** \brief Gets the derivative gain
 *  @return derivative gain
 */
int16_t pid::get_Kd()
{
	return config.Kd;
}

//-----------------------------------------------------------------------------------------------------------
/** \brief Gets the anti-windup gain
 *  @return Anti-windup gain
 */
int16_t pid::get_Kw()
{
	return config.Kw;
}

//-----------------------------------------------------------------------------------------------------------
/** \brief Gets the saturator minimum
 *  @return Minimal controller output value
 */
int16_t pid::get_saturator_min()
{
	return config.min;
}

//-----------------------------------------------------------------------------------------------------------
/** \brief Gets the saturator maximum
 *  @return Maximal controller output value
 */
int16_t pid::get_saturator_max()
{
	return config.max;
}

//-----------------------------------------------------------------------------------------------------------
/** \brief Computes the new PID output value
 *  \details This method uses new actual and reference values to compute the controller output value. 
 * 	     The mode and gain settings are considered during the calculation.
 *  @param new_input The new input for the controller. This is the value being controlled
 *  @param new_setpoint The new setpoint for the controlller. This is the desired value for the input.
 *  @return The new output of the PID
 */
int16_t pid::compute(int16_t new_input, int16_t new_setpoint)
{
	set_input(new_input);
	set_setpoint(new_setpoint);
	return compute();
}

//-----------------------------------------------------------------------------------------------------------
/** \brief Computes the new PID output value
 *  \details This method forces the controller to recompute without updating the input or setpoint 
 */
int16_t pid::compute()
{
	// Create a temp variable to hold control signal before output
	int16_t temp = 0;
	
	// Compute Error
	error = input - setpoint;
	
	// Integrate the error and subtract the current saturation value multiplied by the anti-windup gain.
	esum  = sssub(ssadd(esum,error), ssdiv(ssmul(config.Kw,saturation),1024));
	
	// Compute output based on current PID mode
	switch(config.mode)
	{
		// No output signal while the controller is off
		case OFF:
			temp = 0;
			break;
		
		// Proportional only	
		case P:
			temp = ssdiv(ssmul(config.Kp,error),1024);
			break;
		
		// Proportional and integral	
		case PI:
			temp = ssadd(ssdiv(ssmul(config.Ki,esum),1024), ssdiv(ssmul(config.Kp,error),1024));
			break;
		
		// Proportional and Derivative
		case PD:
			temp = ssadd(ssdiv(ssmul(config.Kd,dinput),1024),ssdiv(ssmul(config.Kp,error),1024));
			break;
		
		// Full PID	
		case PID:
			temp = ssadd(ssdiv(ssmul(config.Ki,esum),1024),
				   ssadd(ssdiv(ssmul(config.Kd,dinput),1024),
					ssdiv(ssmul(config.Kp,error),1024)));
			break;
		
		// Manual mode (keep the current control value)	
		case MANUAL:
			temp = output;
			break;
			
	}
	
	// Saturation and storage of temp before exit
	// During the saturation, the amount "saturated" is stored to use for anti-windup feedback
	if (temp > config.max)
	{
		saturation = sssub(temp, config.max);
		output = config.max;
	}
	else if (temp < config.min)
	{
		saturation = sssub(temp, config.min);
		output = config.min;
	}
	else
	{
		saturation = 0;
		output = temp;
	}

	return output;	
}

//-----------------------------------------------------------------------------------------------------------
/** \brief This overloaded operator "prints the PID." 
 *  \details This gives a detailed readout on the current configuration
 *  @param serpt Reference to a serial port to which the printout will be printed
 *  @param pid Reference to the pid which is being printed
 *  @return A reference to the same serial device on which we write information.
 *          This is used to string together things to write with "<<" operators
 */
emstream& operator << (emstream& serpt, pid& pid)
{
	// Prints out the current pid configuration
	serpt << "PID Mode: " << dec << pid.get_mode() << endl
	      << "Saturation: " << dec << pid.get_saturator_min() << " to " << dec << pid.get_saturator_max() << endl
	      << "Gains:" << endl
		<< "\t1024*Ki = " << dec << pid.get_Ki() << endl
		<< "\t1024*Kp = " << dec << pid.get_Kp() << endl
		<< "\t1024*Kd = " << dec << pid.get_Kd() << endl
		<< "\t1024*Kw = " << dec << pid.get_Kw() << endl;

	return (serpt);
}