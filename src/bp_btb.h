/* 
 * ECE 521 - Computer Design Techniques, Fall 2014
 * Project 2 - Branch Predictor Implementation
 *
 * This module contains almost all required data structures, constants and
 * function declrations for BTB implementation for branch predictor.
 *
 * Author: Aravindhan Dhanasekaran <adhanas@ncsu.edu>
 */

#ifndef BP_BTB_H_
#define BP_BTB_H_

#ifndef bool
#define bool unsigned char
#endif /* !bool */

/* Constants */
#ifndef TRUE
#define TRUE    1
#endif /* TRUE */
#ifndef FALSE
#define FALSE   0
#endif /* FALSE */

#define CACHE_LEVEL_1           1
#define CACHE_LEVEL_2           2
#define CACHE_LEVEL_3           3
#define CACHE_NAME_LEN          24
#define CACHE_ADDR_32BIT_LEN    32
#define CACHE_TRACE_FILE_LEN    256
#define CACHE_DEF_BLK_SIZE      4

#define CACHE_REPL_PLCY_LRU     0
#define CACHE_REPL_PLCY_LFU     1
#define CACHE_WRITE_PLCY_WBWA   0
#define CACHE_WRITE_PLCY_WTNA   1

#define MEM_REF_TYPE_READ       'r'
#define MEM_REF_TYPE_WRITE      'w'

/* Standard typedefs */
typedef unsigned char uchar;
typedef unsigned char boolean;
typedef enum cache_rv__ {
    CACHE_RV_ERR = -1,
    CACHE_RV_OK = 0
} cache_rv;

/* Memory reference: address and refernce type */
typedef struct mem_ref__ {
    uint8_t     ref_type;
    uint32_t    ref_addr;
} mem_ref_t;

/* Cahce line: addr = <tag, index, blk_offset> */
typedef struct cache_line__ {
    uint32_t    tag;
    uint32_t    index;
    uint32_t    offset;
} cache_line_t;

typedef struct cache_tag_data__ {
    uint64_t        age;                    /* age of this block (LRU)  */
    uint32_t        ref_count;              /* ref. count (LFU)         */
    uint8_t         valid;                  /* valid bit of the block   */
    uint8_t         dirty;                  /* dirty bit of the block   */
} cache_tag_data_t;

/* Cache tag store data structure */
typedef struct cache_tagstore__ {
    void                *cache;                 /* ptr ot parent cache      */
    uint32_t            num_sets;               /* # of sets in cache       */
    uint32_t            num_blocks;             /* # of blocks in cache     */
    uint32_t            num_blocks_per_set;     /* # of blocks per set      */
    uint8_t             num_tag_bits;           /* # of bits for tags       */
    uint8_t             num_index_bits;         /* # of bits for index      */
    uint8_t             num_offset_bits;        /* # of bits for blk offset */
    uint32_t            *index;                 /* ptr to tag indices       */
    uint32_t            *tags;                  /* ptr to tag array         */
    cache_tag_data_t    *tag_data;              /* ptr to tag stats         */
    uint32_t            *set_ref_count;         /* row-wise ref count (LFU) */
} cache_tagstore_t;

/* Cache statistics data structure */
typedef struct cache_stats__ {
    uint32_t            num_reads;              /* # of reads               */
    uint32_t            num_writes;             /* # of writes              */
    uint32_t            num_read_hits;          /* # of read hits           */ 
    uint32_t            num_write_hits;         /* # of write hits          */ 
    uint32_t            num_read_misses;        /* # of read misses         */
    uint32_t            num_write_misses;       /* # of write misses        */
    uint32_t            num_write_backs;        /* # of write backs         */
    uint32_t            num_blk_mem_traffic;    /* # of blks transferred    */
    void                *cache;                 /* ptr to parent cache      */
} cache_stats_t;

/* Generic cache data structure */
typedef struct cache_generic__ {
    char                name[CACHE_NAME_LEN];   /* name - L1, L2..          */
    char                trace_file[CACHE_TRACE_FILE_LEN];
    uint8_t             level;                  /* 1, 2, 3 ..               */
    uint16_t            set_assoc;              /* level of associativity   */
    uint32_t            blk_size;               /* cache block size         */
    uint32_t            size;                   /* total cache size         */
    uint8_t             repl_plcy;              /* replacement policy       */
    uint8_t             write_plcy;             /* write policy             */
    cache_stats_t       stats;                  /* cache statistics         */
    cache_tagstore_t    *tagstore;              /* associated tagstore      */
    struct cache_generic__ *next_cache;         /* next higher level cache  */
    struct cache_generic__ *prev_cache;         /* prev lower level cache   */
    uint32_t            nhits;
    uint32_t            nmisses;
    uint32_t            nmiss_predicts;
} cache_generic_t;


/* Function declarations */
void
cache_init(cache_generic_t *pcache, const char *name, 
        const char *trace_file, uint8_t level);
void
cache_cleanup(cache_generic_t *pcache);
void
cache_tagstore_init(cache_generic_t *cache, cache_tagstore_t *tagstore);
void
cache_tagstore_cleanup(cache_generic_t *cache, cache_tagstore_t *tagstore);
bool
cache_handle_memory_request(cache_generic_t *cache, mem_ref_t *mem_ref);
void
cache_handle_read_request(cache_generic_t *cache, mem_ref_t *mem_ref, 
        cache_line_t *line);
void
cache_handle_write_request(cache_generic_t *cache, mem_ref_t *mem_ref, 
        cache_line_t *line);
int32_t
cache_get_first_invalid_block(cache_tagstore_t *tagstore, cache_line_t *line);
int32_t
cache_does_tag_match(cache_tagstore_t *tagstore, cache_line_t *line);
int32_t
cache_get_lru_block(cache_tagstore_t *tagstore, mem_ref_t *mref,
        cache_line_t *line);
int32_t
cache_get_lfu_block(cache_tagstore_t *tagstore, mem_ref_t *mref,
    cache_line_t *line);
int32_t
cache_evict_tag(cache_generic_t *cache, mem_ref_t *mref, cache_line_t *line);
void
cache_handle_dirty_tag_evicts(cache_tagstore_t *tagstore, cache_line_t *line, 
        uint32_t block_id);
void
cache_print_sim_data(cache_generic_t *cache);

#endif /* BP_BTB_H_ */

