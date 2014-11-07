/* 
 * ECE 521 - Computer Design Techniques, Fall 2014
 * Project 2 - Branch Predictor Implementation 
 *
 * This module implements the bimodal branch predictor.
 *
 * Author: Aravindhan Dhanasekaran <adhanas@ncsu.edu>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/time.h>

#include "bp.h"
#include "bp_utils.h"
#include "bp_print.h"


/***************************************************************************
 * Name:    bp_bimodal_get_index
 *
 * Desc:    Returns the bimodal index of the given PC and m value.
 *
 * Params:
 *  pc      program counter value
 *  m       # of LSB bits of PC to be used in calculating the index
 *
 * Returns: unsigned 32-bit bimodal index
 **************************************************************************/
inline uint32_t
bp_bimodal_get_index(int pc, uint16_t m)
{
    uint32_t mask = 0;
    uint32_t index = 0;

    /* Bimodal indices are bits PC[m+1..2] */
    mask = util_get_field_mask(2, m + 1);
    index = pc & mask;
    index >>= 2;

    return index;
}


/***************************************************************************
 * Name:    bp_bimodal_init
 *
 * Desc:    Init routine for bimodal predictor. Allocates required memory
 *          for bimodal predictor table.
 *
 * Params:
 *  bp      ptr to global branch predicotr data
 *
 * Returns: Nothing.
 **************************************************************************/
void
bp_bimodal_init(struct bp_input *bp)
{
    uint32_t            i = 0;
    struct bp_bimodal   *bi = NULL;

    if (!bp && !bp->bimodal) {
        bp_assert(0);
        goto exit;
    }

    bi = bp->bimodal;
    bi->table = (uint8_t *) calloc(1, (sizeof(*bi->table) * bi->nentries));
    if (!bi->table) {
        dprint_err("FATAL: insufficient memory\n");
        printf("FATAL: calloc failed.. insufficient memory\n");
        bp_assert(0);
        goto error_exit;
    }

    for (i = 0; i < bi->nentries; ++i)
        bi->table[i] = BP_W_TAKEN;

exit:
    return;

error_exit:
    exit(0);
}


/***************************************************************************
 * Name:    bp_bimodal_cleanup
 *
 * Desc:    Cleanup routine for bimodal predictor. Frees the memory
 *          allocated for predictor table.
 *
 * Params:
 *  bp      ptr to global branch predictor data.
 *
 * Returns: Nothing.
 **************************************************************************/
void
bp_bimodal_cleanup(struct bp_input *bp)
{
    if (!bp || !bp->bimodal)
        goto exit;

    if (bp->bimodal->table) {
        free(bp->bimodal->table);
        bp->bimodal->table = NULL;
    }
    memset(bp->bimodal, 0, sizeof(*bp->bimodal));
    bp->bimodal = NULL;

exit:
    return;
}

void
bp_bimodal_update(struct bp_input *bp, uint32_t pc, bool taken)
{
    uint8_t     curr_value = 0;
    uint32_t    index = 0;

    if (!bp && !bp->bimodal) {
        bp_assert(0);
        goto exit;
    }

    /*
     * Update our predictor table based on the current value and the actual
     * taken flag. States BP_NOT_TAKEN and BP_TAKEN are saturated states. So,
     * don't bother about them.
     */
    index = bp_bimodal_get_index(pc, bp->bimodal->m2);
    curr_value = bp->bimodal->table[index];
    if (taken) {
        switch (curr_value) {
        case BP_NOT_TAKEN:
        case BP_WNOT_TAKEN:
        case BP_W_TAKEN:
            bp->bimodal->table[index] += 1;
            break;
        default:
            break;
        }
    } else {
        switch (curr_value) {
        case BP_WNOT_TAKEN:
        case BP_W_TAKEN:
        case BP_TAKEN:
            bp->bimodal->table[index] -= 1;
            break;
        default:
            break;
        }
    }

exit:
    return;
}


/***************************************************************************
 * Name:    bp_bimodal_lookup
 *
 * Desc:    Looksup the bimodal predictor table and predicts taken/not 
 *          taken.
 *
 * Params:
 *  bp      ptr to global bp data
 *  pc      program counter value
 *  taken   actually branch taken or not?
 *
 * Returns: bool
 *  TRUE if predictor predicts taken, FALSE otherwise.
 **************************************************************************/
bool
bp_bimodal_lookup(struct bp_input *bp, uint32_t pc, bool taken,
        uint8_t *old_value)
{
    uint32_t    index = 0;

    if (!bp && !bp->bimodal) {
        bp_assert(0);
        goto exit;
    }

    index = bp_bimodal_get_index(pc, bp->bimodal->m2);
    *old_value = bp->bimodal->table[index];
    return (BP_IS_TAKEN(bp->bimodal->table[index]) ? TRUE : FALSE);

exit:
    return FALSE;
}


/***************************************************************************
 * Name:    bp_bimodal_lookup_and_update
 *
 * Desc:    Bimodal predictor lookup/update routine.
 *
 *          Refer to section 3.1.1 in docs/pa1_spec.pdf.
 *
 * Params:
 *  table   ptr to bimodal predictor table
 *  index   index to the above given table
 *  taken   actually taken or not?
 *
 * Returns: bool
 * TRUE if predictor predicts taken, FALSE otherwise.
 **************************************************************************/
bool
bp_bimodal_lookup_and_update(uint8_t *table, uint32_t index, bool taken)
{
    bool        bi_predict = FALSE;
    uint8_t     curr_value = 0;

    /* Check whether our prediction is correct or not. */
    curr_value = table[index];
    if ((taken && (curr_value > BP_WNOT_TAKEN)) ||
            ((!taken) && (curr_value < BP_W_TAKEN)))
        bi_predict = TRUE;

    /*
     * Update our predictor table based on the current value and the actual
     * taken flag. States BP_NOT_TAKEN and BP_TAKEN are saturated states. So,
     * don't bother about them.
     */
    if (taken) {
        switch (curr_value) {
        case BP_NOT_TAKEN:
        case BP_WNOT_TAKEN:
        case BP_W_TAKEN:
            table[index] += 1;
            break;

        default:
            break;
        }
    } else {
        switch (curr_value) {
        case BP_WNOT_TAKEN:
        case BP_W_TAKEN:
        case BP_TAKEN:
            table[index] -= 1;
            break;

        default:
            break;
        }
    }

    return bi_predict;
}


/***************************************************************************
 * Name:    bp_bimodal_handler
 *
 * Desc:    Main handler routine for bimodal predictor. Goes over the trace
 *          file and calls the lookup routine with PC and actual values.
 *
 * Params:
 *  bp_bi   ptr to global predictor data
 *  pc      program counter value
 *  taken   actaully branch taken or not?
 *
 * Returns: Nothing.
 **************************************************************************/
void
bp_bimodal_handler(struct bp_input *bp, uint32_t pc, bool taken)
{
    bool                bi_taken = FALSE;
#ifdef DBG_ON
    uint8_t             old_value = 0;
#endif /* DBG_ON */
    uint32_t            bi_index = 0;
    struct bp_bimodal   *bi = NULL;

    if ((!bp && !bp->bimodal) || !pc) {
        bp_assert(0);
        goto exit;
    }

    bi = bp->bimodal;
    bi_index = bp_bimodal_get_index(pc, bi->m2);
#ifdef DBG_ON
    old_value = bi->table[bi_index];
#endif /* DBG_ON */
    bi_taken = bp_bimodal_lookup_and_update(bi->table, bi_index, taken);
    if (!bi_taken)
        bi->nmisses += 1;

#ifdef DBG_ON
    bp_print_bimodal_curr_entry(bp, bi_index, pc, taken, old_value);
#endif

exit:
    return;
}


/***************************************************************************
 * Name:    
 *
 * Desc:    
 *
 * Params:
 *
 * Returns: 
 **************************************************************************/

