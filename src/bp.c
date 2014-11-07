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
#include "bp_btb.h"
#include "bp_utils.h"
#include "bp_print.h"

/* Global data */
struct bp_bimodal       g_bp_bimodal;
struct bp_gshare        g_bp_gshare;
struct bp_hybrid        g_bp_hybprid;
struct bp_input         g_bp;
struct bp_nargs_table   g_bp_args_table[BP_TYPE_MAX];

cache_generic_t         g_bp_btb;
cache_tagstore_t        g_bp_btb_ts;

const char              *g_bp_type_bimodal = "bimodal";
const char              *g_bp_type_gshare = "gshare";
const char              *g_bp_type_hybrid = "hybrid";

/* Function pointers to bp type handlers as indexed by bp_type */
void (* const g_bp_handlers[]) (struct bp_input *, uint32_t, bool) = {
    NULL,
    NULL,
    bp_bimodal_handler,
    NULL,
    bp_gshare_handler,
    bp_hybrid_handler,
    NULL
};

inline uint32_t
bp_get_curr_entry_id(void)
{
    return g_bp.npredicts;
}

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
 * Params:
 *  bp      ptr to global branch predictor data.
 *
 * Returns: Nothing.
 **************************************************************************/
void
bp_cleanup(struct bp_input *bp)
{
    if (!bp)
        goto exit;

    if (bp->bimodal) {
        bp_bimodal_cleanup(bp);
        bp->bimodal = NULL;
    }
    if (bp->gshare) {
        bp_gshare_cleanup(bp);
        bp->gshare = NULL;
    }
    if (bp->hybrid) {
        bp_hybrid_cleanup(bp);
        bp->hybrid = NULL;
    }
    if (bp->btb) {
        bp->btb = NULL;
    }

    memset(bp, 0, sizeof(*bp));

exit:
    return;
}


/*************************************************************************** 
 * Name:    bp_parse_tracefile
 *
 * Desc:    Parses each entry in the parse file and calls the configured bp
 *          type's handler.
 *
 * Params:
 *  bp          ptr to global bp input data
 *
 * Returns: bool
 *  TRUE on success, FALSE otherwise.
 **************************************************************************/
bool
bp_parse_tracefile(struct bp_input *bp)
{
    bool        taken = FALSE;
    char        newline = '\n';
    char        trace_taken = '\0';
    char        *trace_fpath = NULL;
    FILE        *trace_fptr = NULL;
    uint32_t    pc = 0;

    if (!bp) {
        bp_assert(0);
        goto error_exit;
    }

    trace_fpath = bp->tracefile;
    trace_fptr = fopen(trace_fpath, "r");
     if (!trace_fptr) {
        printf("Error: Unable to open trace file %s.\n", trace_fpath);
        dprint_err("unable to open trace file %s.\n", trace_fpath);
        goto error_exit;
     }

     while(fscanf(trace_fptr, "%x %c%c", &pc, &trace_taken, &newline) != EOF) {
         bp->npredicts += 1;
         taken = ('t' == trace_taken) ? TRUE : FALSE;

         if (BP_IS_BTB_PRESENT) {
             bool       in_btb = FALSE;
             mem_ref_t  mem_ref;

             memset(&mem_ref, 0, sizeof(mem_ref));
             mem_ref.ref_type = MEM_REF_TYPE_READ;
             mem_ref.ref_addr = pc;

             /*
              * In case of BTB miss, the branch is considered to be not taken.
              * Increment the BTB miss-predict counter if the branch is 
              * actually taken. 
              * Fetch the next address.
              */
            in_btb = cache_handle_memory_request(bp->btb, &mem_ref);
            if (!in_btb) {
#ifdef DBG_ON
                dprint("%u. PC : %x %c\n", bp->npredicts, pc, trace_taken);
#endif
                bp->btb->nmisses += 1;
                if (taken)
                    bp->btb->nmiss_predicts += 1;
                 continue;
             }
            bp->btb->nhits += 1;
         }
         (*g_bp_handlers[bp->type]) (bp, pc, taken);
     }

     return TRUE;

error_exit:
     return FALSE;
}


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
    char        *type = NULL;
    uint8_t     iter = 0;
    uint32_t    btb_size = 0;

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
        bp->bimodal->nentries = (1U << bp->bimodal->m2);
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
        bp->gshare->nentries = (1U << bp->gshare->m1);
        if (!bp->gshare->n) {
            /* If n is 0, gshare behaves as a bimodal bp. */
            bp->type = BP_TYPE_BIMODAL;
            bp->bimodal->m2 = bp->gshare->m1;
            bp->bimodal->nentries = bp->gshare->nentries;
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
        bp->hybrid->nentries = (1U << bp->hybrid->k);

        bp->gshare->m1 = atoi(argv[++iter]);
        bp->gshare->n = atoi(argv[++iter]);
        bp->gshare->nentries = (1U << bp->gshare->m1);

        bp->bimodal->m2 = atoi(argv[++iter]);
        bp->bimodal->nentries = (1U << bp->bimodal->m2);
        dprint_info("bp type %s\n", type);
    } else {
        dprint_err("bad bp type %s\n", type);
        goto error_exit;
    }

    /* BTB details */
    btb_size = atoi(argv[++iter]);
    if (btb_size) {
        cache_init(bp->btb, "btb", "", CACHE_LEVEL_1);

        bp->btb->size = btb_size; 
        bp->btb->set_assoc = atoi(argv[++iter]);
        bp->btb->blk_size = CACHE_DEF_BLK_SIZE;
        bp->btb->repl_plcy = CACHE_REPL_PLCY_LRU;
        bp->btb->write_plcy = CACHE_WRITE_PLCY_WBWA;
        cache_tagstore_init(bp->btb, &g_bp_btb_ts);
        
        bp->btb_present = TRUE;
    }

    strncpy(bp->tracefile, argv[++iter], MAX_FILE_NAME_LEN);

    return TRUE;

error_exit:
    return FALSE;
}


/* 42: Life, the Universe and Everything; including branch predictors. */
int
main(int argc, char **argv)
{
    struct bp_input *bp = NULL;

    bp_init();
    bp = &g_bp;

    /* Parse and validate input args, and fill-in the global bp structure. */
    if (!(bp_parse_input(argc, argv, bp))) {
        dprint_err("error in parsing input arguments\n");
        goto error_exit;
    }

#ifdef DBG_ON
    //bp_print_input(&g_bp);
#endif /* DBG_ON */

    switch (bp->type) {
    case BP_TYPE_BIMODAL:
        bp_bimodal_init(bp);
        break;
    case BP_TYPE_GSHARE:
        bp_gshare_init(bp);
        break;
    case BP_TYPE_HYBRID:
        bp_hybrid_init(bp);
        break;
    default:
        dprint_err("invalid bp type %u\n", bp->type);
        bp_assert(0);
        goto error_exit;
    }

    /* Process the tracefile. */
    bp_parse_tracefile(bp);

    /* Print predictor stats and table contents. */
    bp_print_stats(bp);

    bp_cleanup(bp);
    return 0;

error_exit:
    bp_cleanup(bp);
    return -1;
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

