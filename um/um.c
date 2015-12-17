/* Forrest Butler and Amoses Holton
 * Assignment 6 
 * 11/14/15
 *
 * Implementation for UM, allows a user to run the UM with the
 * provided instructions. Contains 8 registers that the 
 * instructions make use of.
 */
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "segment.h"
#include "instructions.h"
#include <bitpack.h>
#include <assert.h>
#define A_LSB 6
#define B_LSB 3
#define C_LSB 0
#define OPCODE_LSB 28
#define VAL_LSB 0
#define REG_ID_LEN 3
#define WORD_LEN 32
#define OPCODE_LEN 4
#define VAL_LENGTH 25
#define WORD_SIZE uint32_t
#define REG_SIZE uint32_t
#define T UM_T

/* Struct that holds contents of a UM, the segmented
 * memory, registers and program counter */
struct T {
        Segment_T memory;
        REG_SIZE *registers;
        WORD_SIZE prog_counter;
};
typedef struct T *T;

/* Creates a new universal machine, initializes the memory and all of the 
 * registers to 0.
 */
T UM_new();

/* Takes a um executable file and runs the program on the um.  */
void UM_run(T um, const char *input);

/* Frees the universal machine and all of its components. */
void UM_free(T *um);

/* Returns the file size (number of words). */
size_t get_file_size(const char *filename);

/* Loads the file's words into the 0th segment */
void load_file(T um, int size, const char *input);

/* Breaks apart the word to determine registers and opcode in order to
 * use the corerct instruction.
 */
void route_instruct(T um, WORD_SIZE instruct);


int main(int argc, char *argv[])
{
        if (argc > 2) {
                fprintf(stderr, "Error: too many arguments\n");
                exit(EXIT_FAILURE);
        }
        else if (argc == 2) {
                UM_T um = UM_new();
                UM_run(um, argv[1]);
                UM_free(&um);
        }
        else {
                fprintf(stderr, "Error: too few arguments\n");
                exit(EXIT_FAILURE);
        }
        return 1;
}

/* Creates a new universal machine, initializes the memory and all of the
 * registers to 0.
 */
T UM_new()
{
        T um = malloc(sizeof(struct UM_T));
        um->memory = Segment_new();
        um->registers = calloc(8, sizeof(WORD_SIZE));
        um->prog_counter = 0;
        return um;
}

/* Takes a um executable file, loads the program and runs each instuction
 * in segment 0 until a halt is reached. 
 */
void UM_run(T um, const char *input)
{
        WORD_SIZE curr_inst;
        int size = get_file_size(input);

        assert(Segment_map(um->memory, size) == 0);

        load_file(um, size, input);

        while(true) {
                curr_inst = Segment_load(um->memory, 0, um->prog_counter);
                route_instruct(um, curr_inst);
                um->prog_counter += 1;
        }
       
        return;
}

/* Retrieves the opcode, registers and any other information in order
 * to perform the correct instruction. If instuction doesn't exist, exits
 * with exit failure.
 */
void route_instruct(T um, WORD_SIZE instruct)
{
        short opcode = Bitpack_getu(instruct, OPCODE_LEN, OPCODE_LSB);
        short a_id = Bitpack_getu(instruct, REG_ID_LEN, A_LSB);
        short b_id = Bitpack_getu(instruct, REG_ID_LEN, B_LSB);
        short c_id = Bitpack_getu(instruct, REG_ID_LEN, C_LSB);
        REG_SIZE *a_ptr = &((um->registers)[a_id]);
        REG_SIZE *b_ptr = &((um->registers)[b_id]);
        REG_SIZE *c_ptr = &((um->registers)[c_id]);
        WORD_SIZE val;

        switch (opcode) {
                case 0:
                        conditional_move(a_ptr, b_ptr, c_ptr);
                        break;
                case 1:
                        load(um->memory, a_ptr, b_ptr, c_ptr);
                        break;
                case 2:
                        store(um->memory, a_ptr, b_ptr, c_ptr);
                        break;
                case 3:
                        add(a_ptr, b_ptr, c_ptr);
                        break;
                case 4:
                        multiply(a_ptr, b_ptr, c_ptr);
                        break;
                case 5:
                        divide(a_ptr, b_ptr, c_ptr);
                        break;
                case 6:
                        nand(a_ptr, b_ptr, c_ptr);
                        break;
                case 7:
                        UM_free(&um);
                        halt();
                        break;
                case 8:
                        map_segment(um->memory, b_ptr, c_ptr);
                        break;
                case 9:
                        unmap_segment(um->memory, c_ptr);
                        break;
                case 10:
                        output(c_ptr);
                        break;
                case 11:
                        input(c_ptr);
                        break;
                case 12:
                        load_program(um->memory, b_ptr, c_ptr);
                        /* Since the value is incremented upon return */
                        um->prog_counter = *c_ptr - 1;
                        break;
                case 13:
                        val = Bitpack_getu(instruct, VAL_LENGTH, VAL_LSB);
                        a_id = Bitpack_getu(instruct, REG_ID_LEN, 
                                            OPCODE_LSB - REG_ID_LEN);
                        a_ptr = &((um->registers)[a_id]);
                        load_value(a_ptr, val);
                        break;
                default:
                        fprintf(stderr, "Invalid Instruction.\n");
                        UM_free(&um);
                        exit(EXIT_FAILURE);
        }
}

/* Takes a filename and checks the file size, if it is a valid um 
 * file, then the number of words in the file is returned.
 */
size_t get_file_size(const char *filename)
{
        struct stat st;
        int size;
        if (stat(filename, &st) != 0) {
                fprintf(stderr, "Incompatible File Size.\n");
                exit(EXIT_FAILURE);
        }
        
        size = st.st_size;

        if (size % 4 != 0) {
                fprintf(stderr, "Incompatible File Size.\n");
                exit(EXIT_FAILURE);
        }
        return size / 4;
}

/* Takes a file name, opens the file and reads a word at a 
 * time in order to store it in segment 0.
 */
void load_file(T um, int size, const char *input)
{
        WORD_SIZE word = 0;
        WORD_SIZE num_words = size;
        unsigned char c;
        FILE *fp = fopen(input, "rb");
        for (WORD_SIZE i = 0; i < num_words; i++) {
            c = getc(fp);
            word = Bitpack_newu(word, 8, 24, (uint64_t)c);
            c = getc(fp);
            word = Bitpack_newu(word, 8, 16, (uint64_t)c);
            c = getc(fp);
            word = Bitpack_newu(word, 8, 8, (uint64_t)c);
            c = getc(fp);
            word = Bitpack_newu(word, 8, 0, (uint64_t)c);
            
            Segment_store(um->memory, 0, i, word);
        }
        fclose(fp);
}

/* Frees the universal machine and all of its components. */
void UM_free(T *um)
{
        Segment_free(&((*um)->memory));
        free((*um)->registers);
        free(*um);
}
