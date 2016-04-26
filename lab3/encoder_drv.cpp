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
	    //DBG(ptr_to_serial, "Constructed " << endl);
      }
      
      // If external interrupt_channel is inbetween 0 and 3, set to "Any edge of INTn generates asynchronously
      // an interrupt request."
      else if(interrupt_ch >= 0 && interrupt_ch <= 3)
      {
	    EICRA &= ~(1<<interrupt_ch);	// Sets the 'interrupt channel passed in' bit to zero
	    EICRA |= 1<<(interrupt_ch - 1);	// Sets the 'interrupt channel' minus one bit to one
	    EIMSK |= (1<<interrupt_ch);
      }
      else
      {
      
	    DBG(ptr_to_serial, "encoder_drv failed" << endl);
	
      }

}

/** \brief TODO This ...
 *  \details TODO The ...
 *  @param param description
 */

// TODO: should encoder_count be specified for each motor?
ISR (INT5_vect)
{
      sh_encoder_count_1->put(sh_encoder_count_1->get() + 1);		// Increment encoder count
      //sh_encoder_count_1->put(1);
      
      //DBG(ptr_to_serial, "Entered Interrupt!" << endl); // Cant use DBG or ptr_to_serial 
   
}
/*
ISR (INT5_vect)
{
      sh_encoder_count_1->put(sh_encoder_count_1->get() + 1);		// Increment encoder count

}

ISR (INT6_vect)
{
      sh_encoder_count_1->put(sh_encoder_count_1->get() + 1);		// Increment encoder count

}

ISR (INT7_vect)
{
      sh_encoder_count_1->put(sh_encoder_count_1->get() + 1);		// Increment encoder count
  
}
*/