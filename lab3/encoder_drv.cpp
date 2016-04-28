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

// Interrupt 7
      EICRB &= ~(1<<interrupt_ch);		// Sets the 'interrupt channel passed in' bit to zero
      EICRB |= 1<<(interrupt_ch - 1);		// Sets the 'interrupt channel' minus one bit to one
// Interrupt 6   
      EICRB &= ~(1<<(interrupt_ch - 2));	// Sets the 'interrupt channel passed in' bit to zero
      EICRB |= 1<<(interrupt_ch - 3);		// Sets the 'interrupt channel' minus one bit to one
// Interrupt 7 & 6
      EIMSK |= (1<<interrupt_ch);
      EIMSK |= 1<<(interrupt_ch - 1);
	    
// Sets direction of port E bits 4 -> 7 to inputs (Direction control)
      DDRE &= 0b00001111;
      PORTE |= 0b11110000;			// Activate pull up resistors for Port E
}


//-------------------------------------------------------------------------------------
/** \brief TODO This ...
 *  \details TODO The ...
 *  @param INT4_vect Interrupt vector for pin E4 (External interrupt)
 */

ISR (INT4_vect)
{
      sh_encoder_old_state_1 -> put(sh_encoder_new_state_1->get());			// Saves old state of motor 1 (channels A and B)
      sh_encoder_new_state_1 -> put(((PINE & _BV(PINE4)) | (PINE & _BV(PINE5))) >> 4);	// Stores new state of motor 1 (channels A and B)
   
// Compare motor 1 encoder state and determine direction, Yellow = A, 5, White = B, 4
// CW  direction (A:B) = -> 0b00 -> 0b10 -> 0b11 -> 0b01 ->
// CCW direction (A:B) = -> 0b01 -> 0b11 -> 0b10 -> 0b00 ->
      switch(sh_encoder_old_state_1->ISR_get())
      {
	case(0b00):
	  if(sh_encoder_new_state_1->ISR_get() == 0b10)
	      sh_encoder_count_1->ISR_put(sh_encoder_count_1->ISR_get() + 1);			// If next state increment
	  else if(sh_encoder_new_state_1->ISR_get() == 0b01)
	      sh_encoder_count_1->ISR_put(sh_encoder_count_1->get() - 1);			// If previous state decrement
	  else
	      sh_encoder_error_count_1->ISR_put(sh_encoder_error_count_1->ISR_get() + 1);	// If neither, increment error count
	  break;
	  
	case(0b10):
	  if(sh_encoder_new_state_1->ISR_get() == 0b11)
	      sh_encoder_count_1->ISR_put(sh_encoder_count_1->ISR_get() + 1);			// If next state increment
	  else if(sh_encoder_new_state_1->ISR_get() == 0b00)
	      sh_encoder_count_1->ISR_put(sh_encoder_count_1->get() - 1);			// If previous state decrement
	  else
	      sh_encoder_error_count_1->ISR_put(sh_encoder_error_count_1->ISR_get() + 1);	// If neither, increment error count
	  break;

	case(0b11):
	  if(sh_encoder_new_state_1->ISR_get() == 0b01)
	      sh_encoder_count_1->ISR_put(sh_encoder_count_1->ISR_get() + 1);			// If next state increment
	  else if(sh_encoder_new_state_1->ISR_get() == 0b10)
	      sh_encoder_count_1->ISR_put(sh_encoder_count_1->ISR_get() - 1);			// If previous state decrement
	  else
	      sh_encoder_error_count_1->ISR_put(sh_encoder_error_count_1->ISR_get() + 1);	// If neither, increment error count
	  break;
	  
	case(0b01):
	  if(sh_encoder_new_state_1->ISR_get() == 0b00)
	      sh_encoder_count_1->ISR_put(sh_encoder_count_1->ISR_get() + 1);			// If next state increment
	  else if(sh_encoder_new_state_1->ISR_get() == 0b11)
	      sh_encoder_count_1->ISR_put(sh_encoder_count_1->ISR_get() - 1);			// If previous state decrement
	  else
	      sh_encoder_error_count_1->ISR_put(sh_encoder_error_count_1->ISR_get() + 1);	// If neither, increment error count
	  break;
      }
}

// Aliases the pin E5 interrupt to run the pin E4 interrupt service routine
ISR_ALIAS(INT5_vect, INT4_vect);

//-------------------------------------------------------------------------------------
/** \brief TODO This ...
 *  \details TODO The ...
 *  @param INT6_vect Interrupt vector for pin E6 (External interrupt)
 */

ISR (INT6_vect)
{
      sh_encoder_old_state_2->ISR_put(sh_encoder_new_state_2->ISR_get());			// Saves old state of motor 2 (channels A and B)
      sh_encoder_new_state_2->ISR_put(((PINE & _BV(PINE6)) | (PINE & _BV(PINE7))) >> 6);	// Stores new state of motor 2 (channels A and B)

// Compare motor 2 encoder state and determine direction, Yellow = A, 7, White = B, 6
// CW  direction (A:B) = -> 0b00 -> 0b10 -> 0b11 -> 0b01 ->
// CCW direction (A:B) = -> 0b01 -> 0b11 -> 0b10 -> 0b00 ->
      switch(sh_encoder_old_state_2->ISR_get())
      {
	case(0b00):
	  if(sh_encoder_new_state_2->ISR_get() == 0b10)
	      sh_encoder_count_2->ISR_put(sh_encoder_count_2->ISR_get() + 1);			// If next state increment
	  else if(sh_encoder_new_state_2->ISR_get() == 0b01)
	      sh_encoder_count_2->ISR_put(sh_encoder_count_2->ISR_get() - 1);			// If previous state decrement
	  else
	      sh_encoder_error_count_2->ISR_put(sh_encoder_error_count_2->ISR_get() + 1);	// If neither, increment error count
	  break;
	  
	case(0b10):
	  if(sh_encoder_new_state_2->ISR_get() == 0b11)
	      sh_encoder_count_2->ISR_put(sh_encoder_count_2->ISR_get() + 1);			// If next state increment
	  else if(sh_encoder_new_state_2->ISR_get() == 0b00)
	      sh_encoder_count_2->ISR_put(sh_encoder_count_2->ISR_get() - 1);			// If previous state decrement
	  else
	      sh_encoder_error_count_2->ISR_put(sh_encoder_error_count_2->ISR_get() + 1);	// If neither, increment error count
	  break;

	case(0b11):
	  if(sh_encoder_new_state_2->ISR_get() == 0b01)
	      sh_encoder_count_2->ISR_put(sh_encoder_count_2->ISR_get() + 1);			// If next state increment
	  else if(sh_encoder_new_state_1->ISR_get() == 0b10)
	      sh_encoder_count_2->ISR_put(sh_encoder_count_2->ISR_get() - 1);			// If previous state decrement
	  else
	      sh_encoder_error_count_2->ISR_put(sh_encoder_error_count_2->ISR_get() + 1);	// If neither, increment error count
	  break;
	  
	case(0b01):
	  if(sh_encoder_new_state_2->ISR_get() == 0b00)
	      sh_encoder_count_2->ISR_put(sh_encoder_count_2->ISR_get() + 1);			// If next state increment
	  else if(sh_encoder_new_state_1->ISR_get() == 0b11)
	      sh_encoder_count_2->ISR_put(sh_encoder_count_2->ISR_get() - 1);			// If previous state decrement
	  else
	      sh_encoder_error_count_2->ISR_put(sh_encoder_error_count_2->ISR_get() + 1);	// If neither, increment error count
	  break;
      }
}

// Aliases the pin E7 interrupt to run the pin E6 interrupt service routine
ISR_ALIAS(INT7_vect, INT6_vect);