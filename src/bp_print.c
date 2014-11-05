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
                bp->btb->size, bp->btb->assoc, bp->tracefile);
        dprint("OUTPUT\n");
        dprint("number of predictions: %u\n", bp->npredicts);
        dprint("number of mispredictions: %u\n", bp->bimodal->nmisses);
        dprint("misprediction rate: %.2f%s\n",
            (((double) bp->bimodal->nmisses / (double) bp->npredicts) * 100),
            "%");
        dprint("FINAL BIMODAL CONTENTS\n");
        bp_print_table(bp->bimodal->nentries, bp->bimodal->table);
        break;

    case BP_TYPE_GSHARE:
        dprint("./sim %s %u %u %u %u %s\n",
                g_bp_type_str[bp->type], bp->gshare->m1, bp->gshare->n,
                bp->btb->size, bp->btb->assoc, bp->tracefile);
        dprint("OUTPUT\n");
        dprint("number of predictions: %u\n", bp->npredicts);
        dprint("number of mispredictions: %u\n", bp->gshare->nmisses);
        dprint("misprediction rate: %.2f%s\n",
            (((double) bp->gshare->nmisses / (double) bp->npredicts) * 100),
            "%");
        dprint("FINAL GSHARE CONTENTS\n");
        bp_print_table(bp->gshare->nentries, bp->gshare->table);
        break;

    case BP_TYPE_HYBRID:
        dprint("./sim %s %u %u %u %u %u %u %s\n",
                g_bp_type_str[bp->type], bp->hybrid->k,
                bp->gshare->m1, bp->gshare->n, bp->bimodal->m2,
                bp->btb->size, bp->btb->assoc, bp->tracefile);
        dprint("OUTPUT\n");
        dprint("number of predictions: %u\n", bp->npredicts);
        dprint("number of mispredictions: %u\n", bp->hybrid->nmisses);
        dprint("misprediction rate: %.2f%s\n",
            (((double) bp->hybrid->nmisses / (double) bp->npredicts) * 100),
            "%");
        dprint("FINAL CHOOSER CONTENTS\n");
        bp_print_table(bp->hybrid->nentries, bp->hybrid->table);
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
bp_print_bimodal_curr_entry(uint8_t *table, uint32_t index, int pc, bool taken,
        uint8_t old_value)
{
    if (!table) {
        bp_assert(0);
        goto exit;
    }

    dprint(" PC: %x %c\n", pc, (taken ? 'y' : 'n'));
    dprint("BIMODAL index: %u old value: %u new value: %u\n",
            index, old_value, table[index]);
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
        dprint("    btb_assoc: %u\n", bp->btb->assoc);
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

