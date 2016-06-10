//===========================================================================================================
/** \file routes.h
 *    This file contains a library of functions required to operate the route features.
 *
 *  Revisions:
 *    \li 06-09-2016 CTR Original File
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
#ifndef ROUTES_H
#define ROUTES_H


//-----------------------------------------------------------------------------------------------------------
/** \brief This namespace includes several functions 
 */
namespace routes 
{
	int16_t                        servo_angle(uint16_t power);
	uint16_t                       servo_power(int16_t angle);
	uint16_t 		       motor_setpoint(uint16_t velocity);
	uint16_t		       motor_velocity(uint16_t setpoint);
} // end namespace routes

#endif // ROUTES_H