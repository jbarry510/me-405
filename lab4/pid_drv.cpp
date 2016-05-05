//***********************************************************************************************************
/** @file pid_drv.cpp
 *    WRITE STUFF HERE
 * 
 *
 */
//***********************************************************************************************************
#include <stdlib.h>                         // Include standard library header files
#include <avr/io.h>                         // Port I/O for SFR's
#include <avr/interrupt.h>
#include "rs232int.h"                       // Include header for serial port class

#include "textqueue.h"                      // Header for text queue class
#include "taskshare.h"			    // Header for thread-safe shared data
#include "shares.h"                         // Shared inter-task communications

#include "pid_drv.h"                    // Header for this task




//-----------------------------------------------------------------------------------------------------------
/** \brief WRITE STUFF HERE
 *  \details WRITE STUFF HERE
 * 
 */

// add more parameters for constructor 
pid_drv::pid_drv(emstream* p_serial_port)
{
      ptr_to_serial = p_serial_port;
   
      // SET UP PID
}
