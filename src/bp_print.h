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
extern void
bp_print_input(struct bp_input *bp);
extern void
bp_print_bimodal_curr_entry(uint8_t *table, uint32_t index, int pc, bool taken,
        uint8_t old_value);
#endif /* DBG_ON */

#endif /* BP_PRINT_H_ */

