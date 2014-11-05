/*
 * Practice code for ECE 521, Fall 2014
 * Project 2 - Branch Predictor Simulator
 *
 * Author: Aravindhan Dhanasekaran <adhanas@ncsu.edu>
 */
 
-------------------------------------------------------------------------------
/* Creating uint32_t masks */

include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

static inline unsigned
util_get_msb_mask(uint32_t num_msb_bits)
{
    return ((~0U) << (32 - num_msb_bits));
}

static inline unsigned
util_get_lsb_mask(uint32_t num_lsb_bits)
{
    return ((~0U) >> (32 - num_lsb_bits));
}

static inline unsigned
util_get_field_mask(uint32_t start_bit, uint32_t end_bit)
{
    return (util_get_lsb_mask(end_bit + 1) & (~util_get_lsb_mask(start_bit)));
}

int
main(void)
{
    int start_bit = 2;
    int end_bit = 6;

    printf("Unshifted mask of [%u..%u] is 0x%X\n", 
        start_bit, end_bit, util_get_field_mask(start_bit, end_bit));

    return 0;
}

-------------------------------------------------------------------------------
/* Enum practice */

#include <stdio.h>
#include <stdlib.h>

typedef enum bp_types__ { 
    BP_TYPE_1BIT        = 1,
    BP_TYPE_BIMODAL     = 2,
    BP_TYPE_GSELECT     = 3,
    BP_TYPE_GSHARE      = 4,
    BP_TYPE_HYBRID      = 5,
} bp_type;

int
main(int argc, char **argv)
{
    int type = 5;

    switch (type) {
    case BP_TYPE_BIMODAL:
        printf("Bimodal\n");
        break;

    case BP_TYPE_GSELECT:
        printf("Gselect\n");
        break;

    case BP_TYPE_HYBRID:
        printf("Hybrid\n");
        break;

    default:
        printf("Unknown type\n");
    }

    return 0;
}

-------------------------------------------------------------------------------

