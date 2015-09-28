/* bit2.h
 * Katherine Hoskins and Amoses Holton
 *
 * Interface for Bit2, which represents a two dimensional array of bits that
 * can be used to store pixel data for a black and white image. 
 */

#include <uarray.h>
#ifndef BIT2_H_INCLUDED
#define BIT2_H_INCLUDED
#define T Bit2_T
typedef struct T *T;


/*Creates a new two dimensional array of bits with dimensions width x height*/
T Bit2_new(int width, int height);

/* Frees all memory allocated for the two dimensional array "array" */
void Bit2_free(T * array);


/*Returns the bit stored at index [row, col] of array "array" */
int Bit2_get(T array, int col, int row);

/*Stores bit "bit" in position [row, col] of the given array */
int Bit2_put(T array, int col, int row, int bit);

/*Returns the width of the two-dimensional array */
int Bit2_width(T array);


/* Returns the height of the two-dimensional array */ 
int Bit2_height(T array);


/* Goes through each element of the two dimensional array by column and applies
 *the "apply" function to each element
 */
void Bit2_map_col_major(T array, void apply(int col, int row, T array,
                                int bit, void *cl), void *cl);

/*Goes through each element of the two dimensional array by row and applies
 *the apply function to each element.
 */ 
void Bit2_map_row_major(T array, void apply(int col, int row, T array,
                                int bit, void *cl), void *cl);


#undef T
#endif
