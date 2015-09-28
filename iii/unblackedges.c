/* unblackedges.c
 * by Katherine Hoskins and Amoses Holton
 * Assignment 2
 *
 * This program takes a pbm formatted file containing a black and white image of
 * pixels. It removes all "black edge pixels" from the file according to the 
 * deifintion of black edge pixel:
 *             - Any pixel that is black and lies on the edge of the image
 *             - Any pixel that is adjacent to a black edge pixel
 * Black edge pixels are changed to white, and the correct image is printed to 
 * stdout in plain pbm format. The resulting image will have the same size as
 * the orignal, but all black edge pixels will have been changed to white. 
 */

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "bit2.h"
#include "except.h"
#include "assert.h"
#include "stack.h"
#include "pnmrdr.h"

/* Struct that holds coordinates for a particular element of the 2D array.*/
struct Pos_Data {
        int row;
        int col;
};

/* Function reads the data contained in the provided file and uses a helper 
 * function to populate a 2-Dimensional array with the data.
 */
void read_data(FILE *srcfile, Bit2_T *array);

/* Function reads the next unread Bit from the Pnmrdr which is passed as the 
 * closure and stores that bit in the 2D array at element [row, col].
 */
void fill_array(int col, int row, Bit2_T array, int Bit, void *cl);

/* Function locates the pixels that are located at the very top and bottom of
 * the original image.
 */
void find_row_edges(int col, int row, Bit2_T array, int Bit, void *cl);

/* Function locates the pixels that are located at the very left and right of
 * the original image. 
 */
void find_col_edges(int col, int row, Bit2_T array, int Bit, void *cl);

/* Function is passed an initial black edge pixels and locates all connected 
 * black edge pixels. When it finds a black pixel, the pixel is changed to 
 * white. 
 */
void find_black_pixels(int init_col, int init_row, Bit2_T array);

/* Function prints the header for a plain pbm file and the data stored in the
 * 2D array to stdout in the proper format. 
 */
void pbmwrite(Bit2_T array);

/* Function prints out the data in the 2D array in plain pbm format to stdout */
void print_array_element(int col, int row, Bit2_T array, int Bit, void *cl);


/*                                 main()
 *
 * Main function for the program. The user may either pass in a file to be read
 * as an argument or via stdin. The function coordinates the reading and 
 * processing of the file as well as the printing of the final result. 
 */
int main(int argc, char *argv[])
{
        FILE *srcfile;
        Bit2_T file_array = NULL;

        if (argc > 2) {
                fprintf(stderr, "Error: too many arguments \n");
                exit(EXIT_FAILURE);
        }
        else if (argc == 1) {

                srcfile = stdin;
                assert(srcfile != NULL);
        }
        else {
                srcfile = fopen (argv[1], "rb");
                assert(srcfile != NULL);
        }
        
        read_data(srcfile, &file_array);
        
        Bit2_map_row_major(file_array, find_row_edges, NULL);
        Bit2_map_col_major(file_array, find_col_edges, NULL);

        pbmwrite(file_array);
        Bit2_free(&file_array);
        fclose(srcfile);
}

/*                                 read_data()
 *
 * Function takes a pointer to an open file containing the data to be read and
 * a pointer to an already initiated 2D array to store the data. It uses the 
 * Pnmrdr interface to ensure that the file is of the proper pbm format and uses
 * a mapper function to read each pixel into the array. 
 */
void read_data(FILE *srcfile, Bit2_T *array)
{
        void *reader = NULL;
        Pnmrdr_mapdata data;
        int width = 0;
        int height = 0;

        reader = Pnmrdr_new(srcfile);
        data = Pnmrdr_data(reader); 

        if (data.type != Pnmrdr_bit) {
                fprintf(stderr, "Error: Incorrect file format. Aborting.\n");
                exit(EXIT_FAILURE);
        }
        
        width = data.width;
        height = data.height;

        *array = Bit2_new(width, height);
        Bit2_map_row_major(*array, fill_array, reader);

        Pnmrdr_free((Pnmrdr_T *)&reader);
}

/*                                 fill_array()
 *
 * Function serves as the apply function that gets the next unread bit from an
 * open file (pointed to by the closure) and stores that Bit in the 2D array at
 * index [row, col].
 */
void fill_array(int col, int row, Bit2_T array, int Bit, void *cl)
{
        (void) Bit;

        int temp = Pnmrdr_get(cl);
        Bit2_put(array, col, row, temp);
}

/*                                 find_row_edges()
 *
 * Function serves as the apply function that finds the coordinates of each 
 * pixel in row 0 and row height-1. If the edge pixel is black, it calls a 
 * helper function to find all other black edge pixels that are connected to it.
 */
void find_row_edges(int col, int row, Bit2_T array, int Bit, void *cl)
{
      (void) cl;

      int height = Bit2_height(array);

      if (row == 0 || row == (height - 1)) {
              if (Bit == 1) {
                      find_black_pixels(col, row, array);
              }
      }
}

/*                                 find_col_edges()
 *
 * Function serves as the apply function that finds the coordinates of each 
 * pixel in col 0 and col width-1. If the edge pixel is black, it calls a 
 * helper function to find all other black edge pixels that are connected to it.
 */
void find_col_edges(int col, int row, Bit2_T array, int Bit, void *cl)
{
      (void) cl;

      int width = Bit2_width(array);

      if (col == 0 || col == (width - 1)) {
              if (Bit == 1) {
                      find_black_pixels(col, row, array);
              }
      }
}

/*                                 find_black_pixels()
 *
 * Function is given the coordinates to an initial black edge pixels and finds
 * any black edge pixels connected to that initial pixel using a while loop and
 * a stack. When a black pixel is found, it is immediately changed to white. 
 * Then the program checks the four pixels adjacent to the current pixel to 
 * determine if they are in bounds and also black. If one adjacent pixel is
 * found to be black, the current pixel's location is pushed to the stack and
 * the process is repeated for that new pixel. If no adjacent pixels are in
 * bounds or black and the current pixel is not the initial black edge pixel,
 * the stack is popped to find the last pixel that was checked and the process
 * continues for that pixel. When the current pixel is back to the initial pixel
 * the loop breaks and the stack is freed to avoid memory leaks. 
 */
void find_black_pixels(int init_col, int init_row, Bit2_T array)
{
        Stack_T stack = Stack_new();
        struct Pos_Data temp;
        int popped_val = 0;
        int height = Bit2_height(array);
        int width = Bit2_width(array);
        int curr_col = init_col;
        int curr_row = init_row;

        while (1) {
                struct Pos_Data *coords = NULL;
                coords = malloc(sizeof(temp));
                assert(coords != NULL);
                (*coords).row = curr_row;
                (*coords).col = curr_col;

                if ((Bit2_get(array, curr_col, curr_row) == 1) ||
                    popped_val == 1 ) {
                        Bit2_put(array, curr_col, curr_row, 0);
                        popped_val = 0;
                        if ((curr_row - 1) >= 0 && 
                             (Bit2_get(array, curr_col, curr_row - 1) == 1)) {
                                Stack_push(stack, coords);
                                curr_row = curr_row - 1;
                                continue; 
                        } 
                        if ((curr_col + 1) < width && 
                             (Bit2_get(array, curr_col + 1, curr_row) == 1)) {
                                Stack_push(stack, coords);
                                curr_col = curr_col + 1;
                                continue; 
                        } 
                        if ((curr_row + 1) < height && 
                             (Bit2_get(array, curr_col, curr_row + 1) == 1)) {
                                Stack_push(stack, coords);
                                curr_row = curr_row + 1;
                                continue; 
                        } 
                        if ((curr_col - 1) >= 0 && 
                             (Bit2_get(array, curr_col - 1, curr_row) == 1)) {
                                Stack_push(stack, coords);
                                curr_col = curr_col - 1;
                                continue; 
                        }
                        if (curr_row == init_row && curr_col == init_col) {
                                free(coords);
                                break;
                        }
                }
                if (curr_row == init_row && curr_col == init_col) {
                        free(coords);
                        break;
                }
                free(coords);
                coords = (struct Pos_Data *)Stack_pop(stack);
                curr_row = (*coords).row;
                curr_col = (*coords).col;
                popped_val = 1;
                free(coords);
        }
        Stack_free(&stack);
}

/*                                  pbmwrite()
 *
 * Function takes a 2D array and prints out the appropriate header information
 * for the data in plain pbm format. Then the function maps a print function 
 * over the elements in the array in row-major order to print the actual data
 * to stdout. 
 */
void pbmwrite(Bit2_T array)
{
        int width = Bit2_width(array);
        int height = Bit2_height(array);
    
        fprintf(stdout, "P1\n");
        fprintf(stdout, "%u %u\n", width, height);
        Bit2_map_row_major(array, print_array_element, NULL);
}


/*                                print_array_element()
 *
 * Function serves as the apply function for the printing process. It prints
 * the current Bit to stdout. To follow plain pbm format the function checks
 * if the current bit is the last in its row or the 70th line character 
 * (including spaces), in which case it prints a newline. 
 */
void print_array_element(int col, int row, Bit2_T array, int Bit, void *cl)
{
        (void) cl;
        (void) array;
        (void) row;
    
        int width = Bit2_width(array);
        int temp = fprintf(stdout, "%u", Bit);
        assert(temp == 1);

        if (col == width - 1 || (col + 1) % 35 == 0) {
                fprintf(stdout, "\n");
                return;
        }

        fprintf(stdout, " ");
}
