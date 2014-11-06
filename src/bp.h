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

#define BP_NOT_TAKEN        0x0
#define BP_WNOT_TAKEN       0x1
#define BP_W_TAKEN          0x2
#define BP_TAKEN            0x3

#ifndef TRUE
#define TRUE    1
#endif /* !TRUE */
#ifndef FALSE
#define FALSE   0
#endif /* !FALSE */
#ifndef bool
#define bool    unsigned char
#endif /* !bool */

/* Util macros. */
#define BP_IS_TAKEN(VALUE)  (VALUE > BP_WNOT_TAKEN)

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
    uint16_t    m2;         /* # of LSB PC bits to index predictor table    */
    uint32_t    nentries;   /* # of entries in table                        */
    uint8_t     *table;     /* ptr to bimodal table; init'zd to BP_W_TAKEN  */
    uint32_t    nmisses;    /* # of miss predictions                        */
};

/* User given gshare predictor config */
struct bp_gshare {
    uint16_t    m1;         /* # of LSB PC bits to index predictor table    */
    uint16_t    n;          /* # of bits in global history register         */
    uint16_t    bhr;        /* global branch history register               */
    uint32_t    nentries;   /* # of entries in table                        */
    uint8_t     *table;     /* ptr to gshare table; init'zd to BP_W_TAKEN   */
    uint32_t    nmisses;    /* # of miss predictions                        */
};

/* User given hybrid predictor config */
struct bp_hybrid {
    uint16_t    k;          /* # of LSB PC bits to index chooser table      */
    uint32_t    nentries;   /* # of entries in table                        */
    uint8_t     *table;     /* ptr to chooser table; init to BP_WNOT_TAKEN  */
    uint32_t    nmisses;    /* # of miss predictions                        */
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
    uint32_t            npredicts;                      /* # of predictions */
    char                tracefile[MAX_FILE_NAME_LEN+1]; /* trace file name  */

    struct bp_bimodal   *bimodal;                       /* bimodal config   */
    struct bp_gshare    *gshare;                        /* gshare config    */
    struct bp_hybrid    *hybrid;                        /* hybrid config    */
    struct bp_btb       *btb;                           /* btb config       */
};

/* Predictor handler function pointer */
void (*bp_handler_type) (struct bp_input *, uint32_t, bool);

/* Externs */
extern struct bp_input g_bp;

/* Function declarations */
inline uint32_t
bp_get_curr_entry_id(void);

void
bp_bimodal_init(struct bp_input *bp);
void
bp_bimodal_cleanup(struct bp_input *bp);
void
bp_bimodal_handler(struct bp_input *bp, uint32_t pc, bool taken);
inline uint32_t
bp_bimodal_get_index(int pc, uint16_t m);
bool
bp_bimodal_lookup(struct bp_input *bp, uint32_t pc, bool taken);
void
bp_bimodal_update(struct bp_input *bp, uint32_t pc, bool taken);

void
bp_gshare_init(struct bp_input *gs);
void
bp_gshare_cleanup(struct bp_input *bp);
void
bp_gshare_handler(struct bp_input *bp, uint32_t pc, bool taken);
inline uint32_t
bp_gshare_get_index(uint32_t pc, struct bp_gshare *gs);
bool
bp_gshare_lookup(struct bp_input *bp, uint32_t pc, bool taken);
inline void
bp_gshare_update_bhr(struct bp_input *bp, bool taken);
void
bp_gshare_update(struct bp_input *bp, uint32_t pc, bool taken);

void
bp_hybrid_init(struct bp_input *gs);
void
bp_hybrid_cleanup(struct bp_input *bp);
void
bp_hybrid_handler(struct bp_input *bp, uint32_t pc, bool taken);
uint32_t
bp_hybrid_get_index(uint32_t pc, struct bp_hybrid *hy);

#endif /* BP_H_ */

