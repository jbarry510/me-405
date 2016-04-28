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

#include "encoder_drv.h"                    // Header for this task

#include "textqueue.h"                      // Header for text queue class
#include "taskshare.h"			    // Header for thread-safe shared data
#include "shares.h"                         // Shared inter-task communications

// uint8_t encoder1_chA = 0;		    // Motor 1 encoder channel A
// uint8_t encoder1_chB = 0;		    // Motor 1 encoder channel B

// uint8_t encoder2_chA = 0;		    // Motor 2 encoder channel A
// uint8_t encoder2_chB = 0;		    // Motor 2 encoder channel B

//-------------------------------------------------------------------------------------
/** \brief TODO This ...
 *  \details TODO The ...
 *  @param interrupt_ch Interrupt channel passed in. Channel 4->7 will be used for the motor encoder.
 */

encoder_drv::encoder_drv(emstream* p_serial_port, uint8_t interrupt_ch)
{
      ptr_to_serial = p_serial_port;
      sh_encoder_count_1->put(0);
      sh_encoder_count_2->put(0);
      
      sh_encoder_old_state_1->put(0);
      sh_encoder_new_state_1->put(0);
     
      sh_encoder_old_state_2->put(0);
      sh_encoder_new_state_2->put(0);
      
      
      SREG |= 1<<7;				// Sets 7th bit to 1 to enable global interrupts
      
      // If external interrupt_channel is inbetween 4 and 7, set to "Any logical change on INTn generates
      // an interrupt request."
//       if(interrupt_ch >= 4 && interrupt_ch <= 7)
//       {
	    EICRB &= ~(1<<interrupt_ch);	// Sets the 'interrupt channel passed in' bit to zero
	    EICRB |= 1<<(interrupt_ch - 1);	// Sets the 'interrupt channel' minus one bit to one
	    EIMSK |= (1<<interrupt_ch);
	    
	  // Sets direction of port E bits 4 -> 7 to inputs (Direction control)
	  DDRE &= 0b00001111;
	  PORTE |= 0b11110000;			// Activate pull up resistors for Port E
//       }
}


//-------------------------------------------------------------------------------------
/** \brief TODO This ...
 *  \details TODO The ...
 *  @param INT4_vect Interrupt vector for pin E4 (External interrupt)
 */

ISR (INT6_vect)
{
      sh_encoder_count_2->put((sh_encoder_count_2->get()) + 1);				// Increment encoder count for motor 2
      sh_encoder_old_state_2 -> put(sh_encoder_new_state_2->get());			// Saves old state of motor 2 (channels A and B)
      sh_encoder_new_state_2 -> put((PINE & _BV(PINE6)) | (PINE & _BV(PINE7)));		// Stores new state of motor 1 (channels A and B)
      sh_encoder_new_state_2 -> put((sh_encoder_new_state_2->get()) >> 6); 		// Shifts state bits to two right most bits of variable

// TODO Add in switch and if statements to increment or decrement encoder count, error else
// TODO Add in task_encoder the old and new encoder_count to determine CW/CCW and value
}

ISR (INT7_vect)
{
      sh_encoder_count_2->put((sh_encoder_count_2->get()) + 1);				// Increment encoder count for motor 2
      sh_encoder_old_state_2 -> put(sh_encoder_new_state_2->get());			// Saves old state of motor 2 (channels A and B)
      sh_encoder_new_state_2 -> put((PINE & _BV(PINE6)) | (PINE & _BV(PINE7)));		// Stores new state of motor 1 (channels A and B)
      sh_encoder_new_state_2 -> put((sh_encoder_new_state_2->get()) >> 6); 		// Shifts state bits to two right most bits of variable
}

// Aliases the other pin E interrupts to run the pin E4 interrupt service routine
// ISR_ALIAS(INT5_vect, INT4_vect);
// ISR_ALIAS(INT6_vect, INT4_vect);
// ISR_ALIAS(INT7_vect, INT4_vect);