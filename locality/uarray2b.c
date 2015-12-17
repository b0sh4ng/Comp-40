/* 
 * uarray2b.c
 * by Katherine Hoskins and Amoses Holton
 *
 * The file contains an implementation for a 2-Dimensional array in which the
 * elements are stored in blocks such that all elements of a single block are
 * stored near eachother in memory. The interface allows a client to create and
 * free 2D arrays, map a function over all the elements by block and access a  
 * single element. 
 *
 */

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "assert.h"
#include "uarray2b.h"
#include "uarray2.h"
#include "math.h"
#define T UArray2b_T

/* Struct that represents the 2D, blocked array. Contains the size of the 
 * elements stored in the array (in bytes), the number of cells that make up
 * one side of a single block (blocksize), and a single UArray2 object that will 
 * store the elements. There are two different height and width values stored,
 * one for the blocked array itself, and one for the dimensions of the contents
 * that a client might store in the array. These values are stored to handle the
 * case where the dimensions and blocksize that the client specifies creates
 * wasted space in some of the blocks. 
 */
struct T {
	int arr_width;
	int arr_height;
	int contents_height;
	int contents_width;
	int size;
	int blocksize;
	UArray2_T arr;
};

/*                             UArray2b_new()
 *
 * Function takes the desired dimensions, blocksize, and size of elements to be
 * stored. Based on these values it creates a new, two-dimensional array where
 * the elements are organized into blocks. The function calculates the 
 * appropriate dimensions of the blocked array based on the arguments the user
 * provides and creates a UArray2 object with those dimensions. The array as 
 * well as the dimension, size and blocksize values are stored in a struct that
 * is returned. 
 */
T UArray2b_new (int width, int height, int size, int blocksize)
{
        assert(width > 0 && height > 0);
	assert(size > 0);
	assert(blocksize >= 1);
	
	UArray2b_T blocked_arr = malloc(sizeof(*blocked_arr));
	
	int block_height_num = (int)ceil(( (double)height) / blocksize);
	int block_width_num =  (int)ceil(( (double)width) / blocksize);
	
	int num_cells_height = block_height_num * blocksize;
	int num_cells_width = block_width_num * blocksize; 
	
	UArray2_T array = UArray2_new(num_cells_width, num_cells_height , size);
	assert(array != NULL);
	
	blocked_arr->arr_width = num_cells_width;
	blocked_arr->arr_height = num_cells_height;
	blocked_arr->contents_width = width;
	blocked_arr->contents_height = height;
	blocked_arr->size = size;
	blocked_arr->blocksize = blocksize;
	blocked_arr->arr = array;
	
	return blocked_arr;
}

/*                             UArray2b_new_64K_block()
 * 
 * Function handles the case where the client would like to default the size of
 * each block in the array to 64KB. Takes the desired array dimensions and the
 * size of the elements to be stored, and from those values calculates the
 * appropriate blocksize. If a single element is larger than 64KB, the blocksize
 * is set to 1. The function creates the new array using UArray2b_new and 
 * returns the struct contiaining the array and its matching data that is 
 * returned from that function call. 
 */
T UArray2b_new_64K_block(int width, int height, int size)
{
        assert(width > 0);
	assert(height > 0);
	assert(size > 0);
	int blocksize = 0;
	double num_cells;
	
	if (size > 64000) {
	        blocksize = 1; 
	}
	else {
	        num_cells = 64000 / size;
	        blocksize = (int) floor(sqrt(num_cells));
	}

	if (width < blocksize || height < blocksize) {
		if (width > height) {
	        	blocksize = height;
	        }
	        else {
	        	blocksize = width;
	        }
	}
	
	UArray2b_T array = UArray2b_new(width, height, size, blocksize);
	
	return array;
}

/*                                   UArray2b_free()
 * 
 * Function takes a pointer to a UArray2b object and frees all memory that was 
 * allocated for that object. This memory includes the struct for the blocked
 * array, the struct for the underlying UArray2 object and the memory used for
 * the single UArray object that actually holds the elements. Returns no values.
 */
void  UArray2b_free  (T *array2b)
{
        assert(array2b != NULL && *array2b != NULL);
	UArray2_free(&(*array2b)->arr);
	free(*array2b);
}

/*                                  UArray2b_width()
 * 
 * Function takes a UArray2b object and returns the width of the entire 2D, 
 * blocked array.
 */
int   UArray2b_width (T array2b)
{
        assert(array2b != NULL);
	return array2b->contents_width; 
}

/*                                  UArray2b_height()
 * 
 * Function takes a UArray2b object and returns the height of the entire 2D, 
 * blocked array.
 */
int   UArray2b_height(T array2b)
{
        assert(array2b != NULL);
	return array2b->contents_height;
}

/*                                  UArray2b_size()
 * 
 * Function takes a UArray2b object and returns the size of the elements stored 
 * in that array, which is the size of each cell in each block. 
 */
int   UArray2b_size  (T array2b)
{
        assert(array2b != NULL); 
	return array2b -> size;
}

/*                                  UArray2b_blocksize()
 * 
 * Function takes a UArray2b object and returns the associated blocksize, which
 * is the number of elements that compose 1 side of a single block. 
 */
int   UArray2b_blocksize(T array2b)
{
        assert(array2b != NULL);
	return array2b -> blocksize; 
}

/*                                  UArray2b_at()
 *
 * Function takes a UArray2b object and the column (i) and row (j) of a desired
 * element in that array. The function returns a pointer to the element at that
 * location by calculating where in the underlying UArray2 the element is 
 * actually stored. 
 */
void *UArray2b_at(T array2b, int i, int j)
{
	
        assert(array2b != NULL);
	assert(i >= 0 && i < (array2b->contents_width));
	assert(j >= 0 && j < (array2b->contents_height));

	void *tmp = NULL;
	int blk_size, num_blocks, block_number, block_element, index, col, row;
	
	blk_size = array2b -> blocksize;

	if (blk_size == 1) {
	        tmp = UArray2_at(array2b->arr, i, j);
		return tmp;
	}
	
	num_blocks = (array2b -> arr_width)/(blk_size);

	block_number = ((j / blk_size * num_blocks) + (i / blk_size));
	block_element = ((j % blk_size) * blk_size) + (i % blk_size);
	
	index = block_number * (blk_size * blk_size) + block_element;
	col = index % (array2b->arr_width);
	row = index / (array2b->arr_width);

	tmp = UArray2_at(array2b->arr, col, row);
	
	return tmp;
}

/*                             UArray2b_map()
 *
 * Function takes a UArray2b object, an apply function with a particular 
 * signature and a pointer to a closure value. The function applies the apply
 * function to all elements in the array by block, meaning that it visits all of
 * the elements in a single block before moving on to the elements in the next
 * block. Returns no value. 
 */
void  UArray2b_map(T array2b, void apply(int i, int j, T array2b,void *elem, 
		   void *cl), void *cl)
{

	
        
        assert(array2b != NULL);
	int length = (array2b->arr_height) * (array2b->arr_width);
	int blk_size = array2b->blocksize;
	int width_in_blocks = (array2b->arr_width)/blk_size;
	int index, block_num, block_element, col, row;

	for (index = 0; index < length; index++) {
		block_num = index / (blk_size * blk_size);
		block_element = index - (block_num * blk_size * blk_size);
		
		col = ((block_num % width_in_blocks) * blk_size) + 
			block_element % blk_size;
		row = ((block_num / width_in_blocks) * blk_size) + 
			block_element / blk_size;		

		if (col >= (array2b->contents_width) || 
		     row >= (array2b->contents_height)) {
			continue; 
		}

		apply(col, row, array2b, UArray2b_at(array2b, col, row), cl);
	}
}