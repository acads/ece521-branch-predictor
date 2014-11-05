/* 
 * ECE 521 - Computer Design Techniques, Fall 2014
 * Project 2 - Branch Predictor Implementation
 *
 * This module contains all required data strcutures and function declrations
 * for the branch predictor implementation.
 *
 * Author: Aravindhan Dhanasekaran <adhanas@ncsu.edu>
 */

#ifndef BP_H_
#define BP_H_

/* Constants */
#define BP_MIN_ARGS         6
#define BP_MAX_ARGS         9
#define BP_REQ_ARGS_BIMODAL 6
#define BP_REQ_ARGS_GSHARE  7
#define BP_REQ_ARGS_HYBRID  9
#define MAX_FILE_NAME_LEN   255

#ifndef TRUE
#define TRUE    1
#endif /* !TRUE */
#ifndef FALSE
#define FALSE   0
#endif /* !FALSE */
#ifndef bool
#define bool    unsigned char
#endif /* !bool */

typedef enum bp_types__ {
    BP_TYPE_START       = 0,
    BP_TYPE_1BIT        = 1,
    BP_TYPE_BIMODAL     = 2,
    BP_TYPE_GSELECT     = 3,
    BP_TYPE_GSHARE      = 4,
    BP_TYPE_HYBRID      = 5,
    BP_TYPE_MAX         = 6,
} bp_type;

/* bp type/req. # of arguments table */
struct bp_nargs_table {
    uint16_t    min_args;
    uint16_t    max_args;
};

/* User given bimodal predictor config */
struct bp_bimodal {
    uint16_t    m2;     /* # of lower order PC bits to use as index to BPT */
};

/* User given gshare predictor config */
struct bp_gshare {
    uint16_t    m1;     /* # of LSB PC bits to index predictor table    */
    uint16_t    n;      /* # of bits in global history register         */
};

/* User given hybrid predictor config */
struct bp_hybrid {
    uint16_t    k;      /* # of LSB PC bits to index chooser table  */
};

/* User given btb config */
struct bp_btb {
    uint16_t    size;   /* size of btb              */
    uint16_t    assoc;  /* set associativity of btb */
};

/* Main bp structure to store user given predictor config */
struct bp_input {
    bp_type             type;                           /* predictor type   */
    bool                btb_present;                    /* btp present?     */
    struct bp_bimodal   *bimodal;                       /* bimodal config   */
    struct bp_gshare    *gshare;                        /* gshare config    */
    struct bp_hybrid    *hybrid;                        /* hybrid config    */
    struct bp_btb       *btb;                           /* btb config       */
    char                tracefile[MAX_FILE_NAME_LEN+1]; /* trace file name  */
};

#endif /* BP_H_ */

