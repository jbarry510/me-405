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

// Menu descriptors
#define MAIN    0
#define HELP    1
#define NUMBER  2
#define MOTOR   3
#define SERVO   4
#define ROUTES  5
#define HIGHWAY 6
#define DRIVE	7
#define CLASS	8

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
	       // In state MAIN the user types characters, which are interpreted as commands.
	       case (MAIN):
		    if (p_serial->check_for_char ())		// Wait for character and read
		    {
			 char_in = p_serial -> getchar ();

			 // Switch statement that responds user command characters
			 switch (char_in)
			 {
			      case ('a'):
				   if(sh_PID_control->get() == 0)
				   {
					sh_PID_control->put(1);		// Set PID flag high
					*p_serial << PMS ("Autonomous Mode: ACTIVE") << endl;
					print_PID_menu();
					transition_to (SERVO);
				   }
				   else if(sh_PID_control->get() == 1)
				   {
					sh_PID_control->put(0);
					*p_serial << PMS ("Autonomous Mode: HALTED") << endl;
					print_main_menu();
					transition_to (MAIN);
				   }
				   break;
				   
			      // The 'm' command: allows for manual motor manipulation
			      case ('m'):
				   *p_serial << PMS ("Enter number, 1 or 2, for motor selection: ") << endl;
				   number_entered = 0;		// Clears number_entered
				   sh_motor_select -> put(0);	// Clears shared sh_motor_select
				   number_state = 0;		// Clears number state
				   transition_to (NUMBER);
				   break;
			      
			      // The 'd' command: allows for manual driving
			      case ('d'):
				   print_drive_menu();
				   sh_power_set_flag -> put(3);
				   sh_servo_set_flag -> put(2);
				   transition_to (DRIVE);
				   break;
				   
			      // The 's' command: allows for manual servo manipulation
			      case ('s'):
				   print_servo_menu();
				   transition_to (SERVO);
				   break;
				   
			      // The 'c' command: transitions to the class required task list
			      case ('c'):
				   print_class_menu();
				   transition_to (ROUTES);
				   break;
				   
			      // The 'h' command: transitions to the highway maintenance task list
			      case ('h'):
				   print_highway_menu();
				   transition_to (HIGHWAY);
				   break;
				   
			      // The '?' command: move to system/microcontroller help menu
			      case ('?'):
				   print_help_menu();
				   transition_to (HELP);
				   break;
				   
			      // A Ctrl-C character causes the CPU to restart
			      case (3):
				   *p_serial << PMS ("Resetting AVR") << endl;
				   wdt_enable (WDTO_120MS);
				   for (;;);
				   break;

			      // The 'r' command: return to Main Menu
			      case ('r'):
				   print_main_menu ();
				   transition_to (MAIN);
				   break;

			      // Unrecognized character: return typed character and prompt user for a retype
			      default:
				   *p_serial << '"' << char_in << PMS ("\" ") << UNKNOWN_CHAR << endl;
				   break;
			 } // End switch for characters
		    } // End if a character was received

	       break; // End of state MAIN
	       
	       // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	       // In state HELP the user types characters, which are interpreted as commands.
	       case (HELP):
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
				   show_status ();
				   break;

			      // The 'd' command: all the tasks dump their stacks
			      case ('d'):
				   print_task_stacks (p_serial);
				   break;
				   
			      // A Ctrl-C character causes the CPU to restart
			      case (3):
				   *p_serial << PMS ("Resetting AVR") << endl;
				   wdt_enable (WDTO_120MS);
				   for (;;);
				   break;

			      // The 'r' command: return to Main Menu
			      case ('r'):
				   print_main_menu ();
				   transition_to (MAIN);
				   break;

			      // Unrecognized character: return typed character and prompt user for a retype
			      default:
				   *p_serial << '"' << char_in << PMS ("\" ") << UNKNOWN_CHAR << endl;
				   break;
			 } // End switch for characters
		    } // End if a character was received
	       break; // End of state HELP


	       // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	       // In state 1, wait for user to enter digits and build into a number
	       case (NUMBER):
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
					transition_to (MOTOR);		// Transition to motor control case
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
					print_main_menu ();
					transition_to (MAIN);			// Transition to main case
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
					print_main_menu ();
					transition_to (MAIN);			// Transition to main case
				   }
				   else
				   {
					*p_serial << PMS ("Please type a number between 0 to 255, then ENTER") << endl;		// Display error message for out of range
					number_entered = 0;			// Clear number entered
				   }	
			      }
			      
			      // Servo position State
			      else if (number_state == 3)
			      {
				   if (number_entered >= 2100 && number_entered <= 3900)
				   {
					sh_servo_setpoint->put(number_entered);	// Set servo position to number_entered
					sh_servo_set_flag->put(1);		// Update servo position
					*p_serial << sh_servo_set_flag->get();
					number_entered = 0;			// Clear number_entered
					number_state = 0;			// Clear number_state
					print_main_menu();
					transition_to (MAIN);
				   }
				   else
				   {
					*p_serial << PMS ("Please type a number between 15 to 29, then ENTER") << endl;		// Display error message for out of range
					number_entered = 0;			// Clear number entered
				   }	
			      }
			      
			      // Circular path radius State
			      else if (number_state == 4)
			      {
				   if (number_entered >= 0 && number_entered <= 225)
				   {
					sh_path_radius->put(number_entered);	// Set servo position to number_entered
					number_entered = 0;			// Clear number_entered
					number_state = 5;			// Clear number_state
					*p_serial << PMS ("Please enter a circular path velocity [0,255]") << endl;		// Display error message for out of range
					transition_to (NUMBER);
				   }
				   else
				   {
					*p_serial << PMS ("Please type a number between 0 to 255, then ENTER") << endl;		// Display error message for out of range
					number_entered = 0;			// Clear number entered
				   }	
			      }
			      // Route path velocity
			      else if (number_state == 5)
			      {
				   if (number_entered >= 0 && number_entered <= 225)
				   {
					sh_path_velocity->put(number_entered);	// Set servo position to number_entered
					number_entered = 0;			// Clear number_entered
					number_state = 0;			// Clear number_state		// Display error message for out of range
					transition_to (ROUTES);
				   }
				   else
				   {
					*p_serial << PMS ("Please type a number between 0 to 255, then ENTER") << endl;		// Display error message for out of range
					number_entered = 0;			// Clear number entered
				   }	   
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
	       case (MOTOR):
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
				   transition_to (NUMBER);
				   break;

			      // The 'b' command allows for motor breaking intensity
			      case ('b'):
				   *p_serial << PMS ("Enter number for Motor ") << sh_motor_select->get() << (" breaking:") << endl;
				   number_state = 2;
				   transition_to (NUMBER);
				   break;

			      // The 's' command stops the motor (enables full brake)
			      case ('f'):
				   sh_braking_full_flag-> put(1);	// Set braking_full_flag high		
				   *p_serial << PMS ("Motor ") << sh_motor_select->get() << PMS (" HALTED!") << endl;
				   print_main_menu ();
				   transition_to (MAIN);
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
				   transition_to (MAIN);
				   break;

			      // If character isn't recognized, ask What's That Function?
			      default:
				   *p_serial << '"' << char_in << PMS ("\" ") << UNKNOWN_CHAR << endl;
				   break;
			 }; // End switch for characters
		    } // End if a character was received

		    break; // End of state 2
		    
	       // Servo control case
	       case (SERVO):
		    if (p_serial->check_for_char ())	// Wait for character and read
		    {    
			 char_in = p_serial -> getchar ();  

			 // Switch statement to respond to commands typed in by user
			 switch (char_in)
			 {
			      // The 'p' command allows for manual servo position setting
			      case ('p'):
				   *p_serial << PMS ("Enter position for Servo") << endl;
				   number_state = 3;
				   transition_to (NUMBER);
				   break;

			      // The 's' command allows for manual servo position setpoint setting
			      case ('s'):
				   *p_serial << PMS ("Enter position setpoint for Servo") << endl;
				   number_state = 3;
 				   transition_to (NUMBER);
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
				   transition_to (MAIN);
				   break;

			      // If character isn't recognized, ask What's That Function?
			      default:
				   *p_serial << '"' << char_in << PMS ("\" ") << UNKNOWN_CHAR << endl;
				   break;
			 }; // End switch for characters
		    } // End if a character was received

		    break; // End of state 3
		    
	       // ME-405 task case
	       case (ROUTES):
		    if (p_serial->check_for_char ())	// Wait for character and read
		    {    
			 char_in = p_serial -> getchar ();  

			 // Switch statement to respond to commands typed in by user
			 switch (char_in)
			 {
			      // The 'l' command activates linear heading adherance
			      case ('l'):
 				   *p_serial << PMS ("Enter velocity of linear path [0,255]") << endl;
				   number_state = 5;
 				   transition_to (NUMBER);
				   break;

			      // The 'c' command activates circular path routing
			      case ('c'):
				   *p_serial << PMS ("Enter radius of circular path [0,255]") << endl;
				   number_state = 4;
 				   transition_to (NUMBER);
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
				   transition_to (MAIN);
				   break;

			      // If character isn't recognized, ask What's That Function?
			      default:
				   *p_serial << '"' << char_in << PMS ("\" ") << UNKNOWN_CHAR << endl;
				   break;
			 }; // End switch for characters
		    } // End if a character was received

		    break; // End of state 4
		    
	       // Highway task case
	       case (HIGHWAY):
		    if (p_serial->check_for_char ())	// Wait for character and read
		    {    
			 char_in = p_serial -> getchar ();  

			 // Switch statement to respond to commands typed in by user
			 switch (char_in)
			 {
			      // The 'l' command activates lane keeping
			      case ('l'):
 				   transition_to (MAIN);
				   break;

			      // The 'c' command activates car avoidance
			      case ('c'):
 				   transition_to (MAIN);
				   break;
				   
			      // The 'p' command activates car passing
			      case ('p'):
 				   transition_to (MAIN);
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
				   transition_to (MAIN);
				   break;

			      // If character isn't recognized, ask What's That Function?
			      default:
				   *p_serial << '"' << char_in << PMS ("\" ") << UNKNOWN_CHAR << endl;
				   break;
			 }; // End switch for characters
		    } // End if a character was received

		    break; // End of state 5
		    
	       // Drive Case
	       case (DRIVE):
		 if (p_serial->check_for_char ())	// Wait for character and read
		    {    
			 char_in = p_serial -> getchar ();

			 // Switch statement to respond to commands typed in by user
			 switch (char_in)
			 {
			      // The 'w' command increments the motor power by 20
			      case ('w'):
				   sh_power_entry -> put(sh_power_entry -> get() +20); // Saturates max power to 255
				   if (sh_power_entry -> get() >= 255)
				     sh_power_entry -> put(255);
					sh_power_set_flag-> put(2);		// Set flag high to indicate a changed power value
				   *p_serial << PMS ("Current Motor Velocities: ") << sh_power_entry->get() << endl;
				   *p_serial << PMS ("Current Servo Position: ") << sh_servo_setpoint->get() << endl;
				   *p_serial << endl;
				   transition_to (DRIVE);
				   break;

			      // The 's' command decrements the motor power by 20
			      case ('s'):
				   sh_power_entry -> put(sh_power_entry->get()-20); // Saturates min power to -255
				    if (sh_power_entry-> get() <= -255)
				     sh_power_entry -> put(-255);
				   sh_power_set_flag-> put(2);		// Set flag high to indicate a changed power value
				   *p_serial << PMS ("Current Motor Velocities: ") << sh_power_entry->get() << endl;
				   *p_serial << PMS ("Current Servo Position: ") << sh_servo_setpoint->get() << endl;
				   *p_serial << endl;
				   transition_to (DRIVE);
				   break;
				   
			      // The 'a' command turns steering servo to the left by -50
			      case ('a'):
				   sh_servo_setpoint -> put(sh_servo_setpoint->get()+100);
				   if (sh_servo_setpoint -> get() >= 4000) // Saturates max angle to 29
				     sh_servo_setpoint -> put(4000);
				   sh_servo_set_flag -> put(1);
				   *p_serial << PMS ("Current Motor Velocities: ") << sh_power_entry->get() << endl;
				   *p_serial << PMS ("Current Servo Position: ") << sh_servo_setpoint->get() << endl;
				   *p_serial << endl;
				   transition_to (DRIVE);
				   break;
				   
			      // The 'd' command turns steering servo to the right by 50
			      case ('d'):
				   sh_servo_setpoint -> put(sh_servo_setpoint->get()-100); // Saturates min angle to 15
				   if (sh_servo_setpoint -> get() <= 2000)
				     sh_servo_setpoint -> put(2000);
				   sh_servo_set_flag -> put(1);
				   *p_serial << PMS ("Current Motor Velocities: ") << sh_power_entry->get() << endl;
				   *p_serial << PMS ("Current Servo Position: ") << sh_servo_setpoint->get() << endl;
				   *p_serial << endl;
				   transition_to (DRIVE);
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
				   transition_to (MAIN);
				   break;

			      // If character isn't recognized, ask What's That Function?
			      default:
				   *p_serial << '"' << char_in << PMS ("\" ") << UNKNOWN_CHAR << endl;
				   break;
			 }; // End switch for characters
		    } // End if a character was received

		    break; // End of state 6
		    
	       // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	       // If ever sent to default state, restart since obvious error
	       default:
		    *p_serial << PMS ("Illegal state! Resetting AVR") << endl;
		    wdt_enable (WDTO_120MS);
		    for (;;);
		    break;

	  } // End switch state

	  runs++;			// Increment counter for debugging
	  delay_ms (50);		// Delay 1 millisecond to all lower priority tasks to run
     }
}


//-----------------------------------------------------------------------------------------------------------
// This method prints the Main Menu message.
void task_user::print_main_menu (void)
{
     *p_serial << endl;
     *p_serial << PMS ("----------------- MAIN MENU -----------------") << endl;
     *p_serial << PMS ("    m:      Motor manual manipulation") << endl;
     *p_serial << PMS ("    s:      Servo manual manipulation") << endl;
     *p_serial << PMS ("    d:      Drive the car!") << endl;
     *p_serial << PMS ("    c:      Class required tasks") << endl;
     *p_serial << PMS ("    h:      Highway maintenance") << endl;
     *p_serial << PMS ("    ?:      Help Menu") << endl;
     *p_serial << PMS ("  Ctl-C:    Reset AVR microcontroller") << endl;
     *p_serial << PMS ("    r:      Return to Main Menu") << endl;
     *p_serial << endl;
}

// This method prints the Drive Car message.
void task_user::print_drive_menu (void)
{
     *p_serial << endl;
     *p_serial << PMS ("--------------CAR DRIVE MENU -----------------") << endl;
     *p_serial << PMS ("    w:      Increment motor velocity +20") << endl;
     *p_serial << PMS ("    s:      Decrement motor velocity -20") << endl;
     *p_serial << PMS ("    a:      Rotate steering to the left -50") << endl;
     *p_serial << PMS ("    d:      Rotate steering to the right +50") << endl;
     *p_serial << PMS ("    b:      Emergency break!") << endl;
     *p_serial << PMS ("  Ctl-C:    Reset AVR microcontroller") << endl;
     *p_serial << PMS ("    r:      Return to Main Menu") << endl;
     *p_serial << endl;
}
// This method prints the Help Menu message.
void task_user::print_help_menu (void)
{
     *p_serial << endl;
     *p_serial << PMS ("----------------- HELP MENU -----------------") << endl;
     *p_serial << PMS ("    t:      Show the current time") << endl;
     *p_serial << PMS ("    s:      Version/Setup information") << endl;
     *p_serial << PMS ("    d:      Stack dump for tasks") << endl;
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
     *p_serial << PMS ("    f:      Full Brake") << endl;
     *p_serial << PMS ("    s:      Velocity setpoint [-40, 40]") << endl;
     *p_serial << PMS ("  Ctl-C:    Reset AVR microcontroller") << endl;
     *p_serial << PMS ("    r:      Return to Main Menu") << endl;
     *p_serial << endl;
}

// This method prints the Servo Menu message.
void task_user::print_servo_menu (void)
{
     *p_serial << endl;
     *p_serial << PMS ("----------------- SERVO MENU -----------------") << endl;
     *p_serial << PMS ("    p:      Servo position with PWM [29 [L], 22 [C], 15 [R]]") << endl;
     *p_serial << PMS ("    s:      Position setpoint") << endl;
     *p_serial << PMS ("  Ctl-C:    Reset AVR microcontroller") << endl;
     *p_serial << PMS ("    r:      Return to Main Menu") << endl;
     *p_serial << endl;
}

// This method prints the Class Menu message.
void task_user::print_class_menu (void)
{
     *p_serial << endl;
     *p_serial << PMS ("----------------- CLASS TASK MENU ------------") << endl;
     *p_serial << PMS ("    l:      Linear heading adherance") << endl;
     *p_serial << PMS ("    c:      Circular path") << endl;
     *p_serial << PMS ("  Ctl-C:    Reset AVR microcontroller") << endl;
     *p_serial << PMS ("    r:      Return to Main Menu") << endl;
     *p_serial << endl;
}

// This method prints the Highway Menu message.
void task_user::print_highway_menu (void)
{
     *p_serial << endl;
     *p_serial << PMS ("----------------- HIGHWAY TASK MENU -----------------") << endl;
     *p_serial << PMS ("    l:      Toggle lane keeping") << endl;
     *p_serial << PMS ("    c:      Toggle car avoidance") << endl;
     *p_serial << PMS ("    p:      Execute highway passing") << endl;
     *p_serial << PMS ("  Ctl-C:    Reset AVR microcontroller") << endl;
     *p_serial << PMS ("    r:      Return to Main Menu") << endl;
     *p_serial << endl;
}

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