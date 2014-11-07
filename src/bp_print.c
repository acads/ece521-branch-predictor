/* 
 * ECE 521 - Computer Design Techniques, Fall 2014
 * Project 2 - Branch Predictor Implementation
 *
 * This module implements print routines for branch predictor implementation.
 *
 * Author: Aravindhan Dhanasekaran <adhanas@ncsu.edu>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "bp.h"
#include "bp_utils.h"

/* Global strings */
const char *g_bp_type_str[] = {
    "start-not-for-use",
    "1-bit counter",
    "bimodal",
    "gselect",
    "gshare",
    "hybrid",
    "end-not-for-use"
};

const char *g_bp_truth_table_str[] = {
    "false",
    "true"
};


static void
bp_print_table(uint32_t nentries, uint8_t *table)
{
    uint32_t    i = 0;

    for (i = 0; i < nentries; ++i)
        dprint("%u\t%u\n", i, table[i]);

    return;
}

void
bp_print_stats(struct bp_input *bp)
{
    if (!bp) {
        bp_assert(0);
        goto error_exit;
    }

    dprint("COMMAND\n");
    switch (bp->type) {
    case BP_TYPE_BIMODAL:
        dprint("./sim %s %u %u %u %s\n",
                g_bp_type_str[bp->type], bp->bimodal->m2,
                bp->btb->size, bp->btb->set_assoc, bp->tracefile);

        dprint("OUTPUT\n");
        if (BP_IS_BTB_PRESENT) {
            dprint("size of BTB: %u\n", bp->btb->size);
            dprint("number of branches: %u\n", bp->npredicts);
            dprint("number of predictions from branch predictor: %u\n",
                    bp->btb->nhits);
            dprint("number of mispredictions from branch predictor: %u\n",
                    bp->bimodal->nmisses);
            dprint("number of branches miss in BTB and taken: %u\n",
                    bp->btb->nmiss_predicts);
            dprint("total mispredictions: %u\n",
                    (bp->bimodal->nmisses + bp->btb->nmiss_predicts));
            dprint("misprediction rate: %.2f%s\n",
                (((double) (bp->btb->nmiss_predicts + bp->bimodal->nmisses) / 
                  (double) bp->npredicts) * 100),
                "%");

            dprint("\n");
            dprint("FINAL BTB CONTENTS\n");
            cache_print_sim_data(bp->btb);
            dprint("\n");
        } else {
            dprint("number of predictions: %u\n", bp->npredicts);
            dprint("number of mispredictions: %u\n", bp->bimodal->nmisses);
            dprint("misprediction rate: %.2f%s\n",
                (((double) bp->bimodal->nmisses / (double) bp->npredicts) * 100),
                "%");
        }

        dprint("FINAL BIMODAL CONTENTS\n");
        bp_print_table(bp->bimodal->nentries, bp->bimodal->table);
        break;

    case BP_TYPE_GSHARE:
        dprint("./sim %s %u %u %u %u %s\n",
                g_bp_type_str[bp->type], bp->gshare->m1, bp->gshare->n,
                bp->btb->size, bp->btb->set_assoc, bp->tracefile);
        dprint("OUTPUT\n");
        if (BP_IS_BTB_PRESENT) {
            dprint("size of BTB: %u\n", bp->btb->size);
            dprint("number of branches: %u\n", bp->npredicts);
            dprint("number of predictions from branch predictor: %u\n",
                    bp->btb->nhits);
            dprint("number of mispredictions from branch predictor: %u\n",
                    bp->gshare->nmisses);
            dprint("number of branches miss in BTB and taken: %u\n",
                    bp->btb->nmiss_predicts);
            dprint("total mispredictions: %u\n",
                    (bp->gshare->nmisses + bp->btb->nmiss_predicts));
            dprint("misprediction rate: %.2f%s\n",
                (((double) (bp->btb->nmiss_predicts + bp->gshare->nmisses) / 
                  (double) bp->npredicts) * 100),
                "%");

            dprint("\n");
            dprint("FINAL BTB CONTENTS\n");
            cache_print_sim_data(bp->btb);
            dprint("\n");
        } else {
            dprint("number of predictions: %u\n", bp->npredicts);
            dprint("number of mispredictions: %u\n", bp->gshare->nmisses);
            dprint("misprediction rate: %.2f%s\n",
                (((double) bp->gshare->nmisses / (double) bp->npredicts) * 100),
                "%");
        }
        
        dprint("FINAL GSHARE CONTENTS\n");
        bp_print_table(bp->gshare->nentries, bp->gshare->table);
        break;

    case BP_TYPE_HYBRID:
        dprint("./sim %s %u %u %u %u %u %u %s\n",
                g_bp_type_str[bp->type], bp->hybrid->k,
                bp->gshare->m1, bp->gshare->n, bp->bimodal->m2,
                bp->btb->size, bp->btb->set_assoc, bp->tracefile);
        dprint("OUTPUT\n");
        dprint("number of predictions: %u\n", bp->npredicts);
        dprint("number of mispredictions: %u\n", bp->hybrid->nmisses);
        dprint("misprediction rate: %.2f%s\n",
            (((double) bp->hybrid->nmisses / (double) bp->npredicts) * 100),
            "%");

        dprint("FINAL CHOOSER CONTENTS\n");
        bp_print_table(bp->hybrid->nentries, bp->hybrid->table);

        dprint("FINAL GSHARE CONTENTS\n");
        bp_print_table(bp->gshare->nentries, bp->gshare->table);

        dprint("FINAL BIMODAL CONTENTS\n");
        bp_print_table(bp->bimodal->nentries, bp->bimodal->table);
        break;

    default:
        bp_assert(0);
        goto error_exit;
    }

    return;

error_exit:
    return;
}

#ifdef DBG_ON
/***************************************************************************
 * Name:    bp_print_bimodal_curr_entry
 *
 * Desc:    Prints the currently being processed entry in bimodal table in
 *          TAs debug run format.
 *
 * Params:
 *
 * Returns: Nothing
 **************************************************************************/
void
bp_print_bimodal_curr_entry(struct bp_input *bp, uint32_t index, uint32_t pc,
        bool taken, uint8_t old_value)
{
    if (!bp || !bp->bimodal || !bp->bimodal->table) {
        bp_assert(0);
        goto exit;
    }

    dprint("%u. PC: %x %c\n", bp->npredicts, pc, (taken ? 't' : 'n'));
    if (BP_IS_BTB_PRESENT)
        dprint("BTB HIT\n");
    dprint("BIMODAL index: %u old value: %u new value %u\n",
            index, old_value, bp->bimodal->table[index]);
exit:
    return;
}


/***************************************************************************
 * Name:    bp_print_bimodal_curr_entry
 *
 * Desc:    Prints the currently being processed entry in gshare table in
 *          TAs debug run format.
 *
 * Params:
 *
 * Returns: Nothing
 **************************************************************************/
void
bp_print_gshare_curr_entry(struct bp_gshare *gs, uint32_t index, uint32_t pc,
        bool taken, uint8_t old_value)
{
    if (!gs && !gs->table) {
        bp_assert(0);
        goto exit;
    }

    dprint("%u. ", bp_get_curr_entry_id());
    dprint("PC: %x %c\n", pc, (taken ? 't' : 'n'));
    dprint("GSHARE index: %u old value: %u new value %u\n",
            index, old_value, gs->table[index]);
    dprint("BHR UPDATED: %u\n", gs->bhr);

exit:
    return;
}


/***************************************************************************
 * Name:    bp_print_hybrid_curr_entry
 *
 * Desc:    Prints the currently being processed entry in hybrid table in
 *          TAs debug run format.
 *
 * Params:
 *
 * Returns: Nothing
 **************************************************************************/
void
bp_print_hybrid_curr_entry(struct bp_input *bp, uint32_t pc, bool taken,
        uint32_t predictor, uint8_t pred_old_value, uint8_t old_value)
{
    uint32_t    index = 0;
    uint32_t    hy_index = 0;

    if (!bp || !bp->hybrid) {
        bp_assert(0);
        goto exit;
    }

    hy_index = bp_hybrid_get_index(pc, bp->hybrid);
    dprint("%u. ", bp_get_curr_entry_id());
    dprint("PC: %x %c\n", pc, (taken ? 't' : 'n'));
    dprint("CHOOSER index: %u old value: %u new value %u\n",
            hy_index, old_value, bp->hybrid->table[hy_index]);

    /* Print the chosen predictor details. */
    if (BP_TYPE_GSHARE == predictor) {
        index = 0;
        index = bp_gshare_get_index(pc, bp->gshare);
        dprint("GSHARE index: %u old value: %u new value %u\n",
                index, pred_old_value, bp->gshare->table[index]);
    } else if (BP_TYPE_BIMODAL == predictor) {
        index = 0;
        index = bp_bimodal_get_index(pc, bp->bimodal->m2);
        dprint("BIMODAL index: %u old value: %u new value %u\n",
                index, pred_old_value, bp->bimodal->table[index]);
    }
    bp_gshare_update_bhr(bp, taken);
    dprint("BHR UPDATED: %u\n", bp->gshare->bhr);


exit:
    return;
}


/***************************************************************************
 * Name:    bp_print_input
 *
 * Desc:    Pretty prints the user entered config of bp.
 *
 * Params:
 *
 * Returns: Nothing.
 **************************************************************************/
void
bp_print_input(struct bp_input *bp)
{
    if (!bp)
        bp_assert(0);

    dprint("bp input data\n");
    dprint("=============\n");
    dprint("    bp_type: %s\n", g_bp_type_str[bp->type]);
    dprint("    btb_present: %s\n", g_bp_truth_table_str[bp->btb_present]);
    
    switch (bp->type) {
    case BP_TYPE_BIMODAL:
        dprint("    bimodal_m2: %u\n", bp->bimodal->m2);
        break;

    case BP_TYPE_GSHARE:
        dprint("    gshare_m1: %u\n", bp->gshare->m1);
        dprint("    gshare_n: %u\n", bp->gshare->n);
        break;

    case BP_TYPE_HYBRID:
        dprint("    hybrid_k: %u\n", bp->hybrid->k);
        dprint("    gshare_m1: %u\n", bp->gshare->m1);
        dprint("    gshare_n: %u\n", bp->gshare->n);
        dprint("    bimodal_m2: %u\n", bp->bimodal->m2);
        break;

    default:
        dprint("    invalid bt type\n");
    }

    if (bp->btb_present) {
        dprint("    btb_size: %u\n", bp->btb->size);
        dprint("    btb_assoc: %u\n", bp->btb->set_assoc);
    }

    dprint("    tracefile: %s\n", bp->tracefile);
    return;
}
#endif /* DBG_ON */

/***************************************************************************
 * Name:    
 *
 * Desc:    
 *
 * Params:
 *
 * Returns: 
 **************************************************************************/

