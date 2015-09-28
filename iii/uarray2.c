/* 
 * uarray2.c
 * by Amoses Holton and Katherine Hoskins
 *
 * The file contains the implementation for a 2-Dimensional array created from
 * the 1-Dimensional UArray interface. The interface includes functions that 
 * allow the user to create and free a 2D array, access the elements of an
 * existing array, obtain dimension information, and map any function over all
 * of the elements by row or column.
 *
 */

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "assert.h"
#include "uarray.h"
#include "uarray2.h"
#define T UArray2_T

/* Struct that represents the 2D array by storing the width and height as well
 * as the size of each element and the 1D array that contains all of the 
 * elements.
 */
struct T {
        int width;
        int height;
        int size;
        UArray_T arr;
};


/*                                 UArray2_new
 *
 * Takes the desired width and height of a 2-Dimensional array and the size of
 * the elements to be stored. Creates a new, two-dimensional array with 
 * dimensions width x height , whose indicies store elements of size "size" 
 * by first allocating memory for the struct and storing the width, height and 
 * size. The 2D array is then represented by a 1D array with the same number of 
 * indices as the 2D representation. Returns the array.
 */
T UArray2_new(int width, int height, int size)
{
        assert(width != 0 && height != 0);
        assert(size != 0);

        int length = width * height;
            UArray2_T array2 = malloc(sizeof(*array2));
            assert(array2 != NULL);

            UArray_T array1 = UArray_new(length, size);

            array2->width = width;
            array2->height = height;
            array2->size = size;
            array2->arr = array1;

            return array2;
}

/*                                 UArray2_free
 *
 * Takes a pointer to a UArray2_T and first frees the 1D UArray that holds all
 * of the elements and then frees the memory used by the 2D array struct.
 */
void UArray2_free(T *array)
{
        assert(array != NULL && *array != NULL);

        UArray_free(&((*array)->arr));
        free(*array);       
}


/*                                UArray2_T
 *
 * Takes a UArray2_T, and the desired index represented as [row, col]. Checks 
 * that the desired indices are in bounds and returns a pointer to the element 
 * at that index. 
 */
void *UArray2_at(T array, int col, int row)
{
        assert(array != NULL);
                assert(col >= 0 && col < (array->width));
                assert(row >= 0 && row < (array->height));

        int index = ((row * array->width) + col);
        void *tmp = UArray_at(array->arr, index);

        return tmp;
}

/*                         UArray2_width
 *
 * Takes a UArray2_T and returns the width of that array.
 */
int UArray2_width (T array)
{
        assert(array != NULL);

        return array->width;
}

/*                        UArray2_height
 *
 * Takes a UArray2_T and returns the height of that array. 
 */
int UArray2_height (T array)
{
        assert(array != NULL);

        return array->height;
}

/*                            UArray2_size
 *
 * Takes a UArray_T  and returns the size of the elements stored in that array.
 */
int UArray2_size (T array)
{
        assert(array != NULL);

        return array->size;
}


/*                       UArray2_map_row_major
 *
 * Takes a UArray2_T, an apply function and a pointer to the closure. It goes 
 * through the two-dimensional array by row and applies the apply function
 * to each element.
 */ 
void UArray2_map_row_major (T array, void apply (int col, int row, 
                             T array, void *element, void *cl), void * cl)
{
        assert(array != NULL);

        int width = array->width;
        int height = array->height;
        
            for (int i = 0; i < height; i++) {
                for (int j = 0; j < width; j++) {
                        apply(j, i, array, UArray2_at(array, j, i), cl);
                }
            }
}

/*                       UArray2_map_col_major
 *
 * Takes a UArray2_T, an apply function and a pointer to the closure.Goes 
 * through the two-dimensional array by column and applies the apply 
 * function to each element.
 */
void UArray2_map_col_major (T array, void apply (int col, int row,
                             T array, void *element, void *cl), void * cl)
{
        assert(array != NULL);

            int width = array->width;
        int height = array->height;
        
            for (int i = 0; i < width; i++) {
                for (int j = 0; j < height; j++) {
                        apply(i, j, array, UArray2_at(array, i, j), cl);
                }
            }
}