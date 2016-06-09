//===========================================================================================================
/** \file pid.h
 *    This file contains the computation functions neccessary to operate the straight line path and 
 *    circular path routing features.
 *    
 *  Revisions:
 *    \li 06-06-2016 CTR Original File
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
//===========================================================================================================

// This define prevents this .H file from being included multiple times in a .CPP file
#ifndef _ROUTES_H_
#define _ROUTES_H_


#include "emstream.h"                       // Header for serial ports and devices
#include "satmath.h"			    // Header for saturated math library


//-----------------------------------------------------------------------------------------------------------
/** \brief This class runs a 16-bit fixed point PID 
 *  \details The PID controller can run in P, PI, PD, PID, and manual modes. When the integral action is
 *	     used, a fourth gain, Kw can be used to help eliminate windup problems. To help with reset
 *	     spikes, the difference computed for the derivative action is on the input value, not the error.
 *	     The block diagram below shows the entirety of the controller and it's context with respect to
 *	     the plant. The @c input and @c output attributes are from the @e controller's perspective, not
 *	     the plant. That is, @c input is the output of the plant into the controller and @c output is the
 *	     input to the plant from the controller.\n
 * \verbatim
                               PID CONTROLLER w/ ANTI-WINDUP 
            ##################################################################
            #                          +------+                              #
            #                          |      |                              #
            #                    +-----+  Kw  <--------------------+         #
            #                    |     |      |                 +--+--+      #
            #                 (-)|     +------+              (+)|     |(-)   #
            #                 +--v--+  +------+             +--->  Σ  <---+  #
            #              (+)|     |  |      |             |   |     |   |  #
            #              +-->  Σ  +--> Ki/s +-------+     |   +-----+   |  #
            #              |  |     |  |      |       |     |             |  #
            #              |  +-----+  +------+       |(+)  |        MAX  |  #
            #     +-----+  |           +------+    +--v--+  |  +-------+  |  #        +------ +
  SETPOINT  #  (+)|     |  |           |      | (+)|     |  |  |    -- |  |  # OUTPUT |       |
  ---------->----->  Σ  +--+----------->  Kp  +---->  Σ  +--+-->   /   +--+--+--------> PLANT +----+------>
            #     |     |              |      |    |     |     | --    |     #        |       |    |
            #     +--^--+              +------+    +--^--+     +-------+     #        +------ +    |
            #     (-)|                 +------+    (+)|        MIN           #                     |
  INPUT     #        |                 |      |       |                      #                     |
  +--------->------- +-----------------> Kd*s +-------+                      #                     |
  |         #                          |      |                              #                     |
  |         #                          +------+                              #                     |
  |         ##################################################################                     |
  +------------------------------------------------------------------------------------------------+
\endverbatim
 *           To use the pid controller you must first call the constructor with a pointer
 *           to a serial object.\n
 *           @c pid* @c motor_pid @c = @c new @c pid(p_serial); \n
 *           The system parameters must then be set. This can be done using the individual set function for
 *           each parameter, or all at once using a configuration struct. For example, we can set the
 *           controller to PI mode with gains Ki=5, Kp=2, and Kw=1. Remember that the gains are entered as
 *           1024 times the desired gain to increase resolution. Additionally, we will set the output
 *           saturator values to +/- 1600. \n
 *           @c motor_pid->set_config(pid::config_t{PI,5120,2048,0,1024,-1600,1600});\n
 *           Now, to run the PID, we must call compute() inside a loop (probably a task) running at a fixed interval.
 *           Changing the delay of the task will change the effect of the Ki and Kd gains.
 *           Suppose we have three @c int16_t shares @c motor_speed, @c motor_setpoint, and @c motor_power.
 *           We can easily run the PID in one line.\n
 *           @c motor_power->put(motor_pid->compute(motor_speed->get(),motor_setpoint->get()));\n
 */
class pid
{

public:	
  
//-----------------------------------------------------------------------------------------------------------
/** \brief This enumeration defines possible modes of operation 
*/
typedef enum mode {
	OFF	= (0x00<<0),	/*!<  Output is zero*/
	P	= (0x01<<0),	/*!<  Proportional action only*/
	PI	= (0x02<<0),	/*!<  Proportional and integral action*/
	PD	= (0x03<<0),	/*!<  Proportional and derivative action*/
	PID	= (0x04<<0),	/*!<  Full PID action*/
	MANUAL	= (0x05<<0)	/*!<  Output can be updated manually using @c set_output()*/
} mode_t;

//-----------------------------------------------------------------------------------------------------------
/** \brief This struct defines a pid configuration
 */
typedef struct config {
	mode_t		mode;			//!< PID @c mode_t
	int16_t		Kp;			//!< (1024x) Proportional Gain
	int16_t		Ki;			//!< (1024x) Integral Gain
	int16_t		Kd;			//!< (1024x) Derivative Gain
	int16_t		Kw;			//!< (1024x) Anti-windup Gain
	int16_t		min;			//!< Minimum control value for saturator
	int16_t		max;			//!< Maximum control value for saturator
} config_t;	

	
protected:
	emstream*	p_serial;		//!< Serial port pointer, used to say hello
	config_t	config;			//!< @c config_t containing PID parameters 
	int16_t		input;			//!< The value being controlled
	int16_t		output;			//!< The controller output
	int16_t		setpoint;		//!< Desired reference value for input
	int16_t		error;			//!< Error
	int16_t		linput;			//!< Last input to use for derivative computation
	int16_t		dinput;			//!< Difference in input value for derivative gain
	int16_t		esum;			//!< Error Sum
	int16_t		saturation;		//!< Saturator value

public:
	// The constructor sets up the pid for use
	pid (emstream*);

	// Set methods
	void set_mode(mode_t mode);
	void set_input(int16_t input);
	void set_output(int16_t output);
	void set_setpoint(int16_t setpoint);
	void set_config(config_t config);
	void set_Kp(int16_t Kp);
	void set_Ki(int16_t Ki);
	void set_Kd(int16_t Kd);
	void set_Kw(int16_t Kw);
	void set_saturator(int16_t min, int16_t max);

	// Get methods
	mode_t get_mode();
	int16_t get_input();
	int16_t get_output();
	int16_t get_setpoint();
	config_t get_config();
	int16_t get_Kp();
	int16_t get_Ki();
	int16_t get_Kd();
	int16_t get_Kw();
	int16_t get_saturator_min();
	int16_t get_saturator_max();

	// Compute method calculates pid controller output value
	int16_t compute();
	int16_t compute(int16_t new_input, int16_t new_setpoint);

}; // end of class pid


// This operator prints the configuration and of the pid. It's not  a part of class pid, but it operates on
// objects of class pid
     emstream& operator << (emstream&, pid&);

#endif // _PID_H_