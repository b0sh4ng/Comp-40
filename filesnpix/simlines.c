/* 
 * simlines.c
 * by Katie Hoskins and Amoses Holton
 * Assignment 1
 *
 * This program takes one or more files as arguments and locates similar lines
 * contained in the files. Similar lines are grouped into "match groups" which 
 * contain the line itself and information regarding where the matching lines
 * are located in the original files (both a filename and line number). After
 * finding match groups, the program reports the groups to the screen.
 *
 */
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "table.h"
#include "except.h"
#include "assert.h"
#include "readaline.h"
#include "atom.h"
#include "seq.h"

/* Function takes a pointer to a pointer to a table of key, value pairs, a 
 * pointer to an open file and a character array containing the name of the
 * file. Reads all lines in the file and adds them to the table as atoms along
 * with their location information. 
 */
void read_all_lines(Table_T **table, FILE *file, char * src_name);

/* Function takes a pointer to a pointer to a character array containing 
 * information from a single line in a file and removes any non word characters
 * and extra spaces from the array. 
 */
void remove_excess_chars(char **line, int size);

/* Function prints out match group data given a key, value and closure value. */
void print_values(const void *key, void **value, void *cl);

/* Function frees all the allocated memory stored in each row of a table */
void free_values (const void *key, void **value, void *cl);

/* Function takes a char array and a current position where a space is located
 * and removes multiple spaces and non-word characters from the array until a
 * new line, EOF or valid character is found. Returns the -1 if a new line or 
 * EOF is found and the index of the last space/invalid character if otherwise. 
 */
int handle_invalid_chars(char *curr_line, int first_space);

/* Checks if memory allocation is successful. */
Except_T Mem_Allocation_Error = { "Memory cannot be allocated" };

/* Struct holds location information for the origin of a particular line */
struct lineLocation {
        char *filename;
        int line_number;
};


/*                                  main
 *
 * Takes arguments from the command line which are the names of the files to be
 * examined for similar lines. Creates a new table that will store match groups
 * and reads all the lines from each file in the order that they appear on the
 * command line. After all lines have been inserted into the table, the function
 * maps a print function over all the elements in the table which reports the 
 * match groups and their locations in the files to the user. Finally, the 
 * function frees the table to avoid memory leaks. 
 */
int main(int argc, char *argv[])
{
        FILE *srcfile;
        Table_T lines_table = Table_new(200, NULL, NULL);
        Table_T *table_ptr = &lines_table;
        int first_match_group = 1;
        int *match_group_num = &first_match_group;

        if (argc == 1) {
                printf("Not enough arguments given");
        }
        
        for (int i = 1; i < argc; i++){
                srcfile = fopen(argv[i], "rb");

                assert(srcfile != NULL);

                read_all_lines(&table_ptr, srcfile, argv[i]);
                fclose(srcfile);
        }

        Table_map(lines_table, print_values, match_group_num);
        Table_map(lines_table, free_values, match_group_num);
        Table_free(table_ptr);
}


/*                           read_all_lines
 *
 * Function takes a pointer to a pointer to a created table, a pointer to an 
 * open file, and a character string containing the name of that file. The 
 * funciton loops through the file, handling each line of the file individually
 * until the EOF is reached. For each line, the function creates a new block of
 * memory that will contain the filename and line number of the current line
 * being read. The funciton uses readaline, which sets a pointer to a pointer to
 * a byte array containing the characters from the line that was just read. If
 * readaline returns that there are 0 characters in that array, the end of the
 * file has been reached and the process stops. Otherwise, extra non-word 
 * characters and spaces are removed from the array with a helper function and
 * the line location data is set. 

 * To store the line data in the table, first the line characters themselves
 * are stored in an atom, which is used as the key for the table. If the atom
 * alraedy exists in the table (meaning that an identical line has been read
 * before), the newest line location data is added onto the sequence that serves
 * as the value for the table. If the atom is not present yet, the function
 * creates a new sequence with the location data and adds it along with its key
 * to the table. 
 */
void read_all_lines(Table_T **table, FILE *file, char *src_name)
{
        int num_chars = 0;
        struct lineLocation temp;
        int line_num = 1;
        char *data = NULL;
        char **datap = &data;
        const char* line_atom;
        Seq_T curr_value;

        while (true){
                struct lineLocation *newest_line = NULL;

                newest_line = malloc(sizeof(temp));
                assert(newest_line != NULL);
                
                num_chars = readaline(file, datap);
                
                if (num_chars == 0){
                        free(newest_line);
                        break;
                }

                remove_excess_chars(datap, num_chars); 

                (*newest_line).filename = src_name;
                (*newest_line).line_number = line_num; 
               
                const char *atom_ptr = *datap;
               
                line_atom = Atom_string(atom_ptr);

                curr_value = Table_get(**table, line_atom);

                if (curr_value == NULL) {
                        Seq_T value_seq = Seq_new (1);

                        Seq_addhi(value_seq, newest_line);
                        
                        Table_put(**table, line_atom, value_seq);               
                }
                else {
                        Seq_addhi(curr_value, newest_line);
                }

                free(*datap);
                line_num++;
        }
        return;
}


/*                           remove_excess_chars
 * 
 * Function takes a pointer to a pointer to a byte array of characters that were
 * read from a line as well as the size of the array. The function examines each
 * character in the array and removes non-word characters and extra spaces. 
 * Valid characters and spaces are copied from the original array to a new byte
 * array, which is reallocated to be of the correct, new size. The original 
 * pointer is then set to point to this new array. 
 */
void remove_excess_chars(char **line, int size)
{
        int i = 0;
        int j = 0;
        char *line_array = NULL;
        char *old_array = *line;

        line_array = malloc (size);
        assert(line_array != NULL);

        while (i < size) {

                if (old_array[i] == '\n') {
                        break;
                }
                else if ((int)old_array[i] == 32) {
                        
                        i = handle_invalid_chars(old_array, i);
                        
                        if (i < 0) {
                                break;
                        }

                        line_array[j] = ' ';
                        j++;
                        i++;
                }
                else if ((int)old_array[i] == 95 ||
                      (((int)old_array[i]>= 48) && ((int)old_array[i]<= 57)) ||
                      (((int)old_array[i]>= 65) && ((int)old_array[i]<= 90)) ||
                      ((old_array[i]>= 97) && (old_array[i]<= 122))) {

                        line_array[j] = old_array[i];
                        j++;
                        i++;
                        continue;
                }
                else {
                        i = handle_invalid_chars(old_array, i);
                        if (i < 0) {
                                break;
                        }
                        if (((int)old_array[i - 1] == 95 ||
                           (((int)old_array[i - 1]>= 48) && 
                            ((int)old_array[i - 1]<= 57)) ||
                           (((int)old_array[i - 1]>= 65) && 
                            ((int)old_array[i - 1]<= 90)) ||
                            ((old_array[i - 1]>= 97) && 
                             (old_array[i - 1]<= 122))) && 
                             (old_array[i + 1] != '\0')){

                                line_array[j] = ' ';
                                j++;
                        }
                        i++;
                }
        }
        
        old_array = realloc(line_array, j+1);
        old_array[j] = '\0';
        free(*line);

        *line = old_array;
}


/*                             handle_invalid_chars
 *
 * Function takes a char array containing characters that were read from a file
 * and the index of the space character to be examined. The function loops 
 * through the subsequence indicies of the array until it finds a valid word 
 * character, at which point it returns the index right before that character. 
 * If the function loop reaches an EOF or new line character before finding a
 * valid character, it returns -1 to indicate that the end of the line was
 * reached in this function. 
 */
int handle_invalid_chars(char *curr_line, int first_space)
{
        int i = first_space;
        while (!(((int)curr_line[i + 1] == 95) ||
                (((int)curr_line[i + 1]>= 48) && 
                 ((int)curr_line[i + 1]<= 57)) || 
                (((int)curr_line[i + 1]>= 65) && 
                 ((int)curr_line[i + 1]<= 90)) ||
                 ((curr_line[i + 1]>= 97) && (curr_line[i + 1]<= 122)))) {

                if (curr_line[i+1] == '\0' || curr_line[i+1] == '\n') {
                        return -1;
                }
                i++;
        }
        return i;
}


/*                               print_values
 * 
 * Function is mapped to each element of a table, and prints out the match 
 * group data for each row of the table according to the specifications.  
 */
void print_values(const void *key, void **value, void *cl)
{

        Seq_T curr_seq = *((Seq_T *)value);
        const char *curr_key = key;
        int len = Seq_length(curr_seq);
        struct lineLocation *curr_line_info = NULL;
        
        if (len == 1) {
                return;
        } 

        if (*(int *)cl == 1) {
                printf("%s\n", curr_key);
                *(int *)cl = 0;
        }
        else {
                printf("\n%s\n", curr_key);
        }

        for (int i = 0; i < len; i++) {
                curr_line_info = Seq_get(curr_seq, i);
                
                printf("%-20s ", (*curr_line_info).filename);
                printf("%7u\n", (*curr_line_info).line_number);
        }
        return;
}

/*                             free_values
 *
 * Function is mapped to each element of a table and frees all the allocated 
 * memory contained in that table in order to avoid memory leaks. 
 */
void free_values (const void *key, void **value, void *cl)
{
        (void) key;
        (void) cl;
        Seq_T curr_seq = *((Seq_T *)value);
        int len = Seq_length(curr_seq);
        struct lineLocation *file_info = NULL;

        for (int i = 0; i < len; i++) {
                file_info = Seq_get(curr_seq, i);
                free(file_info);
         }

        Seq_T *temp = (Seq_T *) value;
        Seq_free(temp);
        free(*value);
        return;
}
