/* Forrest Butler and Amoses Holton
 * Assignment 6 
 * 11/14/15
 *
 * Interface for segmented memory of a universal machine. Allows the user to
 * map and unmap segments, as well as store or load words within segments.
 */
#include <inttypes.h>
#include <stdio.h>
#ifndef SEGMENT_H_INCLUDED
#define SEGMENT_H_INCLUDED
#define ID_SIZE uint32_t
#define WORD_SIZE uint32_t
#define T Segment_T
typedef struct T *T;

/* Takes a file pointer to store the program at segment 0 and returns the newly 
 * created segmented memory.
 */
T Segment_new();

/* Frees all segmented memory created by the program, including the segment 
 * where the program is stored.
 */
void Segment_free(T * seg_memory);

/* Creates a segment of desired size, intializes all words to 0 and returns the 
 * identifier.
 */
ID_SIZE Segment_map(T seg_memory, unsigned size);

/* Unmaps identified segment from memory, allows unchecked runtime error if 
 * segment doesn't exist.
 */
void Segment_unmap(T seg_memory, ID_SIZE id);

/* Returns the word at the offset in the desired segment of memory */
WORD_SIZE Segment_load (T seg_memory, ID_SIZE id, WORD_SIZE offset);

/* Stores the word at the specified offest in the desired segment of memory */
void Segment_store (T seg_memory, ID_SIZE id, WORD_SIZE offset, WORD_SIZE word);

/* Moves the segment identified by source to the target segment. The source 
 * segment is duplicated and replaces the segment at the target ID.
 */
void Segment_move(T seg_memory, ID_SIZE source, ID_SIZE target);

#undef T
#endif
