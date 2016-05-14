//***********************************************************************************************************
/** @file encoder_drv.cpp
 *    This file contains interrupt service routines appropriate to a specific motor. Also shares pertinent
 *    encoder data with task_encoder.cpp, including direction, encoder states, count, and error count.
 * 
 *  Revisions:
 *    @li April 13, 2016 -- BKK ME405 Group 3 original file
 *    @li April 28, 2016 -- BKK Added interrupt enable for ch 6 & 7, interrupts for ch 4 -> 7
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

#include "encoder_drv.h"                    // Header for this task

//-----------------------------------------------------------------------------------------------------------
/** \brief This constructor enables global external interrupts on channels E4->E7 and masks them 
 * 	   appropriately to trigger an interrupt for any logical change.
 *  \details Two interrupt service routines provide independent encoder data for motor 1 and motor 2. This
 *	     data includes direction, state, encoder count, and error count.
 *  @param interrupt_ch Interrupt channel passed in. Channel 4->7 will be used for the motor encoder.
 */

encoder_drv::encoder_drv(emstream* p_serial_port, uint8_t interrupt_ch)
{
      ptr_to_serial = p_serial_port;
      sh_encoder_count_1->put(0);		// Clears motor 1 encoder count
      sh_encoder_count_2->put(0);		// Clears motor 2 encoder count
      
      sh_encoder_old_state_1->put(0);		// Clears motor 1 encoder old state
      sh_encoder_new_state_1->put(0);		// Clears motor 1 encoder new state
     
      sh_encoder_old_state_2->put(0);		// Clears motor 2 encoder old state
      sh_encoder_new_state_2->put(0);		// Clears motor 2 encoder old state
      
      SREG |= 1<<7;				// Sets 7th bit to 1 to enable global interrupts
      
// For external interrupt channels 4->7, trigger for "Any logical change on INTn generates
// an interrupt request."

// External Interrupt 7
      EICRB &= ~(1<<interrupt_ch);		// Sets the 'interrupt channel passed in' bit to zero
      EICRB |= 1<<(interrupt_ch - 1);		// Sets the 'interrupt channel' minus one bit to one
      EIMSK |= (1<<interrupt_ch);		// Set External Interrupt Mask Register for passed in channel
// External Interrupt 6
      EICRB &= ~(1<<(interrupt_ch - 2));	// Sets the 'interrupt channel passed in' bit to zero
      EICRB |= 1<<(interrupt_ch - 3);		// Sets the 'interrupt channel' minus one bit to one
      EIMSK |= 1<<(interrupt_ch - 1);		// Set External Interrupt Mask Register for passed in channel
// Pin Change Interrupt 8
      PCICR |= 1<< (1);
      PCMSK1 |= 1 << (0);
// Pin Change Interrupt 7
      PCICR |= 1 << (0);
      PCMSK0 |= 1 << (7);
// Sets direction of port E bits 6,7,0 to inputs (Direction control)
      DDRE &= 0b00111110;			// Sets pins 6,7,0 to outputs and the others to inputs
      PORTE |= 0b11000001;			// Activate appropriate pull up resistors for Port E
// Sets direction of port B7 to inputs (Direction control)
      DDRB &= ~(1<<DDB7);			// Sets pin 7 to output and rest to inputs
      PORTB |= 1 << PORTB7;			// Activates appropriate pull up resistor
}


//-----------------------------------------------------------------------------------------------------------
/** \brief This interrupt service routine determines the new encoder state, direction, increments or
 *	   decrements encoder count, and flags encoder sequence errors with the error count.
 *  \details Specifically, compare the new state to the previous state and based on the physical quadrature
 *	     of the encoder disk, determines relevant encoder direction and recognizes errors: tick skipping.
 *	     This ISR will be used for channel 4 & 5.
 *  @param INT4_vect Interrupt vector for pin E4 (External interrupt)
 */

ISR (INT4_vect)
{
      // Saves old state of motor 1 (channels A and B)
      sh_encoder_old_state_1 -> put(sh_encoder_new_state_1->get());
      
      // Stores new state of motor 1 (channels A and B)
      sh_encoder_new_state_1 -> put(((PINE & _BV(PINE4)) | (PINE & _BV(PINE5))) >> 4);	
   
// Compare motor 1 encoder state and determine direction, Yellow = A, 5, White = B, 4
// CW  direction (A:B) = -> 0b00 -> 0b10 -> 0b11 -> 0b01 ->
// CCW direction (A:B) = -> 0b01 -> 0b11 -> 0b10 -> 0b00 ->
      switch(sh_encoder_old_state_1->ISR_get())
      {
	case(0b00):
	  if(sh_encoder_new_state_1->ISR_get() == 0b10)		// If next state increment
	      sh_encoder_count_1->ISR_put(sh_encoder_count_1->ISR_get() + 1);
	  else if(sh_encoder_new_state_1->ISR_get() == 0b01)	// If previous state decrement
	      sh_encoder_count_1->ISR_put(sh_encoder_count_1->get() - 1);
	  else							// If neither, increment error count
	      sh_encoder_error_count_1->ISR_put(sh_encoder_error_count_1->ISR_get() + 1);
	  break;
	  
	case(0b10):
	  if(sh_encoder_new_state_1->ISR_get() == 0b11)		// If next state increment
	      sh_encoder_count_1->ISR_put(sh_encoder_count_1->ISR_get() + 1);
	  else if(sh_encoder_new_state_1->ISR_get() == 0b00)	// If previous state decrement
	      sh_encoder_count_1->ISR_put(sh_encoder_count_1->get() - 1);
	  else							// If neither, increment error count
	      sh_encoder_error_count_1->ISR_put(sh_encoder_error_count_1->ISR_get() + 1);
	  break;

	case(0b11):
	  if(sh_encoder_new_state_1->ISR_get() == 0b01)		// If next state increment
	      sh_encoder_count_1->ISR_put(sh_encoder_count_1->ISR_get() + 1);
	  else if(sh_encoder_new_state_1->ISR_get() == 0b10)	// If previous state decrement
	      sh_encoder_count_1->ISR_put(sh_encoder_count_1->ISR_get() - 1);
	  else							// If neither, increment error count
	      sh_encoder_error_count_1->ISR_put(sh_encoder_error_count_1->ISR_get() + 1);
	  break;
	  
	case(0b01):
	  if(sh_encoder_new_state_1->ISR_get() == 0b00)		// If next state increment
	      sh_encoder_count_1->ISR_put(sh_encoder_count_1->ISR_get() + 1);	
	  else if(sh_encoder_new_state_1->ISR_get() == 0b11)	// If previous state decrement
	      sh_encoder_count_1->ISR_put(sh_encoder_count_1->ISR_get() - 1);	
	  else							// If neither, increment error count
	      sh_encoder_error_count_1->ISR_put(sh_encoder_error_count_1->ISR_get() + 1);
	  break;
      }
}

// Aliases the pin E5 interrupt to run the pin E4 interrupt service routine
ISR_ALIAS(INT5_vect, INT4_vect);

//-----------------------------------------------------------------------------------------------------------
/** \brief This interrupt service routine determines the new encoder state, direction, increments or
 *	   decrements encoder count, and flags encoder sequence errors with the error count.
 *  \details Specifically, compare the new state to the previous state and based on the physical quadrature
 *	     of the encoder disk, determines relevant encoder direction and recognizes errors: tick skipping.
 *	     This ISR will be used for channel 6 & 7.
 *  @param INT6_vect Interrupt vector for pin E6 (External interrupt)
 */

ISR (INT6_vect)
{
      // Saves old state of motor 2 (channels A and B)
      sh_encoder_old_state_2->ISR_put(sh_encoder_new_state_2->ISR_get());
      
      // Stores new state of motor 2 (channels A and B)
      sh_encoder_new_state_2->ISR_put(((PINE & _BV(PINE6)) | (PINE & _BV(PINE7))) >> 6);
      
// Compare motor 2 encoder state and determine direction, Yellow = A, 7, White = B, 6
// CW  direction (A:B) = -> 0b00 -> 0b10 -> 0b11 -> 0b01 ->
// CCW direction (A:B) = -> 0b01 -> 0b11 -> 0b10 -> 0b00 ->
      switch(sh_encoder_old_state_2->ISR_get())
      {
	case(0b00):
	  if(sh_encoder_new_state_2->ISR_get() == 0b10)		// If next state increment
	      sh_encoder_count_2->ISR_put(sh_encoder_count_2->ISR_get() + 1);
	  else if(sh_encoder_new_state_2->ISR_get() == 0b01)	// If previous state decrement
	      sh_encoder_count_2->ISR_put(sh_encoder_count_2->ISR_get() - 1);	
	  else							// If neither, increment error count
	      sh_encoder_error_count_2->ISR_put(sh_encoder_error_count_2->ISR_get() + 1);	
	  break;
	  
	case(0b10):
	  if(sh_encoder_new_state_2->ISR_get() == 0b11)		// If next state increment
	      sh_encoder_count_2->ISR_put(sh_encoder_count_2->ISR_get() + 1);
	  else if(sh_encoder_new_state_2->ISR_get() == 0b00)	// If previous state decrement
	      sh_encoder_count_2->ISR_put(sh_encoder_count_2->ISR_get() - 1);	
	  else							// If neither, increment error count
	      sh_encoder_error_count_2->ISR_put(sh_encoder_error_count_2->ISR_get() + 1);
	  break;

	case(0b11):
	  if(sh_encoder_new_state_2->ISR_get() == 0b01)		// If next state increment
	      sh_encoder_count_2->ISR_put(sh_encoder_count_2->ISR_get() + 1);
	  else if(sh_encoder_new_state_2->ISR_get() == 0b10)	// If previous state decrement
	      sh_encoder_count_2->ISR_put(sh_encoder_count_2->ISR_get() - 1);
	  else							// If neither, increment error count
	      sh_encoder_error_count_2->ISR_put(sh_encoder_error_count_2->ISR_get() + 1);
	  break;
	  
	case(0b01):
	  if(sh_encoder_new_state_2->ISR_get() == 0b00)		// If next state increment
	      sh_encoder_count_2->ISR_put(sh_encoder_count_2->ISR_get() + 1);			
	  else if(sh_encoder_new_state_2->ISR_get() == 0b11)	// If previous state decrement
	      sh_encoder_count_2->ISR_put(sh_encoder_count_2->ISR_get() - 1);			
	  else							// If neither, increment error count
	      sh_encoder_error_count_2->ISR_put(sh_encoder_error_count_2->ISR_get() + 1);	
      }
}
// Aliases the pin E7 interrupt to run the pin E6 interrupt service routine
ISR_ALIAS(INT7_vect, INT6_vect);