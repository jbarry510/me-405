//*************************************************************************************
/** @file encoder_drv.cpp
 *    TODO This file contains the...
 * 
 *  Revisions:
 *    @li 04-13-2016 ME405 Group 3 original file
 *
 */
//*************************************************************************************

#include <stdlib.h>                         // Include standard library header files
#include <avr/io.h>
#include <avr/interrupt.h>

#include "rs232int.h"                       // Include header for serial port class

#include "encoder_drv.h"                     // Header for this task

#include "textqueue.h"                      // Header for text queue class
#include "taskshare.h"			    // Header for thread-safe shared data
#include "shares.h"                         // Shared inter-task communications



//-------------------------------------------------------------------------------------
/** \brief TODO This ...
 *  \details TODO The ...
 *  @param interrupt_ch Interrupt channel passed in. Channel 4->7 will be used for the motor encoder.
 */

encoder_drv::encoder_drv(emstream* p_serial_port, uint8_t interrupt_ch)
{
      ptr_to_serial = p_serial_port;
      sh_encoder_count_1->put(0);
      SREG |= 1<<7;				// Sets 7th bit to 1 to enable global interrupts
      
      // If external interrupt_channel is inbetween 4 and 7, set to "Any logical change on INTn generates
      // an interrupt request."
      if(interrupt_ch >= 4 && interrupt_ch <= 7)
      {
	    EICRB &= ~(1<<interrupt_ch);	// Sets the 'interrupt channel passed in' bit to zero
	    EICRB |= 1<<(interrupt_ch - 1);	// Sets the 'interrupt channel' minus one bit to one
	    EIMSK |= (1<<interrupt_ch);
      }
}

//-------------------------------------------------------------------------------------
/** \brief TODO This ...
 *  \details TODO The ...
 *  @param INT4_vect Interrupt vector for pin E4 (External interrupt)
 */

ISR (INT4_vect)
{
    if(EIFR[INTF4] | EIFR[INTF5])				// Checks if pin E interrupt 4 or 5 was enabled
    {  
      sh_encoder_count_1->put(sh_encoder_count_1->get() + 1);	// Increment encoder count for motor 1
      sh_encoder_old_state_1 -> put(sh_encoder_new_state_1);	// Saves old state of motor 1 (channels A and B)
      sh_encoder_new_state_1 -> put((PINE[6] << 1) | PINE[7]);	// Stores new state of motor 1 (channels A and B)
    }
    else if (EIFR[INTF6] | EIFR[INTF7])				// Checks if pin E interrupt 6 or 7 was enabled
    {
      sh_encoder_count_2->put(sh_encoder_count_2->get() + 1);	// Increment encoder count for motor 2
      sh_encoder_old_state_2 -> put(sh_encoder_new_state_1);	// Saves old state of motor 2 (channels A and B)
      sh_encoder_new_state_2 -> put(PINE[4] >> 1 | PINE[5]);	// Stores new state of motor 2 (channels A and B)
    }
    else {}
}

// Aliases the other pin E interrupts to run the pin E4 interrupt service routine
ISR_ALIAS(INT5_vect, INT4_vect);
ISR_ALIAS(INT6_vect, INT4_vect);
ISR_ALIAS(INT7_vect, INT4_vect);