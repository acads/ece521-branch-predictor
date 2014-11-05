/* 
 * ECE 521 - Computer Design Techniques, Fall 2014
 * Project 2 - Branch Predictor Implementation
 *
 * This module implements print routines for branch predictor implementation.
 *
 * Author: Aravindhan Dhanasekaran <adhanas@ncsu.edu>
 */

#ifdef DBG_ON

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

