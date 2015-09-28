/* 
 * bit2.c
 * by Amoses Holton and Katherine Hoskins
 *
 * The file contains the implementation for a 2-Dimensional array of bits 
 * created from the 1-Dimensional Bit interface. The interface includes 
 * functions that allow the user to create and free a 2D array, access the 
 * elements of an existing array, obtain dimension information, and map any 
 * function over all of the elements by row or column.
 *
 */

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "assert.h"
#include "bit.h"
#include "bit2.h"
#define T Bit2_T

/* Struct that represents the 2D array by storing the width and height as well
 * the 1D array that contains all of the elements.
 */
struct T {
        int width;
        int height; 
        Bit_T arr;
};


/*                                 Bit2_new
 *
 * Takes the desired width and height of a 2-Dimensional bit array. 
 * Creates a new, two-dimensional array with dimensions width x height, 
 * whose indicies are capable of storing one bit each. Memory for the struct is
 * allocated and the width and height are stored. The 2D array is then 
 * represented by a 1D array with the same number of indices as the 2D 
 * representation. Returns the array.
 */
T Bit2_new(int width, int height)
{
        int length = width * height;
            Bit2_T array2 = malloc(sizeof(*array2));
            assert(array2 != NULL);

            Bit_T array1 = Bit_new(length);
            array2->width = width;
            array2->height = height;
            array2->arr = array1;

            return array2;
}

/*                                 Bit2_free
 *
 * Takes a pointer to a Bit2_T and first frees the 1D UArray that holds all
 * of the elements and then frees the memory used by the 2D array struct.
 */
void Bit2_free(T *array)
{
        assert(array != NULL && *array != NULL);

        Bit_free(&((*array)->arr));
        free(*array);       
}


/*                                Bit2_get
 *
 * Takes a Bit2_T array, the desired col and desired row. Uses the row and col
 * to calculate the index of the desired bit and returns that element.
 */
int Bit2_get(T array, int col, int row)
{
        assert(array != NULL);
        assert(col >= 0 && col < (array->width));
                assert(row >= 0 && row < (array->height));

        int index = ((row * array->width) + col);
        int curr_bit = Bit_get(array->arr, index);

        return curr_bit;
}

/*                                Bit2_put
 *
 * Takes a Bit2_T array, the desired col, desired row and an integer for the 
 * bit. Uses the row and col to calculate the index of the bit being replaced.
 * Returns the bit that was replaced. 
 */
int Bit2_put(T array, int col, int row, int bit)
{
        assert(array != NULL);    
        assert(col >= 0 && col < (array->width));
        assert(row >= 0 && row < (array->height));

        int index = ((row * array->width) + col);
        int old_bit = Bit_get(array->arr, index);
        Bit_put(array->arr, index, bit);

        return old_bit;
}


/*                         Bit2_width
 *
 * Takes a Bit2_T and returns the width of that array.
 */
int Bit2_width(T array)
{
        assert(array != NULL);
        return array->width;
}

/*                        Bit2_height
 *
 * Takes a Bit2_T and returns the height of that array. 
 */
int Bit2_height(T array)
{
        assert(array != NULL);
        return array->height;
}

/*                       Bit2_map_row_major
 *
 * Takes a Bit2_T, an apply function and a pointer to the closure. It goes 
 * through the two-dimensional array by row and applies the apply function
 * to each element.
 */ 
void Bit2_map_row_major(T array, void apply (int col, int row, 
                             T array, int bit, void *cl), void * cl)
{
        assert(array != NULL);

        int width = array->width;
        int height = array->height;
            for (int i = 0; i < height; i++) {
                for (int j = 0; j < width; j++) {
                        apply(j, i, array, Bit2_get(array, j, i), cl);
                }
            }
}

/*                       Bit2_map_col_major
 *
 * Takes a Bit2_T, an apply function and a pointer to the closure.Goes 
 * through the two-dimensional array by column and applies the apply 
 * function to each element.
 */
void Bit2_map_col_major(T array, void apply (int col, int row,
                             T array, int bit, void *cl), void * cl)
{
        assert(array != NULL);
        
            int width = array->width;
        int height = array->height;
        
            for (int i = 0; i < width; i++) {
                for (int j = 0; j < height; j++) {
                        apply(i, j, array, Bit2_get(array, i, j), cl);
                }
        }
}

