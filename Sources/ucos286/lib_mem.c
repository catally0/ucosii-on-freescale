/*
*********************************************************************************************************
*                                               uC/LIB
*                                       CUSTOM LIBRARY MODULES
*
*                          (c) Copyright 2004-2008; Micrium, Inc.; Weston, FL
*
*               All rights reserved.  Protected by international copyright laws.
*
*               uC/LIB is provided in source form for FREE evaluation, for educational
*               use or peaceful research.  If you plan on using uC/LIB in a commercial
*               product you need to contact Micrium to properly license its use in your
*               product.  We provide ALL the source code for your convenience and to
*               help you experience uC/LIB.  The fact that the source code is provided
*               does NOT mean that you can use it without paying a licensing fee.
*
*               Knowledge of the source code may NOT be used to develop a similar product.
*
*               Please help us continue to provide the Embedded community with the finest
*               software available.  Your honesty is greatly appreciated.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                     STANDARD MEMORY OPERATIONS
*
* Filename      : lib_mem.c
* Version       : V1.25
* Programmer(s) : ITJ
*                 FGK
*********************************************************************************************************
* Note(s)       : (1) NO compiler-supplied standard library functions are used in library or product software.
*
*                     (a) ALL standard library functions are implemented in the custom library modules :
*
*                         (1) \<Custom Library Directory>\lib*.*
*
*                         (2) \<Custom Library Directory>\Ports\<cpu>\<compiler>\lib*_a.*
*
*                               where
*                                       <Custom Library Directory>      directory path for custom library software
*                                       <cpu>                           directory name for specific processor (CPU)
*                                       <compiler>                      directory name for specific compiler
*
*                     (b) Product-specific library functions are implemented in individual products.
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/

#define    LIB_MEM_MODULE
#include  "lib_mem.h"


/*$PAGE*/
/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                           LOCAL CONSTANTS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                          LOCAL DATA TYPES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                            LOCAL TABLES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/

#if (LIB_MEM_CFG_POOL_EN == DEF_ENABLED)
CPU_INT08U   Mem_Heap[LIB_MEM_CFG_HEAP_SIZE];                   /* General-purpose memory heap segment.                     */
MEM_POOL    *Mem_PoolTbl;
MEM_POOL     Mem_PoolHeap;
#endif


/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/

#if (LIB_MEM_CFG_POOL_EN == DEF_ENABLED)
static  CPU_SIZE_T   Mem_PoolSegGetBlkSize(void        *pmem_addr,
                                           CPU_SIZE_T   blk_nbr,
                                           CPU_SIZE_T   size,
                                           CPU_SIZE_T   align);

static  void        *Mem_PoolSegAlloc     (MEM_POOL    *pmem_pool,
                                           CPU_SIZE_T   size,
                                           CPU_SIZE_T   align);
#endif


/*
*********************************************************************************************************
*                                     LOCAL CONFIGURATION ERRORS
*********************************************************************************************************
*/


/*$PAGE*/
/*
*********************************************************************************************************
*                                           Mem_Init()
*
* Description : (1) Initialize Memory Management Module :
*
*                   (a) Initialize memory pools :
*                       (1) Initialize memory pool table
*                       (2) Initialize memory pool table index
*                       (b) Initialize memory pool heap  index
*
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  Mem_Init (void) 
{
#if (LIB_MEM_CFG_POOL_EN == DEF_ENABLED)
    MEM_POOL  *pmem_pool;


                                                                /* Init heap mem seg pool                               */
    pmem_pool = &Mem_PoolHeap;
    pmem_pool->Type                = (LIB_MEM_TYPE)LIB_MEM_TYPE_HEAP;
    pmem_pool->PoolPtrs            = (void      **)0;
    pmem_pool->PoolAddrPtr         = (void       *)0;
    pmem_pool->BlkSize             = (CPU_SIZE_T  )0;
    pmem_pool->BlkNbr              = (CPU_SIZE_T  )0;
    pmem_pool->BlkIx               = (MEM_POOL_IX )0;
    pmem_pool->SegAddrNextAvailPtr = (void       *)&Mem_Heap[0];
    pmem_pool->SegAddrPtr          = (void       *)&Mem_Heap[0];
    pmem_pool->SegSizeTot          = (CPU_SIZE_T  )LIB_MEM_CFG_HEAP_SIZE;
    pmem_pool->SegSizeRem          = (CPU_SIZE_T  )LIB_MEM_CFG_HEAP_SIZE;
    pmem_pool->SegPrimListPrevPtr  = (MEM_POOL   *)0;
    pmem_pool->SegPrimListNextPtr  = (MEM_POOL   *)0;
    pmem_pool->SegSecListPrevPtr   = (MEM_POOL   *)0;
    pmem_pool->SegSecListNextPtr   = (MEM_POOL   *)0;

    Mem_PoolTbl = &Mem_PoolHeap;
#endif
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                              Mem_Clr()
*
* Description : Clear data buffer (see Note #2).
*
* Argument(s) : pmem        Pointer to memory buffer to clear.
*
*               size        Number of data buffer octets to clear.
*
* Return(s)   : none.
*
* Caller(s)   : various.
*
* Note(s)     : (1) Null clears allowed (i.e. 0-octet size).
*
*                   See also 'Mem_Set()  Note #1'.
*
*               (2) Clear data by setting each data octet to 0.
*********************************************************************************************************
*/

void  Mem_Clr (void        *pmem,
               CPU_SIZE_T   size)
{
    Mem_Set((void     *)pmem,
            (CPU_INT08U)0,                                      /* See Note #2.                                         */
            (CPU_SIZE_T)size);
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                              Mem_Set()
*
* Description : Fill data buffer with specified data octet.
*
* Argument(s) : pmem        Pointer to memory buffer to fill with specified data octet.
*
*               data_val    Data fill octet value.
*
*               size        Number of data buffer octets to fill.
*
* Return(s)   : none.
*
* Caller(s)   : various.
*
* Note(s)     : (1) Null sets allowed (i.e. 0-octet size).
*
*               (2) For best CPU performance, optimized to fill data buffer using 'CPU_ALIGN'-sized data 
*                   words.
*
*                   (a) Since many word-aligned processors REQUIRE that multi-octet words be accessed on 
*                       word-aligned addresses, 'CPU_ALIGN'-sized words MUST be accessed on 'CPU_ALIGN'd
*                       addresses.
*
*               (3) Modulo arithmetic is used to determine whether a memory buffer starts on a 'CPU_ALIGN'
*                   address boundary.
*
*                   Modulo arithmetic in ANSI-C REQUIREs operations performed on integer values.  Thus, 
*                   address values MUST be cast to an appropriately-sized integer value PRIOR to any
*                   mem_align_modulo arithmetic operation.
*********************************************************************************************************
*/

void  Mem_Set (void        *pmem,
               CPU_INT08U   data_val,
               CPU_SIZE_T   size)
{
    CPU_SIZE_T   size_rem;
    CPU_ALIGN    data_align;
    CPU_ALIGN   *pmem_align;
    CPU_INT08U  *pmem_08;
    CPU_INT08U   mem_align_modulo;
    CPU_INT08U   i;


    if (size < 1) {                                             /* See Note #1.                                         */
        return;
    }
    if (pmem == (void *)0) {
        return;
    }


    data_align = 0;
    for (i = 0; i < sizeof(CPU_ALIGN); i++) {                   /* Fill each data_align octet with data val.            */
        data_align <<=  DEF_OCTET_NBR_BITS;
        data_align  |= (CPU_ALIGN)data_val;
    }

    size_rem         = (CPU_SIZE_T)size;
    mem_align_modulo = (CPU_INT08U)((CPU_ADDR)pmem % sizeof(CPU_ALIGN));    /* See Note #3.                             */

    pmem_08 = (CPU_INT08U *)pmem;
    if (mem_align_modulo != 0) {                                /* If leading octets avail,                   ...       */
        i = mem_align_modulo;
        while ((size_rem > 0) &&                                /* ... start mem buf fill with leading octets ...       */
               (i        < sizeof(CPU_ALIGN ))) {               /* ... until next CPU_ALIGN word boundary.              */
           *pmem_08++ = data_val;
            size_rem -= sizeof(CPU_INT08U);
            i++;
        }
    }

    pmem_align = (CPU_ALIGN *)pmem_08;                          /* See Note #2a.                                        */
    while (size_rem >= sizeof(CPU_ALIGN)) {                     /* While mem buf aligned on CPU_ALIGN word boundaries,  */
       *pmem_align++ = data_align;                              /* ... fill mem buf with    CPU_ALIGN-sized data.       */
        size_rem    -= sizeof(CPU_ALIGN);
    }

    pmem_08 = (CPU_INT08U *)pmem_align;
    while (size_rem > 0) {                                      /* Finish mem buf fill with trailing octets.            */
       *pmem_08++   = data_val;
        size_rem   -= sizeof(CPU_INT08U);
    }
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                             Mem_Copy()
*
* Description : Copy data octets from one buffer to another buffer.
*
* Argument(s) : pdest       Pointer to destination memory buffer.
*
*               psrc        Pointer to source      memory buffer.
*
*               size        Number of data buffer octets to copy.
*
* Return(s)   : none.
*
* Caller(s)   : various.
*
* Note(s)     : (1) Null copies allowed (i.e. 0-octet size).
*
*               (2) Memory buffers NOT checked for overlapping.
*
*               (3) For best CPU performance, optimized to copy data buffer using 'CPU_ALIGN'-sized data 
*                   words.
*
*                   (a) Since many word-aligned processors REQUIRE that multi-octet words be accessed on 
*                       word-aligned addresses, 'CPU_ALIGN'-sized words MUST be accessed on 'CPU_ALIGN'd
*                       addresses.
*
*               (4) Modulo arithmetic is used to determine whether a memory buffer starts on a 'CPU_ALIGN'
*                   address boundary.
*
*                   Modulo arithmetic in ANSI-C REQUIREs operations performed on integer values.  Thus, 
*                   address values MUST be cast to an appropriately-sized integer value PRIOR to any
*                   mem_align_modulo arithmetic operation.
*********************************************************************************************************
*/
/*$PAGE*/
#if ((!defined(uC_CFG_OPTIMIZE_ASM_EN)) || \
     ((defined(uC_CFG_OPTIMIZE_ASM_EN)) && \
              (uC_CFG_OPTIMIZE_ASM_EN   != DEF_ENABLED)))
void  Mem_Copy (void        *pdest,
                void        *psrc,
                CPU_SIZE_T   size)
{
    CPU_SIZE_T    size_rem;
    CPU_ALIGN    *pmem_align_dest;
    CPU_ALIGN    *pmem_align_src;
    CPU_INT08U   *pmem_08_dest;
    CPU_INT08U   *pmem_08_src;
    CPU_INT08U    i;
    CPU_INT08U    mem_align_modulo_dest;
    CPU_INT08U    mem_align_modulo_src;
    CPU_BOOLEAN   mem_aligned;


    if (size < 1) {                                             /* See Note #1.                                         */
        return;
    }
    if (pdest == (void *)0) {
        return;
    }
    if (psrc  == (void *)0) {
        return;
    }


    size_rem              = (CPU_SIZE_T  )size;

    pmem_08_dest          = (CPU_INT08U *)pdest;
    pmem_08_src           = (CPU_INT08U *)psrc;
                                                                /* See Note #4.                                         */
    mem_align_modulo_dest = (CPU_INT08U  )((CPU_ADDR)pmem_08_dest % sizeof(CPU_ALIGN));
    mem_align_modulo_src  = (CPU_INT08U  )((CPU_ADDR)pmem_08_src  % sizeof(CPU_ALIGN));

    mem_aligned           = (mem_align_modulo_dest == mem_align_modulo_src) ? DEF_YES : DEF_NO;

    if (mem_aligned == DEF_YES) {                               /* If mem bufs' alignment offset equal, ...             */
                                                                /* ... optimize copy for mem buf alignment.             */
        if (mem_align_modulo_dest != 0) {                       /* If leading octets avail,                   ...       */
            i = mem_align_modulo_dest;
            while ((size_rem   >  0) &&                         /* ... start mem buf copy with leading octets ...       */
                   (i          <  sizeof(CPU_ALIGN ))) {        /* ... until next CPU_ALIGN word boundary.              */
               *pmem_08_dest++ = *pmem_08_src++;
                size_rem      -=  sizeof(CPU_INT08U);
                i++;
            }
        }

        pmem_align_dest = (CPU_ALIGN *)pmem_08_dest;            /* See Note #3a.                                        */
        pmem_align_src  = (CPU_ALIGN *)pmem_08_src;
        while (size_rem      >=  sizeof(CPU_ALIGN)) {           /* While mem bufs aligned on CPU_ALIGN word boundaries, */
           *pmem_align_dest++ = *pmem_align_src++;              /* ... copy psrc to pdest with CPU_ALIGN-sized words.   */
            size_rem         -=  sizeof(CPU_ALIGN);
        }

        pmem_08_dest = (CPU_INT08U *)pmem_align_dest;
        pmem_08_src  = (CPU_INT08U *)pmem_align_src;
    }

    while (size_rem > 0) {                                      /* For unaligned mem bufs or trailing octets, ...       */
       *pmem_08_dest++ = *pmem_08_src++;                        /* ... copy psrc to pdest by octets.                    */
        size_rem      -=  sizeof(CPU_INT08U);
    }
}
#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                              Mem_Cmp()
*
* Description : Verify that ALL data octets in two memory buffers are identical in sequence.
*
* Argument(s) : p1_mem      Pointer to first  memory buffer.
*
*               p2_mem      Pointer to second memory buffer.
*
*               size        Number of data buffer octets to compare.
*
* Return(s)   : DEF_YES, if 'size' number of data octets are identical in both memory buffers.
*
*               DEF_NO,  otherwise.
*
* Caller(s)   : various.
*
* Note(s)     : (1) Null compares allowed (i.e. 0-octet size); 'DEF_YES' returned to indicate identical 
*                   null compare.
*
*               (2) Many memory buffer comparisons vary ONLY in the least significant octets -- e.g. 
*                   network address buffers.  Consequently, memory buffer comparison is more efficient 
*                   if the comparison starts from the end of the memory buffers which will abort sooner 
*                   on dissimilar memory buffers that vary only in the least significant octets.
*
*               (3) For best CPU performance, optimized to compare data buffers using 'CPU_ALIGN'-sized 
*                   data words.
*
*                   (a) Since many word-aligned processors REQUIRE that multi-octet words be accessed on 
*                       word-aligned addresses, 'CPU_ALIGN'-sized words MUST be accessed on 'CPU_ALIGN'd
*                       addresses.
*
*               (4) Modulo arithmetic is used to determine whether a memory buffer starts on a 'CPU_ALIGN'
*                   address boundary.
*
*                   Modulo arithmetic in ANSI-C REQUIREs operations performed on integer values.  Thus, 
*                   address values MUST be cast to an appropriately-sized integer value PRIOR to any
*                   mem_align_modulo arithmetic operation.
********************************************************************************************************
*/
/*$PAGE*/
CPU_BOOLEAN  Mem_Cmp (void        *p1_mem,
                      void        *p2_mem,
                      CPU_SIZE_T   size)
{
    CPU_SIZE_T    size_rem;
    CPU_ALIGN    *p1_mem_align;
    CPU_ALIGN    *p2_mem_align;
    CPU_INT08U   *p1_mem_08;
    CPU_INT08U   *p2_mem_08;
    CPU_INT08U    i;
    CPU_INT08U    mem_align_modulo_1;
    CPU_INT08U    mem_align_modulo_2;
    CPU_BOOLEAN   mem_aligned;
    CPU_BOOLEAN   mem_cmp;


    if (size < 1) {                                             /* See Note #1.                                         */
        return (DEF_YES);
    }
    if (p1_mem == (void *)0) {
        return (DEF_NO);
    }
    if (p2_mem == (void *)0) {
        return (DEF_NO);
    }


    mem_cmp            =  DEF_YES;                              /* Assume mem bufs are identical until cmp fails.       */
    size_rem           =  size;
                                                                /* Start @ end of mem bufs (see Note #2).               */
    p1_mem_08          = (CPU_INT08U *)p1_mem + size;
    p2_mem_08          = (CPU_INT08U *)p2_mem + size;
                                                                /* See Note #4.                                         */
    mem_align_modulo_1 = (CPU_INT08U  )((CPU_ADDR)p1_mem_08 % sizeof(CPU_ALIGN));
    mem_align_modulo_2 = (CPU_INT08U  )((CPU_ADDR)p2_mem_08 % sizeof(CPU_ALIGN));

    mem_aligned        = (mem_align_modulo_1 == mem_align_modulo_2) ? DEF_YES : DEF_NO;

    if (mem_aligned == DEF_YES) {                               /* If mem bufs' alignment offset equal, ...             */
                                                                /* ... optimize cmp for mem buf alignment.              */
        if (mem_align_modulo_1 != 0) {                          /* If trailing octets avail,                  ...       */
            i = mem_align_modulo_1;
            while ((mem_cmp == DEF_YES) &&                      /* ... cmp mem bufs while identical &         ...       */
                   (size_rem > 0)       &&                      /* ... start mem buf cmp with trailing octets ...       */
                   (i        > 0)) {                            /* ... until next CPU_ALIGN word boundary.              */
                p1_mem_08--;
                p2_mem_08--;
                if (*p1_mem_08 != *p2_mem_08) {                 /* If ANY data octet(s) NOT identical, cmp fails.       */
                     mem_cmp = DEF_NO;
                }
                size_rem -= sizeof(CPU_INT08U);
                i--;
            }
        }

        if (mem_cmp == DEF_YES) {                               /* If cmp still identical, cmp aligned mem bufs.        */
            p1_mem_align = (CPU_ALIGN *)p1_mem_08;              /* See Note #3a.                                        */
            p2_mem_align = (CPU_ALIGN *)p2_mem_08;

            while ((mem_cmp  == DEF_YES) &&                     /* Cmp mem bufs while identical & ...                   */
                   (size_rem >= sizeof(CPU_ALIGN))) {           /* ... mem bufs aligned on CPU_ALIGN word boundaries.   */
                p1_mem_align--;
                p2_mem_align--;
                if (*p1_mem_align != *p2_mem_align) {           /* If ANY data octet(s) NOT identical, cmp fails.       */
                     mem_cmp = DEF_NO;
                }
                size_rem -= sizeof(CPU_ALIGN);
            }

            p1_mem_08 = (CPU_INT08U *)p1_mem_align;
            p2_mem_08 = (CPU_INT08U *)p2_mem_align;
        }
    }

    while ((mem_cmp == DEF_YES) &&                              /* Cmp mem bufs while identical ...                     */
           (size_rem > 0)) {                                    /* ... for unaligned mem bufs or trailing octets.       */
        p1_mem_08--;
        p2_mem_08--;
        if (*p1_mem_08 != *p2_mem_08) {                         /* If ANY data octet(s) NOT identical, cmp fails.       */
             mem_cmp = DEF_NO;
        }
        size_rem -= sizeof(CPU_INT08U);
    }

    return (mem_cmp);
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                           Mem_PoolCreate()
*
* Description : Create a memory pool.
*
* Argument(s) : pmem_base_addr      Memory pool base address :
*
*                                       Null (0) address        Memory pool allocated from general-purpose 
*                                                                   heap.
*                                       Non-null address        Memory pool allocated from specified base 
*                                                                   address of dedicated memory.
*
*               mem_size            Size      of memory pool segment (in octets).
*
*               blk_nbr             Number    of memory pool blocks to initialize.
*
*               blk_size            Size      of memory pool blocks to initialize (in octets).
*
*               blk_align_octets    Alignment of memory pool blocks to initialize (in octets).
*
*               pmem_pool           Pointer to a memory pool structure to be created.
*
*               poctets_reqd        Pointer to a variable to ... :
*
*                                       (a) Return the number of octets required to successfully 
*                                               allocate the memory pool, if any errors;
*                                       (b) Return 0, otherwise.
*
*               perr        Pointer to variable that will receive the return error code from this function :
*
*                               LIB_ERR_NONE                    Memory pool successfully initialized.
*
* Return(s)   : none.
*
* Caller(s)   : various.
*
* Note(s)     : none.
*
*               (x) Pointers to variables that return length or size values MUST be initialized to return
*                   zero PRIOR to all other validation or function handling in case of any error(s).
*********************************************************************************************************
*/

#if (LIB_MEM_CFG_POOL_EN == DEF_ENABLED)
void  Mem_PoolCreate (void        *pmem_base_addr,
                      CPU_SIZE_T   mem_size,
                      CPU_SIZE_T   blk_nbr,
                      CPU_SIZE_T   blk_size,
                      CPU_SIZE_T   blk_align_octets,
                      MEM_POOL    *pmem_pool,
                      CPU_SIZE_T  *poctets_reqd,
                      LIB_ERR     *perr)
{
#if (CPU_CFG_CRITICAL_METHOD == CPU_CRITICAL_METHOD_STATUS_LOCAL)
    CPU_SR         cpu_sr;
#endif
    MEM_POOL      *pmem_pool_tbl;
    MEM_POOL      *pmem_pool_next;
    MEM_POOL      *pmem_pool_seg;
    MEM_POOL      *pmem_pool_heap;
    CPU_INT08U    *seg_addr_ptr;
    void         **ppool_ptr;
    void          *pmem_blk;
    CPU_INT08U    *pmem_addr_stk;
    CPU_INT08U    *pmem_addr_blk;
    CPU_SIZE_T     stk_tot_size;
    CPU_SIZE_T     stk_mem_size;
    CPU_SIZE_T     blk_tot_size;
    CPU_SIZE_T     size_rem;
    CPU_SIZE_T     tot_size;
    CPU_SIZE_T     j;

                                                                /* Init rem'ing octets for err (see Note #x).               */
    if (poctets_reqd != (CPU_SIZE_T *)0) {
       *poctets_reqd  = (CPU_SIZE_T  )0;
    } else {
       *perr = LIB_MEM_ERR_NULL_PTR;
        return;
    }

                                                                /* ---------------- VALIDATE MEM POOL ARGS ---------------- */
    if (blk_nbr < 1) {
       *perr = LIB_MEM_ERR_INVALID_PARAM;
        return;
    }

    if (blk_size < 1) {
       *perr = LIB_MEM_ERR_INVALID_BLK_SIZE;
        return;
    }

    if (blk_align_octets < 1) {
        blk_align_octets = 1;
    }

    if (pmem_pool == (void *)0) {
       *perr = LIB_MEM_ERR_NULL_PTR;
        return;
    }

    if (pmem_base_addr == (void *)0) {
        seg_addr_ptr = &Mem_Heap[0];                            /* Select heap memory segment pool                          */
        mem_size     =  LIB_MEM_CFG_HEAP_SIZE;
    } else  {
        seg_addr_ptr =  pmem_base_addr;

        if (mem_size < 1) {
           *perr = LIB_MEM_ERR_INVALID_SEG_SIZE;
            return;
        }
    }

    pmem_pool->Type                = (LIB_MEM_TYPE)LIB_MEM_TYPE_POOL;
    pmem_pool->PoolPtrs            = (void      **)0;
    pmem_pool->PoolAddrPtr         = (void       *)0;
    pmem_pool->PoolSize            = (CPU_SIZE_T  )0;
    pmem_pool->BlkSize             = (CPU_SIZE_T  )blk_size;
    pmem_pool->BlkNbr              = (CPU_SIZE_T  )blk_nbr;
    pmem_pool->BlkIx               = (MEM_POOL_IX )0;
    pmem_pool->SegAddrPtr          = (void       *)seg_addr_ptr;
    pmem_pool->SegAddrNextAvailPtr = (void       *)0;
    pmem_pool->SegSizeTot          = (CPU_SIZE_T  )mem_size;
    pmem_pool->SegSizeRem          = (CPU_SIZE_T  )0;
    pmem_pool->SegPrimListPrevPtr  = (MEM_POOL   *)0;
    pmem_pool->SegPrimListNextPtr  = (MEM_POOL   *)0;
    pmem_pool->SegSecListPrevPtr   = (MEM_POOL   *)0;
    pmem_pool->SegSecListNextPtr   = (MEM_POOL   *)0;


                                                                /* Search & Add Pool into Pool Tbl                          */
    pmem_pool_tbl = Mem_PoolTbl;

    CPU_CRITICAL_ENTER();
    while (pmem_pool_tbl != (MEM_POOL *)0) {
                                                                /* Segment found, pool added on secundary list              */
        if ((seg_addr_ptr == pmem_pool_tbl->SegAddrPtr) && (mem_size == pmem_pool_tbl->SegSizeTot)) {

            pmem_pool_next                   = pmem_pool_tbl->SegSecListNextPtr;
            pmem_pool->SegSecListPrevPtr     = pmem_pool_tbl;
            pmem_pool->SegSecListNextPtr     = pmem_pool_next;
            pmem_pool_tbl->SegSecListNextPtr = pmem_pool;
            if (pmem_pool_next != (MEM_POOL *)0) {
                pmem_pool_next->SegSecListPrevPtr = pmem_pool;
            }

            pmem_pool_seg = pmem_pool_tbl;                      /* Store Pool Ptr with segment information                  */

            break;
                                                                /* Segment not found, pool added on primary list            */
        } else if ((seg_addr_ptr + mem_size) <= pmem_pool_tbl->SegAddrPtr) {

            pmem_pool_tbl->SegPrimListPrevPtr = pmem_pool;
            pmem_pool->SegPrimListNextPtr     = pmem_pool_tbl;

            if (pmem_pool_tbl == Mem_PoolTbl) {                 /* Update Mem_PoolTbl since seg is the head of link list    */
                Mem_PoolTbl = pmem_pool;
            }

                                                                /* Initialize segment information                           */
            pmem_pool->SegAddrNextAvailPtr = pmem_pool->SegAddrPtr;
            pmem_pool->SegSizeRem          = pmem_pool->SegSizeTot;

            pmem_pool_seg = pmem_pool;                          /* Store Pool Ptr with segment information                  */

            break;
                                                                /* Segment found, and falls inside a previously defined one */
        } else if ((seg_addr_ptr >= pmem_pool_tbl->SegAddrPtr) &&
                   (seg_addr_ptr < (void *)((CPU_INT32U)(pmem_pool_tbl->SegAddrPtr) + pmem_pool_tbl->SegSizeTot))) {
            CPU_CRITICAL_EXIT();
           *perr = LIB_MEM_ERR_SEG_OVERLAP;
                                                                /* Clear pmem_pool structure on error                       */
            pmem_pool->Type                = (LIB_MEM_TYPE)LIB_MEM_TYPE_NONE;
            pmem_pool->PoolPtrs            = (void      **)0;
            pmem_pool->PoolAddrPtr         = (void       *)0;
            pmem_pool->PoolSize            = (CPU_SIZE_T  )0;
            pmem_pool->BlkSize             = (CPU_SIZE_T  )0;
            pmem_pool->BlkNbr              = (CPU_SIZE_T  )0;
            pmem_pool->BlkIx               = (MEM_POOL_IX )0;
            pmem_pool->SegAddrPtr          = (void       *)0;
            pmem_pool->SegAddrNextAvailPtr = (void       *)0;
            pmem_pool->SegSizeTot          = (CPU_SIZE_T  )0;
            pmem_pool->SegSizeRem          = (CPU_SIZE_T  )0;
            pmem_pool->SegPrimListPrevPtr  = (MEM_POOL   *)0;
            pmem_pool->SegPrimListNextPtr  = (MEM_POOL   *)0;
            pmem_pool->SegSecListPrevPtr   = (MEM_POOL   *)0;
            pmem_pool->SegSecListNextPtr   = (MEM_POOL   *)0;

            return;
        }
                                                                /* Segment not found in tbl, pool added to end of prim list */
        if (pmem_pool_tbl->SegPrimListNextPtr == (MEM_POOL *)0) {
            pmem_pool_tbl->SegPrimListNextPtr = pmem_pool;
            pmem_pool->SegPrimListPrevPtr     = pmem_pool_tbl;

                                                                /* Initialize segment information                           */
            pmem_pool->SegAddrNextAvailPtr = pmem_pool->SegAddrPtr;
            pmem_pool->SegSizeRem          = pmem_pool->SegSizeTot;

            pmem_pool_seg = pmem_pool;                          /* Store Pool Ptr with segment information                  */

            break;
        }

        pmem_pool_tbl = pmem_pool->SegPrimListNextPtr;
    }
    CPU_CRITICAL_EXIT();

                                                                /* Get Ptr to Heap memory segment pool                      */
    if (pmem_base_addr == (void *)0) {
        pmem_pool_heap = pmem_pool_seg;                         /* Segment pool is the heap memory segment pool             */
    } else  {
        pmem_pool_heap = (MEM_POOL *)0;

                                                                /* Search heap memory segment pool                          */
        pmem_pool_tbl = Mem_PoolTbl;

        CPU_CRITICAL_ENTER();
        while (pmem_pool_tbl != (MEM_POOL *)0) {
                                                                /* Heap segment found                                       */
            if ((pmem_pool_tbl->SegAddrPtr == &Mem_Heap[0]) &&
                (pmem_pool_tbl->SegSizeTot == LIB_MEM_CFG_HEAP_SIZE)) {

                pmem_pool_heap = pmem_pool_tbl;
                break;
            }

            pmem_pool_tbl = pmem_pool->SegPrimListNextPtr;
        }
        CPU_CRITICAL_EXIT();

        if (pmem_pool_heap == (MEM_POOL *)0) {
           *perr = LIB_MEM_ERR_HEAP_NOT_FOUND;

                                                                /* Delete memory pool since heap could not be found         */
/*#### warning lib_mem.c: Delete Memory Pool from Tbl */

                                                                /* Clear pmem_pool structure on error                       */
            pmem_pool->Type                = (LIB_MEM_TYPE)LIB_MEM_TYPE_NONE;
            pmem_pool->PoolPtrs            = (void      **)0;
            pmem_pool->PoolAddrPtr         = (void       *)0;
            pmem_pool->PoolSize            = (CPU_SIZE_T  )0;
            pmem_pool->BlkSize             = (CPU_SIZE_T  )0;
            pmem_pool->BlkNbr              = (CPU_SIZE_T  )0;
            pmem_pool->BlkIx               = (MEM_POOL_IX )0;
            pmem_pool->SegAddrPtr          = (void       *)0;
            pmem_pool->SegAddrNextAvailPtr = (void       *)0;
            pmem_pool->SegSizeTot          = (CPU_SIZE_T  )0;
            pmem_pool->SegSizeRem          = (CPU_SIZE_T  )0;
            pmem_pool->SegPrimListPrevPtr  = (MEM_POOL   *)0;
            pmem_pool->SegPrimListNextPtr  = (MEM_POOL   *)0;
            pmem_pool->SegSecListPrevPtr   = (MEM_POOL   *)0;
            pmem_pool->SegSecListNextPtr   = (MEM_POOL   *)0;

            return;
        }
    }

                                                                /* --------------------- MEM POOL PTRS -------------------- */
    CPU_CRITICAL_ENTER();
    if (pmem_base_addr == (void *)0) {                          /* Stack Ptrs & Memory blocks are from Heap segment         */
                                                                /* Retrieve next available address in memory segment        */
        pmem_addr_stk  = (CPU_INT08U *)pmem_pool_heap->SegAddrNextAvailPtr;
                                                                /* Compute total memory usage for memory pool pointers      */
        stk_tot_size   =  Mem_PoolSegGetBlkSize(pmem_addr_stk, blk_nbr, sizeof(void *), sizeof(void *));
        pmem_addr_blk  =  pmem_addr_stk + stk_tot_size;         /* Adjust next available address for memory blocks          */
                                                                /* Compute total memory usage for memory blocks             */
        blk_tot_size   =  Mem_PoolSegGetBlkSize(pmem_addr_blk, blk_nbr, blk_size, blk_align_octets);

        size_rem       =  pmem_pool_heap->SegSizeRem;
        tot_size       =  stk_tot_size + blk_tot_size;

        if (tot_size > size_rem) {
            CPU_CRITICAL_EXIT();

           *poctets_reqd = tot_size - size_rem;
           *perr         = LIB_MEM_ERR_HEAP_FULL;

                                                                /* Delete memory pool since heap could not be found         */
/*#### warning lib_mem.c: Delete Memory Pool from Tbl */

                                                                /* Clear pmem_pool structure on error                       */
            pmem_pool->Type                = (LIB_MEM_TYPE)LIB_MEM_TYPE_NONE;
            pmem_pool->PoolPtrs            = (void      **)0;
            pmem_pool->PoolAddrPtr         = (void       *)0;
            pmem_pool->PoolSize            = (CPU_SIZE_T  )0;
            pmem_pool->BlkSize             = (CPU_SIZE_T  )0;
            pmem_pool->BlkNbr              = (CPU_SIZE_T  )0;
            pmem_pool->BlkIx               = (MEM_POOL_IX )0;
            pmem_pool->SegAddrPtr          = (void       *)0;
            pmem_pool->SegAddrNextAvailPtr = (void       *)0;
            pmem_pool->SegSizeTot          = (CPU_SIZE_T  )0;
            pmem_pool->SegSizeRem          = (CPU_SIZE_T  )0;
            pmem_pool->SegPrimListPrevPtr  = (MEM_POOL   *)0;
            pmem_pool->SegPrimListNextPtr  = (MEM_POOL   *)0;
            pmem_pool->SegSecListPrevPtr   = (MEM_POOL   *)0;
            pmem_pool->SegSecListNextPtr   = (MEM_POOL   *)0;

            return;
        }
                                                                /* Alloc from heap the stack of pointers to memory blocks   */
        ppool_ptr = (void **)Mem_PoolSegAlloc(pmem_pool_heap, blk_nbr * sizeof(void *), sizeof(void *));

        for (j = 0; j < blk_nbr; j++) {
            pmem_blk = (void *)Mem_PoolSegAlloc(pmem_pool_heap, blk_size, blk_align_octets);

            if (pmem_blk == (void *)0) {
                CPU_CRITICAL_EXIT();

               *perr = LIB_MEM_ERR_HEAP_FULL;

                                                                /* Delete memory pool since heap could not be found         */
/*#### warning lib_mem.c: Delete Memory Pool from Tbl */

                                                                /* Clear pmem_pool structure on error                       */
                pmem_pool->Type                = (LIB_MEM_TYPE)LIB_MEM_TYPE_NONE;
                pmem_pool->PoolPtrs            = (void      **)0;
                pmem_pool->PoolAddrPtr         = (void       *)0;
                pmem_pool->PoolSize            = (CPU_SIZE_T  )0;
                pmem_pool->BlkSize             = (CPU_SIZE_T  )0;
                pmem_pool->BlkNbr              = (CPU_SIZE_T  )0;
                pmem_pool->BlkIx               = (MEM_POOL_IX )0;
                pmem_pool->SegAddrPtr          = (void       *)0;
                pmem_pool->SegAddrNextAvailPtr = (void       *)0;
                pmem_pool->SegSizeTot          = (CPU_SIZE_T  )0;
                pmem_pool->SegSizeRem          = (CPU_SIZE_T  )0;
                pmem_pool->SegPrimListPrevPtr  = (MEM_POOL   *)0;
                pmem_pool->SegPrimListNextPtr  = (MEM_POOL   *)0;
                pmem_pool->SegSecListPrevPtr   = (MEM_POOL   *)0;
                pmem_pool->SegSecListNextPtr   = (MEM_POOL   *)0;

                return;
            }

            ppool_ptr[j] = pmem_blk;
        }

    } else {                                                    /* Stack Ptrs from heap & Memory blocks from specific seg   */
                                                                /* Retrieve next available address in heap                  */
        pmem_addr_stk  = (CPU_INT08U *)pmem_pool_heap->SegAddrNextAvailPtr;
                                                                /* Compute total memory usage for memory pool pointers      */
        stk_tot_size   =  Mem_PoolSegGetBlkSize(pmem_addr_stk, blk_nbr, sizeof(void *), sizeof(void *));
        size_rem       =  pmem_pool_heap->SegSizeRem;

        if (stk_tot_size > size_rem) {
            CPU_CRITICAL_EXIT();

           *poctets_reqd = stk_tot_size - size_rem;
           *perr         = LIB_MEM_ERR_HEAP_FULL;

                                                                /* Delete memory pool since heap could not be found         */
/*#### warning lib_mem.c: Delete Memory Pool from Tbl */

                                                                /* Clear pmem_pool structure on error                       */
            pmem_pool->Type                = (LIB_MEM_TYPE)LIB_MEM_TYPE_NONE;
            pmem_pool->PoolPtrs            = (void      **)0;
            pmem_pool->PoolAddrPtr         = (void       *)0;
            pmem_pool->PoolSize            = (CPU_SIZE_T  )0;
            pmem_pool->BlkSize             = (CPU_SIZE_T  )0;
            pmem_pool->BlkNbr              = (CPU_SIZE_T  )0;
            pmem_pool->BlkIx               = (MEM_POOL_IX )0;
            pmem_pool->SegAddrPtr          = (void       *)0;
            pmem_pool->SegAddrNextAvailPtr = (void       *)0;
            pmem_pool->SegSizeTot          = (CPU_SIZE_T  )0;
            pmem_pool->SegSizeRem          = (CPU_SIZE_T  )0;
            pmem_pool->SegPrimListPrevPtr  = (MEM_POOL   *)0;
            pmem_pool->SegPrimListNextPtr  = (MEM_POOL   *)0;
            pmem_pool->SegSecListPrevPtr   = (MEM_POOL   *)0;
            pmem_pool->SegSecListNextPtr   = (MEM_POOL   *)0;

            return;
        }

                                                                /* Retrieve next available address in memory segment        */
        pmem_addr_blk  = (CPU_INT08U *)pmem_pool_seg->SegAddrNextAvailPtr;
                                                                /* Compute total memory usage for memory blocks             */
        blk_tot_size   =  Mem_PoolSegGetBlkSize(pmem_addr_blk, blk_nbr, blk_size, blk_align_octets);
        size_rem       =  pmem_pool_seg->SegSizeRem;

        if (blk_tot_size > size_rem) {
            CPU_CRITICAL_EXIT();

           *poctets_reqd = blk_tot_size - size_rem;
           *perr         = LIB_MEM_ERR_SEG_FULL;

                                                                /* Delete memory pool since heap could not be found         */
/*#### warning lib_mem.c: Delete Memory Pool from Tbl */

                                                                /* Clear pmem_pool structure on error                       */
            pmem_pool->Type                = (LIB_MEM_TYPE)LIB_MEM_TYPE_NONE;
            pmem_pool->PoolPtrs            = (void      **)0;
            pmem_pool->PoolAddrPtr         = (void       *)0;
            pmem_pool->PoolSize            = (CPU_SIZE_T  )0;
            pmem_pool->BlkSize             = (CPU_SIZE_T  )0;
            pmem_pool->BlkNbr              = (CPU_SIZE_T  )0;
            pmem_pool->BlkIx               = (MEM_POOL_IX )0;
            pmem_pool->SegAddrPtr          = (void       *)0;
            pmem_pool->SegAddrNextAvailPtr = (void       *)0;
            pmem_pool->SegSizeTot          = (CPU_SIZE_T  )0;
            pmem_pool->SegSizeRem          = (CPU_SIZE_T  )0;
            pmem_pool->SegPrimListPrevPtr  = (MEM_POOL   *)0;
            pmem_pool->SegPrimListNextPtr  = (MEM_POOL   *)0;
            pmem_pool->SegSecListPrevPtr   = (MEM_POOL   *)0;
            pmem_pool->SegSecListNextPtr   = (MEM_POOL   *)0;

            return;
        }
                                                                /* Alloc stack of pointers for mem blks from heap   */
        ppool_ptr = (void **)Mem_PoolSegAlloc(pmem_pool_heap, blk_nbr * sizeof(void *), sizeof(void *));

        for (j = 0; j < blk_nbr; j++) {
            pmem_blk = (void *)Mem_PoolSegAlloc(pmem_pool_seg, blk_size, blk_align_octets);

            if (pmem_blk == (void *)0) {
                CPU_CRITICAL_EXIT();

               *perr = LIB_MEM_ERR_SEG_FULL;

                                                                /* Delete memory pool since heap could not be found         */
/*#### warning lib_mem.c: Delete Memory Pool from Tbl */

                                                                /* Clear pmem_pool structure on error                       */
                pmem_pool->Type                = (LIB_MEM_TYPE)LIB_MEM_TYPE_NONE;
                pmem_pool->PoolPtrs            = (void      **)0;
                pmem_pool->PoolAddrPtr         = (void       *)0;
                pmem_pool->PoolSize            = (CPU_SIZE_T  )0;
                pmem_pool->BlkSize             = (CPU_SIZE_T  )0;
                pmem_pool->BlkNbr              = (CPU_SIZE_T  )0;
                pmem_pool->BlkIx               = (MEM_POOL_IX )0;
                pmem_pool->SegAddrPtr          = (void       *)0;
                pmem_pool->SegAddrNextAvailPtr = (void       *)0;
                pmem_pool->SegSizeTot          = (CPU_SIZE_T  )0;
                pmem_pool->SegSizeRem          = (CPU_SIZE_T  )0;
                pmem_pool->SegPrimListPrevPtr  = (MEM_POOL   *)0;
                pmem_pool->SegPrimListNextPtr  = (MEM_POOL   *)0;
                pmem_pool->SegSecListPrevPtr   = (MEM_POOL   *)0;
                pmem_pool->SegSecListNextPtr   = (MEM_POOL   *)0;

                return;
            }

            ppool_ptr[j] = pmem_blk;
        }
    }

    pmem_pool->PoolPtrs    = ppool_ptr;
    pmem_pool->PoolAddrPtr = pmem_addr_blk;
    pmem_pool->PoolSize    = blk_tot_size;
    pmem_pool->BlkIx       = blk_nbr;

    CPU_CRITICAL_EXIT();

   *perr = LIB_ERR_NONE;
}
#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                          Mem_PoolBlkGet()
*
* Description : Retrieve a memory block from memory pool.
*
* Argument(s) : mem_pool_ix     Index of memory pool to get memory block.
*
*               mem_size        Size of requested memory (in octets).
*
*               perr            Pointer to variable that will receive the return error code from this function :
*
*                                   LIB_ERR_NONE                    Memory block retrieved successfully.
*
* Return(s)   : Pointer to memory block, if NO errors.
*
*               Pointer to NULL,         otherwise.
*
* Caller(s)   : various.
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if (LIB_MEM_CFG_POOL_EN == DEF_ENABLED)
void  *Mem_PoolBlkGet (MEM_POOL     *pmem_pool,
                       CPU_SIZE_T    size,
                       LIB_ERR      *perr)
{
#if (CPU_CFG_CRITICAL_METHOD == CPU_CRITICAL_METHOD_STATUS_LOCAL)
    CPU_SR   cpu_sr;
#endif
    void    *pmem_blk;


    if (pmem_pool == (MEM_POOL *)0) {
       *perr = LIB_MEM_ERR_NULL_PTR;
        return ((void *)0);
    }

    CPU_CRITICAL_ENTER();
    if (pmem_pool->Type != LIB_MEM_TYPE_POOL) {
        CPU_CRITICAL_EXIT();
       *perr = LIB_MEM_ERR_INVALID_POOL;
        return ((void *)0);
    }

    if (size > pmem_pool->BlkSize) {
        CPU_CRITICAL_EXIT();
       *perr = LIB_MEM_ERR_INVALID_BLK_SIZE;
        return ((void *)0);
    }

    if (pmem_pool->BlkIx == 0) {
        CPU_CRITICAL_EXIT();
       *perr = LIB_MEM_ERR_NONE_AVAIL;
        return ((void *)0);
    }

#if 1
    if (pmem_pool->BlkIx > pmem_pool->BlkNbr) {
        CPU_CRITICAL_EXIT();
       *perr = /*LIB_MEM_ERR_MEM_BLK_STK_CORRUPTED*/0;
        return ((void *)0);
    }
#endif

    pmem_pool->BlkIx--;
    pmem_blk = pmem_pool->PoolPtrs[pmem_pool->BlkIx];
    CPU_CRITICAL_EXIT();

   *perr = LIB_ERR_NONE;
    return (pmem_blk);
}
#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                          Mem_PoolBlkFree()
*
* Description : Return a memory block to memory pool.
*
* Argument(s) : mem_pool_ix     Memory pool index.
*
*               pmem_addr       Pointer to memory block address to free.
*
*               perr            Pointer to variable that will receive the return error code from this function :
*
*                                   LIB_ERR_NONE                    Memory block successfully freed.
*
* Return(s)   : Pointer to memory block, if NO errors.
*
*               Pointer to NULL,         otherwise.
*
* Caller(s)   : various.
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if (LIB_MEM_CFG_POOL_EN == DEF_ENABLED)
void  Mem_PoolBlkFree (MEM_POOL     *pmem_pool,
                       void         *pmem_addr,
                       LIB_ERR      *perr)
{
#if (CPU_CFG_CRITICAL_METHOD == CPU_CRITICAL_METHOD_STATUS_LOCAL)
    CPU_SR   cpu_sr;
#endif
    void    *pmem_blk_end;

    if (pmem_pool == (MEM_POOL *)0) {
       *perr = LIB_MEM_ERR_NULL_PTR;
        return;
    }

    if (pmem_addr == (void *)0) {
       *perr = LIB_MEM_ERR_NULL_PTR;
        return;
    }

    CPU_CRITICAL_ENTER();
    if (pmem_pool->Type != LIB_MEM_TYPE_POOL) {
        CPU_CRITICAL_EXIT();
       *perr = LIB_MEM_ERR_INVALID_POOL;
        return;
    }

    if (pmem_pool->BlkIx >= pmem_pool->BlkNbr) {
        CPU_CRITICAL_EXIT();
       *perr = LIB_MEM_ERR_POOL_FULL;
        return;
    }

    pmem_blk_end = (void *)(((CPU_INT08U *)pmem_pool->PoolAddrPtr) + pmem_pool->PoolSize - pmem_pool->BlkSize);

    if ((pmem_addr < pmem_pool->PoolAddrPtr) &&
        (pmem_addr > pmem_blk_end)) {
        CPU_CRITICAL_EXIT();
       *perr = LIB_MEM_ERR_INVALID_ADDR;
        return;
    }

    pmem_pool->PoolPtrs[pmem_pool->BlkIx] = pmem_addr;
    pmem_pool->BlkIx++;
    CPU_CRITICAL_EXIT();

   *perr = LIB_ERR_NONE;
}
#endif


/*$PAGE*/
/*
*********************************************************************************************************
*********************************************************************************************************
*                                           LOCAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                        Mem_PoolSegGetBlkSize()
*
* Description : Computes total memory segment size for number of blocks with specific size and alignment.
*
* Argument(s) : pmem_addr   Memory address of the beginning of the memory block.
*
*               nbr_blk     Number of memory blocks to allocate.
*
*               size        Size of the memory block to allocate.
*
*               align       Required block memory alignment (in octets).
*
* Return(s)   : Total size of memory segment used to allocate the number of blocks.
*
* Caller(s)   : Mem_PoolInit().
*
* Note(s)     : The total memory segment size value is calculated as follows :
*
*                               -----                  ------------------------
*                                 ^      Mem     --->  |                      |        Align
*                                 |         addr       |                      |  <---       offset
*                                 |                    |                      |        (see Note #2)
*                                 |        -----       ------------------------
*                                 |          ^         |                      |
*                                 |          |         |                      |
*                                 |          |         |         Size         |
*                                 |                    |                      |
*                                 |      Mem Blk  #1   |                      |
*                                 |     (see Note #3)  |                      |
*                                 |                    ------------------------
*                                 |          |         |                      |
*                                 |          |         |        Align         |
*                                 |          v         |                      |
*                                 |        -----       ------------------------
*                                 |                    |      Mem Blk #2      |
*                                                      |      Mem Blk #3      |
*                              Size                    |          .           |
*                                  tot                 |          .           |
*                           (see Note #1)              |          .           |
*                                                      |   Mem Blk #(N - 2)   |
*                                 |        -----       ------------------------
*                                 |          ^         |                      |
*                                 |          |         |                      |
*                                 |          |         |         Size         |
*                                 |          |         |                      |
*                                 |  Mem Blk #(N - 1)  |                      |
*                                 |          |         ------------------------
*                                 |          |         |                      |
*                                 |          |         |        Align         |
*                                 |          v         |                      |
*                                 |        -----       ------------------------
*                                 |          ^         |                      |
*                                 |          |         |                      |
*                                 |      Mem Blk #N    |         Size         |
*                                 |          |         |                      |
*                                 v          v         |                      |
*                               -----      -----       ------------------------
*
*
*                       where
*                                   Align                       Offset required to align first memory block address
*                                        offset
*                                   Size                        Memory block requested size including possible alignment offset
*                                       blk
*                                   N                           Number of block to allocate (nbr_blk)               
*
*                                   Size                        Size of the memory block to allocate (size)
*
*                                   Align                       Required block memory alignment (align)
*
*                                   Mem                         Memory address of the beginning of the memory block
*                                      addr
*
*                   (1)    Size       = [ Align       + (Size    * (N - 1)) + Size ]
*                              tot      [      offset        blk                   ]
*
*                   (2) If memory address is not aligned, compute alignment offset:
*
*                          Align      = [ Align - (Mem     % Align) ]
*                               offset  [             addr          ]
*
*                   (3) a) Size       = [ Size + Align - (Size % Align) ]
*                               blk     [                               ]
*
*                       b) Size       = [ Size ]                If Size and Align are equal
*                               blk     [      ]
*
*********************************************************************************************************
*/
#if (LIB_MEM_CFG_POOL_EN == DEF_ENABLED)
static  CPU_SIZE_T  Mem_PoolSegGetBlkSize (void        *pmem_addr,
                                           CPU_SIZE_T   blk_nbr,
                                           CPU_SIZE_T   size,
                                           CPU_SIZE_T   align)
{
    CPU_SIZE_T  align_offset;
    CPU_SIZE_T  blk_size;
    CPU_SIZE_T  tot_size;


    if (align == 0) {
        return (0);
    }

    if ((((CPU_ADDR)pmem_addr) % align) != 0) {                 /* See Note #2.                                             */
        align_offset = align - (((CPU_ADDR)pmem_addr) % align);
    } else {
        align_offset = 0;
    }

    if (size != align) {
        blk_size = size + align - (size % align);               /* See Note #3a                                             */
    } else {
        blk_size = size;                                        /* See Note #3b                                             */
    }

    tot_size = align_offset + blk_size * (blk_nbr - 1) + size;  /* See Note #1.                                             */

    return (tot_size);
}
#endif

/*$PAGE*/
/*
*********************************************************************************************************
*                                        Mem_PoolSegAlloc()
*
* Description : Allocates memory from specific segment.
*
* Argument(s) : mem_seg_ix  Index of memory segment.
*               ----------  Argument validated in Mem_PoolInit()
*
*               size        Size of memory to allocate.
*               ----        Argument validated in Mem_PoolInit()
*
*               align       Required starting memory alignment (in octets).
*               -----       Argument validated in Mem_PoolInit()
*
* Return(s)   : Pointer to allocated memory, if NO errors.
*
*               Pointer to NULL, otherwise.
*
* Caller(s)   : Mem_PoolInit().
*
* Note(s)     : (1) Allocated memory from the specific segment is NEVER freed after allocation.
*********************************************************************************************************
*/
#if (LIB_MEM_CFG_POOL_EN == DEF_ENABLED)
static  void  *Mem_PoolSegAlloc  (MEM_POOL    *pmem_pool,
                                  CPU_SIZE_T   size,
                                  CPU_SIZE_T   align)
{
#if (CPU_CFG_CRITICAL_METHOD == CPU_CRITICAL_METHOD_STATUS_LOCAL)
    CPU_SR       cpu_sr;
#endif
    CPU_INT08U  *pmem_addr;
    CPU_SIZE_T   mem_size_rem;
    CPU_SIZE_T   mem_align;
    CPU_SIZE_T   align_offset;
    CPU_SIZE_T   tot_size;
    CPU_SIZE_T   size_rem;


    CPU_CRITICAL_ENTER();
    pmem_addr = (CPU_INT08U *)pmem_pool->SegAddrNextAvailPtr;   /* Retrieve next available address in memory segment        */

    mem_align = (CPU_SIZE_T)(((CPU_INT32U)pmem_addr) % align);  /* Compute necessary alignment                              */
    if (mem_align != 0) {
        align_offset = align - mem_align;
    } else {
        align_offset = 0;
    }

    tot_size = align_offset + size;
    size_rem = pmem_pool->SegSizeRem;

    if (tot_size > size_rem) {                                  /* Do not allocate if size extends beyond memory segment    */
        CPU_CRITICAL_EXIT();
        return ((void *)0);
    }

    pmem_addr += align_offset;                                  /* Adjust alignment of memory address                       */

    pmem_pool->SegAddrNextAvailPtr  = pmem_addr + size;         /* Advance next avail address depending on allocated size   */
    pmem_pool->SegSizeRem          -= tot_size;                 /* Adjust remaining size of memory segment                  */

    CPU_CRITICAL_EXIT();

    return ((void *)pmem_addr);
}
#endif

