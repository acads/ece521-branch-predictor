/* 
 * ECE 521 - Computer Design Techniques, Fall 2014
 * Project 2 - Branch Predictor Implementation
 *
 * This module contains all required data strcutures and function declrations 
 * for the branch predictor implementation.
 *
 * Author: Aravindhan Dhanasekaran <adhanas@ncsu.edu>
 */

#ifndef BP_PRINT_H_
#define BP_PRINT_H_

#include "bp.h"
#include "bp_utils.h"

/* Externs */
void
bp_print_stats(struct bp_input *bp);

#ifdef DBG_ON
void
bp_print_input(struct bp_input *bp);
void
bp_print_bimodal_curr_entry(struct bp_input *bp, uint32_t index, uint32_t pc,
        bool taken, uint8_t old_value);
void
bp_print_gshare_curr_entry(struct bp_gshare *gs, uint32_t index, uint32_t pc,
        bool taken, uint8_t old_value);
void
bp_print_hybrid_curr_entry(struct bp_input *bp, uint32_t pc, bool taken,
        uint32_t predictor, uint8_t pred_old_value, uint8_t old_value);
#endif /* DBG_ON */

#endif /* BP_PRINT_H_ */

