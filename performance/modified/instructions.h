/* Forrest Butler and Amoses Holton
 * Assignment 6
 * 11/14/15
 *
 * Interface for instructions of a specified univerasl machine that has
 * 14 possible instructions.
 */
#include <inttypes.h>
#include <stdio.h>
#include "segment.h"
#ifndef INSTRUCTIONS_H_INCLUDED
#define INSTRUCTIONS_H_INCLUDED
#define REG_SIZE uint32_t
#define WORD_SIZE uint32_t

/* When reg_c is not equal to zero then reg_a gets the value at reg_b.
 * Otherwise, no move is performed.
 */
void conditional_move(Segment_T seg_memory, REG_SIZE *reg_a, REG_SIZE *reg_b, 
                        REG_SIZE *reg_c);

/* Stores the value in reg_c into the segment identified by reg_a at
 * the word offset by reg_b. 
 */
void store(Segment_T seg_memory, REG_SIZE *reg_a, REG_SIZE *reg_b, 
           REG_SIZE *reg_c);

/* Loads the value in segment identified by reg_b and word offset reg_c into
 * reg_a.
 */
void load(Segment_T seg_memory, REG_SIZE *reg_a, REG_SIZE *reg_b, 
          REG_SIZE *reg_c);

/* Adds the values in reg_b and reg_c and stores the 32-bit value into reg_a. */
void add(Segment_T seg_memory, REG_SIZE *reg_a, REG_SIZE *reg_b, 
           REG_SIZE *reg_c);

/* Multiplies the values in reg_b and reg_c and stores the 32-bit value into
 * reg_a.
 */
void multiply(Segment_T seg_memory, REG_SIZE *reg_a, REG_SIZE *reg_b, 
                REG_SIZE *reg_c);

/* Divides the values in reg_a and reg_b and stores the outcome into reg_a. */
void divide(Segment_T seg_memory, REG_SIZE *reg_a, REG_SIZE *reg_b, 
                REG_SIZE *reg_c);

/* Performs a bitwise AND on reg_b and reg_c, performs a bitwise NOT on the 
 * outcome and stores that value in reg_a.
 */
void nand(Segment_T seg_memory, REG_SIZE *reg_a, REG_SIZE *reg_b, 
           REG_SIZE *reg_c);

/* Stops any and all computation. */
void halt();

/* Creates a new segment of length reg_c and stores the new segment identifier
 * into reg_b.
 */
void map_segment(Segment_T seg_memory, REG_SIZE *reg_b, REG_SIZE *reg_c);

/* Unmaps the segment identiified by reg_c. */
void unmap_segment(Segment_T seg_memory, REG_SIZE *reg_b, REG_SIZE *reg_c);

/* The value in reg_c is displayed on the I/O device if it is in the range of
 * 0-255.
 */
void output(Segment_T seg_memory, REG_SIZE *reg_b, REG_SIZE *reg_c);

/* Universal machine waits for input on the I/O device and once it arrives, it's
 * stored in reg_c (if it's between 0-255). Reg_a is filled with a full 32-bit
 * word where every bit is 1 if the end of input is signaled.
 */
void input(Segment_T seg_memory, REG_SIZE *reg_b, REG_SIZE *reg_c);

/* Stores the value located within the word into a register located within the
 * word.
 */
void load_value(REG_SIZE *reg_a, WORD_SIZE value);

#endif
