//***********************************************************************************************************
/** @file task_user.cpp
 *    This file contains source code for a user interface task for a ME405/FreeRTOS test suite. 
 *
 *  Revisions:
 *    @li 09-30-2012 JRR Original file was a one-file demonstration with two tasks
 *    @li 10-05-2012 JRR Split into multiple files, one for each task
 *    @li 10-25-2012 JRR Changed to a more fully C++ version with class task_user
 *    @li 11-04-2012 JRR Modified from the data acquisition example to the test suite
 *    @li 01-04-2014 JRR Changed base class names to TaskBase, TaskShare, etc.
 *    @li April 29, 2016 -- BKK Cleaned up comments, added return command to Main Menu
 *    @li May 8, 2016 -- BKK Added PID control flag and user option
 *
 *  License:
 *	This file is copyright 2015 by JR Ridgely and released under the Lesser GNU Public License, 
 *	version 2. It intended for educational use only, but its use is not limited thereto.
 *	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
 *	IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 *	FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 *	CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *	DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *	DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 *	IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 *	THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */
//***********************************************************************************************************
#include <avr/io.h>			// Port I/O for SFR's
#include <avr/wdt.h>			// Watchdog timer header

#include "textqueue.h"			// Header for text queue class
#include "taskshare.h"			// Header for thread-safe shared data
#include "shares.h"			// Shared inter-task communications

#include "task_user.h"			// Header for this file

// Precompiler Directives:
// Renaming ASCII representations of keyboard characters to intelligent names
#define ASCII_BACKSPACE 127		// Backspace (actually DEL)
#define ASCII_RETURN 13			// Return
#define ASCII_SPACE 32			// Space
#define ASCII_DASH 45			// Dash (negative)
#define ASCII_NEWLINE 10		// Carriage Return
#define ASCII_ESCAPE 27			// Escape

// This constant sets how many RTOS ticks the task delays if the user's not talking. 
// The duration is calculated to be about 5 ms.
const TickType_t ticks_to_delay = ((configTICK_RATE_HZ / 1000) * 5);


//-----------------------------------------------------------------------------------------------------------
/** This constructor creates a new data acquisition task. It's main job is to call the parent class's
 *  constructor which does most of the work.
 *  @param a_name A character string which will be the name of this task
 *  @param a_priority The priority at which this task will initially run (default: 0)
 *  @param a_stack_size The size of this task's stack in bytes (default: configMINIMAL_STACK_SIZE)
 *  @param p_ser_dev Pointer to a serial device (port, radio, SD card, etc.) which can be used by this task
 *		     to communicate (default: NULL)
 */

task_user::task_user (const char* a_name, unsigned portBASE_TYPE a_priority, size_t a_stack_size, 
				  emstream* p_ser_dev):TaskBase (a_name, a_priority, a_stack_size, p_ser_dev)
{
	// Nothing is done in the body of this constructor. All the work is done in the
	// call to the frt_task constructor on the line just above this one
}


//-----------------------------------------------------------------------------------------------------------
/** This task interacts with the user so that a motor may be selected, the power for that motor may be set,
 *  the intensity of braking may also be set, along with a full braking option. It includes two help
 *  messages: Initial menu to choose global options, a motor menu to select power or braking intensity.
 * TODO: Add a increment and decrement for power increase
 * 	 Add a kill button to turn everything off if necessary
 */

void task_user::run (void)
{
     char char_in;                          	// Character read from serial device
     time_stamp a_time;                      	// Holds the time so it can be displayed

     int16_t number_entered = 0;             	// Holds a number being entered by user
     uint8_t number_state = 0;			// State of of what type of number is inputted
     bool negative_number_entered = false;	// Flag to indicate a negative number has been inputted
     uint8_t char_num_count = 0;		// Counts characters so backspace will delete accordingly 

     sh_motor_select -> put(0);			// Motor select share

     // Command mode (state 1), where the user interface task can jump to successive states
     print_main_menu();

     // This is an infinite loop; it runs until the power is turned off; one for each task.
     for (;;)
     {
	  // Run the finite state machine. The variable 'state' is kept by parent class
	  switch (state)
	  {
	       // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	       // In state 0 the user types characters, which are interpreted as commands.
	       case (0):
		    if (p_serial->check_for_char ())		// Wait for character and read
		    {
			 char_in = p_serial -> getchar ();

			 // Switch statement that responds user command characters
			 switch (char_in)
			 {
			      // The 't' command: returns current runtime
			      case ('t'):
				   *p_serial << (a_time.set_to_now ()) << endl;
				   break;

			      // The 's' command: returns version and status information
			      case ('s'):
				   show_status();
				   break;

			      // The 'd' command: all the tasks dump their stacks
			      case ('d'):
				   print_task_stacks (p_serial);
				   break;
				   
			      // The 'a' command: enables PID control
			      case ('a'):
				   if(sh_PID_control->get() == 0)
				   {
					sh_PID_control->put(1);		// Set PID flag high
					*p_serial << PMS ("Autonomous Mode: ACTIVE") << endl;
					print_PID_menu();
					transition_to (3);

				   }
				   else if(sh_PID_control->get() == 1)
				   {
					sh_PID_control->put(0);
					*p_serial << PMS ("Autonomous Mode: HALTED") << endl;
					print_main_menu();
					transition_to (0);
				   }
				   break;
				   
			      // The 'm' command: allows for motor selection to be made
			      case ('m'):
				   *p_serial << PMS ("Enter number, 1 or 2, for motor selection: ") << endl;
				   number_entered = 0;		// Clears number_entered
				   sh_motor_select -> put(0);	// Clears shared sh_motor_select
				   number_state = 0;		// Clears number state
				   transition_to (1);
				   break;

			      // A Ctrl-C character causes the CPU to restart
			      case (3):
				   *p_serial << PMS ("Resetting AVR") << endl;
				   wdt_enable (WDTO_120MS);
				   for (;;);
				   break;

			      // The 'r' command: return to Main Menu
			      case ('r'):
				   print_main_menu();
				   break;

			      // Unrecognized character: return typed character and prompt user for a retype
			      default:
				   *p_serial << '"' << char_in << PMS ("\" ") << UNKNOWN_CHAR << endl;
				   break;
			 }; // End switch for characters
		    } // End if a character was received

	       break; // End of state 0

	       // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	       // In state 1, wait for user to enter digits and build into a number
	       case (1):
		    if (p_serial->check_for_char ())		// Wait for character and read
		    {
			 char_in = p_serial -> getchar ();

			 // Respond only to: numeric characters, Enter, or Esc.
			 if (char_in >= '0' && char_in <= '9')
			 { 
			      *p_serial << char_in;
			      number_entered *= 10;
			      number_entered += char_in - '0';
			      char_num_count++;
			 }

			 // Dash, indicating a negative number 
			 else if (char_in == ASCII_DASH)
			 {
			      negative_number_entered = true;
			      *p_serial << char_in;
			 }

			 // Backspace
			 else if (char_in == ASCII_BACKSPACE && char_num_count > 0)
			 {
			      *p_serial << char_in;		// Print Backspace
			      *p_serial << (char)ASCII_SPACE;	// Print Space
			      *p_serial << char_in;		// Print Backspace

			      number_entered /= 10;		// Divide by 10 to remove ones place
			      char_num_count--;			// Decrement until equal to zero
			 }

			 // Line Feed, Carriage return is ignored; the newline character ends the entry
			 else if (char_in == ASCII_NEWLINE)
			      *p_serial << "\r";

			 // Carriage return ends numeric entry
			 else if (char_in == ASCII_RETURN)
			 {
			      if(negative_number_entered == true)		// Print a dash if a negative
				   *p_serial << endl << PMS ("Number Entered: -") << number_entered << endl;
			      else if(negative_number_entered == false)		// Print number entered
				   *p_serial << endl << PMS ("Number Entered: ") << number_entered << endl;

	
			      // Motor Selection State
			      if (number_state == 0)
			      {
				   if (number_entered == 1 || number_entered == 2)
				   {	
					sh_motor_select -> put(number_entered);	// Set to motor selected
					print_motor_menu ();		// Print motor menu
					number_entered = 0;		// Clear number_entered
					transition_to (2);		// Transition to motor control case
				   }
				   else
				   {
					*p_serial << PMS ("Please type a 1 or 2 for motor select, then ENTER") << endl;
					number_entered = 0;		// Clear number_entered  
				   }
			      }

			      // Power Setting State
			      else if (number_state == 1)
			      {
				   if (number_entered >= 0 && number_entered <= 255)
				   {
					if(negative_number_entered == true)	// Check if a negative number was entered
					{
					     number_entered *= -1;		// Change sign of number
					     negative_number_entered = false;	// Clear negative number flag
					}

					sh_power_entry -> put(number_entered);	// Set share to number_entered
					sh_power_set_flag-> put(1);		// Set flag high to indicate a changed power value

					number_entered = 0;			// Clear number_entered
					number_state = 0;			// Clear number_state
					print_main_menu();
					transition_to (0);			// Transition to main case
				   }
    
				   else
				   {
					*p_serial << PMS ("Please type a number between -255 to 255, then ENTER") << endl;	// Display error message for out of range
					number_entered = 0;				// Clear number entered
				   }
			      }	

			      // Braking State
			      else if (number_state == 2)
			      {
				   if (number_entered >= 0 && number_entered <= 255)
				   {
					sh_braking_entry -> put(number_entered);	// Move number_entered into shared sh_braking_entry
					sh_braking_set_flag-> put(1);		// Make braking_set_flag high to indicate a changed braking value

					number_entered = 0;			// Clear number_entered
					number_state = 0;			// Clear number_state
					print_main_menu();
					transition_to (0);			// Transition to main case
				   }
    
				   else
				   {
					*p_serial << PMS ("Please type a number between 0 to 255, then ENTER") << endl;		// Display error message for out of range
					number_entered = 0;			// Clear number entered
				   }	
			      }
			      
			      // Motor 1 PID setpoint input
			      // TODO Add limits to setpoints
			      else if (number_state == 3)
			      {
				   sh_setpoint_1->put(number_entered);
				   *p_serial << PMS ("Motor 1 setpoint: ") << number_entered << endl;
				   number_entered = 0;			// Clear number entered
				   number_state = 4;			// Transition to input Motor 2 setpoint
				   *p_serial << PMS ("Input setpoint for Motor 2: ") << endl;
				   transition_to (1);

			      }
			      
			      // Motor 2 PID setpoint input
			      // TODO Add limits to setpoints
			      else if (number_state == 4)
			      {
				   sh_setpoint_2->put(number_entered);
				   *p_serial << PMS ("Motor 2 setpoint: ") << number_entered << endl;
				   number_state = 0; 			// Clear number_state
				   print_main_menu();
				   transition_to (0);
			      }
			 }
				
			 else
			 {
			      *p_serial << '"' << char_in << PMS ("\" ") << UNKNOWN_CHAR << endl;
			      number_entered = 0;
			 }
		    }

		    // Check the print queue to see if another task has sent something to be printed
		    else if (p_print_ser_queue->check_for_char ())
			 p_serial->putchar (p_print_ser_queue->getchar ());

		    break; // End of state 1

	       // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	       // Motor control case
	       case (2):
		    if (p_serial->check_for_char ())	// Wait for character and read
		    {    
			 char_in = p_serial -> getchar ();  

			 // Switch statement to respond to commands typed in by user
			 switch (char_in)
			 {
			      // The 'p' command allows for the motor power number to be inputted
			      case ('p'):
				   *p_serial << PMS ("Enter number for Motor ") << sh_motor_select->get() << (" power:") << endl;
				   number_state = 1;
				   transition_to (1);
				   break;

			      // The 'b' command allows for motor breaking intensity
			      case ('b'):
				   *p_serial << PMS ("Enter number for Motor ") << sh_motor_select->get() << (" breaking:") << endl;
				   number_state = 2;
				   transition_to (1);
				   break;

			      // The 's' command stops the motor (enables full brake)
			      case ('s'):
				   sh_braking_full_flag-> put(1);	// Set braking_full_flag high		
				   *p_serial << PMS ("Motor ") << sh_motor_select->get() << PMS (" HALTED!") << endl;
				   print_main_menu ();
				   transition_to (0);
				   break;

			      // A control-C character causes the CPU to restart
			      case (3):
				   *p_serial << PMS ("Resetting AVR") << endl;
				   wdt_enable (WDTO_120MS);
				   for (;;);
				   break;
				   
			      // The 'r' command returns user to Main Menu
			      case ('r'):
				   print_main_menu ();
				   transition_to (0);
				   break;

			      // If character isn't recognized, ask What's That Function?
			      default:
				   *p_serial << '"' << char_in << PMS ("\" ") << UNKNOWN_CHAR << endl;
				   break;
			 }; // End switch for characters
		    } // End if a character was received

		    break; // End of state 2
		    
	       // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	       // In state 3 the user inputs PID controls
	       case (3):
		    if (p_serial->check_for_char ())		// Wait for character and read
		    {
			 char_in = p_serial -> getchar ();

			 // Switch statement that responds user command characters
			 switch (char_in)
			 {

			      // The 's' command: for setpoint motor 1 and motor 2
			      case ('s'):
				   *p_serial << PMS ("Input setpoint for Motor 1: ") << endl;
				   number_state = 3;
				   transition_to (1);
				   break;
				   
			      // A Ctrl-C character causes the CPU to restart
			      case (3):
				   *p_serial << PMS ("Resetting AVR") << endl;
				   wdt_enable (WDTO_120MS);
				   for (;;);
				   break;

			      // The 'r' command: return to Main Menu
			      case ('r'):
				   sh_PID_control->put(0);		// Clear PID flag
				   print_main_menu();
				   transition_to (0);
				   break;

			      // Unrecognized character: return typed character and prompt user for a retype
			      default:
				   *p_serial << '"' << char_in << PMS ("\" ") << UNKNOWN_CHAR << endl;
				   break;
			 }; // End switch for characters
		    } // End if a character was received

	       break; // End of state 0
				
	       // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	       // If ever sent to default state, restart since obvious error
	       default:
		    *p_serial << PMS ("Illegal state! Resetting AVR") << endl;
		    wdt_enable (WDTO_120MS);
		    for (;;);
		    break;

	  } // End switch state

	  runs++;			// Increment counter for debugging

	  delay_ms (1);			// Delay 1 millisecond to all lower priority tasks to run
     }
}


//-----------------------------------------------------------------------------------------------------------
// This method prints the Main Menu message.
void task_user::print_main_menu (void)
{
     *p_serial << endl;
     *p_serial << PMS ("----------------- MAIN MENU -----------------") << endl;
     *p_serial << PMS ("    t:      Show the current time") << endl;
     *p_serial << PMS ("    s:      Version/Setup information") << endl;
     *p_serial << PMS ("    d:      Stack dump for tasks") << endl;
     *p_serial << PMS ("    a:      Enable/Disable Autonomous Mode (PID)") << endl;
     *p_serial << PMS ("    m:      Motor select and power value ") << endl;
     *p_serial << PMS ("  Ctl-C:    Reset AVR microcontroller") << endl;
     *p_serial << PMS ("    r:      Return to Main Menu") << endl;
     *p_serial << endl;
}


//-----------------------------------------------------------------------------------------------------------
// This method prints the Motor Menu message.
void task_user::print_motor_menu (void)
{
     *p_serial << endl;
     *p_serial << PMS ("----------------- MOTOR MENU -----------------") << endl;
     *p_serial << PMS ("    p:      Motor POWER with PWM [-255, 255]") << endl;
     *p_serial << PMS ("    b:      Motor BRAKE with PWM [-255, 255]") << endl;
     *p_serial << PMS ("    s:      Full Brake") << endl;
     *p_serial << PMS ("  Ctl-C:    Reset AVR microcontroller") << endl;
     *p_serial << PMS ("    r:      Return to Main Menu") << endl;
     *p_serial << endl;
}


//-----------------------------------------------------------------------------------------------------------
// This method prints the Motor Menu message.
void task_user::print_PID_menu (void)
{
     *p_serial << endl;
     *p_serial << PMS ("--------------- AUTONOMOUS MENU ---------------") << endl;
     *p_serial << PMS ("    s:      Enter setpoints xxxxxx for Motor 1 then Motor 2") << endl;
     *p_serial << PMS ("  Ctl-C:    Reset AVR microcontroller") << endl;
     *p_serial << PMS ("    r:      Return to Main Menu") << endl;
     *p_serial << endl;
}


//-----------------------------------------------------------------------------------------------------------
/** This method displays information about the status of the system, including the following: 
 *    \li The name and version of the program
 *    \li The name, status, priority, and free stack space of each task
 *    \li Processor cycles used by each task
 *    \li Amount of heap space free and setting of RTOS tick timer
 */

void task_user::show_status (void)
{
     time_stamp the_time;			// Holds current time for printing

     // First print the program version, compile date, etc. 
     *p_serial << endl << PROGRAM_VERSION << PMS (__DATE__) << endl 
		       << PMS ("System time: ") << the_time.set_to_now ()
		       << PMS (", Heap: ") << heap_left() << "/" << configTOTAL_HEAP_SIZE
		       #ifdef OCR5A
		       << PMS (", OCR5A: ") << OCR5A << endl << endl;
		       #elif (defined OCR3A)
		       << PMS (", OCR3A: ") << OCR3A << endl << endl;
		       #else
		       << PMS (", OCR1A: ") << OCR1A << endl << endl;
		       #endif

     // Print status of all tasks
     print_task_list (p_serial);
     *p_serial << endl;
     // Print status of all shared variables
     print_all_shares (p_serial);
}