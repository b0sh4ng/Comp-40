/* 
 * brightness.c
 * by Amoses Holton and Katie Hoskins, 9.13.15
 * Assignement 1
 * 
 * This program takes an image that is in pgm format, calculates the
 * average brightness of that image and prints that value to the screen. 
 * The Pnmrdr interface found in Pnmrdr.h is used to read the actual pixel 
 * data from the image.
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "pnmrdr.h"
#include "except.h"

/* Uses Pnmrdr to catch image format exceptions, reads image header data and 
 * returns the average
 */
double average_image_brightness(FILE *srcfile);

/* Reads individual pixel data and returns total brightness*/
double total_pixel_brightness(void *source, double size, double denominator);


/*                               main
 *
 * Takes a single file as an argument or from stdin. Gives an error if more than
 * one argument is given. Calls helper functions to calculate the average 
 * brightness of the image.
 *
 */
int main(int argc, char *argv[])
{
        FILE *srcfile;
        double average = 0;

        if (argc > 2) {
                fprintf(stderr, "Error: too many arguments \n");
                exit(EXIT_FAILURE);
        }
        else if (argc == 1) {
                srcfile = stdin;
                if (srcfile == NULL) {
                        fprintf(stderr, "Error opening file \n");
                        exit(EXIT_FAILURE);
                }
        }
        else {
                srcfile = fopen (argv[1], "rb");
                if (srcfile == NULL) {
                        fprintf(stderr, "Error opening file \n");
                        exit(EXIT_FAILURE);
                }
        }
        
        average = average_image_brightness(srcfile);
        printf("%0.3f \n", average);
        fclose(srcfile);
}


/*                       
 *                       average_image_brightness
 *
 * Takes a pointer to an open image file, checks for any image format 
 * exceptions. Uses the Pnmrdr interface to read image header information.
 * Calls total_pixel_brightness to read individual pixel data and uses that
 * information to calculate average image brightness. Returns that value. 
 */
double average_image_brightness(FILE *srcfile)
{
        void *reader = NULL;
        Pnmrdr_mapdata data;
        double image_size = 0;
        double denominator = 0;
        double total = 0;
        double average = 0;

        TRY
                reader = Pnmrdr_new(srcfile);
                data = Pnmrdr_data(reader);
                
        EXCEPT(Pnmrdr_Badformat)
                fprintf(stderr, "Cannot read image, wrong format \n");
                Pnmrdr_free(reader);
                exit(EXIT_FAILURE);
        EXCEPT(Pnmrdr_Count) 
                fprintf(stderr, "Error reading pixels \n");
                Pnmrdr_free(reader);
                exit(EXIT_FAILURE);
        END_TRY;

        if (data.type != Pnmrdr_gray) {
                fprintf(stderr, "Not a .pgm file \n");
                Pnmrdr_free(reader);
                exit(EXIT_FAILURE);
        }
        
        image_size = data.width * data.height;
        denominator = data.denominator;
        total = total_pixel_brightness(reader, image_size, denominator);
        average = total / image_size;
        
        return average;
}


/*
 *                            total_pixel_brightness
 *
 * Takes a pointer to a Pnmrdr struct containing the original image information,
 * the total number of pixels in the image (calculated from the header) and the
 * denominator value for the image. Reads and sums the brightness values for 
 * each individual pixel and returns that total. 
 */
double total_pixel_brightness(void *source, double size, double denominator) 
{
        double running_total = 0;
        double temp = 0;
     
        for (int i = 0; i < size; i++) {
                temp = Pnmrdr_get(source);
                running_total += temp / denominator;
        }
        return running_total;  
}

