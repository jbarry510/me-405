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

uint16_t routes::servo_angle(uint16_t old_pos, uint16_t first_pos, uint16_t current_pos)
{
     uint16_t coefficient = -34;
     uint16_t constant = 3034;
     return old_pos + (constant + ((coefficient)*(first_pos - current_pos)));
}
