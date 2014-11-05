/* 
 * ECE 521 - Computer Design Techniques, Fall 2014
 * Project 2 - Branch Predictor Implementation 
 *
 * This module implements main branch predictor.
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
#ifdef DBG_ON
#include "bp_print.h"
#endif /* DBG_ON */

/* Global data */
struct bp_bimodal       g_bp_bimodal;
struct bp_gshare        g_bp_gshare;
struct bp_hybrid        g_bp_hybprid;
struct bp_btb           g_bp_btb;
struct bp_input         g_bp;
struct bp_nargs_table   g_bp_args_table[BP_TYPE_MAX];

const char              *g_bp_type_bimodal = "bimodal";
const char              *g_bp_type_gshare = "gshare";
const char              *g_bp_type_hybrid = "hybrid";

/*************************************************************************** 
 * Name:    bp_init 
 *
 * Desc:    Init routine for bp.
 *
 * Params:  None.
 *
 * Returns: Nothing.
 **************************************************************************/
void
bp_init(void)
{
    memset(&g_bp, 0, sizeof(g_bp));
    g_bp.bimodal = &g_bp_bimodal;
    g_bp.gshare = &g_bp_gshare;
    g_bp.hybrid = &g_bp_hybprid;
    g_bp.btb = &g_bp_btb;

    /* Fill in the req. nargs table */
    g_bp_args_table[0].min_args = BP_MIN_ARGS;
    g_bp_args_table[0].max_args = BP_MAX_ARGS;
    g_bp_args_table[BP_TYPE_BIMODAL].min_args = BP_REQ_ARGS_BIMODAL;
    g_bp_args_table[BP_TYPE_BIMODAL].max_args = BP_REQ_ARGS_BIMODAL;
    g_bp_args_table[BP_TYPE_GSHARE].min_args = BP_REQ_ARGS_GSHARE;
    g_bp_args_table[BP_TYPE_GSHARE].max_args = BP_REQ_ARGS_GSHARE;
    g_bp_args_table[BP_TYPE_HYBRID].min_args = BP_REQ_ARGS_HYBRID;
    g_bp_args_table[BP_TYPE_HYBRID].max_args = BP_REQ_ARGS_HYBRID;

    return;
}

/*************************************************************************** 
 * Name:    bp_cleanup 
 *
 * Desc:    bp cleanup routine. Usually called in exit path.
 *
 * Params:  None.
 *
 * Returns: Nothing.
 **************************************************************************/
void
bp_cleanup(void)
{
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


/*************************************************************************** 
 * Name:    bp_parse_input
 *
 * Desc:    Parses the input arguments and fills in the glboal bp structure.
 *
 * Params:
 *  argc    # of input arguments
 *  argv    user entered argument string
 *  bp      ptr to the bp structure to be filled in
 *
 * Returns: bool
 *  TRUE on success, FALSE otherwise.
 **************************************************************************/
static bool
bp_parse_input(int argc, char **argv, struct bp_input *bp)
{
    char    *type = NULL;
    uint8_t iter = 0;

    if ((0 == argc) || (!bp)) {
        dprint_err("invalid arguments\n");
        goto error_exit;
    }

    if ((argc < g_bp_args_table[0].min_args) || 
            (argc > g_bp_args_table[0].max_args)) {
        dprint_err("bad # of args %u\n", argc);
        goto error_exit;
    }

    /* Interpret the aruguments based on bp type */
    type = argv[++iter];
    if (!strncmp(type, g_bp_type_bimodal, strlen(g_bp_type_bimodal))) {
        /* sim bimodal <M2> <BTB size> <BTB assoc> <tracefile> */
        if ((argc < g_bp_args_table[BP_TYPE_BIMODAL].min_args) || 
                (argc > g_bp_args_table[BP_TYPE_BIMODAL].max_args)) {
            dprint_err("bad # of args %u, for bimodal\n", argc);
            goto error_exit;
        }
        
        bp->type = BP_TYPE_BIMODAL;
        bp->bimodal->m2 = atoi(argv[++iter]);
        bp->gshare = NULL;
        bp->hybrid = NULL;
        dprint_info("bp type %s\n", type);
    } else if (!strncmp(type, g_bp_type_gshare, strlen(g_bp_type_gshare))) {
        /* sim gshare <M1> <N> <BTB size> <BTB assoc> <tracefile> */
        if ((argc < g_bp_args_table[BP_TYPE_GSHARE].min_args) || 
                (argc > g_bp_args_table[BP_TYPE_GSHARE].max_args)) {
            dprint_err("bad # of args %u, for gshare\n", argc);
            goto error_exit;
        }

        bp->type = BP_TYPE_GSHARE;
        bp->gshare->m1 = atoi(argv[++iter]);
        bp->gshare->n = atoi(argv[++iter]);
        if (!bp->gshare->n) {
            /* If n is 0, gshare behaves as a bimodal bp. */
            bp->type = BP_TYPE_BIMODAL;
            bp->bimodal->m2 = bp->gshare->m1;
            bp->gshare = NULL;
            dprint_info("bp type %s (as n = 0)\n", type);
        } else {
            bp->bimodal = NULL;
            dprint_info("bp type %s\n", type);
        }
        bp->hybrid = NULL;
    } else if (!strncmp(type, g_bp_type_hybrid, strlen(g_bp_type_hybrid))) {
        /* sim hybrid <K> <M1> <N> <M2> <BTB size> <BTB assoc> <tracefile> */
        if ((argc < g_bp_args_table[BP_TYPE_HYBRID].min_args) || 
                (argc > g_bp_args_table[BP_TYPE_HYBRID].max_args)) {
            dprint_err("bad # of args %u, for hybrid\n", argc);
            goto error_exit;
        }
        bp->type = BP_TYPE_HYBRID;
        bp->hybrid->k = atoi(argv[++iter]);
        bp->gshare->m1 = atoi(argv[++iter]);
        bp->gshare->n = atoi(argv[++iter]);
        bp->bimodal->m2 = atoi(argv[++iter]);
        dprint_info("bp type %s\n", type);
    } else {
        dprint_err("bad bp type %s\n", type);
        goto error_exit;
    }

    bp->btb->size = atoi(argv[++iter]);
    bp->btb->assoc = atoi(argv[++iter]);
    bp->btb_present = ((bp->btb->size && bp->btb->assoc) ? TRUE : FALSE);

    strncpy(bp->tracefile, argv[++iter], MAX_FILE_NAME_LEN);

    return TRUE;

error_exit:
    return FALSE;
}


/* 42: Life, the Universe and Everything; including branch predictors. */
int
main(int argc, char **argv)
{
    bp_init();

    /* Parse and validate input args, and fill-in the global bp structure. */
    if (!(bp_parse_input(argc, argv, &g_bp))) {
        dprint_err("error in parsing input arguments\n");
        goto error_exit;
    }

#ifdef DBG_ON
    bp_print_input(&g_bp);
#endif /* DBG_ON */

    bp_cleanup();
    return 0;

error_exit:
    bp_cleanup();
    return -1;
}
