/* Forrest Butler and Amoses Holton
 * Assignment 6
 * 11/14/15
 * 
 * Implementation of segmented memory that allows a user to store and load 
 * words as well as map and unmap segments. 
 */
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <seq.h>
#include <assert.h>
#include <string.h>
#include "segment.h"
#define ID_SIZE uint32_t
#define WORD_SIZE uint32_t
#define MAP_INCREMENT 1000
#define T Segment_T

/* Struct that holds the unused ids and the set of segments that the client
 * uses.
 */
struct T {
        Seq_T unmapped_ids;
        WORD_SIZE **segments;
        unsigned num_segments;
};


/* Takes a file pointer to store the program at segment 0 and returns the newly
 * created segmented memory.
 */
T Segment_new()
{
        WORD_SIZE i;
        T seg_mem = malloc(sizeof(struct Segment_T));
        assert(seg_mem != NULL);
        Seq_T ids = Seq_new(MAP_INCREMENT);
        WORD_SIZE **segments = calloc(sizeof(WORD_SIZE*), MAP_INCREMENT);


        for (i = 0; i < MAP_INCREMENT; i++) {
                Seq_addhi(ids, (void *)(uintptr_t)i);
        }

        seg_mem->unmapped_ids = ids;
        seg_mem->segments = segments;
        seg_mem->num_segments = MAP_INCREMENT;

        return seg_mem;
}

/* Frees all segmented memory created by the program, including the segment
 * where the program is stored.
 */
void Segment_free(T * seg_memory) {
        int len = (*seg_memory)->num_segments;
        int i;

        for (i = 0; i < len; i++) {
                if ( (((*seg_memory)->segments)[i]) != NULL ) {
                        free( (((*seg_memory)->segments)[i]) );
                }
        }

        free((*seg_memory)->segments); 
        Seq_free(&((*seg_memory)->unmapped_ids));
        free (*seg_memory);
}

/* Creates a segment of desired size, intializes all words to 0 and returns the
 * identifier.
 */
ID_SIZE Segment_map(T seg_memory, unsigned size)
{
        /*Invariant at work here in the 'if' case */
        WORD_SIZE *seg = calloc(sizeof(WORD_SIZE), (size + 1));

        assert(seg != NULL);
        seg[0] = size;
        ID_SIZE id, i;
        WORD_SIZE **segments = seg_memory->segments;
        Seq_T unmapped = seg_memory->unmapped_ids;
        ID_SIZE unmapped_length = Seq_length(unmapped);
        ID_SIZE segs_length = seg_memory->num_segments;

        /* If there aren't any more unmapped id's, we replenish the unmapped id
         * sequence with MAP_INCREMENT amount.
         */
        if (unmapped_length == 0) {
                for (i = segs_length; i < segs_length + MAP_INCREMENT; i++) {
                        Seq_addhi(unmapped, (void *)(uintptr_t)i);

                }

                WORD_SIZE **temp = calloc (sizeof(WORD_SIZE*), 
                                                (segs_length + MAP_INCREMENT));
                memcpy(temp, segments, sizeof(WORD_SIZE*) * (segs_length));
                free (segments);

                assert (segments != NULL);
                seg_memory->segments = temp;
                segs_length = segs_length + MAP_INCREMENT;
                seg_memory->num_segments = segs_length;
        }
        id = (ID_SIZE)(uintptr_t)Seq_remlo(unmapped);

        (seg_memory->segments)[id] = seg;


        return id;
}

/* Unmaps identified segment from memory, allows unchecked runtime error if
 * segment doesn't exist.
 */
void Segment_unmap(T seg_memory, ID_SIZE id)
{

        Seq_addlo(seg_memory->unmapped_ids, (void *)(uintptr_t)id);
        free((seg_memory->segments)[id]);
        (seg_memory->segments)[id] = NULL;
}

/* Returns the word at the offset in the desired segment of memory*/
WORD_SIZE Segment_load(T seg_memory, ID_SIZE id, WORD_SIZE offset)
{
        WORD_SIZE *seg;
        seg = (seg_memory->segments)[id];
        return seg[offset + 1];
}

/* Stores the word at the specified offest in the desired segment of memory. */
void Segment_store(T seg_memory, ID_SIZE id, WORD_SIZE offset, WORD_SIZE word)
{
        WORD_SIZE *seg = (seg_memory->segments)[id];
        seg[offset + 1] = word;
}

/* Moves the segment identified by source to the target segment. The source
 * segment is duplicated and replaces the segment at the target ID.  
 */
void Segment_move(T seg_memory, ID_SIZE source, ID_SIZE target)
{
        WORD_SIZE *src = (seg_memory->segments)[source];
        WORD_SIZE size = src[0];
        Segment_unmap(seg_memory, target);
        assert (Segment_map(seg_memory, size) == target);
        WORD_SIZE *tar = (seg_memory->segments)[target];
        memcpy(tar, src, ((size + 1) * sizeof(WORD_SIZE)));
}

/* Returns a pointer to a desired segment located at source.*/
WORD_SIZE *Segment_ptr(T seg_memory, ID_SIZE source)
{
        return (seg_memory->segments)[source] + 1;   
}