/* Forrest Butler and Amoses Holton
 * Assignment 6
 * 11/14/15
 *
 * Implmentation of the 14 instructions that a simple machine uses.
 * Allows the user to perform value specific actions or memory 
 * specific functions as long they provide their memory and 
 * registers. 
 */
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include "segment.h"
#include "instructions.h"
#define REG_SIZE uint32_t
#define WORD_SIZE uint32_t

/* When reg_c is not equal to zero then reg_a gets the value at reg_b.
 * Otherwise, no move is performed.
 */
void conditional_move(REG_SIZE *reg_a, REG_SIZE *reg_b, REG_SIZE *reg_c)
{
        if (*reg_c != 0) {
                *reg_a = *reg_b;
        } 
}

/* Loads the value in segment identified by reg_b and word offset reg_c into
 * reg_a.
 */
void load(Segment_T seg_memory, REG_SIZE *reg_a, REG_SIZE *reg_b, 
          REG_SIZE *reg_c)
{
        *reg_a = Segment_load(seg_memory, *reg_b, *reg_c);
}

/* Stores the value in reg_c into the segment identified by reg_a at
 * the word offset by reg_b. 
 */
void store(Segment_T seg_memory, REG_SIZE *reg_a, REG_SIZE *reg_b, 
           REG_SIZE *reg_c)
{
        Segment_store(seg_memory, *reg_a, *reg_b, *reg_c);
}

/* Adds the values in reg_b and reg_c and stores the 32-bit value into reg_a. */
void add(REG_SIZE *reg_a, REG_SIZE *reg_b, REG_SIZE *reg_c)
{
        *reg_a = (*reg_b + *reg_c); 
}

/* Multiplies the values in reg_b and reg_c and stores the 32-bit value into
 * reg_a.
 */
void multiply(REG_SIZE *reg_a, REG_SIZE *reg_b, REG_SIZE *reg_c)
{
        *reg_a = (*reg_b * *reg_c);
}

/* Divides the values in reg_a and reg_b and stores the outcome into reg_a. */
void divide(REG_SIZE *reg_a, REG_SIZE *reg_b, REG_SIZE *reg_c)
{
        *reg_a = (*reg_b / *reg_c);
}

/* Performs a bitwise AND on reg_b and reg_c, performs a bitwise NOT on the 
 * outcome and stores that value in reg_a.
 */
void nand(REG_SIZE *reg_a, REG_SIZE *reg_b, REG_SIZE *reg_c)
{
        *reg_a = ~(*reg_b & *reg_c);
}

/* Stops any and all computation. */
void halt()
{
        exit(EXIT_SUCCESS);
}

/* Creates a new segment of length reg_c and stores the new segment identifier
 * into reg_b.
 */
void map_segment(Segment_T seg_memory, REG_SIZE *reg_b, REG_SIZE *reg_c)
{
        *reg_b = Segment_map(seg_memory, *reg_c);
}

/* Unmaps the segment identiified by reg_c. */
void unmap_segment(Segment_T seg_memory, REG_SIZE *reg_c)
{
        Segment_unmap(seg_memory, *reg_c);
}

/* The value in reg_c is displayed on the I/O device if it is in the range of 
 * 0-255.
 */
void output(REG_SIZE *reg_c)
{
        fprintf(stdout, "%c", (char)(*reg_c));
}

/* Universal machine waits for input on the I/O device and once it arrives, it's
 * stored in reg_c (if it's between 0-255). Reg_a is filled with a full 32-bit
 * word where every bit is 1 if the end of input is signaled.
 */
void input(REG_SIZE *reg_c)
{
        *reg_c = fgetc(stdin); 
        
        if (*reg_c ==  (unsigned)EOF) {
                *reg_c = ~0;
        }
}

/* Loads the segment identified by reg_b and makes a duplicate to replace
 * contents of segment 0 (which is abandoned). Program pointer is set to point
 * to the segment stored at segment 0 at the word offset of reg_c.
 */
void load_program(Segment_T seg_memory, REG_SIZE *reg_b, REG_SIZE *reg_c)
{
        if (*reg_b != 0) {
                Segment_move(seg_memory, *reg_b, 0);        
        }  
        (void) reg_c;
}

/* Stores the value located within the word into a register located within the
 * word.
 */
void load_value(REG_SIZE *reg_a, WORD_SIZE value)
{
        *reg_a = value;
}
