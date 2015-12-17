/* ppmtrans.c
 *
 * by Katherine Hoskins and Amoses Holton
 *
 * Program takes in an image in ppm format and transforms it according to the
 * user's specification. The image can be provided either as an argument on
 * the command line or via stdin. The user can specifiy which type of rotation
 * they would like to have performed as well as a preferred image storage 
 * method (block major, column major or row major). Choices for rotations are
 * 0, 90, 180 and 270 degrees. If no preferred storage method is given - the
 * program defaults to image storage by row. If no desired rotation is given,
 * the program defaults to a 0 degree rotation. The resulting rotation is
 * printed to stdout in the binary ppm format.  The program also times how much
 * CPU time it takes to rotate an image, and this data can be found in the
 * Timing_Info.txt file after running the program. If the transformation was
 * not successful, the program reports the reason for the failure and exits with
 * EXIT_FAILURE. If the program successfully performs the rotation, it exits 
 * with EXIT_SUCCESS after printing the image. 
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "assert.h"
#include "a2methods.h"
#include "a2plain.h"
#include "a2blocked.h"
#include "pnm.h"
#include "cputiming.h"
#define A2 A2Methods_UArray2

static void
usage(const char *progname)
{
	fprintf(stderr, "Usage: %s [-rotate <angle>] "
		"[-{row,col,block}-major] [filename]\n",
		progname);
	exit(1);
}

/* Function rotates the original image according to the users specifications.*/
Pnm_ppm transform_image (int rotation, Pnm_ppm original_image, 
			A2Methods_mapfun *map, A2Methods_T methods);

/* Apply function for the mapping method that rotates an image by 90 degrees*/
void rotate_90_degrees(int i, int j, A2 original_image, void *elem,
			 void *rotated_image);

/* Apply function for the mapping method that rotates an image by 180 degrees*/
void rotate_180_degrees(int old_col, int old_row, A2 original_image, void *elem,
			 void *rotated_image);

/* Apply function for the mapping method that rotates an image by 270 degrees*/
void rotate_270_degrees(int old_col, int old_row, A2 original_image, void *elem,
			 void *rotated_image);

/*                                 main()
 *
 * Main function for the program reads in an image from the command line or from
 * stdin, as well as any specifications the user provided for a storage method
 * and/or desired rotation. Calls helper functions to perform the proper
 * rotation on the image and prints the final image to stdout in the ppm format.
 */
int main(int argc, char *argv[]) 
{
        int i;
        int rotation = 0;
	FILE *srcfile;
 	Pnm_ppm original_image;
	Pnm_ppm transformed_image;

        /* default to UArray2 methods */
        A2Methods_T methods = uarray2_methods_plain; 
        assert(methods);

        /* default to best map */
        A2Methods_mapfun *map = methods->map_default; 
        assert(map);

#define SET_METHODS(METHODS, MAP, WHAT) do {                            \
                methods = (METHODS);                                    \
                assert(methods != NULL);                                \
                map = methods->MAP;                                     \
                if (map == NULL) {                                      \
                        fprintf(stderr, "%s does not support "          \
                                        WHAT "mapping\n",               \
                                argv[0]);                               \
                        exit(1);                                        \
                }                                                       \
        } while (0)

	for (i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "-row-major")) {
			SET_METHODS(uarray2_methods_plain, map_row_major,
				    "row-major");
		} else if (!strcmp(argv[i], "-col-major")) {
			SET_METHODS(uarray2_methods_plain, map_col_major,
				    "column-major");
		} else if (!strcmp(argv[i], "-block-major")) {
			SET_METHODS(uarray2_methods_blocked, map_block_major,
				    "block-major");
		} else if (!strcmp(argv[i], "-rotate")) {
			if (!(i + 1 < argc)) {      /* no rotate value */
				usage(argv[0]);
			}
			char *endptr;
			rotation = strtol(argv[++i], &endptr, 10);
			if (!(rotation == 0 || rotation == 90
			      || rotation == 180 || rotation == 270)) {
				fprintf(stderr, "Rotation must be "
					"0, 90 180 or 270\n");
				usage(argv[0]);
				exit(EXIT_FAILURE);
			}
			if (!(*endptr == '\0')) {    /* Not a number */
				usage(argv[0]);
			      	exit(EXIT_FAILURE);
			}
		} else if (*argv[i] == '-') {
			fprintf(stderr, "%s: unknown option '%s'\n", argv[0],
				argv[i]);
			exit(EXIT_FAILURE);
		} else if (argc - i > 1) {
			fprintf(stderr, "Too many arguments\n");
			usage(argv[0]);
		} else {
			break;
		}
	}
	
	if (i < argc) {
                srcfile = fopen (argv[i], "rb");
                assert(srcfile != NULL);
        }
        else {
                srcfile = stdin;
                assert(srcfile != NULL);
        }
	original_image = Pnm_ppmread (srcfile, methods);
  
        if (rotation == 0) {
                Pnm_ppmwrite(stdout, original_image);
                Pnm_ppmfree(&original_image);
        }
        else{
                transformed_image = transform_image (rotation, original_image, 
                                                     map, methods);
                Pnm_ppmwrite(stdout, transformed_image);        
                Pnm_ppmfree(&original_image);
                Pnm_ppmfree(&transformed_image);
        }
        fclose(srcfile);
	exit (EXIT_SUCCESS);
}


/*                               transform_image
 *
 * Function takes the desired rotation, the image to be transformed, the 
 * appropriate mapping function for the original image and a method suite. 
 * It coordinates rotation of the image based on the desired rotation and
 * handles the timing feature, only timing the mapping functions that are 
 * used to actually move pixels to the rotated image. It returns the new,
 * transformed image as a Pnm_ppm. 
 */
Pnm_ppm transform_image (int rotation, Pnm_ppm original_image, A2Methods_mapfun 
			*map, A2Methods_T methods)
{
        Pnm_ppm transformed_image = (Pnm_ppm) malloc(sizeof(*original_image));
	FILE * timing_info;
	int original_width = original_image->width;
	int original_height = original_image->height;
	int original_denominator = original_image->denominator;
	int RGB_size = sizeof(struct Pnm_rgb);
	CPUTime_T timer;
	double time_used;
	int num_pixels;

	timer = CPUTime_New();
	num_pixels = original_height * original_width;

	transformed_image->denominator = original_denominator;
	transformed_image->methods = methods;

	
        if (rotation == 90) {
                transformed_image->pixels = methods->new(original_height, 
			        	              original_width, RGB_size);
		transformed_image->height = original_width;
		transformed_image->width = original_height;
		
		CPUTime_Start(timer);
		map((original_image->pixels), rotate_90_degrees, 
                                                             transformed_image);
	       	time_used = CPUTime_Stop(timer);

        }
        else if (rotation == 180) {
                transformed_image->pixels = methods->new(original_width, 
                                                     original_height, RGB_size);
		transformed_image->height = original_height;
		transformed_image->width = original_width;
		
		CPUTime_Start(timer);
		map((original_image->pixels), rotate_180_degrees, 
                        transformed_image);
	       	time_used = CPUTime_Stop(timer);
        }
        else if (rotation == 270) {
	        transformed_image->pixels = methods->new(original_height,
                                                      original_width, RGB_size);
		transformed_image->height = original_width;
		transformed_image->width = original_height;

		CPUTime_Start(timer);
                map((original_image->pixels),rotate_270_degrees, 
                        transformed_image);
		time_used = CPUTime_Stop(timer);
	}  
	timing_info = fopen ("Timing_Info.txt", "w+");
	fprintf (timing_info, "Rotation of %u degrees completed in %.0f \
                             nanoseconds.\n", rotation, time_used);
	fprintf (timing_info, "Number of pixels = %u. Time per pixel = %.0f \
	                     nanoseconds.\n", num_pixels, time_used/num_pixels);
	fclose(timing_info);
        CPUTime_Free(&timer);
	return transformed_image;
}

/*                                rotate_90_degrees()
 *
 * Function serves as the apply function for the mapping function when the user
 * wants to rotate an image by 90 degrees. Based on the given column and row
 * information it takes, the function calculates where the current pixel should
 * reside in the transformed image and copies that pixel to the proper location
 * in the new image which is accessed via the closure. 
 */
void rotate_90_degrees(int old_col, int old_row, A2 original_image, void *elem,
                       void *rotated_image)
{
	(void) original_image;	
	int original_height, rotated_col, rotated_row;
    	Pnm_rgb curr_rgb = (Pnm_rgb)elem;
	Pnm_rgb destination;
	Pnm_ppm new_image = (Pnm_ppm)rotated_image;
	const struct A2Methods_T *methods = (new_image->methods);
	
	original_height = new_image->width;
	rotated_col = original_height - old_row - 1;
	rotated_row = old_col;
		
	destination = (Pnm_rgb)methods->at(new_image->pixels, rotated_col,
                                           rotated_row);
	*destination = *curr_rgb;
}

/*                                rotate_180_degrees() 
 *
 * Function serves as the apply function for the mapping function when the user 
 * wants to rotate an image by 180 degrees. Based on the given column and row
 * information it takes, it calculates where the current pixel should reside in
 * the transformed image and copies that pixel to the proper location in the 
 * new image (which is accessed via the closure).
 */
void rotate_180_degrees(int old_col, int old_row, A2 original_image, void *elem,
                        void *rotated_image)
{
	(void) original_image;
	int original_height, original_width, rotated_col, rotated_row;
        Pnm_rgb curr_rgb = (Pnm_rgb)elem;
	Pnm_rgb destination;
	Pnm_ppm new_image = (Pnm_ppm)rotated_image;
	
	const struct A2Methods_T *methods = (new_image->methods);
	
	original_height = methods->height(original_image);
	original_width = methods->width(original_image);
	
	rotated_col = original_width - old_col - 1;
	rotated_row = original_height - old_row - 1;
	
	destination = (Pnm_rgb)methods->at(new_image->pixels, rotated_col,
                                           rotated_row);
	*destination = *curr_rgb;
}

/*                                 rotate_270_degrees()
 *
 * Function serves as the apply function for the mapping function when the user
 * wants to rotate an image by 180 degrees. Based on the given column and row
 * information it takes, the function calculates where the current pixel should
 * be in the rotated image and copies that pixel to that location in the new
 * image (which is accessed via the closure). 
 */
void rotate_270_degrees(int old_col, int old_row, A2 original_image, void *elem,
                        void *rotated_image)
{
	(void) original_image;
	int original_width, rotated_col, rotated_row;
        Pnm_rgb curr_rgb = (Pnm_rgb)elem;
	Pnm_rgb destination;
	Pnm_ppm new_image = (Pnm_ppm)rotated_image;
	
	const struct A2Methods_T *methods = (new_image->methods);
	original_width = methods->width(original_image);
	
	rotated_col = old_row;
	rotated_row = original_width - old_col -1;
	
	destination = (Pnm_rgb)methods->at(new_image->pixels, rotated_col, 
                                           rotated_row);
	*destination = *curr_rgb;
}

