/********************************************************
bitpack.c
Amoses Holton & Forrest Butler
Assignment 4 Arith
10/22/15
bitpack.c is an interface used to store bit-level code words in signed
or unsigned integer types, through the use of setting and extracting 
fields using individual or mulitple bits in Big-endian order. 
*******************************************************/


#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "assert.h"
#include <stdbool.h>
#include <stdint.h>
#include "except.h"
#include "stdint.h"
#include <inttypes.h>

/* Exception for modifying word with a overly wide value */
Except_T Bitpack_Overflow = {"Overflow packing bits"};

/* All expansion macros for switching codeword size */
#define SIGNED_TYPE int64_t
#define UNSIGNED_TYPE uint64_t
#define TYPE_SIZE 64

/*                     Bitpack_fitsu
 *
 * Returns whether an unsigned integer of given type can fit
 * its significant bits in the given width.
 */
bool Bitpack_fitsu(UNSIGNED_TYPE n, unsigned width)
{
        assert(width <= TYPE_SIZE);
        int offset = TYPE_SIZE - width;
        UNSIGNED_TYPE temp = n;
        temp = temp << offset;
        temp = temp >> offset;
        if (temp == n && width != 0) {
                return true;
        }
        else {
                return false;
        }

}

/*                        Bitpack_fitss
 *
 * Returns whether a signed integer of given type can fit
 * its significant bits in the given width.
 */
bool Bitpack_fitss(SIGNED_TYPE n, unsigned width)
{
        assert(width <= TYPE_SIZE);
        int offset = TYPE_SIZE - width;
        SIGNED_TYPE temp = n;
        temp = temp << offset;
        temp = temp >> offset;
        if (temp == n && width != 0) {
                return true;
        }
        else {
                return false;
        }
}

/*                          Bitpack_getu
 * 
 * Returns an unsigned codeword representing the field of given width,
 * starting at the given least significant bit.
 */ 
UNSIGNED_TYPE Bitpack_getu(UNSIGNED_TYPE word, unsigned width, unsigned lsb)
{
        if (width == 0) {
                return 0;
        }
        assert(width <= TYPE_SIZE);
        assert(width + lsb <= TYPE_SIZE);
        UNSIGNED_TYPE temp = word;

        temp = temp << (TYPE_SIZE - (lsb + width));
        temp = temp >> (TYPE_SIZE - width);
        
        return temp;
}

/*                         Bitpack_gets
 * 
 * Returns a signed codeword representing the field of given width,
 * starting at the given least significant bit. 
 */ 
SIGNED_TYPE Bitpack_gets(UNSIGNED_TYPE word, unsigned width, unsigned lsb)
{
        if (width == 0){
                return 0;
        }
        assert(width <= TYPE_SIZE);
        assert(width + lsb <= TYPE_SIZE);
        SIGNED_TYPE temp = word;

        temp = temp << (TYPE_SIZE - (lsb + width));
        temp = temp >> (TYPE_SIZE - width);
        
        return temp;
}


/*                            Bitpack_newu
 *
 * Returns a the given word as an unsigned codeword with the given 
 * value overwritten starting at the least significant bit, assuming the 
 * value fits within the given width. 
 */
UNSIGNED_TYPE Bitpack_newu(UNSIGNED_TYPE word, unsigned width, unsigned lsb,
                                                         UNSIGNED_TYPE value)
{
        assert(width <= TYPE_SIZE);
        assert(width + lsb <= TYPE_SIZE);
        if (!Bitpack_fitsu(value, width)) {
                RAISE(Bitpack_Overflow);
        }

        UNSIGNED_TYPE orig_word = word;
        UNSIGNED_TYPE value_copy = value;
        UNSIGNED_TYPE mask = ~0;
        mask = mask << (TYPE_SIZE - (width + lsb));
        mask = mask >> (TYPE_SIZE - width);
        mask = mask << lsb;
        mask = ~mask;

        orig_word = orig_word & mask;
        value_copy = value_copy << lsb;
        orig_word = orig_word | value_copy;

        return orig_word;
}

/*                            Bitpack_news
 *
 * Returns a the given word as a signed codeword with the given 
 * value overwritten starting at the least significant bit, assuming the 
 * value fits within the given width. 
 */
UNSIGNED_TYPE Bitpack_news(UNSIGNED_TYPE word, unsigned width, unsigned lsb,
                                                             SIGNED_TYPE value)
{
        assert(width <= TYPE_SIZE);
        assert(width + lsb <= TYPE_SIZE);
        if (!Bitpack_fitss(value, width)) {
                RAISE(Bitpack_Overflow);
        }
        UNSIGNED_TYPE orig_word = word;
        UNSIGNED_TYPE value_copy = (UNSIGNED_TYPE) value;
        
        UNSIGNED_TYPE mask = ~0;
        mask = mask << (TYPE_SIZE - (width + lsb));
        mask = mask >> (TYPE_SIZE - width);
        mask = mask << lsb;
        mask = ~mask;

        orig_word = orig_word & mask;
        value_copy = value_copy << (TYPE_SIZE -  width);
        value_copy = value_copy >> (TYPE_SIZE - (width + lsb));
        orig_word = orig_word | value_copy;

        return orig_word;
}
