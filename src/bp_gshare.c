/* 
 * ECE 521 - Computer Design Techniques, Fall 2014
 * Project 2 - Branch Predictor Implementation 
 *
 * This module implements the gshare branch predictor. 
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
 * Name:    bp_gshare_get_index
 *
 * Desc:    Returns the gshare index of the given PC by XORing n MSB bits
 *          of PC with n-bit BHR.
 *
 * Params:
 *  pc      program counter value
 *  bhr     branch history register value
 *  n       # of MSB bits of PC to be used for XORing
 *
 * Returns: unsigned 32-bit gshare index to gshare predictor table
 **************************************************************************/
inline uint32_t
bp_gshare_get_index(uint32_t pc, struct bp_gshare *gs)
{
    uint16_t    m = 0;
    uint16_t    n = 0;
    uint16_t    bhr = 0;
    uint32_t    index = 0;
    uint32_t    mask = 0;

    if (!gs) {
        bp_assert(0);
        goto error_exit;
    }
    m = gs->m1;
    n = gs->n;
    bhr = gs->bhr;

    /* BHR is only n-bits wide. So, convert the 32-bit bhr to n-bit BHR. */
    bhr <<= (m - n);

    /* Gshare indices are bits of PC[m+1..2] and XORed with BHR. */
    mask = util_get_field_mask(2, m + 1);
    index = pc & mask;
    index >>= 2;

    /* index = bhr ^ m LSbits of PC */
    index ^= bhr;

    return index;

error_exit:
    return 0;
}


/***************************************************************************
 * Name:    bp_gshare_init
 *
 * Desc:    Init routine for gshare predictor. Allocates required memory for
 *          gshare predictor table.
 *
 * Params:
 *  bp      ptr to global bp data
 *
 * Returns: Nothing.
 **************************************************************************/
void
bp_gshare_init(struct bp_input *bp)
{
    uint32_t            i = 0;
    struct bp_gshare    *gs = NULL;

    if (!bp && !bp->gshare) {
        bp_assert(0);
        goto exit;
    }
    gs = bp->gshare;

    gs->table = (uint8_t *) calloc(1, (sizeof(*gs->table) * gs->nentries));
    if (!gs->table) {
        dprint_err("FATAL: insufficient memory\n");
        printf("FATAL: calloc failed.. insufficient memory\n");
        bp_assert(0);
        goto error_exit;
    }

    gs->bhr = 0;
    for (i = 0; i < gs->nentries; ++i)
        gs->table[i] = BP_W_TAKEN;

exit:
    return;

error_exit:
    exit(0);
}


/***************************************************************************
 * Name:    bp_gshare_cleanup
 *
 * Desc:    Cleanup routine for gshare predictor. Frees the memory allocated
 *          for the predictor table.
 *
 * Params:
 *  bp      ptr to global branch predcitor data
 *
 * Returns: Nothing.
 **************************************************************************/
void
bp_gshare_cleanup(struct bp_input *bp)
{
    if (!bp || !bp->gshare)
        goto exit;

    if (bp->gshare->table) {
        free(bp->gshare->table);
        bp->gshare->table = NULL;
    }
    memset(bp->gshare, 0, sizeof(*bp->gshare));
    bp->gshare = NULL;

exit:
    return;
}


/***************************************************************************
 * Name:    bp_gshare_update_bhr
 *
 * Desc:    Updates the bhr based on the actual branch outcome.
 *
 * Params:
 *  bp      ptr to global bp data
 *  taken   actually branch taken or not?
 *
 * Returns: Nothing.
 **************************************************************************/
inline void
bp_gshare_update_bhr(struct bp_input *bp, bool taken)
{
    if (taken)
        bp->gshare->bhr |= (1U << (bp->gshare->n - 1));
    else
        bp->gshare->bhr &= ~(1U << (bp->gshare->n - 1));

    return;
}


/***************************************************************************
 * Name:    bp_gshare_update
 *
 * Desc:    Updates the gshare predictor table value based on the actual
 *          branch outcome.
 *
 * Params:
 *  bp      ptr to global bp data
 *  pc      program counter value
 *  taken   actually branch taken or not?
 *
 * Returns: Nothing.
 **************************************************************************/
void
bp_gshare_update(struct bp_input *bp, uint32_t pc, bool taken)
{
    uint8_t     curr_value;
    uint32_t    index;

    if (!bp && !bp->gshare) {
        bp_assert(0);
        goto exit;
    }

    /*
     * Update our predictor table based on the current value and the actual
     * taken flag. States BP_NOT_TAKEN and BP_TAKEN are saturated states. So,
     * don't bother about them.
     */
    index = bp_gshare_get_index(pc, bp->gshare);
    curr_value = bp->gshare->table[index];
    if (taken) {
        switch (curr_value) {
        case BP_NOT_TAKEN:
        case BP_WNOT_TAKEN:
        case BP_W_TAKEN:
            bp->gshare->table[index] += 1;
            break;
        default:
            break;
        }
    } else {
        switch (curr_value) {
        case BP_WNOT_TAKEN:
        case BP_W_TAKEN:
        case BP_TAKEN:
            bp->gshare->table[index] -= 1;
            break;
        default:
            break;
        }
    }

exit:
    return;
}


/***************************************************************************
 * Name:    bp_gshare_lookup
 *
 * Desc:    Looksup the gsharepredictor table and predicts taken/not 
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
bp_gshare_lookup(struct bp_input *bp, uint32_t pc, bool taken)
{
    uint32_t    index = 0;

    if (!bp && !bp->gshare) {
        bp_assert(0);
        goto exit;
    }   

    index = bp_gshare_get_index(pc, bp->gshare);
    return (BP_IS_TAKEN(bp->gshare->table[index]) ? TRUE : FALSE);

exit:
    return FALSE;
}


/***************************************************************************
 * Name:    bp_gshare_lookup_and_update
 *
 * Desc:    Gshare predictor lookup/update routine.
 *
 * Params:
 *  gs      ptr to the gshare predictor data
 *  index   index to the gshare predictor table
 *  taken   actually branch taken or not?
 *
 * Returns: bool
 *  TRUE if predictor predicts taken, FALSE otherwise.
 **************************************************************************/
bool
bp_gshare_lookup_and_update(struct bp_gshare *gs, uint32_t index, bool taken)
{
    bool    gs_predict = FALSE;
    uint8_t curr_value = 0;

    if (!gs && !gs->table) {
        bp_assert(0);
        goto error_exit;
    }

    /* Check whether prediction is correct or not. */
    curr_value = gs->table[index];
    if ((taken && (curr_value > BP_WNOT_TAKEN)) ||
            ((!taken) && (curr_value < BP_W_TAKEN)))
        gs_predict = TRUE;

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
            gs->table[index] += 1;
            break;

        default:
            break;
        }
    } else {
        switch (curr_value) {
        case BP_WNOT_TAKEN:
        case BP_W_TAKEN:
        case BP_TAKEN:
            gs->table[index] -= 1;
            break;

        default:
            break;
        }
    }

    /*
     * Update the global BHR.
     *  1. Shift the BHR to the right by 1.
     *  2. If gshare predicts taken, set the MSbit; else clear it.
     */
    curr_value = gs->table[index];
    gs->bhr >>= 1;
    if (taken)
        gs->bhr |= (1U << (gs->n - 1));
    else
        gs->bhr &= ~(1U << (gs->n - 1));

    /* Finally update miss predicts counter. */
    if (!gs_predict)
        gs->nmisses += 1;

    return gs_predict;

error_exit:
    return FALSE;
}


/***************************************************************************
 * Name:    bp_gshare_handler
 *
 * Desc:    Main handler routine for gshare predictor. Calculates the index
 *          with the given PC and calls the lookup routine. This will be
 *          called for every address in the trace file.
 *
 * Params:
 *  bp      ptr to global predictor data
 *  pc      program counter value
 *  taken   actually branch taken or not?
 *
 * Returns: Nothing.
 **************************************************************************/
void
bp_gshare_handler(struct bp_input *bp, uint32_t pc, bool taken)
{
#ifdef DBG_ON
    uint8_t             old_value = 0;
#endif /* DBG_ON */
    uint32_t            gs_index = 0;
    struct bp_gshare    *gs = NULL;

    if ((!bp && !bp->gshare) || !pc) {
        bp_assert(0);
        goto exit;
    }

    gs = bp->gshare;
    gs_index = bp_gshare_get_index(pc, gs);

#ifdef DBG_ON
    old_value = gs->table[gs_index];
#endif /* DBG_ON */

    bp_gshare_lookup_and_update(gs, gs_index, taken);

#ifdef DBG_ON
    bp_print_gshare_curr_entry(gs, gs_index, pc, taken, old_value);
#endif /* DBG_ON */

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

