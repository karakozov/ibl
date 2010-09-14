#ifndef _PSCLOC_H
#define _PSCLOC_H
/*************************************************************************************
 * FILE PURPOSE: Local Power Saver Controller definitions
 *************************************************************************************
 * FILE NAME: pscloc.h
 *
 * DESCRIPTION: Provides local definitions for the power saver controller
 *
 *************************************************************************************/
 
 
#define BOOTBITMASK(x,y)      (   (   (  ((uint32)1 << (((uint32)x)-((uint32)y)+(uint32)1) ) - (uint32)1 )   )   <<  ((uint32)y)   )
#define BOOT_READ_BITFIELD(z,x,y)   (((uint32)z) & BOOTBITMASK(x,y)) >> (y)
#define BOOT_SET_BITFIELD(z,f,x,y)  (((uint32)z) & ~BOOTBITMASK(x,y)) | ( (((uint32)f) << (y)) & BOOTBITMASK(x,y) )

/* Register offsets */
#define PSC_REG_PTCMD           0x120
#define PSC_REG_PSTAT	        0x128
#define PSC_REG_PDSTAT(x)       (0x200 + (4*(x)))
#define PSC_REG_PDCTL(x)        (0x300 + (4*(x)))
#define PSC_REG_MDCFG(x)        (0x600 + (4*(x)))
#define PSC_REG_MDSTAT(x)       (0x800 + (4*(x)))
#define PSC_REG_MDCTL(x)        (0xa00 + (4*(x)))


/* Macros to access register fields */
/* PDCTL */
#define PSC_REG_PDCTL_SET_NEXT(x,y)    BOOT_SET_BITFIELD((x),(y),0,0)
#define PSC_REG_PDCTL_SET_PDMODE(x,y)  BOOT_SET_BITFIELD((x),(y),15,12)

/* PDSTAT */
#define PSC_REG_PDSTAT_GET_STATE(x)  BOOT_READ_BITFIELD((x),4,0))

/* MDCFG */
#define PSC_REG_MDCFG_GET_PD(x)         BOOT_READ_BITFIELD((x),20,16)
#define PSC_REG_MDCFG_GET_RESET_ISO(x)  BOOT_READ_BITFIELD((x),14,14)

/* MDCTL */
#define PSC_REG_MDCTL_SET_NEXT(x,y)       BOOT_SET_BITFIELD((x),(y),4,0)
#define PSC_REG_MDCTL_SET_LRSTZ(x,y)      BOOT_SET_BITFIELD((x),(y),8,8)
#define PSC_REG_MDCTL_GET_LRSTZ(x)        BOOT_READ_BITFIELD((x),8,8)
#define PSC_REG_MDCTL_SET_RESET_ISO(x,y)  BOOT_SET_BITFIELD((x),(y),12,12)


/* MDSTAT */
#define PSC_REG_MDSTAT_GET_STATUS(x)   BOOT_READ_BITFIELD((x),5,0)
#define PSC_REG_MDSTAT_GET_LRSTZ(x)    BOOT_READ_BITFIELD((x),8,8)
#define PSC_REG_MDSTAT_GET_LRSTDONE(x) BOOT_READ_BITFIELD((x),9,9)

/* PDCTL states */
#define PSC_REG_VAL_PDCTL_NEXT_ON   1
#define PSC_REG_VAL_PDCTL_NEXT_OFF  0

#define PSC_REG_VAL_PDCTL_PDMODE_SLEEP  0

/* MDCTL states */
#define PSC_REG_VAL_MDCTL_NEXT_SWRSTDISABLE     0
#define PSC_REG_VAL_MDCTL_NEXT_OFF              2
#define PSC_REG_VAL_MDCTL_NEXT_ON               3  /* Yes, 3 is the on state command */


/* MDSTAT states */
#define PSC_REG_VAL_MDSTAT_STATE_ON                 3
#define PSC_REG_VAL_MDSTAT_STATE_ENABLE_IN_PROG     0x24
#define PSC_REG_VAL_MDSTAT_STATE_OFF                2
#define PSC_REG_VAL_MDSTAT_STATE_DISABLE_IN_PROG1   0x20
#define PSC_REG_VAL_MDSTAT_STATE_DISABLE_IN_PROG2   0x21
#define PSC_REG_VAL_MDSTAT_STATE_DISABLE_IN_PROG3   0x22


/* Timeout limit on checking PTSTAT. This is the number of times the
 * wait function will be called before giving up. */
#define PSC_PTSTAT_TIMEOUT_LIMIT    100





#endif /* _PSCLOC_H */
