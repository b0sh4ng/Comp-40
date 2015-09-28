/* sudoku.c
 * by Katherine Hoskins and Amoses Holton
 * Assignment 2
 *
 * This program takes a pgm file containing a solved sudoku puzzle and checks
 * if the solution is valid or not according to the rules of sudoku. If the 
 * solution is correct, the program calls exit(0). Otherwise, it calls exit(1).
 */

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "pnmrdr.h"
#include "assert.h"
#include "uarray2.h"

/* Function takes an empty 2-Dimensional array to store the solution and a 
 * pointer to the source file. Checks that the solution format is correct and
 * reads the solution into the array. 
 */ 
void read_in_solution(UArray2_T array, FILE *srcfile);

/* Function takes the next unread element from the solution file and stores it
 * in the 2-Dimensional array at index [row, col]
 */
void fill_array(int col, int row, UArray2_T array, void *element, void *cl);

/* Function takes a 2-Dimensional array containing the solution to be checked 
 * and uses various helper functions to determine if the solution is valid.
 */
void check_solution(UArray2_T array);

/* Function checks that each row of the puzzle is valid (contains no repeated
 * elements).
 */
void row_check(int col, int row, UArray2_T array, void *element, void *cl);

/* Function checks that each column of the puzzle is valid (contains no
 * repeated elements).
 */
void col_check(int col, int row, UArray2_T array, void *element, void *cl);

/* Function checks that each of the 9 3x3 subsections of the puzzle is valid
 * (contains no repeated elements).
 */
void box_check(UArray2_T sol_arr, int col, int row, UArray2_T freq_arr);

/* Function takes a 2-Dimensional array and sets the current element to 0 */
void reset_to_zero(int col, int row, UArray2_T array, void *element, void *cl);

/* Function determines if the current element of the array has already been 
 * seen in the portion of the puzzle that is currently being checked. 
 */
void check_for_duplicate(UArray2_T sol_arr, UArray2_T freq_arr, void *element);

/*                                 main()
 *
 * Main function for the program. Takes a file containing the puzzle to be 
 * checked as an argument or from stdin and creates a new 9x9 2-Dimensional 
 * array to store the solution while it is being examined. Calls functions to 
 * read in the solution and check the puzzle. Calls exit(0) if the program
 * returns from the check function successfully to indicate that the puzzle was
 * indeed correct. 
 */
int main(int argc, char *argv[])
{
        FILE *srcfile;
        UArray2_T solution = UArray2_new(9, 9, sizeof(int));

        assert(argc <= 2);

        if (argc == 1) {

                srcfile = stdin;
                assert(srcfile != NULL);
        }
        else {
                srcfile = fopen (argv[1], "rb");
                assert(srcfile != NULL);
        }
        
        read_in_solution(solution, srcfile);
        fclose(srcfile);

        check_solution(solution);
        UArray2_free(&solution);
        exit(0);    
}

/*                                read_in_solution() 
 *
 * Function takes an empty 2-Dimensional array where the solution will be stored
 * for checking and a pointer to the file with the solution to be checked. Using
 * the Pnmrdr interface, it ensures that the format of the solution is correct
 * and uses a helper function to fill the 2-D array with the puzzle entries. 
 */ 
void read_in_solution(UArray2_T array, FILE *srcfile)
{

        void *reader = NULL;
        Pnmrdr_mapdata data;
        int width = 0;
        int height = 0;
        int max_intensity = 0;

        reader = Pnmrdr_new(srcfile);
        data = Pnmrdr_data(reader); 

        assert(data.type == Pnmrdr_gray);
        
        width = data.width;
        height = data.height;
        max_intensity = data.denominator;

        assert(max_intensity == 9);
        assert(width == 9 && height == 9);

        UArray2_map_row_major(array, fill_array, reader);
        Pnmrdr_free((Pnmrdr_T *)&reader);
}

/*                            fill_array()
 *
 * Function is mapped to each element of the 2-Dimensional array storing the 
 * sudoku solution. At each index of the array, it gets the next unread integer
 * from the puzzle using Pnmrdr and stores it in the array. 
 */
void fill_array(int col, int row, UArray2_T array, void *element, void *cl)
{
        (void) col;
        (void) row;
        (void) array;

        assert(sizeof(*(int *)element) == UArray2_size(array));

        int temp = Pnmrdr_get(cl);
        assert(temp > 0 && temp < 10);
        *(int *)element = temp;
}

/*                           check_solution()
 *
 * Function coordinates the solution checking process by calling helper 
 * functions that check each portion of the puzzle. Creates a new 9x1 array that
 * contains ones and zeros. If a 1 is stored in element i of the array, this 
 * indicates that the number i+1 has already been seen in that portion of the 
 * puzzle - and thus the solution is incorrect. The function also frees this
 * array at the end of the checking process to avoid memory leaks. 
 */
void check_solution(UArray2_T array)
{

        UArray2_T curr_line_arr = UArray2_new(9, 1, sizeof(int));
        
        UArray2_map_row_major(array, row_check, &curr_line_arr);
        UArray2_map_col_major(array, col_check, &curr_line_arr);

        for (int i = 0; i < 9; i += 3) {
                for (int j = 0; j < 9; j += 3) {
                        box_check(array, j, i, curr_line_arr);
                }
        }

        UArray2_free(&curr_line_arr);
}

/*                            row_check()
 *
 * Function is mapped to each element of the array in row-major order. For each
 * element it uses a helper function to check if that number has already been
 * seen in the current row. Resets the 9x1 frequency array to all 0's each time
 * a new row is examined. 
 */
void row_check(int col, int row, UArray2_T array, void *element, void *cl)
{
        (void) row;
        (void) array;
  
        assert(sizeof(*(int *)element) == UArray2_size(array));

        if (col == 0) {
                UArray2_map_row_major(*(UArray2_T *)cl, reset_to_zero, NULL);
        }

        check_for_duplicate(array, *(UArray2_T *)cl, element);
}

/*                            col_check()
 *
 * Function is mapped to each element of the array in column-major order. For
 * each element it uses a helper function to check if that number has already
 * been seen in the current column. Reesets the 9x1 frequency array to all 0's
 * each time a new column is examined. 
 */
void col_check(int col, int row, UArray2_T array, void *element, void *cl)
{
        (void) col;
        (void) array;
        
        assert(sizeof(*(int *)element) == UArray2_size(array));

        if (row == 0) {
                UArray2_map_row_major(*(UArray2_T *)cl, reset_to_zero, NULL);
        }

        check_for_duplicate(array, *(UArray2_T *)cl, element);
}

/*                             box_check()
 *
 * Function is passed the upper right hand corner of one of the 3x3 subsections
 * of the sudoku puzzle and loops through each element of the section using the
 * frequency array to determine if there are any repeated elements. Calls a 
 * helper function to perform the actual duplicate check. 
 */
void box_check(UArray2_T array, int col, int row, UArray2_T freq_arr)
{
        UArray2_map_row_major(freq_arr, reset_to_zero, NULL);

        for (int i = row; i < (row + 3); i++) {
                for (int j = col; j < (col + 3); j++) {
                        int *curr_num = UArray2_at(array, j, i);
                        assert(sizeof(*curr_num) == UArray2_size(array));
                        check_for_duplicate(array, freq_arr, (void *)curr_num);
                }
        }
}

/*                              reset_to_zero()
 *
 * Function is mapped to a 2-Dimensional array and replaces each element of the
 * array with a 0. 
 */
void reset_to_zero(int col, int row, UArray2_T array, void *element, void *cl)
{
        (void) col;
        (void) row;
        (void) cl;
        (void) array;

        assert(sizeof(*(int *)element) == UArray2_size(array));

        *(int *)element = 0;
}

/*                               check_for_duplicate()
 *
 * Function takes the 2-Dimensional array containing the solution being checked,
 * the 9x1 frequency array and a pointer to the current element in the solution
 * puzzle being checked. Looks at index (current element - 1) of the frequency
 * array to determine if the current element has already been seen in the 
 * current portion of the puzzle. Calls exit(1) if a duplicate is found. 
 */
void check_for_duplicate(UArray2_T sol_arr, UArray2_T freq_arr, void *element)
{
        int curr_num = *(int *)element;
        int *already_seen = NULL;

        already_seen = (int *)(UArray2_at(freq_arr, (curr_num - 1), 0));
        assert(sizeof(*already_seen) == UArray2_size(sol_arr));
        
        if (*already_seen == 0) {
                *already_seen = 1;
        }
        else {
                UArray2_free(&sol_arr);
                UArray2_free(&freq_arr);
                exit(1);
        }   
}
