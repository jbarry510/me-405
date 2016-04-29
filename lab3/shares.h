//***********************************************************************************************************
/** @file shares.h
 *    This file contains extern declarations for queues and other inter-task data
 *    communication objects used in a ME405/507/FreeRTOS project. 
 *
 *  Revisions:
 *    @li 09-30-2012 JRR Original file was a one-file demonstration with two tasks
 *    @li 10-05-2012 JRR Split into multiple files, one for each task plus a main one
 *    @li 10-29-2012 JRR Reorganized with global queue and shared data references
 *    @li 01-04-2014 JRR Re-reorganized, allocating shares with new now
 *    @li April 28, 2016 -- BKK Added shared variables for encoder count, old and new state, error count
 *
 *  License:
 *	This file is copyright 2015 by JR Ridgely and released under the Lesser GNU Public License, 
 * 	version 2. It intended for educational use only, but its use is not limited thereto.
 *	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
 *	IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 *	FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 *	CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *	DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *	DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 *	IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 *	THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */
//***********************************************************************************************************

/// This define prevents this .h file from being included multiple times in a .cpp file
#ifndef _SHARES_H_
#define _SHARES_H_

//-----------------------------------------------------------------------------------------------------------
/// Externs: In this section, we declare variables and functions that are used in all (or at least two) of
/// the files in the data acquisition project. Each of these items will also be declared exactly once,
/// without the keyword 'extern', in one .cpp file as well as being declared extern here. 

/// This queue allows tasks to send characters to the user interface task for display.
extern TextQueue* p_print_ser_queue;

/// Motor selection share
extern TaskShare<int8_t>* sh_motor_select;

/// Power value share
extern TaskShare<int16_t>* sh_power_entry;
/// Flag share indicating power value has changed
extern TaskShare<int8_t>* sh_power_set_flag;

/// Braking value share
extern TaskShare<int16_t>* sh_braking_entry;
/// Flag share indicating braking value has changed
extern TaskShare<int8_t>* sh_braking_set_flag;

/// Flag share indicating full braking requested
extern TaskShare<int8_t>* sh_braking_full_flag;

/// Encoder counts:Motor 1 encoder count
extern TaskShare<volatile uint16_t>* sh_encoder_count_1;	/// Motor 1
extern TaskShare<volatile uint16_t>* sh_encoder_count_2;	/// Motor 2

/// Motor 1 encoder states
extern TaskShare<volatile uint8_t>* sh_encoder_old_state_1;	/// Previous state
extern TaskShare<volatile uint8_t>* sh_encoder_new_state_1;	/// Next state

/// Motor 2 encoder states
extern TaskShare<volatile uint8_t>* sh_encoder_old_state_2;	/// Previous state
extern TaskShare<volatile uint8_t>* sh_encoder_new_state_2;	/// Next state

/// Tick jump error count
extern TaskShare<uint16_t>* sh_encoder_error_count_1;		// Motor 1
extern TaskShare<uint16_t>* sh_encoder_error_count_2;		// Motor 2

#endif /// _SHARES_H_