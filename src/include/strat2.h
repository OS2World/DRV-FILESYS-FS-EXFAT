/*
 * strat2.h - extended disk driver interface (strategy 2: request lists, driver and volume
 *            characteristics)
 *
 * Copyright (c) 2026 OS2World.
 * Released under the GNU Lesser General Public License, version 2.1 or later,
 * like the rest of FS-EXFAT.
 *
 * Clean-room replacement: written from the "Storage Device Driver Reference"
 * (Extended Device Driver Interface: Driver Capabilities Structure, Volume
 * Characteristics Structure, Request Lists, read/write requests, scatter/gather
 * descriptors), see doc/PROVENANCE.md. Field names follow their use in ifs/ifsswap.c
 * and ifs/ifsmount.c; layouts and values follow the reference.
 */

#ifndef FSEXFAT_STRAT2_H_INCLUDED
#define FSEXFAT_STRAT2_H_INCLUDED

#pragma pack(push, 1)

/* ---- Driver Capabilities Structure (DCS) ---- */

typedef struct _DriverCaps
{
   unsigned short Reserved;        /* must be zero                          */
   unsigned char  VerMajor;        /* interface version, 1 in the first release */
   unsigned char  VerMinor;
   unsigned long  Capabilities;    /* GDC_DD_* bits                         */
   unsigned long  Strategy2;       /* 16:16 entry point, multi-request I/O  */
   unsigned long  SetFSDInfo;      /* 16:16 entry points, 0:0 if not provided */
   unsigned long  ChgPriority;
   unsigned long  SetRestPos;
   unsigned long  GetBoundary;
} DriverCaps, far *P_DriverCaps;

#define GDC_DD_Mirror    0x00000008UL   /* bit 3: disk mirroring supported   */
#define GDC_DD_Duplex    0x00000010UL   /* bit 4: disk duplexing supported   */
#define GDC_DD_No_Block  0x00000020UL   /* bit 5: Strategy2 does not block   */

/* ---- Volume Characteristics Structure (VCS) ---- */

typedef struct _VolChars
{
   unsigned short VolDescriptor;   /* VC_* bits                             */
   unsigned short AvgSeekTime;     /* ms, 0xFFFF unknown                    */
   unsigned short AvgLatency;      /* ms, 0xFFFF unknown                    */
   unsigned short TrackMinBlocks;
   unsigned short TrackMaxBlocks;
   unsigned short HeadsPerCylinder;
   unsigned long  VolCylinderCount;
   unsigned long  VolMedianBlock;
   unsigned short MaxSGList;       /* 0: adapter has no scatter/gather      */
} VolChars, far *P_VolChars;

#define VC_REMOVABLE_MEDIA   0x0001   /* bit 0: removable media             */
#define VC_READ_ONLY_DEVICE  0x0002   /* bit 1: read-only volume            */
#define VC_RAM_DISK          0x0004   /* bit 2: seek time independent of position */
#define VC_OUTBOARD_CACHE    0x0008   /* bit 3: outboard cache supported    */
#define VC_SCATTER_GATHER    0x0010   /* bit 4: adapter scatter/gather      */
#define VC_READ_PREFETCH     0x0020   /* bit 5: read prefetch supported     */

/* ---- Request list header (20 bytes) ---- */

typedef struct _Req_List_Header
{
   unsigned short Count;             /* number of requests in the list      */
   unsigned short Reserved;          /* must be 0                           */
   void (far *Notify_Address)(void); /* 16:16, called when the list is done */
   unsigned short Request_Control;   /* RLH_* bits                          */
   unsigned char  Block_Dev_Unit;    /* logical unit all requests go to     */
   unsigned char  Lst_Status;        /* overall status                      */
   unsigned long  DD_Reserved1;      /* for the device driver               */
   unsigned long  DD_Reserved2;
} Req_List_Header;

#define RLH_Single_Req     0x0002     /* only one request in the list        */
#define RLH_Exe_Req_Seq    0x0004     /* execute the requests in sequence    */
#define RLH_Term_On_Error  0x0008     /* do not execute the rest after an error */
#define RLH_Notify_Err     0x0010     /* notify immediately on error only    */
#define RLH_Notify_Done    0x0020     /* notify on completion                */

/* ---- Request header (32 bytes) ---- */

typedef struct _Req_Header
{
   unsigned short Length;            /* offset of the next request, RH_LAST_REQ for the last */
   unsigned char  Old_Command;       /* command prefix: PB_REQ_LIST         */
   unsigned char  Command_Code;      /* PB_READ_X, PB_WRITE_X, ...          */
   unsigned long  Head_Offset;       /* offset from the list header         */
   unsigned char  Req_Control;       /* bits 4/5: notify on error / completion */
   unsigned char  Priority;          /* PRIO_* value                        */
   unsigned char  Status;            /* low nibble: progress, high: error   */
   unsigned char  Error_Code;        /* valid when the status shows an error */
   void (far *Notify_Address)(void); /* 16:16, per-request notification     */
   unsigned long  Hint_Pointer;      /* 16:16 pointer into the list, -1 none */
   unsigned long  DD_Reserved1;
   unsigned long  DD_Reserved2;
   unsigned long  DD_Reserved3;
} Req_Header;

#define RH_LAST_REQ     0xFFFF       /* Length value of the last request    */

#define PB_REQ_LIST     0x1C         /* command prefix of an extended request */
#define PB_READ_X       0x1E         /* extended READ                       */
#define PB_WRITE_X      0x1F         /* extended WRITE                      */
#define PB_WRITEV_X     0x20         /* extended WRITE VERIFY               */
#define PB_PREFETCH_X   0x21         /* extended READ PREFETCH              */

#define PRIO_PREFETCH         0x00
#define PRIO_LAZY_WRITE       0x01
#define PRIO_READ_AHEAD       0x02
#define PRIO_BACKGROUND_SYNC  0x04
#define PRIO_FOREGROUND_SYNC  0x08
#define PRIO_PAGER            0x10
#define PRIO_URGENT           0x80

/* ---- read / write / write verify request (52 bytes + descriptors) ---- */

typedef struct _PB_Read_Write
{
   Req_Header     RqHdr;
   unsigned long  Start_Block;       /* first 512-byte block                */
   unsigned long  Block_Count;       /* number of 512-byte blocks           */
   unsigned long  Blocks_Xferred;    /* set by the driver                   */
   unsigned short RW_Flags;          /* bit 0 write through, bit 1 outboard cache */
   unsigned short SG_Desc_Count;     /* number of descriptors that follow   */
   unsigned long  Reserved;
} PB_Read_Write;

/* ---- scatter/gather descriptor ---- */

typedef struct _SG_Descriptor
{
   unsigned long BufferPtr;          /* 32-bit physical address             */
   unsigned long BufferSize;         /* bytes                               */
} SG_Descriptor;

#pragma pack(pop)

#endif /* FSEXFAT_STRAT2_H_INCLUDED */
