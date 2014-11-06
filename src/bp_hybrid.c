/* 
 * ECE 521 - Computer Design Techniques, Fall 2014
 * Project 2 - Branch Predictor Implementation 
 *
 * This module implements the hybrid branch predictor. 
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
 * Name:    bp_hybrid_init
 *
 * Desc:    Init routine for hybrid predictor. Allocates memory for chooser
 *          table and inits bimodal and gshare predictors.
 *
 * Params:
 *  bp      ptr to global bp data
 *
 * Returns: Nothing.
 **************************************************************************/
void
bp_hybrid_init(struct bp_input *bp)
{
    uint32_t            i = 0;
    struct bp_hybrid *hy = NULL;

    if (!bp && (!bp->hybrid || !bp->gshare || !bp->bimodal)) {
        bp_assert(0);
        goto exit;
    }
    hy = bp->hybrid;

    hy->table = (uint8_t *) calloc(1, (sizeof(*hy->table) * hy->nentries));
    if (!hy->table) {
        dprint_err("FATAL: insufficient memory\n");
        printf("FATAL: calloc failed.. insufficient memory\n");
        bp_assert(0);
        goto error_exit;
    }

    for (i = 0; i < hy->nentries; ++i)
        hy->table[i] = BP_WNOT_TAKEN;

    /* Hybrid uses both - bimodal & ghsare predictors. Init them. */
    bp_bimodal_init(bp);
    bp_gshare_init(bp);

exit:
    return;

error_exit:
    exit(0);
}


/***************************************************************************
 * Name:    bp_hybrid_cleanup 
 *
 * Desc:    Cleanup routine for hybrid predictor. Frees the memory allocated
 *          for the chooser table.
 *
 * Params:
 *  bp      ptr to the global bp data
 *
 * Returns: Nothing.
 **************************************************************************/
void
bp_hybrid_cleanup(struct bp_input *bp)
{
    if (!bp && (!bp->hybrid || !bp->gshare || !bp->bimodal)) 
        goto exit;

    if (bp->hybrid->table) {
        free(bp->hybrid->table);
        bp->hybrid->table = NULL;
    }
    memset(bp->hybrid, 0, sizeof(*bp->hybrid));
    bp->hybrid = NULL;

    /* Hybrid uses both - bimodal & ghsare predictors. Clean 'em up here. */
    bp_bimodal_cleanup(bp);
    bp_gshare_cleanup(bp);

exit:
    return;
}


/***************************************************************************
 * Name:    bp_hybrid_get_index
 *
 * Desc:    Calculates the hybrid index to the chooser table from given PC
 *          and k value.
 *
 * Params:
 *  pc      program counter value (an address)
 *  hy      ptr to the hybrid bp data
 *
 * Returns: unsigned 32-bit hybrid index
 **************************************************************************/
inline uint32_t
bp_hybrid_get_index(uint32_t pc, struct bp_hybrid *hy)
{
    uint32_t    mask = 0;
    uint32_t    index = 0;

    /* Hybrid inddices are bit PC[2 .. k+1] */
    mask = util_get_field_mask(2, (hy->k + 1));
    index = pc & mask;
    index >>= 2;

    return index;
}


/***************************************************************************
 * Name:    bp_hybrid_update
 *
 * Desc:    Hybrid predictor chooser table update routine.
 *
 * Params:
 *  bp      ptr to global bp data
 *  pc      program counter value
 *  taken   actually branch taken or not?
 *  bi      bimodal prediction
 *  gs      gshare prediction
 *
 * Returns: Nothing.
 **************************************************************************/
void
bp_hybrid_update(struct bp_input *bp, uint32_t pc, 
        bool taken, bool bi, bool gs)
{
    uint8_t     curr_value = 0;
    uint32_t    index = 0;

    if (!bp && !bp->hybrid) {
        bp_assert(0);
        goto exit;
    }

    /* 
     * If both - gshare & bimodal are same as actual outcome,
     * don't bother to update the chooser table.
     */
    if ((taken && (bi && gs)) || (!taken && (!bi && !gs)))
        goto exit;

    index = bp_hybrid_get_index(pc, bp->hybrid);
    curr_value = bp->hybrid->table[index];
    if (taken) {
        if (gs && !bi) {
            switch (curr_value) {
            case BP_NOT_TAKEN:
            case BP_WNOT_TAKEN:
            case BP_W_TAKEN:
                bp->hybrid->table[index] += 1;
            default:
                break;
            }
        } else if (!gs && bi) {
            switch (curr_value) {
            case BP_WNOT_TAKEN:
            case BP_W_TAKEN:
            case BP_TAKEN:
                bp->hybrid->table[index] -= 1;
            default:
                break;
            }
        }
    } else {
        if (!gs && bi) {
            switch (curr_value) {
            case BP_NOT_TAKEN:
            case BP_WNOT_TAKEN:
            case BP_W_TAKEN:
                bp->hybrid->table[index] += 1;
            default:
                break;
            }
        } else if (gs && !bi) {
            switch (curr_value) {
            case BP_WNOT_TAKEN:
            case BP_W_TAKEN:
            case BP_TAKEN:
                bp->hybrid->table[index] -= 1;
            default:
                break;
            }
        }
    }

exit:
    return;
}


/***************************************************************************
 * Name:    bp_hybrid_lookup
 *
 * Desc:    Hybrid predictor chooser table lookup routine.
 *
 * Params:
 *  bp      ptr to global bp data
 *  pc      program counter contents
 *  taken   actually branch taken or not?
 *
 * Returns: bool
 *  TRUE if predictor predicts taken, FALSE otherwise.
 **************************************************************************/
bool
bp_hybrid_lookup(struct bp_input *bp, uint32_t pc, bool taken)
{
    uint32_t    index = 0;

    if (!bp && !bp->hybrid) {
        bp_assert(0);
        goto exit;
    }

    index = bp_hybrid_get_index(pc, bp->hybrid);
    return (BP_IS_TAKEN(bp->hybrid->table[index]) ? TRUE : FALSE);

exit:
    return FALSE;
}


/***************************************************************************
 * Name:    bp_hybrid_handler
 *
 * Desc:    Main handler routine for hybrid predictor. Will be called for
 *          every PC address. It calculates the chooser index and then
 *          predicts based on bimodal and gshare predictors' outcome.
 *          Finally, updates the chooser table and either gshare/bimodal
 *          based on the prediction.
 *
 * Params:
 *  bp      ptr to the global bp data
 *  pc      program counter value
 *  taken   actually branch taken or not?
 *
 * Returns: Nothing.
 **************************************************************************/
void
bp_hybrid_handler(struct bp_input *bp, uint32_t pc, bool taken)
{
    bool                bi_taken = FALSE;
    bool                gs_taken = FALSE;
    bool                hy_taken = FALSE;
#ifdef DBG_ON
    uint8_t             old_value = 0;
    uint32_t            hy_index = 0;
#endif /* DBG_ON */
    struct bp_hybrid    *hy = NULL;

    if (!bp && !bp->hybrid) {
        bp_assert(0);
        goto exit;
    }
    hy = bp->hybrid;

#ifdef DBG_ON
    hy_index = bp_hybrid_get_index(pc, bp->hybrid);
    old_value = bp->hybrid->table[hy_index];
#endif /* DBG_ON */

    /* Check the bimodal and gshare predictors. */
    bi_taken = bp_bimodal_lookup(bp, pc, taken);
    gs_taken = bp_gshare_lookup(bp, pc, taken);

    /* Lookup hybrid table. */
    hy_taken = bp_hybrid_lookup(bp, pc, taken);

    /* Update the predictor tables now. */
    bp_gshare_update_bhr(bp, taken);
    if (hy_taken)
        bp_gshare_update(bp, pc, taken);
    else
        bp_bimodal_update(bp, pc, taken);

    /* Update the hybrid table based on other predictor outcomes. */
    bp_hybrid_update(bp, pc, taken, bi_taken, gs_taken);

    /* Finally update the hybrid miss predicts. */
    if (!hy_taken) 
        hy->nmisses += 1;

#ifdef DBG_ON
    bp_print_hybrid_curr_entry(bp, pc, taken, hy_taken, old_value);
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

