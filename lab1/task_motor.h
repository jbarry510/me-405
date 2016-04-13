//======================================================================================
/** @file task_motor.h
 *    TODO: This file contains a 
 *
 *  License:
 *    This file is copyright 2012 by JR Ridgely and released under the Lesser GNU 
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
//======================================================================================

// This define prevents this .H file from being included multiple times in a .CPP file
#ifndef _Task_MOTOR_H_
#define _Task_MOTOR_H_

#include <stdlib.h>                         // Prototype declarations for I/O functions
#include <avr/io.h>                         // Header for special function registers

#include "emstream.h"                       // Header for serial ports and devices
#include "FreeRTOS.h"                       // Header for the FreeRTOS RTOS
#include "taskbase.h"                       // ME405/507 base task class
#include "task.h"                           // Header for FreeRTOS task functions
#include "queue.h"                          // Header for FreeRTOS queues
#include "semphr.h"                         // Header for FreeRTOS semaphores
#include "motor_drv.h"                      // Include header for the motor class

class task_motor : public TaskBase
{
public:
	// This constructor creates a generic task of which many copies can be made
	task_motor (const char*, unsigned portBASE_TYPE, size_t, emstream*);
 
	// This method is called by the RTOS once to run the task loop for ever and ever.
	void run (void);
};

#endif // _Task_MOTOR_H__


//-------------------------------------------------------------------------------------
/** @brief   TODO: This class ...
 *  @details TODO: This class contains a 
 * 
 * 
 * 
 * 
 * stuff?
 */