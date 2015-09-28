/* 
 * readaline.c
 * by Katie Hoskins and Amoses Holton, 9.16.15
 * Assignment 1
 *
 * This program takes a pointer to an open file and a pointer to a pointer to a 
 * block of memory. It reads the next unread line in the file, storing the 
 * characters in a Hansons Sequence. The elements of the sequence are then 
 * copied to a byte array. The pointer to a pointer that was passed as an
 * argument is set to point to the array and the size of the array is returned.
 *
 */
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "except.h"
#include "assert.h"
#include "readaline.h"
#include <seq.h>

/* Given a sequence and an open file, function fills sequence with the 
 * characters in the next unread line in the file. Returns false if reaches end 
 * of file and true if otherwise.
 */
bool populate_sequence(Seq_T seq, FILE *src_file);

/* Given a sequence and its size, creates a new byte array, copies sequence 
 * elements to the array and returns a pointer to the array.
 */
char *copy_to_array(Seq_T seq, int size);

/* Checks if the pointer to the file is null. */
Except_T Null_File = { "Source file does not exist" };

/* Checks if the pointer to a pointer is null. */
Except_T Null_Pointer = { "Pointer does not exist" };

/*                                readaline
 * Takes a pointer to a open file and a pointer to a pointer to a block of 
 * memory as arguments. Provided neither are null, creates a sequence and 
 * populates it using a helper function. Uses another helper function to copy
 * sequence elements to a byte array. Sets the pointer to a pointer to the 
 * byte array. Returns the length.
 *
 */
size_t readaline(FILE *inputfd, char **datap) 
{
        assert(inputfd != NULL);
        assert(datap != NULL);
        
        int len = 0;
        Seq_T seq_chars = Seq_new(200);

        if (populate_sequence(seq_chars, inputfd) == 0) {
                Seq_free(&seq_chars);
                *datap = NULL;
                return 0;               
        }

        len = Seq_length(seq_chars);
        char *array_chars = copy_to_array(seq_chars, len);

        *datap = array_chars;
                
        Seq_free(&seq_chars);

        return len;
}
 

/*                               populate_sequence
 * Takes a pointer to pointer to a sequence and a pointer to an open file as
 * arguments. Creates a 1 byte memory block using malloc and stores the next
 * unread character from the file in that location. Then adds a pointer to that
 * character to the end of the sequence. If the character is the newline 
 * character, the function returns true, indicating the entire line was read.
 * If the character is the EOF character, it is not stored in the sequence and
 * the function returns false.
 */
bool populate_sequence(Seq_T seq, FILE *src_file)
{               
        int i = 0;

        while (true) {
                char *tmp_char = NULL;
                tmp_char = malloc(1);
                assert(tmp_char != NULL);
                *tmp_char = fgetc(src_file);
                
                if (ferror(src_file)) {
                        printf("Error reading input file characters\n");
                }

                if (*tmp_char == EOF) {
                        free(tmp_char);
                        if (i == 0) {
                                return false;
                        }
                        else {
                                return true;
                        }
                        return true;
                }

                Seq_addhi(seq, tmp_char);
                i++;

                if (*tmp_char == '\n'){
                        return true;
                }
        }
        return false;
}


/*                             copy_to_array
 *
 * Takes a pointer to a pointer to a sequence and its size as the 
 * arguments. Allocates memory for a byte array of that size and copies all of 
 * elements from the sequence into the array. Returns a pointer to the byte
 * array.
 */
char *copy_to_array(Seq_T seq, int size)
{ 
        char * char_array = NULL;
                
        char_array = malloc(size+1);
        assert(char_array != NULL);

        for (int j = 0; j < size; j++) {
                char *copy_char = Seq_get(seq, j);
                char_array[j] = *copy_char;
                free( (char *)Seq_get(seq,j) );
        }

        char_array[size] = '\0';
        return char_array;
}
