/****************************************************************************
*
*    Copyright (C) 2005 - 2013 by Vivante Corp.
*
*    This program is free software; you can redistribute it and/or modify
*    it under the terms of the GNU General Public License as published by
*    the Free Software Foundation; either version 2 of the license, or
*    (at your option) any later version.
*
*    This program is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*    GNU General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with this program; if not write to the Free Software
*    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
*****************************************************************************/


#ifndef __gc_hal_kernel_h_
#define __gc_hal_kernel_h_

#include "gc_hal.h"
#include "gc_hal_kernel_hardware.h"
#include "gc_hal_driver.h"

#if gcdENABLE_VG
#include "gc_hal_kernel_vg.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
***** New MMU Defination *******************************************************/
#define gcdMMU_MTLB_SHIFT           22
#define gcdMMU_STLB_4K_SHIFT        12
#define gcdMMU_STLB_64K_SHIFT       16

#define gcdMMU_MTLB_BITS            (32 - gcdMMU_MTLB_SHIFT)
#define gcdMMU_PAGE_4K_BITS         gcdMMU_STLB_4K_SHIFT
#define gcdMMU_STLB_4K_BITS         (32 - gcdMMU_MTLB_BITS - gcdMMU_PAGE_4K_BITS)
#define gcdMMU_PAGE_64K_BITS        gcdMMU_STLB_64K_SHIFT
#define gcdMMU_STLB_64K_BITS        (32 - gcdMMU_MTLB_BITS - gcdMMU_PAGE_64K_BITS)

#define gcdMMU_MTLB_ENTRY_NUM       (1 << gcdMMU_MTLB_BITS)
#define gcdMMU_MTLB_SIZE            (gcdMMU_MTLB_ENTRY_NUM << 2)
#define gcdMMU_STLB_4K_ENTRY_NUM    (1 << gcdMMU_STLB_4K_BITS)
#define gcdMMU_STLB_4K_SIZE         (gcdMMU_STLB_4K_ENTRY_NUM << 2)
#define gcdMMU_PAGE_4K_SIZE         (1 << gcdMMU_STLB_4K_SHIFT)
#define gcdMMU_STLB_64K_ENTRY_NUM   (1 << gcdMMU_STLB_64K_BITS)
#define gcdMMU_STLB_64K_SIZE        (gcdMMU_STLB_64K_ENTRY_NUM << 2)
#define gcdMMU_PAGE_64K_SIZE        (1 << gcdMMU_STLB_64K_SHIFT)

#define gcdMMU_MTLB_MASK            (~((1U << gcdMMU_MTLB_SHIFT)-1))
#define gcdMMU_STLB_4K_MASK         ((~0U << gcdMMU_STLB_4K_SHIFT) ^ gcdMMU_MTLB_MASK)
#define gcdMMU_PAGE_4K_MASK         (gcdMMU_PAGE_4K_SIZE - 1)
#define gcdMMU_STLB_64K_MASK        ((~((1U << gcdMMU_STLB_64K_SHIFT)-1)) ^ gcdMMU_MTLB_MASK)
#define gcdMMU_PAGE_64K_MASK        (gcdMMU_PAGE_64K_SIZE - 1)

/* Page offset definitions. */
#define gcdMMU_OFFSET_4K_BITS       (32 - gcdMMU_MTLB_BITS - gcdMMU_STLB_4K_BITS)
#define gcdMMU_OFFSET_4K_MASK       ((1U << gcdMMU_OFFSET_4K_BITS) - 1)
#define gcdMMU_OFFSET_16K_BITS      (32 - gcdMMU_MTLB_BITS - gcdMMU_STLB_16K_BITS)
#define gcdMMU_OFFSET_16K_MASK      ((1U << gcdMMU_OFFSET_16K_BITS) - 1)

#define gcdMMU_MTLB_PRESENT         0x00000001
#define gcdMMU_MTLB_EXCEPTION       0x00000002
#define gcdMMU_MTLB_4K_PAGE         0x00000000

#define gcdMMU_STLB_PRESENT         0x00000001
#define gcdMMU_STLB_EXCEPTION       0x00000002
#define gcdMMU_STLB_4K_PAGE         0x00000000

/*******************************************************************************
***** Stuck Dump Level ********************************************************/

#define gcdSTUCK_DUMP_MINIMAL       1
#define gcdSTUCK_DUMP_MIDDLE        2
#define gcdSTUCK_DUMP_MAXIMAL       3

/*******************************************************************************
***** Process Secure Cache ****************************************************/

#define gcdSECURE_CACHE_LRU         1
#define gcdSECURE_CACHE_LINEAR      2
#define gcdSECURE_CACHE_HASH        3
#define gcdSECURE_CACHE_TABLE       4

typedef struct _gcskLOGICAL_CACHE * gcskLOGICAL_CACHE_PTR;
typedef struct _gcskLOGICAL_CACHE   gcskLOGICAL_CACHE;
struct _gcskLOGICAL_CACHE
{
    /* Logical address. */
    gctPOINTER                      logical;

    /* DMAable address. */
    gctUINT32                       dma;

#if gcdSECURE_CACHE_METHOD == gcdSECURE_CACHE_HASH
    /* Pointer to the previous and next hash tables. */
    gcskLOGICAL_CACHE_PTR           nextHash;
    gcskLOGICAL_CACHE_PTR           prevHash;
#endif

#if gcdSECURE_CACHE_METHOD != gcdSECURE_CACHE_TABLE
    /* Pointer to the previous and next slot. */
    gcskLOGICAL_CACHE_PTR           next;
    gcskLOGICAL_CACHE_PTR           prev;
#endif

#if gcdSECURE_CACHE_METHOD == gcdSECURE_CACHE_LINEAR
    /* Time stamp. */
    gctUINT64                       stamp;
#endif
};

typedef struct _gcskSECURE_CACHE * gcskSECURE_CACHE_PTR;
typedef struct _gcskSECURE_CACHE
{
    /* Cache memory. */
    gcskLOGICAL_CACHE               cache[1 + gcdSECURE_CACHE_SLOTS];

    /* Last known index for LINEAR mode. */
    gcskLOGICAL_CACHE_PTR           cacheIndex;

    /* Current free slot for LINEAR mode. */
    gctUINT32                       cacheFree;

    /* Time stamp for LINEAR mode. */
    gctUINT64                       cacheStamp;

#if gcdSECURE_CACHE_METHOD == gcdSECURE_CACHE_HASH
    /* Hash table for HASH mode. */
    gcskLOGICAL_CACHE              hash[256];
#endif
}
gcskSECURE_CACHE;

/*******************************************************************************
***** Process Database Management *********************************************/

typedef enum _gceDATABASE_TYPE
{
    gcvDB_VIDEO_MEMORY = 1,             /* Video memory created. */
    gcvDB_COMMAND_BUFFER,               /* Command Buffer. */
    gcvDB_NON_PAGED,                    /* Non paged memory. */
    gcvDB_CONTIGUOUS,                   /* Contiguous memory. */
    gcvDB_SIGNAL,                       /* Signal. */
    gcvDB_VIDEO_MEMORY_LOCKED,          /* Video memory locked. */
    gcvDB_CONTEXT,                      /* Context */
    gcvDB_IDLE,                         /* GPU idle. */
    gcvDB_MAP_MEMORY,                   /* Map memory */
    gcvDB_MAP_USER_MEMORY,              /* Map user memory */
    gcvDB_SYNC_POINT,                   /* Sync point. */
}
gceDATABASE_TYPE;

#define gcdDATABASE_TYPE_MASK           0x000000FF
#define gcdDB_VIDEO_MEMORY_TYPE_MASK    0x0000FF00
#define gcdDB_VIDEO_MEMORY_TYPE_SHIFT   8

#define gcdDB_VIDEO_MEMORY_POOL_MASK    0x00FF0000
#define gcdDB_VIDEO_MEMORY_POOL_SHIFT   16

typedef struct _gcsDATABASE_RECORD *    gcsDATABASE_RECORD_PTR;
typedef struct _gcsDATABASE_RECORD
{
    /* Pointer to kernel. */
    gckKERNEL                           kernel;

    /* Pointer to next database record. */
    gcsDATABASE_RECORD_PTR              next;

    /* Type of record. */
    gceDATABASE_TYPE                    type;

    /* Data for record. */
    gctPOINTER                          data;
    gctPHYS_ADDR                        physical;
    gctSIZE_T                           bytes;
}
gcsDATABASE_RECORD;

typedef struct _gcsDATABASE *           gcsDATABASE_PTR;
typedef struct _gcsDATABASE
{
    /* Pointer to next entry is hash list. */
    gcsDATABASE_PTR                     next;
    gctSIZE_T                           slot;

    /* Process ID. */
    gctUINT32                           processID;

    /* Sizes to query. */
    gcsDATABASE_COUNTERS                vidMem;
    gcsDATABASE_COUNTERS                nonPaged;
    gcsDATABASE_COUNTERS                contiguous;
    gcsDATABASE_COUNTERS                mapUserMemory;
    gcsDATABASE_COUNTERS                mapMemory;

    gcsDATABASE_COUNTERS                vidMemType[gcvSURF_NUM_TYPES];
    /* Counter for each video memory pool. */
    gcsDATABASE_COUNTERS                vidMemPool[gcvPOOL_NUMBER_OF_POOLS];
    gctPOINTER                          counterMutex;

    /* Idle time management. */
    gctUINT64                           lastIdle;
    gctUINT64                           idle;

    /* Pointer to database. */
    gcsDATABASE_RECORD_PTR              list[48];

#if gcdSECURE_USER
    /* Secure cache. */
    gcskSECURE_CACHE                    cache;
#endif

    gctPOINTER                          handleDatabase;
    gctPOINTER                          handleDatabaseMutex;

#if gcdPROCESS_ADDRESS_SPACE
    gckMMU                              mmu;
#endif
}
gcsDATABASE;

/* Create a process database that will contain all its allocations. */
gceSTATUS
gckKERNEL_CreateProcessDB(
    IN gckKERNEL Kernel,
    IN gctUINT32 ProcessID
    );

/* Add a record to the process database. */
gceSTATUS
gckKERNEL_AddProcessDB(
    IN gckKERNEL Kernel,
    IN gctUINT32 ProcessID,
    IN gceDATABASE_TYPE Type,
    IN gctPOINTER Pointer,
    IN gctPHYS_ADDR Physical,
    IN gctSIZE_T Size
    );

/* Remove a record to the process database. */
gceSTATUS
gckKERNEL_RemoveProcessDB(
    IN gckKERNEL Kernel,
    IN gctUINT32 ProcessID,
    IN gceDATABASE_TYPE Type,
    IN gctPOINTER Pointer
    );

/* Destroy the process database. */
gceSTATUS
gckKERNEL_DestroyProcessDB(
    IN gckKERNEL Kernel,
    IN gctUINT32 ProcessID
    );

/* Find a record to the process database. */
gceSTATUS
gckKERNEL_FindProcessDB(
    IN gckKERNEL Kernel,
    IN gctUINT32 ProcessID,
    IN gctUINT32 ThreadID,
    IN gceDATABASE_TYPE Type,
    IN gctPOINTER Pointer,
    OUT gcsDATABASE_RECORD_PTR Record
    );

/* Query the process database. */
gceSTATUS
gckKERNEL_QueryProcessDB(
    IN gckKERNEL Kernel,
    IN gctUINT32 ProcessID,
    IN gctBOOL LastProcessID,
    IN gceDATABASE_TYPE Type,
    OUT gcuDATABASE_INFO * Info
    );

/* Dump the process database. */
gceSTATUS
gckKERNEL_DumpProcessDB(
    IN gckKERNEL Kernel
    );

/* Dump the video memory usage for process specified. */
gceSTATUS
gckKERNEL_DumpVidMemUsage(
    IN gckKERNEL Kernel,
    IN gctINT32 ProcessID
    );

gceSTATUS
gckKERNEL_FindDatabase(
    IN gckKERNEL Kernel,
    IN gctUINT32 ProcessID,
    IN gctBOOL LastProcessID,
    OUT gcsDATABASE_PTR * Database
    );

gceSTATUS
gckKERNEL_FindHandleDatbase(
    IN gckKERNEL Kernel,
    IN gctUINT32 ProcessID,
    OUT gctPOINTER * HandleDatabase,
    OUT gctPOINTER * HandleDatabaseMutex
    );

gceSTATUS
gckKERNEL_GetProcessMMU(
    IN gckKERNEL Kernel,
    OUT gckMMU * Mmu
    );

gceSTATUS
gckKERNEL_SetRecovery(
    IN gckKERNEL Kernel,
    IN gctBOOL   Recovery,
    IN gctUINT32 StuckDump
    );

gceSTATUS
gckMMU_FlatMapping(
    IN gckMMU Mmu,
    IN gctUINT32 Physical
    );

gceSTATUS
gckMMU_GetPageEntry(
    IN gckMMU Mmu,
    IN gctUINT32 Address,
    IN gctUINT32_PTR *PageTable
    );

gceSTATUS
gckMMU_FreePagesEx(
    IN gckMMU Mmu,
    IN gctUINT32 Address,
    IN gctSIZE_T PageCount
    );

gceSTATUS
gckKERNEL_CreateIntegerDatabase(
    IN gckKERNEL Kernel,
    OUT gctPOINTER * Database
    );

gceSTATUS
gckKERNEL_DestroyIntegerDatabase(
    IN gckKERNEL Kernel,
    IN gctPOINTER Database
    );

gceSTATUS
gckKERNEL_AllocateIntegerId(
    IN gctPOINTER Database,
    IN gctPOINTER Pointer,
    OUT gctUINT32 * Id
    );

gceSTATUS
gckKERNEL_FreeIntegerId(
    IN gctPOINTER Database,
    IN gctUINT32 Id
    );

gceSTATUS
gckKERNEL_QueryIntegerId(
    IN gctPOINTER Database,
    IN gctUINT32 Id,
    OUT gctPOINTER * Pointer
    );

/* Pointer rename  */
gctUINT32
gckKERNEL_AllocateNameFromPointer(
    IN gckKERNEL Kernel,
    IN gctPOINTER Pointer
    );

gctPOINTER
gckKERNEL_QueryPointerFromName(
    IN gckKERNEL Kernel,
    IN gctUINT32 Name
    );

gceSTATUS
gckKERNEL_DeleteName(
    IN gckKERNEL Kernel,
    IN gctUINT32 Name
    );

#if gcdSECURE_USER
/* Get secure cache from the process database. */
gceSTATUS
gckKERNEL_GetProcessDBCache(
    IN gckKERNEL Kernel,
    IN gctUINT32 ProcessID,
    OUT gcskSECURE_CACHE_PTR * Cache
    );
#endif

/*******************************************************************************
********* Timer Management ****************************************************/
typedef struct _gcsTIMER *           gcsTIMER_PTR;
typedef struct _gcsTIMER
{
    /* Start and Stop time holders. */
    gctUINT64                           startTime;
    gctUINT64                           stopTime;
}
gcsTIMER;

/******************************************************************************\
********************************** Structures **********************************
\******************************************************************************/

/* gckDB object. */
struct _gckDB
{
    /* Database management. */
    gcsDATABASE_PTR             db[16];
    gctPOINTER                  dbMutex;
    gcsDATABASE_PTR             freeDatabase;
    gcsDATABASE_RECORD_PTR      freeRecord;
    gcsDATABASE_PTR             lastDatabase;
    gctUINT32                   lastProcessID;
    gctUINT64                   lastIdle;
    gctUINT64                   idleTime;
    gctUINT64                   lastSlowdown;
    gctUINT64                   lastSlowdownIdle;
    gctPOINTER                  nameDatabase;
    gctPOINTER                  nameDatabaseMutex;

    gctPOINTER                  pointerDatabase;
    gctPOINTER                  pointerDatabaseMutex;
};

typedef struct _gckVIRTUAL_COMMAND_BUFFER * gckVIRTUAL_COMMAND_BUFFER_PTR;
typedef struct _gckVIRTUAL_COMMAND_BUFFER
{
    gctPHYS_ADDR                physical;
    gctPOINTER                  userLogical;
    gctPOINTER                  kernelLogical;
    gctSIZE_T                   bytes;
    gctSIZE_T                   pageCount;
    gctPOINTER                  pageTable;
    gctUINT32                   gpuAddress;
    gctUINT                     pid;
    gckVIRTUAL_COMMAND_BUFFER_PTR   next;
    gckVIRTUAL_COMMAND_BUFFER_PTR   prev;
    gckKERNEL                   kernel;
#if gcdPROCESS_ADDRESS_SPACE
    gckMMU                      mmu;
#endif
}
gckVIRTUAL_COMMAND_BUFFER;

/* gckKERNEL object. */
struct _gckKERNEL
{
    /* Object. */
    gcsOBJECT                   object;

    /* Pointer to gckOS object. */
    gckOS                       os;

    /* Core */
    gceCORE                     core;

    /* Pointer to gckHARDWARE object. */
    gckHARDWARE                 hardware;

    /* Pointer to gckCOMMAND object. */
    gckCOMMAND                  command;

    /* Pointer to gckEVENT object. */
    gckEVENT                    eventObj;

    /* Pointer to context. */
    gctPOINTER                  context;

    /* Pointer to gckMMU object. */
    gckMMU                      mmu;

    /* Arom holding number of clients. */
    gctPOINTER                  atomClients;

#if VIVANTE_PROFILER
    /* Enable profiling */
    gctBOOL                     profileEnable;
    /* Clear profile register or not*/
    gctBOOL                     profileCleanRegister;
#endif

#ifdef QNX_SINGLE_THREADED_DEBUGGING
    gctPOINTER                  debugMutex;
#endif

    /* Database management. */
    gckDB                       db;
    gctBOOL                     dbCreated;

#if gcdENABLE_RECOVERY
    gctPOINTER                  resetFlagClearTimer;
    gctPOINTER                  resetAtom;
    gctUINT64                   resetTimeStamp;
#endif

    /* Pointer to gckEVENT object. */
    gcsTIMER                    timers[8];
    gctUINT32                   timeOut;

#if gcdENABLE_VG
    gckVGKERNEL                 vg;
#endif

    /* Virtual command buffer list. */
    gckVIRTUAL_COMMAND_BUFFER_PTR virtualBufferHead;
    gckVIRTUAL_COMMAND_BUFFER_PTR virtualBufferTail;
    gctPOINTER                    virtualBufferLock;

    /* Enable virtual command buffer. */
    gctBOOL                     virtualCommandBuffer;

#if gcdDVFS
    gckDVFS                     dvfs;
#endif

#if gcdANDROID_NATIVE_FENCE_SYNC
    gctHANDLE                   timeline;
#endif

    /* Enable recovery. */
    gctBOOL                     recovery;

    /* Level of dump information after stuck. */
    gctUINT                     stuckDump;
};

struct _FrequencyHistory
{
    gctUINT32                   frequency;
    gctUINT32                   count;
};

/* gckDVFS object. */
struct _gckDVFS
{
    gckOS                       os;
    gckHARDWARE                 hardware;
    gctPOINTER                  timer;
    gctUINT32                   pollingTime;
    gctBOOL                     stop;
    gctUINT32                   totalConfig;
    gctUINT32                   loads[8];
    gctUINT8                    currentScale;
    struct _FrequencyHistory    frequencyHistory[16];
};

/* gckCOMMAND object. */
struct _gckCOMMAND
{
    /* Object. */
    gcsOBJECT                   object;

    /* Pointer to required object. */
    gckKERNEL                   kernel;
    gckOS                       os;

    /* Number of bytes per page. */
    gctSIZE_T                   pageSize;

    /* Current pipe select. */
    gcePIPE_SELECT              pipeSelect;

    /* Command queue running flag. */
    gctBOOL                     running;

    /* Idle flag and commit stamp. */
    gctBOOL                     idle;
    gctUINT64                   commitStamp;

    /* Command queue mutex. */
    gctPOINTER                  mutexQueue;

    /* Context switching mutex. */
    gctPOINTER                  mutexContext;

#if VIVANTE_PROFILER_CONTEXT
    /* Context sequence mutex. */
    gctPOINTER                  mutexContextSeq;
#endif

    /* Command queue power semaphore. */
    gctPOINTER                  powerSemaphore;

    /* Current command queue. */
    struct _gcskCOMMAND_QUEUE
    {
        gctSIGNAL               signal;
        gctPHYS_ADDR            physical;
        gctPOINTER              logical;
    }
    queues[gcdCOMMAND_QUEUES];

    gctPHYS_ADDR                physical;
    gctPOINTER                  logical;
    gctUINT32                   offset;
    gctINT                      index;
#if gcmIS_DEBUG(gcdDEBUG_TRACE)
    gctUINT                     wrapCount;
#endif

    /* The command queue is new. */
    gctBOOL                     newQueue;

    /* Context management. */
    gckCONTEXT                  currContext;

    /* Pointer to last WAIT command. */
    gctPHYS_ADDR                waitPhysical;
    gctPOINTER                  waitLogical;
    gctSIZE_T                   waitSize;

    /* Command buffer alignment. */
    gctSIZE_T                   alignment;
    gctSIZE_T                   reservedHead;
    gctSIZE_T                   reservedTail;

    /* Commit counter. */
    gctPOINTER                  atomCommit;

    /* Kernel process ID. */
    gctUINT32                   kernelProcessID;

    /* End Event signal. */
    gctSIGNAL                   endEventSignal;

#if gcdSECURE_USER
    /* Hint array copy buffer. */
    gctBOOL                     hintArrayAllocated;
    gctUINT                     hintArraySize;
    gctUINT32_PTR               hintArray;
#endif

#if gcdPROCESS_ADDRESS_SPACE
    gckMMU                      currentMmu;
    struct _gckENTRYQUEUE       queue;
#endif
};

typedef struct _gcsEVENT *      gcsEVENT_PTR;

/* Structure holding one event to be processed. */
typedef struct _gcsEVENT
{
    /* Pointer to next event in queue. */
    gcsEVENT_PTR                next;

    /* Event information. */
    gcsHAL_INTERFACE            info;

    /* Process ID owning the event. */
    gctUINT32                   processID;

#ifdef __QNXNTO__
    /* Kernel. */
    gckKERNEL                   kernel;
#endif

    gctBOOL                     fromKernel;
}
gcsEVENT;

/* Structure holding a list of events to be processed by an interrupt. */
typedef struct _gcsEVENT_QUEUE * gcsEVENT_QUEUE_PTR;
typedef struct _gcsEVENT_QUEUE
{
    /* Time stamp. */
    gctUINT64                   stamp;

    /* Source of the event. */
    gceKERNEL_WHERE             source;

    /* Pointer to head of event queue. */
    gcsEVENT_PTR                head;

    /* Pointer to tail of event queue. */
    gcsEVENT_PTR                tail;

    /* Next list of events. */
    gcsEVENT_QUEUE_PTR          next;
}
gcsEVENT_QUEUE;

/*
    gcdREPO_LIST_COUNT defines the maximum number of event queues with different
    hardware module sources that may coexist at the same time. Only two sources
    are supported - gcvKERNEL_COMMAND and gcvKERNEL_PIXEL. gcvKERNEL_COMMAND
    source is used only for managing the kernel command queue and is only issued
    when the current command queue gets full. Since we commit event queues every
    time we commit command buffers, in the worst case we can have up to three
    pending event queues:
        - gcvKERNEL_PIXEL
        - gcvKERNEL_COMMAND (queue overflow)
        - gcvKERNEL_PIXEL
*/
#define gcdREPO_LIST_COUNT      3

/* gckEVENT object. */
struct _gckEVENT
{
    /* The object. */
    gcsOBJECT                   object;

    /* Pointer to required objects. */
    gckOS                       os;
    gckKERNEL                   kernel;

    /* Time stamp. */
    gctUINT64                   stamp;
    gctUINT64                   lastCommitStamp;

    /* Queue mutex. */
    gctPOINTER                  eventQueueMutex;

    /* Array of event queues. */
#if gcdPROCESS_ADDRESS_SPACE
    gcsEVENT_QUEUE              queues[29];
#else
    gcsEVENT_QUEUE              queues[30];
#endif
    gctUINT8                    lastID;
    gctPOINTER                  freeAtom;

    /* Pending events. */
#if gcdSMP
#if gcdMULTI_GPU
    gctPOINTER                  pending3D[gcdMULTI_GPU];
#endif
    gctPOINTER                  pending;
#else
#if gcdMULTI_GPU
    volatile gctUINT            pending3D[gcdMULTI_GPU];
#endif
    volatile gctUINT            pending;
#endif
#if gcdMULTI_GPU
    gctUINT32                   busy;
#endif

    /* List of free event structures and its mutex. */
    gcsEVENT_PTR                freeEventList;
    gctSIZE_T                   freeEventCount;
    gctPOINTER                  freeEventMutex;

    /* Event queues. */
    gcsEVENT_QUEUE_PTR          queueHead;
    gcsEVENT_QUEUE_PTR          queueTail;
    gcsEVENT_QUEUE_PTR          freeList;
    gcsEVENT_QUEUE              repoList[gcdREPO_LIST_COUNT];
    gctPOINTER                  eventListMutex;

    gctPOINTER                  submitTimer;
};

/* Free all events belonging to a process. */
gceSTATUS
gckEVENT_FreeProcess(
    IN gckEVENT Event,
    IN gctUINT32 ProcessID
    );

gceSTATUS
gckEVENT_Stop(
    IN gckEVENT Event,
    IN gctUINT32 ProcessID,
    IN gctPHYS_ADDR Handle,
    IN gctPOINTER Logical,
    IN gctSIGNAL Signal,
    IN OUT gctSIZE_T * waitSize
    );

typedef struct _gcsLOCK_INFO * gcsLOCK_INFO_PTR;
typedef struct _gcsLOCK_INFO
{
    gctUINT32                   GPUAddresses[gcdMAX_GPU_COUNT];
    gctPOINTER                  pageTables[gcdMAX_GPU_COUNT];
    gctUINT32                   lockeds[gcdMAX_GPU_COUNT];
    gckKERNEL                   lockKernels[gcdMAX_GPU_COUNT];
    gckMMU                      lockMmus[gcdMAX_GPU_COUNT];
}
gcsLOCK_INFO;

typedef struct _gcsGPU_MAP * gcsGPU_MAP_PTR;
typedef struct _gcsGPU_MAP
{
    gctINT                      pid;
    gcsLOCK_INFO                lockInfo;
    gcsGPU_MAP_PTR              prev;
    gcsGPU_MAP_PTR              next;
}
gcsGPU_MAP;

/* gcuVIDMEM_NODE structure. */
typedef union _gcuVIDMEM_NODE
{
    /* Allocated from gckVIDMEM. */
    struct _gcsVIDMEM_NODE_VIDMEM
    {
        /* Owner of this node. */
        gckVIDMEM               memory;

        /* Dual-linked list of nodes. */
        gcuVIDMEM_NODE_PTR      next;
        gcuVIDMEM_NODE_PTR      prev;

        /* Dual linked list of free nodes. */
        gcuVIDMEM_NODE_PTR      nextFree;
        gcuVIDMEM_NODE_PTR      prevFree;

        /* Information for this node. */
        gctUINT32               offset;
        gctSIZE_T               bytes;
        gctUINT32               alignment;

#ifdef __QNXNTO__
        /* Client/server vaddr (mapped using mmap_join). */
        gctPOINTER              logical;
#endif

        /* Locked counter. */
        gctINT32                locked;

        /* Memory pool. */
        gcePOOL                 pool;
        gctUINT32               physical;

        /* Process ID owning this memory. */
        gctUINT32               processID;

#if gcdDYNAMIC_MAP_RESERVED_MEMORY && gcdENABLE_VG
        gctPOINTER              kernelVirtual;
#endif
    }
    VidMem;

    /* Allocated from gckOS. */
    struct _gcsVIDMEM_NODE_VIRTUAL
    {
        /* Pointer to gckKERNEL object. */
        gckKERNEL               kernel;

        /* Information for this node. */
        /* Contiguously allocated? */
        gctBOOL                 contiguous;
        /* mdl record pointer... a kmalloc address. Process agnostic. */
        gctPHYS_ADDR            physical;
        gctSIZE_T               bytes;
        /* do_mmap_pgoff address... mapped per-process. */
        gctPOINTER              logical;

        /* Page table information. */
        /* Used only when node is not contiguous */
        gctSIZE_T               pageCount;

        /* Used only when node is not contiguous */
        gctPOINTER              pageTables[gcdMAX_GPU_COUNT];
        /* Pointer to gckKERNEL object who lock this. */
        gckKERNEL               lockKernels[gcdMAX_GPU_COUNT];
        /* Actual physical address */
        gctUINT32               addresses[gcdMAX_GPU_COUNT];

        /* Mutex. */
        gctPOINTER              mutex;

        /* Locked counter. */
        gctINT32                lockeds[gcdMAX_GPU_COUNT];

        /* Process ID owning this memory. */
        gctUINT32               processID;
    }
    Virtual;
}
gcuVIDMEM_NODE;

/* gckVIDMEM object. */
struct _gckVIDMEM
{
    /* Object. */
    gcsOBJECT                   object;

    /* Pointer to gckOS object. */
    gckOS                       os;

    /* Information for this video memory heap. */
    gctUINT32                   baseAddress;
    gctSIZE_T                   bytes;
    gctSIZE_T                   freeBytes;

    /* Mapping for each type of surface. */
    gctINT                      mapping[gcvSURF_NUM_TYPES];

    /* Sentinel nodes for up to 8 banks. */
    gcuVIDMEM_NODE              sentinel[8];

    /* Allocation threshold. */
    gctSIZE_T                   threshold;

    /* The heap mutex. */
    gctPOINTER                  mutex;
};

typedef struct _gcsVIDMEM_NODE * gckVIDMEM_NODE;
typedef struct _gcsVIDMEM_NODE
{
    /* Pointer to gcuVIDMEM_NODE. */
    gcuVIDMEM_NODE_PTR          node;

    /* Reference count. */
    gctPOINTER                  reference;

    /* Name for client to import. */
    gctUINT32                   name;

#if gcdPROCESS_ADDRESS_SPACE
    /* Head of mapping list. */
    gcsGPU_MAP_PTR              mapHead;

    /* Tail of mapping list. */
    gcsGPU_MAP_PTR              mapTail;

    gctPOINTER                  mapMutex;
#endif

    /* Surface Type. */
    gceSURF_TYPE                type;

    /* Pool from which node is allocated. */
    gcePOOL                     pool;
}
gcsVIDMEM_NODE;

typedef struct _gcsVIDMEM_HANDLE * gckVIDMEM_HANDLE;
typedef struct _gcsVIDMEM_HANDLE
{
    /* Pointer to gckVIDMEM_NODE. */
    gckVIDMEM_NODE              node;

    /* Handle for current process. */
    gctUINT32                   handle;

    /* Reference count for this handle. */
    gctPOINTER                  reference;
}
gcsVIDMEM_HANDLE;

gceSTATUS
gckVIDMEM_HANDLE_Reference(
    IN gckKERNEL Kernel,
    IN gctUINT32 ProcessID,
    IN gctUINT32 Handle
    );

gceSTATUS
gckVIDMEM_HANDLE_Dereference(
    IN gckKERNEL Kernel,
    IN gctUINT32 ProcessID,
    IN gctUINT32 Handle
    );

gceSTATUS
gckVIDMEM_NODE_Allocate(
    IN gckKERNEL Kernel,
    IN gcuVIDMEM_NODE_PTR VideoNode,
    IN gceSURF_TYPE Type,
    IN gcePOOL Pool,
    IN gctUINT32 * Handle
    );

gceSTATUS
gckVIDMEM_Node_Lock(
    IN gckKERNEL Kernel,
    IN gckVIDMEM_NODE Node,
    OUT gctUINT32 *Address
    );

gceSTATUS
gckVIDMEM_NODE_Unlock(
    IN gckKERNEL Kernel,
    IN gckVIDMEM_NODE Node,
    IN gctUINT32 ProcessID
    );

gceSTATUS
gckVIDMEM_NODE_Dereference(
    IN gckKERNEL Kernel,
    IN gckVIDMEM_NODE Node
    );

gceSTATUS
gckVIDMEM_NODE_Name(
    IN gckKERNEL Kernel,
    IN gctUINT32 Handle,
    IN gctUINT32 * Name
    );

gceSTATUS
gckVIDMEM_NODE_Import(
    IN gckKERNEL Kernel,
    IN gctUINT32 Name,
    IN gctUINT32 * Handle
    );

gceSTATUS
gckVIDMEM_HANDLE_LookupAndReference(
    IN gckKERNEL Kernel,
    IN gctUINT32 Handle,
    OUT gckVIDMEM_NODE * Node
    );

gceSTATUS
gckVIDMEM_HANDLE_Lookup(
    IN gckKERNEL Kernel,
    IN gctUINT32 ProcessID,
    IN gctUINT32 Handle,
    OUT gckVIDMEM_NODE * Node
    );

#if gcdPROCESS_ADDRESS_SPACE
gceSTATUS
gckEVENT_DestroyMmu(
    IN gckEVENT Event,
    IN gckMMU Mmu,
    IN gceKERNEL_WHERE FromWhere
    );
#endif

/* gckMMU object. */
struct _gckMMU
{
    /* The object. */
    gcsOBJECT                   object;

    /* Pointer to gckOS object. */
    gckOS                       os;

    /* Pointer to gckHARDWARE object. */
    gckHARDWARE                 hardware;

    /* The page table mutex. */
    gctPOINTER                  pageTableMutex;

    /* Page table information. */
    gctSIZE_T                   pageTableSize;
    gctPHYS_ADDR                pageTablePhysical;
    gctUINT32_PTR               pageTableLogical;
    gctUINT32                   pageTableEntries;

    /* Master TLB information. */
    gctSIZE_T                   mtlbSize;
    gctPHYS_ADDR                mtlbPhysical;
    gctUINT32_PTR               mtlbLogical;
    gctUINT32                   mtlbEntries;

    /* Free entries. */
    gctUINT32                   heapList;
    gctBOOL                     freeNodes;

    gctPOINTER                  staticSTLB;
    gctBOOL                     enabled;

    gctUINT32                   dynamicMappingStart;

    gctUINT32_PTR               mapLogical;
#if gcdPROCESS_ADDRESS_SPACE
    gctPOINTER                  pageTableDirty[gcdMAX_GPU_COUNT];
    gctPOINTER                  stlbs;
#endif
};

gceSTATUS
gckOS_CreateKernelVirtualMapping(
    IN gctPHYS_ADDR Physical,
    IN gctSIZE_T Bytes,
    OUT gctSIZE_T * PageCount,
    OUT gctPOINTER * Logical
    );

gceSTATUS
gckOS_DestroyKernelVirtualMapping(
    IN gctPOINTER Logical,
    IN gctSIZE_T Bytes
    );

gceSTATUS
gckKERNEL_AllocateVirtualCommandBuffer(
    IN gckKERNEL Kernel,
    IN gctBOOL InUserSpace,
    IN OUT gctSIZE_T * Bytes,
    OUT gctPHYS_ADDR * Physical,
    OUT gctPOINTER * Logical
    );

gceSTATUS
gckKERNEL_DestroyVirtualCommandBuffer(
    IN gckKERNEL Kernel,
    IN gctSIZE_T Bytes,
    IN gctPHYS_ADDR Physical,
    IN gctPOINTER Logical
    );

gceSTATUS
gckKERNEL_GetGPUAddress(
    IN gckKERNEL Kernel,
    IN gctPOINTER Logical,
    OUT gctUINT32 * Address
    );

gceSTATUS
gckKERNEL_QueryGPUAddress(
    IN gckKERNEL Kernel,
    IN gctUINT32 GpuAddress,
    OUT gckVIRTUAL_COMMAND_BUFFER_PTR * Buffer
    );

gceSTATUS
gckKERNEL_AttachProcess(
    IN gckKERNEL Kernel,
    IN gctBOOL Attach
    );

gceSTATUS
gckKERNEL_AttachProcessEx(
    IN gckKERNEL Kernel,
    IN gctBOOL Attach,
    IN gctUINT32 PID
    );

#if gcdSECURE_USER
gceSTATUS
gckKERNEL_MapLogicalToPhysical(
    IN gckKERNEL Kernel,
    IN gcskSECURE_CACHE_PTR Cache,
    IN OUT gctPOINTER * Data
    );

gceSTATUS
gckKERNEL_FlushTranslationCache(
    IN gckKERNEL Kernel,
    IN gcskSECURE_CACHE_PTR Cache,
    IN gctPOINTER Logical,
    IN gctSIZE_T Bytes
    );
#endif

gceSTATUS
gckHARDWARE_QueryIdle(
    IN gckHARDWARE Hardware,
    OUT gctBOOL_PTR IsIdle
    );

/******************************************************************************\
******************************* gckCONTEXT Object *******************************
\******************************************************************************/

gceSTATUS
gckCONTEXT_Construct(
    IN gckOS Os,
    IN gckHARDWARE Hardware,
    IN gctUINT32 ProcessID,
    OUT gckCONTEXT * Context
    );

gceSTATUS
gckCONTEXT_Destroy(
    IN gckCONTEXT Context
    );

gceSTATUS
gckCONTEXT_Update(
    IN gckCONTEXT Context,
    IN gctUINT32 ProcessID,
    IN gcsSTATE_DELTA_PTR StateDelta
    );

#if gcdLINK_QUEUE_SIZE
void
gckLINKQUEUE_Enqueue(
    IN gckLINKQUEUE LinkQueue,
    IN gctUINT32 start,
    IN gctUINT32 end
    );

void
gckLINKQUEUE_GetData(
    IN gckLINKQUEUE LinkQueue,
    IN gctUINT32 Index,
    OUT gckLINKDATA * Data
    );
#endif

#if gcdPROCESS_ADDRESS_SPACE
void
gckENTRYQUEUE_Enqueue(
    IN gckENTRYQUEUE Queue,
    IN gctUINT32 physical,
    IN gctUINT32 bytes
    );

void
gckENTRYQUEUE_GetData(
    IN gckENTRYQUEUE Queue,
    IN gctUINT32 Index,
    OUT gckENTRYDATA * Data
    );

void
gckENTRYQUEUE_Dequeue(
    IN gckENTRYQUEUE Queue
    );
#endif

#ifdef __cplusplus
}
#endif

#endif /* __gc_hal_kernel_h_ */
