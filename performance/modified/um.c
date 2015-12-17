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
#define REG_ID_LEN 3
#define WORD_LEN 32
#define OPCODE_LEN 4
#define VAL_LENGTH 25
#define A_LSB 6
#define B_LSB 3
#define C_LSB 0
#define OPCODE_LSB 28
#define VAL_LSB 0
#define LOAD_VAL_LSB OPCODE_LSB - REG_ID_LEN
#define WORD_SIZE uint32_t
#define REG_SIZE uint32_t
#define T UM_T

/* Global function pointer arrays */
void (*three_reg[7]) (Segment_T seg_memory, REG_SIZE *reg_a, REG_SIZE *reg_b, 
                    REG_SIZE *reg_c);
void (*two_reg[5]) (Segment_T seg_memory, REG_SIZE *reg_b, REG_SIZE *reg_c);

WORD_SIZE *prog_copy;

/* Struct that holds contents of a UM, the segmented
 * memory, registers and program counter */
struct T {
        Segment_T memory; 
        REG_SIZE *registers;
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

/* Initializes the global function pointer array for opcode 0-6 */
void function_array_init();

/* Loads the segment identified by reg_b and makes a duplicate to replace
 * contents of segment 0 (which is abandoned). Program pointer is set to point
 * to the segment stored at segment 0 at the word offset of reg_c.
 */
void load_program(Segment_T seg_memory, REG_SIZE *reg_b, REG_SIZE *reg_c);


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
        um->registers = calloc(8, sizeof(REG_SIZE));
        function_array_init();
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

        while (true) {
                curr_inst = *prog_copy;
                route_instruct(um, curr_inst);
                prog_copy += 1;
                curr_inst = *prog_copy;
                route_instruct(um, curr_inst);
                prog_copy += 1;
                curr_inst = *prog_copy;
                route_instruct(um, curr_inst);
                prog_copy += 1;
        }
        return;
}

/* Retrieves the opcode, registers and any other information in order
 * to perform the correct instruction. If instuction doesn't exist, exits
 * with exit failure.
 */
void route_instruct(T um, WORD_SIZE instruct)
{
        WORD_SIZE opcode = instruct;
        opcode = opcode << (WORD_LEN - (OPCODE_LSB + (OPCODE_LEN)));
        opcode = opcode >> (WORD_LEN - (OPCODE_LEN));

        if (opcode < 7) {

                WORD_SIZE a_id = instruct;
                a_id = a_id << (WORD_LEN - (A_LSB + (REG_ID_LEN)));
                a_id = a_id >> (WORD_LEN - (REG_ID_LEN));
            
                WORD_SIZE b_id = instruct;
                b_id = b_id << (WORD_LEN - (B_LSB + (REG_ID_LEN)));
                b_id = b_id >> (WORD_LEN - (REG_ID_LEN));

                WORD_SIZE c_id = instruct;
                c_id = c_id << (WORD_LEN - (C_LSB + (REG_ID_LEN)));
                c_id = c_id >> (WORD_LEN - (REG_ID_LEN));

                REG_SIZE *a_ptr = &((um->registers)[a_id]);
                REG_SIZE *b_ptr = &((um->registers)[b_id]);
                REG_SIZE *c_ptr = &((um->registers)[c_id]);
                (*three_reg[opcode]) (um->memory, a_ptr, b_ptr, c_ptr);
        }

        else if (opcode > 7 && opcode < 13) {
                WORD_SIZE b_id = instruct;
                b_id = b_id << (WORD_LEN - (B_LSB + (REG_ID_LEN)));
                b_id = b_id >> (WORD_LEN - (REG_ID_LEN));

                WORD_SIZE c_id = instruct;
                c_id = c_id << (WORD_LEN - (C_LSB + (REG_ID_LEN)));
                c_id = c_id >> (WORD_LEN - (REG_ID_LEN));

                REG_SIZE *b_ptr = &((um->registers)[b_id]);
                REG_SIZE *c_ptr = &((um->registers)[c_id]);

                (*two_reg[opcode - 8]) (um->memory, b_ptr, c_ptr);

                if (opcode == 12) {                
                        prog_copy = prog_copy + *c_ptr - 1;
                }
        }
            
        else{
                WORD_SIZE val;
                switch (opcode) {
                        case 7:
                                UM_free(&um);
                                halt();
                                break;
                        case 13:
                                val = instruct;
                                val = val << (WORD_LEN - (VAL_LSB + 
                                                                (VAL_LENGTH)));
                                val = val >> (WORD_LEN - (VAL_LENGTH));

                                WORD_SIZE a_id = instruct;
                                a_id = a_id << (WORD_LEN - (LOAD_VAL_LSB + 
                                                                (REG_ID_LEN)));
                                a_id = a_id >> (WORD_LEN - (REG_ID_LEN));

                                REG_SIZE *a_ptr = &((um->registers)[a_id]);
                                load_value(a_ptr, val);
                                break;
                        default:
                                fprintf(stderr, "Invalid Instruction.\n");
                                UM_free(&um);
                                exit(EXIT_FAILURE);
                }
            
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
        prog_copy = Segment_ptr(um->memory, 0);
        fclose(fp);
}

void function_array_init()
{
        three_reg[0] = conditional_move;
        three_reg[1] = load;
        three_reg[2] = store;
        three_reg[3] = add;
        three_reg[4] = multiply;
        three_reg[5] = divide;
        three_reg[6] = nand;
        two_reg[0] = map_segment;
        two_reg[1] = unmap_segment;
        two_reg[2] = output;
        two_reg[3] = input;
        two_reg[4] = load_program;

}
/* Frees the universal machine and all of its components. */
void UM_free(T *um)
{
        Segment_free(&((*um)->memory));
        free((*um)->registers);
        free(*um);
}

void load_program(Segment_T seg_memory, REG_SIZE *reg_b, REG_SIZE *reg_c)
{
        if (*reg_b != 0) {
                Segment_move(seg_memory, *reg_b, 0);        
        }  

        prog_copy = Segment_ptr(seg_memory, 0);
        (void) reg_c;
}

