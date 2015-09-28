/* uarray2.h
 * Katherine Hoskins and Amoses Holton
 * September 21, 2015
 * 
 * Interface for UArray2, which represents a two dimensional array using
 * a one-dimensional array interface under the covers.
 *  
 */
#include <uarray.h>
#ifndef UARRAY2_H_INCLUDED
#define UARRAY2_H_INCLUDED
#define T UArray2_T
typedef struct T *T;


/* Creates a new, two-dimensional array with dimensions width x height , whose 
 * indicies store elements of size "size" 
 */
T UArray2_new(int width, int height, int size);

/* Deallocates all memory allocated for the two dimensional UArray2 instance.*/
void UArray2_free(T *array);

/* Returns a pointer to the element at index [row, col] of array "array"*/ 
void *UArray2_at(T array, int col, int row);

/* Returns the width of the array */
int UArray2_width(T array);

/* Returns the height of the array */
int UArray2_height(T array);

/* Returns the size of the elements stored in the array*/
int UArray2_size(T array); 

/* Goes through the two-dimensional array by row and applies the apply function
 * to each element.
 */ 
void UArray2_map_row_major(T array, void apply (int col, int row, 
                             T array, void *element, void *cl), void * cl);

/* Goes through the two-dimensional array by column and applies the apply 
 * function to each element 
 */
void UArray2_map_col_major(T array, void apply (int col, int row,
                             T array, void *element, void *cl), void * cl);


#undef T
#endif
