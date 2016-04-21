//**************************************************************************************
/** @file task_user.cpp
 *    This file contains source code for a user interface task for a ME405/FreeRTOS
 *    test suite. 
 *
 *  Revisions:
 *    @li 09-30-2012 JRR Original file was a one-file demonstration with two tasks
 *    @li 10-05-2012 JRR Split into multiple files, one for each task
 *    @li 10-25-2012 JRR Changed to a more fully C++ version with class task_user
 *    @li 11-04-2012 JRR Modified from the data acquisition example to the test suite
 *    @li 01-04-2014 JRR Changed base class names to TaskBase, TaskShare, etc.
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
//**************************************************************************************
#include "task_user.h"                      // Header for this file
#include <avr/io.h>                         // Port I/O for SFR's
#include <avr/wdt.h>                        // Watchdog timer header

#include "textqueue.h"                      // Header for text queue class
#include "taskshare.h"			    // Header for thread-safe shared data
#include "shares.h"                         // Shared inter-task communications


// Precompiler Directives
// Renaming ASCII representations of keyboard characters to intelligent names
#define ASCII_BACKSPACE 127		// Backspace (actually DEL)
#define ASCII_RETURN 13			// Return
#define ASCII_SPACE 32			// Space
#define ASCII_DASH 45			// Dash (negative)
#define ASCII_NEWLINE 10		// Carriage Return
#define ASCII_ESCAPE 27			// Escape


/** This constant sets how many RTOS ticks the task delays if the user's not talking.
 *  The duration is calculated to be about 5 ms.
 */
const TickType_t ticks_to_delay = ((configTICK_RATE_HZ / 1000) * 5);


//-------------------------------------------------------------------------------------
/** This constructor creates a new data acquisition task. Its main job is to call the
 *  parent class's constructor which does most of the work.
 *  @param a_name A character string which will be the name of this task
 *  @param a_priority The priority at which this task will initially run (default: 0)
 *  @param a_stack_size The size of this task's stack in bytes 
 *                      (default: configMINIMAL_STACK_SIZE)
 *  @param p_ser_dev Pointer to a serial device (port, radio, SD card, etc.) which can
 *                   be used by this task to communicate (default: NULL)
 */

task_user::task_user (const char* a_name, unsigned portBASE_TYPE a_priority, size_t a_stack_size, emstream* p_ser_dev)
	: TaskBase (a_name, a_priority, a_stack_size, p_ser_dev)
{
	// Nothing is done in the body of this constructor. All the work is done in the
	// call to the frt_task constructor on the line just above this one
}


//-------------------------------------------------------------------------------------
/** This task interacts with the user for force him/her to do what he/she is told. It
 *  is just following the modern government model of "This is the land of the free...
 *  free to do exactly what you're told." 
 * TODO: Add a increment and decrement for power increase
 * 	 Add a kill button to turn everything off if necessary
 */

void task_user::run (void)
{
	char char_in;                           // Character read from serial device
	time_stamp a_time;                      // Holds the time so it can be displayed
	int16_t number_entered = 0;             // Holds a number being entered by user
	sh_motor_select -> put(0);		// Holds the value of which motor is selected to input characteristic values
	uint8_t number_state = 0;		// State of motor to switch between 
	bool negative_number_entered = false;
	uint8_t char_num_count = 0;		// Counts characters inputted so backspace will delete accordingly

	// Tell the user how to get into command mode (state 1), where the user interface
	// task does interesting things such as diagnostic printouts
	//*p_serial << PMS ("Press 'h' or '?' for help") << endl;
	print_help_message ();
	
	// This is an infinite loop; it runs until the power is turned off. There is one 
	// such loop inside the code for each task
	for (;;)
	{
		// Run the finite state machine. The variable 'state' is kept by parent class
		switch (state)
		{
			// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
			// In state 0, we're in command mode, so when the user types characters, 
			// the characters are interpreted as commands to do something
			case (0):

				if (p_serial->check_for_char ())            // If the user typed a character, read the character
				{
					char_in = p_serial -> getchar ();

					// In this switch statement, we respond to different characters as
					// commands typed in by the user
					switch (char_in)
					{
						// The 't' command asks what time it is right now
						case ('t'):
							*p_serial << (a_time.set_to_now ()) << endl;
							break;

						// The 's' command asks for version and status information
						case ('s'):
							show_status ();
							break;

						// The 'd' command has all the tasks dump their stacks
						case ('d'):
							print_task_stacks (p_serial);
							break;

						// The 'h' command is a plea for help; '?' works also
						case ('h'):
						case ('?'):
							print_help_message ();
							break;

						// The 'm' command selects the motor for power input
						case ('m'):
							*p_serial << PMS ("Enter number for motor selection: ") << endl;
							number_entered = 0;		// Clears number_entered
							sh_motor_select -> put(0);	// Clears shared sh_motor_select
							number_state = 0;		// Clears number state
							transition_to (1);
							break;

						// A control-C character causes the CPU to restart
						case (3):
							*p_serial << PMS ("Resetting AVR") << endl;
							wdt_enable (WDTO_120MS);
							for (;;);
							break;

						// If character isn't recognized, ask What's That Function?
						default:
							*p_serial << '"' << char_in << PMS ("\": WTF?") << endl;
							break;
					}; // End switch for characters
				} // End if a character was received

				break; // End of state 0

			// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
			// In state 1, wait for user to enter digits and build into a number
			case (1):
				if (p_serial->check_for_char ())        // If the user typed a character, read the character
				{
					char_in = p_serial -> getchar ();

					// Respond to numeric characters, Enter, or Esc (only). Numbers are
					// put into the numeric value that is being built up
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
						
						      number_entered /= 10;		// Divide number_entered by 10 to remove ones place
						      char_num_count--;			// Decrement char_num_count until equal to zero
					  
					}
					
					// Line Feed, Carriage return is ignored; the newline character ends the entry
					else if (char_in == ASCII_NEWLINE)
					{
						*p_serial << "\r";
					}
					// Carriage return or Escape ends numeric entry
					else if (char_in == ASCII_RETURN || char_in == ASCII_ESCAPE)
					{
						*p_serial << endl << PMS ("Number Entered: ") << number_entered << endl;
						
						// Motor Selection State
						if (number_state == 0)
						{
						    if (number_entered == 1 || number_entered == 2)
						    {
							  sh_motor_select -> put(number_entered);		// Set shared value to motor selected
							  print_help_motor();					// Print motor menu
							  number_entered = 0;					// Clear number_entered
							  transition_to (2);					// Transition to motor control case
						    }
						    else
						    {
						      *p_serial << PMS ("Please enter a 1 or 2 for motor select.") << endl;
						      number_entered = 0;					// Clear number_entered
						      
						    }
						}
						
						// Power Setting State
						else if (number_state == 1)
						{
						    if (number_entered >= 0 && number_entered <= 255)
						    {
							  if(negative_number_entered == true)			// Check if a negative number was entered
							  {
							    number_entered *= -1;				// Change sign of number to negative
							    negative_number_entered = false;			// Clear negative number flag
							  }

							  sh_power_entry -> put(number_entered);	// Move number_entered into shared sh_power_entry
							  sh_power_set_flag-> put(1);			// Make power_set_flag high to indicate a changed power value

							  number_entered = 0;					// Clear number_entered
							  number_state = 0;					// Clear number_state
							  print_help_message ();
							  transition_to (0);					// Transition to main case
						    }
						    else
						    {
						      *p_serial << PMS ("Please enter a number between -255 to 255.") << endl;		// Display error message for out of range
						      number_entered = 0;				// Clear number entered
						    }
						}
						
						// Braking State
						else if (number_state == 2)
						{
						    if (number_entered >= 0 && number_entered <= 255)
						    {
							  sh_braking_entry -> put(number_entered);	// Move number_entered into shared sh_braking_entry
							  sh_braking_set_flag-> put(1);			// Make braking_set_flag high to indicate a changed braking value

							  number_entered = 0;					// Clear number_entered
							  number_state = 0;					// Clear number_state
							  print_help_message ();
							  transition_to (0);					// Transition to main case
						    }
						    else
						    {
						      *p_serial << PMS ("Please enter a number between 0 to 255.") << endl;		// Display error message for out of range
						      number_entered = 0;				// Clear number entered
						    }
						}
					}
					else
					{
						*p_serial << PMS ("<invalid char \"") << char_in << PMS ("\">");
						number_entered = 0;
					}
				}

				// Check the print queue to see if another task has sent this task 
				// something to be printed
				else if (p_print_ser_queue->check_for_char ())
				{
					p_serial->putchar (p_print_ser_queue->getchar ());
				}

				break; // End of state 1

			//Motor control case
			case (2):
				if (p_serial->check_for_char ())            // If the user typed a character, read the character
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

						// The 'b' command asks for version and status information
						case ('b'):
							*p_serial << PMS ("Enter number for Motor ") << sh_motor_select->get()	 << (" breaking:") << endl;
							number_state = 2;
							transition_to (1);
							break;

						// The 's' command stops the motor (enables full brake)
						case ('s'):
							if (sh_motor_select->get() == 1)
							{
							      sh_braking_full_flag-> put(1);			// Make braking_full_flag high
							}
							else if (sh_motor_select->get() == 2)
							{
							      sh_braking_full_flag-> put(1);			// Make braking_full_flag high
							}
							
							*p_serial << PMS ("Motor ") << sh_motor_select->get() << PMS (" HALTED!") << endl;
							print_help_message ();
							transition_to (0);
							break;

						// The 'h' command is a plea for help; '?' works also
						case ('h'):
							break;
							
						case ('?'):
							print_help_motor();
							break;
							
						case ('r'):
						      print_help_message ();
						      transition_to (0);
						      break;

						// A control-C character causes the CPU to restart
						case (3):
							*p_serial << PMS ("Resetting AVR") << endl;
							wdt_enable (WDTO_120MS);
							for (;;);
							break;

						// If character isn't recognized, ask What's That Function?
						default:
							*p_serial << '"' << char_in << PMS ("\": WTF?") << endl;
							break;
					}; // End switch for characters
				} // End if a character was received

				break; // End of state 2
				
			// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
			// We should never get to the default state. If we do, complain and restart
			default:
				*p_serial << PMS ("Illegal state! Resetting AVR") << endl;
				wdt_enable (WDTO_120MS);
				for (;;);
				break;

		} // End switch state

		runs++;                             // Increment counter for debugging

		// No matter the state, wait for approximately a millisecond before we 
		// run the loop again. This gives lower priority tasks a chance to run
		delay_ms (1);
	}
}

//-------------------------------------------------------------------------------------
/** This method prints a simple help message.
 */

void task_user::print_help_message (void)
{
	*p_serial << endl;
	*p_serial << PROGRAM_VERSION << endl;
	*p_serial << endl;
	*p_serial << PMS ("--------------- MAIN MENU ---------------") << endl;
	*p_serial << PMS ("  t:     Show the current time") << endl;
	*p_serial << PMS ("  s:     Version/Setup information") << endl;
	*p_serial << PMS ("  d:     Stack dump for tasks") << endl;
	*p_serial << PMS ("  m:     Motor select and Power value") << endl;
	*p_serial << PMS ("  Ctl-C: Reset AVR") << endl;
	*p_serial << PMS ("  h:     Help!") << endl;
	*p_serial << endl;
}

//-------------------------------------------------------------------------------------
/** This method prints a simple help message.
 */
void task_user::print_help_motor (void)
{
	*p_serial << endl;
	*p_serial << PROGRAM_VERSION << endl;
	*p_serial << endl;
	*p_serial << PMS ("--------------- MOTOR MENU ---------------") << endl;
	*p_serial << PMS ("  p:     Motor POWER with PWM [-255, 255]") << endl;
	*p_serial << PMS ("  b:     Motor BRAKE with PWM [-255, 255]") << endl;
	*p_serial << PMS ("  s:     Full Brake") << endl;
	*p_serial << PMS ("  Ctl-C: Reset AVR") << endl;
	*p_serial << PMS ("  h:     Help!") << endl;
	*p_serial << PMS ("  r:     Return to Main Menu") << endl;
	*p_serial << endl;
}


//-------------------------------------------------------------------------------------
/** This method displays information about the status of the system, including the
 *  following: 
 *    \li The name and version of the program
 *    \li The name, status, priority, and free stack space of each task
 *    \li Processor cycles used by each task
 *    \li Amount of heap space free and setting of RTOS tick timer
 */

void task_user::show_status (void)
{
	time_stamp the_time;					// Holds current time for printing

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

	// Have the tasks print their status; then the same for the shared data items
	print_task_list (p_serial);
	*p_serial << endl;
	print_all_shares (p_serial);
}